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
        return;
    }
    Cheat::SetIsEnabled(isEnabled);
}

void Godmode::Run()
{
    if (this->m_bIsEnabled)
    {
        this->mem->WriteMemoryAtPointer(
            this->m_pBaseAddress,
            this->m_vPlayer1HealthOffsets,
            &this->m_iPlayerHealthLock
        );
        this->mem->WriteMemoryAtPointer(
            this->m_pBaseAddress,
            this->m_vPlayer2HealthOffsets,
            &this->m_iPlayerHealthLock
        );
    }
}
