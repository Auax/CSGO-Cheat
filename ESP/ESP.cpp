#include "Offsets.h"
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <tchar.h>
#include <WinUser.h>

#include "World.h"
#include "Memory.h"
#include "Entity.h"

// Global
Memory memory;
RECT WBounds;
HWND EspHWND;

// PENS
HPEN BoxPenR = CreatePen(PS_SOLID, 1, RGB(255, 0, 0)); // RED
HPEN BoxPenG = CreatePen(PS_SOLID, 1, RGB(0, 255, 0)); // GREEN
HPEN BoxPenB = CreatePen(PS_SOLID, 1, RGB(0, 0, 255)); // RED

// Draw a rectangle on-screen using a custom pen
void DrawBox(const HDC hdc, const int left, const int top, const int right, const int bottom, HPEN pen) {
    SelectObject(hdc, pen);
    Rectangle(hdc, left, top, right, bottom);
}

// Draw a rectangle around the character using the foot position and head position
// Depends on DrawBox method
void DrawAround(const HDC hdc, const Vector3 foot, const Vector3 head, HPEN pen)
{
    const float height = head.y - foot.y;
    const float width = height / 2.4f;
    DrawBox(hdc, foot.x - (width / 2), foot.y, head.x + (width / 2), head.y, pen);
}

struct Vector3 WorldToScreen(const RECT w_bounds, const struct Vector3 pos, const struct view_matrix_t matrix)
{
    struct Vector3 out{};
    float x = matrix.matrix[0] * pos.x + matrix.matrix[1] * pos.y + matrix.matrix[2] * pos.z + matrix.matrix[3];
    float y = matrix.matrix[4] * pos.x + matrix.matrix[5] * pos.y + matrix.matrix[6] * pos.z + matrix.matrix[7];
    out.z = matrix.matrix[12] * pos.x + matrix.matrix[13] * pos.y + matrix.matrix[14] * pos.z + matrix.matrix[15];

    x *= 1.f / out.z;
    y *= 1.f / out.z;

    const int width = w_bounds.right - w_bounds.left;
    const int height = w_bounds.bottom + w_bounds.left;

    out.x = width * .5f;
    out.y = height * .5f;

    out.x += 0.5f * x * width + 0.5f;
    out.y -= 0.5f * y * height + 0.5f;

    return out;
}


LRESULT CALLBACK WndProc(const HWND hwnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;

        const int win_width = WBounds.right - WBounds.left;
        const int win_height = WBounds.bottom + WBounds.left;

        const HDC hdc = BeginPaint(hwnd, &ps);
        const HDC memhdc = CreateCompatibleDC(hdc);
        const HBITMAP Membitmap = CreateCompatibleBitmap(hdc, win_width, win_height);
        SelectObject(memhdc, Membitmap);
        FillRect(memhdc, &WBounds, WHITE_BRUSH);

        const view_matrix_t vm = memory.RPM<view_matrix_t>(memory.client_dll + dwViewMatrix);
        const int local_team = memory.RPM<int>(memory.RPM<DWORD>(memory.client_dll + dwEntityList) + m_iTeamNum);

        for (int id = 1; id < 64; id++)
        {
	        const uintptr_t pEnt = memory.RPM<DWORD>(static_cast<SIZE_T>(memory.client_dll) + dwEntityList + (id * 0x10));
            const int team = memory.RPM<int>(pEnt + m_iTeamNum);
            
            // Initialize Entity class
            Entity entity(&memory, pEnt);
            if (team != local_team)
            {
                // Enemy health value
                int health = memory.RPM<int>(pEnt + m_iHealth);

                // BONES
                Vector3 head_pos = entity.GetBonePosition(8);
                Vector3 neck_pos = entity.GetBonePosition(7);

                // Transform World Position to screen
                Vector3 screen_head_pos = WorldToScreen(WBounds, head_pos, vm);
                Vector3 screen_neck_pos = WorldToScreen(WBounds, neck_pos, vm);

                // ENEMY POSITION
                Vector3 pos = memory.RPM<Vector3>(pEnt + m_vecOrigin);
                // Transform World Position to screen
                Vector3 screen_pos = WorldToScreen(WBounds, pos, vm);

                if (screen_pos.z >= 0.01f && health > 0 && health < 101)
                {
                    DrawAround(memhdc, screen_pos, screen_head_pos, BoxPenB);
                    // DrawAround Head to Neck Line
                    screen_head_pos.x -= 10;
                    screen_head_pos.y -= 10;
                    screen_head_pos.z -= 10;
                    DrawAround(memhdc, screen_neck_pos, screen_head_pos, BoxPenG);

                }
            }
        }
        BitBlt(hdc, 0, 0, win_width, win_height, memhdc, 0, 0, SRCCOPY);
        DeleteObject(Membitmap);
        DeleteDC(memhdc);
        EndPaint(hwnd, &ps);
        ValidateRect(hwnd, &WBounds);
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

DWORD WorkLoop()
{
    while (true)
    {
        InvalidateRect(EspHWND, &WBounds, true);
        Sleep(16); // 16 ms * 60 fps ~ 1000 ms
    }
}

uintptr_t get_module_base_address(DWORD procId, const wchar_t* modName)
{
    uintptr_t modBaseAddr = 0;
    const HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry))
        {
            do
            {
                if (!_wcsicmp(modEntry.szModule, modName))
                {
                    modBaseAddr = reinterpret_cast<uintptr_t>(modEntry.modBaseAddr);
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

int main()
{

    wchar_t moduleName[] = L"Client.dll";
    const HWND procHwnd = FindWindowA(nullptr, "Counter-Strike: Global Offensive");
    std::cout << "procHwnd: " << procHwnd << std::endl;
    GetClientRect(procHwnd, &WBounds);
    std::cout << "client.dll module address: " << memory.client_dll << std::endl;

    WNDCLASSEX WClass;
    MSG Msg;
    WClass.cbSize = sizeof(WNDCLASSEX);
    WClass.style = NULL;
    WClass.lpfnWndProc = WndProc;
    WClass.cbClsExtra = NULL;
    WClass.cbWndExtra = NULL;
    WClass.hInstance = reinterpret_cast<HINSTANCE>(GetWindowLongA(procHwnd, GWL_HINSTANCE));
    WClass.hIcon = nullptr;
    WClass.hCursor = nullptr;
    WClass.hbrBackground = WHITE_BRUSH;
    WClass.lpszMenuName = L" ";
    WClass.lpszClassName = L" ";
    WClass.hIconSm = nullptr;
    RegisterClassEx(&WClass);

    const HINSTANCE Hinstance = nullptr;
    EspHWND = CreateWindowExA(WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED, " ", " ", WS_POPUP,
        WBounds.left, WBounds.top, WBounds.right - WBounds.left, WBounds.bottom + WBounds.left, nullptr, nullptr, Hinstance, nullptr);

    SetLayeredWindowAttributes(EspHWND, RGB(255, 255, 255), 255, LWA_COLORKEY);
    ShowWindow(EspHWND, 1);
    CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(&WorkLoop), nullptr, NULL, nullptr);

    while (GetMessageA(&Msg, nullptr, NULL, NULL) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessageA(&Msg);
        Sleep(1);
    }
    ExitThread(0);
}
