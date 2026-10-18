#pragma once

#include "cheat.h"
#include "../structs/types.h"

#include <vector>

class Noclip : public Cheat
{
private:
    Vec3 m_vLockedPosition;
    std::vector<size_t> m_vPositionOffsets = { 0x8C, 0x8, 0x28, 0x4, 0x4, 0x688 };
    float m_fMovementSpeed = 0.1f;

public:
    Noclip(Memory * mem) : Cheat(mem) {};
    virtual const std::string GetCheatName() override;
    virtual void SetIsEnabled(bool isEnabled) override;
    virtual void Run() override;
};
