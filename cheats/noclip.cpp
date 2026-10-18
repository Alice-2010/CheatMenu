#include "noclip.h"
#include "../structs/CKKeyboard.h"

const auto isPressed = [](char keyState) {
    return static_cast<unsigned char>(keyState) == 0xFF;
};

const std::string Noclip::GetCheatName()
{
    return "Noclip";
}

void Noclip::SetIsEnabled(bool isEnabled)
{
    if (isEnabled)
    {
        if (
            !this->ResolveBaseAddress() ||
            !this->mem->ReadMemoryAtPointer(
                this->m_pBaseAddress,
                this->m_vPositionOffsets,
                &this->m_vLockedPosition
            )
        )
        {
            Cheat::SetIsEnabled(false);
            return;
        }
    }
    Cheat::SetIsEnabled(isEnabled);
}

void Noclip::Run()
{
    if (this->m_bIsEnabled)
    {
        CKKeyboard keyboard;
        if (!this->mem->ReadMemoryAtPointer(this->m_pBaseAddress, {0x18, 0xC, 0x0}, &keyboard))
            return;

        if (isPressed(keyboard.m_c1))
            this->m_vLockedPosition.x += this->m_fMovementSpeed;
        else if (isPressed(keyboard.m_c2))
            this->m_vLockedPosition.x -= this->m_fMovementSpeed;
        if (isPressed(keyboard.m_c3))
            this->m_vLockedPosition.y += this->m_fMovementSpeed;
        else if (isPressed(keyboard.m_c4))
            this->m_vLockedPosition.y -= this->m_fMovementSpeed;
        if (isPressed(keyboard.m_c5))
            this->m_vLockedPosition.z += this->m_fMovementSpeed;
        else if (isPressed(keyboard.m_c6))
            this->m_vLockedPosition.z -= this->m_fMovementSpeed;

        this->mem->WriteMemoryAtPointer(this->m_pBaseAddress, this->m_vPositionOffsets, &this->m_vLockedPosition);
    }
}
