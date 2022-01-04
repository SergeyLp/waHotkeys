#include "pch.h"
#include "ly_wa_redefine.h"

struct GenPlugins {
    int version;                    //Out
    char* description;          //Out
    int (*init)();                   //Out
    void(*config)();            //Out
    void(*quit)();               //Out
    HWND hwndParent;        //In
    HINSTANCE hDllInstance;//In
};

int init();
void config();
void quit();

GenPlugins plugin = {
    0x10, nullptr,
    init, config, quit,
    0, 0
};
const HWND& waWnd = plugin.hwndParent;

int media_keys[] = {
    VK_MEDIA_PLAY_PAUSE ,
    VK_MEDIA_NEXT_TRACK,
    VK_MEDIA_PREV_TRACK,
    VK_MEDIA_STOP,
    VK_PLAY,
};

enum KeysWithWinlogoButtons {
    idPlayPause = 0xE000,
    idStop,
    idNext,
    idPrev,
    idInfo,
    idDelete,
};

void reg_keys() {
    //id_play_pause = VK_MEDIA_PLAY_PAUSE; // GlobalAddAtom(_T("VK_MEDIA_PLAY_PAUSE"));
    //const BOOL res = RegisterHotKey(plugin.hwndParent, id_play_pause, 0, VK_MEDIA_PLAY_PAUSE);

    for (int key : media_keys) {
        const BOOL res = RegisterHotKey(waWnd, key, 0, key);
        if (!res) MessageBeep(MB_ICONWARNING);
    }

    BOOL res;
    res = RegisterHotKey(waWnd, idPlayPause, MOD_WIN | MOD_NOREPEAT, VK_F2);
    res &= RegisterHotKey(waWnd, idStop, MOD_WIN | MOD_SHIFT | MOD_NOREPEAT, VK_F2);
    res &= RegisterHotKey(waWnd, idNext, MOD_WIN | MOD_NOREPEAT, VK_F4);
    res &= RegisterHotKey(waWnd, idPrev, MOD_WIN | MOD_SHIFT | MOD_NOREPEAT, VK_F4);
    res &= RegisterHotKey(waWnd, idInfo, MOD_WIN | MOD_NOREPEAT, VK_F3);
    res &= RegisterHotKey(waWnd, idDelete, MOD_WIN | MOD_NOREPEAT, VK_OEM_3);  //`~
    if (!res) MessageBeep(MB_ICONWARNING);
}

inline LRESULT waMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0) {
    return SendMessage(plugin.hwndParent, message, wParam, lParam);
}

inline LRESULT waIPC(WPARAM wParam, LPARAM lParam = 0) {
    return waMessage(WM_WA_IPC, wParam, lParam);
}

inline void waForcePlay(){
    const LRESULT status = waIPC(0, IPC_ISPLAYING);
    if (status != WA_PLAYING)
        waMessage(WM_COMMAND, WA_BUTTON_PLAY);
}

WNDPROC pWndProcOld;
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_HOTKEY) {
        const WORD hotkey = LOWORD(wParam);
        switch (hotkey) {
        case VK_MEDIA_PLAY_PAUSE:
        case idPlayPause:
        {
            const LRESULT status = waIPC(0, IPC_ISPLAYING);
            if (status == WA_PLAYING)
                waMessage(WM_COMMAND, WA_BUTTON_PAUSE);
            else
                waMessage(WM_COMMAND, WA_BUTTON_PLAY);
        }
            break;
        case idNext:
        case VK_MEDIA_NEXT_TRACK:
            waMessage(WM_COMMAND, WA_BUTTON_NEXT);
            waForcePlay();
            break;
        case idPrev:
        case VK_MEDIA_PREV_TRACK:
            waMessage(WM_COMMAND, WA_BUTTON_PREV);
            waForcePlay();
            break;
        case idStop:
        case VK_MEDIA_STOP:
            waMessage(WM_COMMAND, WA_BUTTON_STOP);
            break;
        case VK_PLAY:
            waMessage(WM_COMMAND, WA_BUTTON_PLAY);
            break;
        case idInfo:
            waMessage(WM_SYSCOMMAND, SC_RESTORE);
            waMessage(WM_COMMAND, WA_SHOW_FILE_INFO_DLG/*, IPC_CB_ONSHOWWND*/);
            break;
        case idDelete:
        {
            const int position = waIPC(0, IPC_GETLISTPOS);
            const LRESULT file_path = waIPC(position, IPC_GETPLAYLISTFILE);
            if (file_path != NULL) {
                const HWND hPeWnd = (HWND)waIPC(IPC_GETWND_PE, IPC_GETWND);
                SendMessage(hPeWnd, WM_WA_IPC, IPC_PE_DELETEINDEX, (LPARAM)position);              
                
                waMessage(WM_COMMAND, WA_BUTTON_NEXT);

                char path_with_00[MAX_PATH + 1];    // Add another one for SH
                char* out_p = path_with_00;
                const char* in_p = (const char*) file_path;
                for (*out_p = *in_p; *in_p != '\0' && ((int)in_p - (int)file_path) < MAX_PATH;) {
                    *(++out_p) = *(++in_p);
                }
                *out_p = '\0';   //terminating '\0'
                *(++out_p) = '\0';   // Add another one for SHFileOperationA

                SHFILEOPSTRUCTA file_op;
                file_op.pFrom = path_with_00;
                file_op.hwnd = waWnd;
                file_op.wFunc = FO_DELETE;
                file_op.fFlags = FOF_ALLOWUNDO | FOF_FILESONLY | FOF_NO_CONNECTED_ELEMENTS;
                file_op.pTo = NULL;
                SHFileOperationA(&file_op);
            }           
        }
        break;

        default:
            MessageBeep(MB_ICONINFORMATION);
            return FALSE;
            break;
        }
        return TRUE;
    }
    else
        return CallWindowProc(pWndProcOld, hwnd, message, wParam, lParam);
    
}


int init() {
    #if _DEBUG
    MessageBeep(0xFFFFFFFF);
    #endif // _DEBUG

    pWndProcOld = (WNDPROC)GetWindowLong(waWnd, GWL_WNDPROC);
    SetWindowLong(waWnd, GWL_WNDPROC, (long)WndProc);

    reg_keys();
    return 0;
}

void config(){
    MessageBox(plugin.hwndParent, _T("Keys reload"), _T("Hot keys plugin"), MB_OK | MB_ICONINFORMATION);
    reg_keys();
}

void quit(){
    //No need call UnregisterHotKey: hwndParent already not exist.
}

extern "C" {
    __declspec(dllexport) GenPlugins* winampGetGeneralPurposePlugin() {

        static char desc[] = "Ly Ascetic Hotkeys";
        plugin.description = desc;

        return &plugin;
    }
}