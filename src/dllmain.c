#include "hook.h"

HANDLE dllHandle = NULL;
minesweeperData g_Data = {0,0};
HRESULT(__stdcall* g_origEndScene)(LPDIRECT3DDEVICE9 pDevice) = NULL;
HRESULT(__stdcall* g_trampEndScene)(LPDIRECT3DDEVICE9 pDevice) = NULL;
extern HWND window;
unsigned char g_backupBytes[0xF];
void mainHack()
{
    AllocConsole();
    FILE* oldStdOut = NULL;
    freopen_s(&oldStdOut, "CONOUT$", "w", stdout);

    readData();
    printInfo();

    getDxFunc((void**)&g_origEndScene,ENDSCENE);

    printf("EndScene function: 0x%p\n", g_origEndScene);
    Tramp64(g_origEndScene,&hkEndScene,(void**)&g_trampEndScene,0xF,g_backupBytes);
    printf("Hooked EndScene function: 0x%p\n", &hkEndScene);
    printf("Trampoline function: 0x%p\n",g_trampEndScene);

    while (1)
    {
        if (GetAsyncKeyState(VK_DELETE) & 0x8000)
            break;
        Sleep(5);
    }

    restore(g_origEndScene, g_backupBytes, 0xF);

    fclose(oldStdOut);
    FreeConsole();
    FreeLibraryAndExitThread(dllHandle, 0);
}




void readData()
{
    g_Data.g_baseAddress = (uintptr_t)GetModuleHandleA(0);

    unsigned int gameOffset[2] = {0xAAA38, 0x18};
    uintptr_t pointer = *(uintptr_t*)(g_Data.g_baseAddress+gameOffset[0]);
    g_Data.g_gameObject = (gameStruct*)*(uintptr_t*)(pointer + gameOffset[1]);

    if(g_Data.g_gameObject->clickCount < 1)
    {
        printf("Click on any tile to reveal info\n");

        while(1)
        {
            if(g_Data.g_gameObject->clickCount > 0)
                break;
            Sleep(10);
        }

    }

    //[[[[[[[Minesweeper.exe + 0xAAA38] + 0x18] + 0x58] + 0x10] + (0x8 * i)] + 0x10] + 0x0]
    unsigned int bombOffsets[] = {0xAAA38,0x18,0x58, 0x10, 0x8, 0x10, 0x0};
    createBombMatrix(bombOffsets);

}

void createBombMatrix(const unsigned int* bombOffsets)
{
    memset(g_Data.g_BombMatrix, 0, sizeof(g_Data.g_BombMatrix));

    for(unsigned int i = 0; i < g_Data.g_gameObject->colSizeY; i++)
    {
        uintptr_t pointer = *(uintptr_t*)(g_Data.g_baseAddress+bombOffsets[0]);
        pointer = *(uintptr_t*)(pointer + bombOffsets[1]);
        pointer = *(uintptr_t*)(pointer + bombOffsets[2]);
        pointer = *(uintptr_t*)(pointer + bombOffsets[3]);
        pointer = *(uintptr_t*)(pointer + (bombOffsets[4]*i));
        pointer = *(uintptr_t*)(pointer + bombOffsets[5]);

        memcpy(g_Data.g_BombMatrix[i], (void*)pointer, g_Data.g_gameObject->rowSizeX);
    }
}

void printInfo()
{
    printf("Base Address: 0x%p\n",(void*)g_Data.g_baseAddress);
    printf("GameObject Address: 0x%p\n",g_Data.g_gameObject);
    printf("Board size: %dx%d\n", g_Data.g_gameObject->rowSizeX, g_Data.g_gameObject->colSizeY);
    printf("Bomb Count: %d\n",g_Data.g_gameObject->BombCount);

    printf("Press HOME to refresh program\n");

    for(unsigned int j = 0; j < g_Data.g_gameObject->rowSizeX; j++)
    {
        for(unsigned int i = 0; i < g_Data.g_gameObject->colSizeY; i++)
        {
            if(g_Data.g_BombMatrix[i][j])
                printf("[X] ");
            else
                printf("[O] ");
        }
        printf("\n");
    }
    printf("\n");

}


BOOL WINAPI DllMain(HINSTANCE hinstDLL,  // handle to DLL module
                   DWORD fdwReason,     // reason for calling function
                   LPVOID lpReserved)  // reserved
{
    // Perform actions based on the reason for calling.
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        dllHandle = hinstDLL;
        CloseHandle(CreateThread(NULL, (SIZE_T)NULL, (LPTHREAD_START_ROUTINE)mainHack, NULL, 0, NULL));
        return TRUE;  // Successful DLL_PROCESS_ATTACH.
    }

    return FALSE;
}