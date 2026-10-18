#pragma once

#include "cheat.h"
#include "../structs/types.h"

#include <vector>

class Godmode : public Cheat
{
private:
    float m_iPlayerHealthLock = 200;
    std::vector<size_t> m_vPlayer1HealthOffsets = { 0x8C, 0x6C, 0x60 };
    std::vector<size_t> m_vPlayer2HealthOffsets = { 0x8C, 0x6C, 0x68 };

public:
    Godmode(Memory * mem) : Cheat(mem) {};
    virtual const std::string GetCheatName() override;
    virtual void SetIsEnabled(bool isEnabled) override;
    virtual void Run() override;
};
