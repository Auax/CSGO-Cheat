#include "Memory.h" 
#include <iostream>

Memory::Memory()
{
    this->hProcess = NULL;
    this->dwPid = NULL;

    if (!attachToProcess(TARGET_PROCESS)) {
        // Failed to attatch to process
        return;
    }

    // Assign module address for client.dll and engine.dll
    this->client_dll = getModuleBaseAddress(CLIENT_DLL_NAME);
    this->engine_dll = getModuleBaseAddress(ENGINE_DLL_NAME);
}

Memory::~Memory()
{
    CloseHandle(this->hProcess);
}

// Returns first PID found from process name
uint32_t Memory::getProcessId(const wchar_t* process_name)
{
	const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(entry);
    do
    {
        if (!wcscmp(entry.szExeFile, process_name))
            return entry.th32ProcessID;
    } while (Process32Next(snapshot, &entry));
    CloseHandle(snapshot);
    return 0;
}

// Attach memory manager to process
// Will return false if the handle operation failed
bool Memory::attachToProcess(const wchar_t* process_name)
{
    dwPid = getProcessId(process_name);
    // Simply open a handle to target
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
    return hProcess;
}

uintptr_t Memory::getModuleBaseAddress(const wchar_t* mod_name) const
{
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwPid);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry))
        {
            do
            {
                if (!_wcsicmp(modEntry.szModule, mod_name))
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
