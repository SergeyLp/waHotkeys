#include "pch.h"

typedef int(__cdecl *MYPROC)(LPWSTR);

struct GenPlugins {
    int version;                    //Out
    char* description;          //Out
    int(*init)();                   //Out
    void(*config)();            //Out
    void(*quit)();               //Out
    HWND hwndParent;        //In
    HINSTANCE hDllInstance;//In
};
GenPlugins plugin_struct;




int main(){
    const HINSTANCE hinstLib = LoadLibrary(TEXT("exm_dll.dll"));
    if (hinstLib != NULL) {
        const MYPROC ProcAdd = (MYPROC)GetProcAddress(hinstLib, "myPuts");

        // If the function address is valid, call the function.
        if (NULL != ProcAdd) {
            (ProcAdd)((TCHAR*)_T("Message sent to the DLL function\n"));
        } else {
            printf("Error\n");
        }

        auto get_plugin_struct_fn = GetProcAddress(hinstLib, "winampGetGeneralPurposePlugin");
        

        const BOOL fFreeResult = FreeLibrary(hinstLib);
    }


    //puts("Hello");
    getchar();
}
