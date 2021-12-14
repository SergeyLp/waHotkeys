#include "pch.h"
#include <stdlib.h>

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

ATOM id_play_pause;

int media_keys[] = {
    VK_MEDIA_PLAY_PAUSE ,
    VK_MEDIA_NEXT_TRACK,
    VK_MEDIA_PREV_TRACK,
    VK_MEDIA_STOP,
    VK_PLAY,
};

//LPARAM commands_table[] = {
//    0,
//
//};

void reg_keys() {
    //id_play_pause = VK_MEDIA_PLAY_PAUSE; // GlobalAddAtom(_T("VK_MEDIA_PLAY_PAUSE"));
    //const BOOL res = RegisterHotKey(plugin.hwndParent, id_play_pause, 0, VK_MEDIA_PLAY_PAUSE);

    for (int key : media_keys) {
        const BOOL res = RegisterHotKey(plugin.hwndParent, key, 0, key);
        if (!res) MessageBeep(MB_ICONEXCLAMATION);
    }
}

inline LRESULT waMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0) {
    return SendMessage(plugin.hwndParent, message, wParam, lParam);
}

inline LRESULT waIPC(WPARAM wParam, LPARAM lParam = 0) {
    return waMessage(WM_WA_IPC, wParam, lParam);
}

//LRESULT get_command_by_hotkey(WORD hot_key) {
//    LRESULT i;
//    for (i = 1; i < _countof(media_keys); i++) {
//        if (media_keys[i] == hot_key) {
//
//        }
//    }
//
//}

WNDPROC lpWndProcOld;
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_HOTKEY) {
        const WORD hotkey = LOWORD(wParam);
        if (hotkey == VK_MEDIA_PLAY_PAUSE) {
            const LRESULT status = waIPC(0, IPC_ISPLAYING);
            if (status == 1)    // Winamp is playing
                waMessage(WM_COMMAND, WINAMP_BUTTON3);
            else 
                waMessage(WM_COMMAND, WINAMP_BUTTON2);
        } else {
            switch (hotkey) {
                case VK_MEDIA_NEXT_TRACK:
                    waMessage(WM_COMMAND, WINAMP_BUTTON5);
                    break;
                case VK_MEDIA_PREV_TRACK:
                    waMessage(WM_COMMAND, WINAMP_BUTTON1);
                    break;
                case VK_MEDIA_STOP:
                    waMessage(WM_COMMAND, WINAMP_BUTTON4);
                    break;
                case VK_PLAY:
                    waMessage(WM_COMMAND, WINAMP_BUTTON2);
                    break;
        
            default:
                break;
            }
        }

        return FALSE;
    }

    return CallWindowProc(lpWndProcOld, hwnd, message, wParam, lParam);
}


int init() {
    #if _DEBUG
    MessageBeep(0xFFFFFFFF);
    #endif // _DEBUG

    lpWndProcOld = (WNDPROC)GetWindowLong(plugin.hwndParent, GWL_WNDPROC);
    SetWindowLong(plugin.hwndParent, GWL_WNDPROC, (long)WndProc);

    reg_keys();
    return 0;
}

void config(){
    MessageBox(plugin.hwndParent, _T("TODO: Keys reload"), _T("Hot keys plugin"), MB_OK | MB_ICONINFORMATION);
    reg_keys();
}

void quit(){
    //const BOOL res = UnregisterHotKey(plugin.hwndParent, id_play_pause);
}

extern "C" {
    __declspec(dllexport) GenPlugins* winampGetGeneralPurposePlugin() {

        static char desc[] = "·• Ly Ascetic Hotkeys •·";
        plugin.description = desc;

        return &plugin;
    }
}