#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <string>

constexpr auto TARGET_PROCESS = L"csgo.exe";
constexpr auto CLIENT_DLL_NAME = L"client.dll";
constexpr auto ENGINE_DLL_NAME = L"engine.dll";

class Memory
{
private:
    DWORD dwPid;
    HANDLE hProcess;

    // Methods
    static uint32_t getProcessId(const wchar_t* process_name);
    bool attachToProcess(const wchar_t* process_name);
    uintptr_t getModuleBaseAddress(const wchar_t* mod_name) const;

public:
    SIZE_T client_dll, engine_dll;

    // RPM Wrapper
    template<typename T>
    T RPM(const SIZE_T address)
    {
        // The buffer for data is going to be read from memory
        T buffer;
        ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), &buffer, sizeof(T), nullptr);
        return buffer;
    }

    Memory();
    ~Memory();
};

