#include "memory.h"

#include <TlHelp32.h>

const char *g_gameName = "Alice.exe";

void Memory::ConnectToGame()
{
    if (g_gameHandle != NULL)
    {
        if (this->IsConnected())
            return;
        CloseHandle(g_gameHandle);
        g_gameHandle = NULL;
    }

    HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (processSnapshot == INVALID_HANDLE_VALUE)
    {
        printf("Failed to enumerate processes (error %lu).\n", GetLastError());
        return;
    }

    PROCESSENTRY32 process = {};
    process.dwSize = sizeof(process);
    if (Process32First(processSnapshot, &process))
    {
        do
        {
            if (lstrcmpiA(process.szExeFile, g_gameName) == 0)
            {
                this->processID = process.th32ProcessID;
                break;
            }
        } while (Process32Next(processSnapshot, &process));
    }
    CloseHandle(processSnapshot);

    if (this->processID == 0)
    {
        printf("Failed to find process %s.\n", g_gameName);
        return;
    }

    g_gameHandle = OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION |
            PROCESS_VM_OPERATION |
            PROCESS_VM_READ |
            PROCESS_VM_WRITE,
        FALSE,
        this->processID);
    if (g_gameHandle == NULL)
    {
        printf("Failed to open %s (error %lu).\n", g_gameName, GetLastError());
        this->processID = 0;
        return;
    }

    BOOL isWow64 = FALSE;
    if (IsWow64Process(g_gameHandle, &isWow64) && isWow64)
        this->pointerSize = sizeof(uint32_t);
    else
        this->pointerSize = sizeof(void *);
}

bool Memory::IsConnected() const
{
    DWORD exitCode;
    return g_gameHandle != NULL && GetExitCodeProcess(g_gameHandle, &exitCode) && exitCode == STILL_ACTIVE;
}

void *Memory::GetProcessBaseAddress()
{
    if (g_gameHandle == NULL || processID == 0)
    {
        printf("Cannot get process base address: game is not connected.\n");
        return NULL;
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
        processID);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        printf("Failed to enumerate process modules (error %lu).\n", GetLastError());
        return NULL;
    }

    MODULEENTRY32 module = {};
    module.dwSize = sizeof(module);

    void *baseAddress = NULL;
    if (Module32First(snapshot, &module))
        baseAddress = module.modBaseAddr;
    else
        printf("Failed to get process base address (error %lu).\n", GetLastError());

    CloseHandle(snapshot);
    return baseAddress;
}

size_t Memory::GetPointerSize() const
{
    return this->pointerSize;
}

static bool ResolvePointer(Memory *mem, void *&address, const std::vector<size_t> &offsets)
{
    for (size_t offset : offsets)
    {
        uintptr_t nextAddressValue = 0;
        if (!mem->ReadMemory(address, &nextAddressValue, mem->GetPointerSize()) ||
            nextAddressValue == 0)
        {
            printf("Failed to resolve pointer at address: %p (error %lu)\n",
                address, GetLastError());
            return false;
        }
        address = static_cast<char *>(reinterpret_cast<void *>(nextAddressValue)) + offset;
    }

    return address != NULL;
}

bool Memory::ReadMemory(void *address, void *buffer, size_t size)
{
    return g_gameHandle != NULL && address != NULL && buffer != NULL &&
        ReadProcessMemory(g_gameHandle, address, buffer, size, NULL) != FALSE;
}

bool Memory::ReadMemoryAtPointer(void *address, const std::vector<size_t> &offsets, void *buffer, size_t size)
{
    return ResolvePointer(this, address, offsets) && ReadMemory(address, buffer, size);
}

bool Memory::WriteMemory(void *address, void *buffer, size_t size)
{
    return g_gameHandle != NULL && address != NULL && buffer != NULL &&
        WriteProcessMemory(g_gameHandle, address, buffer, size, NULL) != FALSE;
}

bool Memory::WriteMemoryAtPointer(void *address, const std::vector<size_t> &offsets, void *buffer, size_t size)
{
    return ResolvePointer(this, address, offsets) && WriteMemory(address, buffer, size);
}

void Memory::CloseMemoryHandles()
{
    if (g_gameHandle != NULL)
    {
        CloseHandle(g_gameHandle);
        g_gameHandle = NULL;
    }
    FreeLibrary(g_kernel32);
}
