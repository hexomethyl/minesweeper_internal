#include "hook.h"

extern minesweeperData g_Data;
extern HRESULT(__stdcall* g_origEndScene)(LPDIRECT3DDEVICE9 pDevice);
extern HRESULT(__stdcall* g_trampEndScene)(LPDIRECT3DDEVICE9 pDevice);
HWND window;
#define ENDSCENEINDEX 42
LPD3DXFONT g_Directx_Font;
static BOOL __stdcall EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    DWORD wndProcId;
    GetWindowThreadProcessId(handle, &wndProcId);

    if (GetCurrentProcessId() != wndProcId)
        return TRUE; // skip to next window

    if (handle == GetConsoleWindow()) return TRUE;

    window = handle;
    return FALSE; // window found abort search
}

static HWND GetProcessWindow()
{
    window = NULL;
    EnumWindows(EnumWindowsCallback, 0);
    return window;
}


int getDxFunc(void** pOut, unsigned int index)
{
    IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    IDirect3DDevice9* pDummyDevice = NULL;

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = 1;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = GetProcessWindow();

    HRESULT dummyDeviceCreated = IDirect3D9_CreateDevice(pD3D,
                                                         D3DADAPTER_DEFAULT,
                                                         D3DDEVTYPE_HAL,
                                                         d3dpp.hDeviceWindow,
                                                         D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                                         &d3dpp,
                                                         &pDummyDevice);

    void** vTable = (void**)pDummyDevice->lpVtbl;
    *pOut = vTable[index]; //get virtual function address at index
    IDirect3D9_Release(pDummyDevice);
    IDirect3D9_Release(pD3D);
    return 1;
}

void DrawString(int x, int y, D3DCOLOR Colour, LPD3DXFONT m_font, const char* format)
{
    RECT Position = {x,y,0,0};
    m_font->lpVtbl->DrawTextA(m_font,0,format,strlen(format),&Position,DT_NOCLIP,Colour);
}

HRESULT __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
    readData();
    D3DXCreateFontA(pDevice, 16, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH,
                    "Consolas", &g_Directx_Font);
    DrawString(55, 7, D3DCOLOR_XRGB(255, 0, 0), g_Directx_Font, "EndScene hooked");
    for(unsigned int j = 0; j < g_Data.g_gameObject->rowSizeX; j++)
    {
        for(unsigned int i = 0; i < g_Data.g_gameObject->colSizeY; i++)
        {
            if(g_Data.g_BombMatrix[i][j])
                DrawString(35 + 18 * i, 32 + 18 * j, D3DCOLOR_XRGB(255, 0, 0), g_Directx_Font, "B");
        }
    }
    g_Directx_Font->lpVtbl->Release(g_Directx_Font);
    return g_trampEndScene(pDevice); //Jmp to trampoline function
}

void Tramp64(void* pTarget, void* pDetour, void** ppOriginal, unsigned int len, unsigned char* backupBytes)
{
    int MinLen = 14;

    if (len < MinLen) return;

    BYTE jmpStub[] = {
            0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, // jmp qword ptr [$+6]
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // ptr
    };

    BYTE* jmpStubAddress = (jmpStub+6);
    SIZE_T x64bitAddress = 8;
    memcpy(backupBytes,pTarget,len); //Save function prologue to unhook later

    void* pTrampoline = VirtualAlloc(0, len + sizeof(jmpStub), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    //Allocate RWX page for trampoline function
    DWORD dwOld = 0;
    VirtualProtect(pTarget, len, PAGE_EXECUTE_READWRITE, &dwOld);
    //Set original function page to RWX

    void* afterJmp = pTarget + len;
    //Calculate address after jmp stub

    //Setup trampoline function
    memcpy(jmpStubAddress, &afterJmp, x64bitAddress); //Write address of where trampoline will jump to in original function into jmp stub
    memcpy(pTrampoline, pTarget, len);// Copy function prologue to trampoline function
    memcpy(pTrampoline + len, jmpStub, sizeof(jmpStub)); //Write jmp to allow us to jump to original function

    //Hook original function
    memcpy(jmpStubAddress, &pDetour, x64bitAddress); //Write address of detour function
    memcpy(pTarget, jmpStub, sizeof(jmpStub)); //Write jmp stub to jump to our hkFunction

    for (int i = MinLen; i < len; i++)
    {
        //minLen may not equal len, thus nop the extra bytes incase its junk
        *(BYTE*)(pTarget + i) = 0x90; //nop
    }

    VirtualProtect(pTarget, len, dwOld, &dwOld);//Restore page permissions
    *ppOriginal = pTrampoline; //Save trampoline function address
}

void restore(void* pTarget, unsigned char* backupBytes, unsigned int len)
{
    DWORD oldProtect;
    VirtualProtect(pTarget, len, PAGE_EXECUTE_READWRITE, &oldProtect);
    //Set page RWX
    memcpy(pTarget,backupBytes,len);
    //Write original function prologue
    VirtualProtect(pTarget, len, oldProtect, &oldProtect);
    //Restore page permissions
}