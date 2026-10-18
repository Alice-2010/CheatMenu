#include "memory.h"

#include <bcrypt.h>
#include <Psapi.h>
#include <TlHelp32.h>

#include <vector>

const char *g_gameName = "Alice.exe";

void Memory::ConnectToGame()
{
    if (this->g_gameHandle != NULL)
    {
        if (this->IsConnected())
            return;
        CloseHandle(this->g_gameHandle);
        this->g_gameHandle = NULL;
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

    this->g_gameHandle = OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION |
            PROCESS_VM_OPERATION |
            PROCESS_VM_READ |
            PROCESS_VM_WRITE,
        FALSE,
        this->processID);
    if (this->g_gameHandle == NULL)
    {
        printf("Failed to open %s (error %lu).\n", g_gameName, GetLastError());
        this->processID = 0;
        return;
    }

    BOOL isWow64 = FALSE;
    if (IsWow64Process(this->g_gameHandle, &isWow64) && isWow64)
        this->pointerSize = sizeof(uint32_t);
    else
        this->pointerSize = sizeof(void *);
}

bool Memory::IsConnected() const
{
    DWORD exitCode;
    return this->g_gameHandle != NULL && GetExitCodeProcess(this->g_gameHandle, &exitCode) && exitCode == STILL_ACTIVE;
}

std::string Memory::GetGameSHA256()
{
    if (!this->IsConnected())
    {
        printf("Cannot hash game file: game is not connected.\n");
        return "";
    }

    std::vector<char> imagePath(32768);
    DWORD imagePathLength = static_cast<DWORD>(imagePath.size());
    if (!QueryFullProcessImageNameA(
            this->g_gameHandle,
            0,
            imagePath.data(),
            &imagePathLength))
    {
        printf("Failed to get game file path (error %lu).\n", GetLastError());
        return "";
    }

    HANDLE file = CreateFileA(
        imagePath.data(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);
    if (file == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open game file for hashing (error %lu).\n", GetLastError());
        return "";
    }

    BCRYPT_ALG_HANDLE algorithm = NULL;
    BCRYPT_HASH_HANDLE hash = NULL;
    DWORD objectLength = 0;
    DWORD resultLength = 0;
    NTSTATUS status = BCryptOpenAlgorithmProvider(
        &algorithm,
        BCRYPT_SHA256_ALGORITHM,
        NULL,
        0);
    if (status != 0)
    {
        CloseHandle(file);
        printf("Failed to open SHA-256 provider (status 0x%08lX).\n",
            static_cast<unsigned long>(status));
        return "";
    }

    status = BCryptGetProperty(
        algorithm,
        BCRYPT_OBJECT_LENGTH,
        reinterpret_cast<PUCHAR>(&objectLength),
        sizeof(objectLength),
        &resultLength,
        0);
    if (status != 0)
    {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        CloseHandle(file);
        printf("Failed to get SHA-256 object size (status 0x%08lX).\n",
            static_cast<unsigned long>(status));
        return "";
    }

    std::vector<unsigned char> hashObject(objectLength);
    status = BCryptCreateHash(
        algorithm,
        &hash,
        hashObject.data(),
        objectLength,
        NULL,
        0,
        0);
    if (status != 0)
    {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        CloseHandle(file);
        printf("Failed to create SHA-256 hash (status 0x%08lX).\n",
            static_cast<unsigned long>(status));
        return "";
    }

    std::vector<unsigned char> buffer(1024 * 1024);
    DWORD bytesRead = 0;
    bool readSucceeded = true;
    bool hashSucceeded = true;
    do
    {
        if (!ReadFile(file, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, NULL))
        {
            readSucceeded = false;
            break;
        }
        if (bytesRead != 0)
        {
            status = BCryptHashData(hash, buffer.data(), bytesRead, 0);
            if (status != 0)
            {
                hashSucceeded = false;
                break;
            }
        }
    } while (bytesRead != 0);

    std::string result;
    if (readSucceeded && hashSucceeded)
    {
        std::vector<unsigned char> digest(32);
        status = BCryptFinishHash(hash, digest.data(), static_cast<ULONG>(digest.size()), 0);
        if (status == 0)
        {
            const char *hexDigits = "0123456789ABCDEF";
            result.reserve(digest.size() * 2);
            for (unsigned char byte : digest)
            {
                result.push_back(hexDigits[byte >> 4]);
                result.push_back(hexDigits[byte & 0x0F]);
            }
        }
    }

    if (result.empty())
    {
        if (!readSucceeded)
            printf("Failed to read game file while hashing (error %lu).\n", GetLastError());
        else if (!hashSucceeded)
            printf("Failed to update SHA-256 hash (status 0x%08lX).\n",
                static_cast<unsigned long>(status));
        else
            printf("Failed to finish SHA-256 hash (status 0x%08lX).\n",
                static_cast<unsigned long>(status));
    }

    BCryptDestroyHash(hash);
    BCryptCloseAlgorithmProvider(algorithm, 0);
    CloseHandle(file);
    return result;
}

void *Memory::GetProcessBaseAddress()
{
    if (this->g_gameHandle == NULL || this->processID == 0)
    {
        printf("Cannot get process base address: game is not connected.\n");
        return NULL;
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
        this->processID);
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
    return this->g_gameHandle != NULL && address != NULL && buffer != NULL &&
        ReadProcessMemory(this->g_gameHandle, address, buffer, size, NULL) != FALSE;
}

bool Memory::ReadMemoryAtPointer(void *address, const std::vector<size_t> &offsets, void *buffer, size_t size)
{
    return ResolvePointer(this, address, offsets) && ReadMemory(address, buffer, size);
}

bool Memory::WriteMemory(void *address, void *buffer, size_t size)
{
    return this->g_gameHandle != NULL && address != NULL && buffer != NULL &&
        WriteProcessMemory(this->g_gameHandle, address, buffer, size, NULL) != FALSE;
}

bool Memory::WriteMemoryAtPointer(void *address, const std::vector<size_t> &offsets, void *buffer, size_t size)
{
    return ResolvePointer(this, address, offsets) && WriteMemory(address, buffer, size);
}

void Memory::CloseMemoryHandles()
{
    if (this->g_gameHandle != NULL)
    {
        CloseHandle(this->g_gameHandle);
        this->g_gameHandle = NULL;
    }
    FreeLibrary(this->g_kernel32);
}
