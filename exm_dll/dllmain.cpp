#include "pch.h"

struct GenPlugins {
    int version;                    //Out
    char* description;          //Out
    int (*const init)();                   //Out
    void(*const config)();            //Out
    void(*const quit)();               //Out
    HWND hwndParent;        //In
    HINSTANCE hDllInstance;//In
};

static int init() {
    MessageBeep(0xFFFFFFFF);
    return 0;
}
static void config(){}
static void quit(){}

static GenPlugins plugin = {
    0x10, nullptr,
    init, config, quit,
    0, 0
};

extern "C" {
    __declspec(dllexport) GenPlugins* winampGetGeneralPurposePlugin() {

        static char desc[] = "·• Ly Ascetic Hotkeys •·";
        plugin.description = desc;

        return &plugin;
    }
}