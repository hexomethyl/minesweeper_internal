#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#include <tlhelp32.h>
HANDLE dllHandle = NULL;

typedef enum
{
    playing = 1, win = 3, lost = 4
} gameState_e;

typedef struct gameStruct_t
{
    char pad_0000[8]; //0x0000
    int32_t BombCount; //0x0008
    int32_t gridSizeX; //0x000C
    int32_t gridSizeY; //0x0010
    char pad_0014[4]; //0x0014
    int32_t tilesShown; //0x0018
    int32_t clickCount; //0x001C
    float timer; //0x0020
    int32_t Difficulty; //0x0024
    gameState_e gameState; //0x0028
    char pad_002C[8]; //0x002C

} gameStruct;

uintptr_t calculateMultiLevelPointer(uintptr_t address, const unsigned int* offsets, unsigned int size);

void createBombMatrix(uintptr_t baseAddress, gameStruct* gameObject,const unsigned int* bombOffsets);

void mainHack();

void readData();

BOOL WINAPI DllMain(HINSTANCE hinstDLL,  // handle to DLL module
                    DWORD fdwReason,     // reason for calling function
                    LPVOID lpReserved);  // reserved


uintptr_t baseAddress = 0;
gameStruct* gameObject = NULL;

void mainHack()
{
    AllocConsole();
    FILE* oldStdOut = NULL;
    freopen_s(&oldStdOut, "CONOUT$", "w", stdout);

    readData();


    while (1)
    {
        if (GetAsyncKeyState(VK_DELETE) & 0x8000)
            FreeLibraryAndExitThread(dllHandle, 0);

        if(GetAsyncKeyState(VK_HOME) & 0x8000)
        {
            readData();
            while (GetKeyState(VK_HOME) & 0x8000) //Ensure button press doesn't spam
            {}
            Sleep(4500);
        }
        Sleep(5);
    }
    
}

void readData()
{
    baseAddress = (uintptr_t)GetModuleHandleA(0);
    unsigned int gameOffset[2] = {0xAAA38, 0x18};
    printf("Base Address: 0x%12llX\n",baseAddress);
    //[[[[[[[Minesweeper.exe + 0xAAA38] + 0x18]
    gameObject = (gameStruct*)*(uintptr_t*)calculateMultiLevelPointer(baseAddress, gameOffset, 2);

    printf("GameObject Address: %p\n",gameObject);
    printf("Click on any tile to reveal info\n");
    
    while(1)
    {
        if(gameObject->clickCount > 0)
            break;
        Sleep(10);
    }

    printf("Board size: %dx%d\n",gameObject->gridSizeX, gameObject->gridSizeY);
    printf("Bomb Count: %d\n",gameObject->BombCount);

    unsigned int bombOffsets[] = {0xAAA38,0x18,0x58, 0x10, 0x8, 0x10, 0x0};
    //[[[[[[[Minesweeper.exe + 0xAAA38] + 0x18] + 0x58] + 0x10] + (0x8 * i)] + 0x10] + 0x0]

    createBombMatrix(baseAddress, gameObject, bombOffsets);
    printf("Press HOME to refresh program\n");
}



void createBombMatrix(uintptr_t baseAddress, gameStruct* gameObject,const unsigned int* bombOffsets)
{
    BYTE bombMatrix[gameObject->gridSizeX][gameObject->gridSizeY];
    memset(bombMatrix, 0, sizeof(bombMatrix));

    for(unsigned int i = 0; i < gameObject->gridSizeX; i++)
    {
        uintptr_t pointer = *(uintptr_t*)(baseAddress+bombOffsets[0]);
        pointer = *(uintptr_t*)(pointer + bombOffsets[1]);
        pointer = *(uintptr_t*)(pointer + bombOffsets[2]);
        pointer = *(uintptr_t*)(pointer + bombOffsets[3]);
        pointer = *(uintptr_t*)(pointer + (bombOffsets[4]*i));
        pointer = *(uintptr_t*)(pointer + bombOffsets[5]);

        //pointer += (bombOffsets[6]);
        //printf("Column Address: %p\n",pointer);
        memcpy(bombMatrix[i], (void*)pointer, gameObject->gridSizeY);
    }


    for(unsigned int i = 0; i < gameObject->gridSizeY; i++)
    {
        for(unsigned int j = 0; j < gameObject->gridSizeX; j++)
        {

            if(bombMatrix[j][i])
                printf("[X] ");
            else
                printf("[O] ");
        }
        printf("\n");
    }
    printf("\n");
}


uintptr_t calculateMultiLevelPointer(uintptr_t address, const unsigned int* offsets, unsigned int size)
{
    uintptr_t pointer = address;
    for (unsigned int i = 0; i < size; i++)
    {
        if( i > 0)
            pointer = *(uintptr_t*)pointer;
        pointer += offsets[i];
    }
    return pointer;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,  // handle to DLL module
                   DWORD fdwReason,     // reason for calling function
                   LPVOID lpReserved)  // reserved
{
    // Perform actions based on the reason for calling.
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        dllHandle = hinstDLL;
        CreateThread(NULL, (SIZE_T)NULL, (LPTHREAD_START_ROUTINE)mainHack, NULL, 0, NULL);
        return TRUE;  // Successful DLL_PROCESS_ATTACH.
    }

    return FALSE;
}