#include "godmode.h"

const std::string Godmode::GetCheatName()
{
    return "Godmode";
}

void Godmode::SetIsEnabled(bool isEnabled)
{
    if (isEnabled && !this->ResolveBaseAddress())
    {
        Cheat::SetIsEnabled(false);
        printf("Failed to resolve base address (error %lu)\n", GetLastError());
        return;
    }
    Cheat::SetIsEnabled(isEnabled);
}

void Godmode::Run()
{
    if (this->m_bIsEnabled)
    {
        bool success = this->mem->WriteMemoryAtPointer(
            this->m_pBaseAddress,
            this->m_vPlayer1HealthOffsets,
            &this->m_iPlayerHealthLock
        );
        if (!success)
        {
            Cheat::SetIsEnabled(false);
            printf("Failed to write player 1 health (error %lu)\n", GetLastError());
            return;
        }
        success = this->mem->WriteMemoryAtPointer(
            this->m_pBaseAddress,
            this->m_vPlayer2HealthOffsets,
            &this->m_iPlayerHealthLock
        );
        if (!success)
        {
            Cheat::SetIsEnabled(false);
            printf("Failed to write player 2 health (error %lu)\n", GetLastError());
            return;
        }
    }
}
