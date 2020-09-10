#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch(fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

HMODULE proxyLoadLibraryW(LPCWSTR lpLibFileName) {
    if (!lstrcmpW(lpLibFileName, L"dsound.dll")) {
        OutputDebugStringW(L"dsound proxied.");
        return LoadLibraryW(L"system32\\dsound.dll");
        /*
        char dsoundAbsPath[MAX_PATH+10];

        //will resolve correctly to system(win98) or system32(later windows)
        GetSystemDirectoryA(dsoundAbsPath, MAX_PATH);
        strncat(dsoundAbsPath, "\\DSOUND.DLL", MAX_PATH);
        OutputDebugStringA(dsoundAbsPath);
        return LoadLibraryA(dsoundAbsPath);
        */
    } else {
        return LoadLibraryW(lpLibFileName);
    }
}