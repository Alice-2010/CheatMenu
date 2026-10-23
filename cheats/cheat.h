#pragma once

#include "../memory.h"

#include <cstddef>
#include <cstdint>
#include <string>

#define SHA256_STEAM "73B5DC089D2A9678A2BD519647986F23E5C705ADDDA4981029774CA179625B68"

#define BASE_ADDRESS_STEAM 0x44B8A8

class Cheat
{
public:
    bool m_bIsEnabled = false;

    Cheat(Memory *memory) { this->mem = memory; };
    virtual const std::string GetCheatName() { return ""; };
    virtual void SetIsEnabled(bool isEnabled) { this->m_bIsEnabled = isEnabled; };
    virtual void Run() {};
    void SetSha256(std::string sha256)
    {
        if (sha256 == SHA256_STEAM)
        {
            printf("Detected Steam version of the game.\n");
            this->m_pBaseOffset = BASE_ADDRESS_STEAM;
        }
        else
        {
            printf("Detected unknown version of the game (SHA-256: %s).\n", sha256.c_str());
            this->m_pBaseOffset = 0;
        }
    }

protected:
    bool ResolveBaseAddress()
    {
        void *gameBaseAddress = this->mem->GetProcessBaseAddress();
        if (gameBaseAddress == NULL) {
            printf("Failed to get process base address.\n");
            return false;
        }

        this->m_pBaseAddress = reinterpret_cast<void *>(
            reinterpret_cast<uintptr_t>(gameBaseAddress) +
            this->m_pBaseOffset);
        return true;
    }

    void *m_pBaseAddress = NULL;
    uintptr_t m_pBaseOffset = 0;
    Memory *mem = NULL;
};
