#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <windows.h>
#include <string>

class Memory
{
public:
    void ConnectToGame();
    bool IsConnected() const;
    std::string GetGameSHA256();
    void *GetProcessBaseAddress();
    size_t GetPointerSize() const;
    bool ReadMemory(void *address, void *buffer, size_t size);
    bool ReadMemoryAtPointer(void *address, const std::vector<size_t> &offsets, void *buffer, size_t size);
    bool WriteMemory(void *address, void *buffer, size_t size);
    bool WriteMemoryAtPointer(void *address, const std::vector<size_t> &offsets, void *buffer, size_t size);
    template <typename T> bool ReadMemory(void *address, T *buffer)
    {
        return ReadMemory(address, buffer, sizeof(T));
    }
    template <typename T> bool ReadMemoryAtPointer(void *address, const std::vector<size_t> &offsets, T *buffer)
    {
        return ReadMemoryAtPointer(address, offsets, buffer, sizeof(T));
    }
    template <typename T> bool WriteMemory(void *address, T *buffer)
    {
        return WriteMemory(address, buffer, sizeof(T));
    }
    template <typename T> bool WriteMemoryAtPointer(void *address, const std::vector<size_t> &offsets, T *buffer)
    {
        return WriteMemoryAtPointer(address, offsets, buffer, sizeof(T));
    }
    void CloseMemoryHandles();

private:
    HMODULE g_kernel32 = LoadLibraryA("kernel32.dll");
    HANDLE g_gameHandle = NULL;
    DWORD processID = 0;
    size_t pointerSize = sizeof(void *);
};
