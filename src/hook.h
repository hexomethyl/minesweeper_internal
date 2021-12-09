#ifndef MINESWEEPER_INTERNAL_HOOK_H
#define MINESWEEPER_INTERNAL_HOOK_H
#endif //MINESWEEPER_INTERNAL_HOOK_H

#include <d3d9.h>
#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#include <tlhelp32.h>
#include <d3dx9core.h>

typedef enum
{
    playing = 1, win = 3, lost = 4
} gameState_e;

typedef struct gameStruct_t
{
    char pad_0000[8]; //0x0000
    int32_t BombCount; //0x0008
    int32_t rowSizeX; //0x000C
    int32_t colSizeY; //0x0010
    char pad_0014[4]; //0x0014
    int32_t tilesShown; //0x0018
    int32_t clickCount; //0x001C
    float timer; //0x0020
    int32_t Difficulty; //0x0024
    gameState_e gameState; //0x0028
    char pad_002C[8]; //0x002C

} gameStruct;

#define ENDSCENE 42
#define MAXSIZE 99

typedef struct minesweeperData_t
{
    uintptr_t g_baseAddress;
    gameStruct* g_gameObject;
    BYTE g_BombMatrix[MAXSIZE][MAXSIZE];
}minesweeperData;

HRESULT __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice);

void Tramp64(void* pTarget, void* pDetour, void** ppOriginal, unsigned int len, unsigned char* backupBytes);

void restore(void* pTarget, unsigned char* backupBytes, unsigned int len);

int getDxFunc(void** pOut, unsigned int index);

void DrawString(int x, int y, D3DCOLOR Colour, LPD3DXFONT m_font, const char* format);

void createBombMatrix(const unsigned int* bombOffsets);

void printInfo();

void mainHack();

void readData();

BOOL WINAPI DllMain(HINSTANCE hinstDLL,  // handle to DLL module
                    DWORD fdwReason,     // reason for calling function
                    LPVOID lpReserved);  // reserved