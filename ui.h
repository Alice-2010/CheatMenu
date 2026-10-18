#pragma once

#include "memory.h"
#include "cheats/cheat.h"
#include "libs/imgui/imgui.h"
#include "libs/imgui/imgui_impl_dx12.h"

#include <map>
#include <windef.h>

class UI {
private:
    HWND m_hwnd;
    ImGuiContext* m_pContext = nullptr;
    Memory *mem = new Memory();
    std::map<std::string, Cheat*> m_vCheats;

public:
    bool m_bIsClosing = false;

    void Initialize(HWND);
    void ConnectToGame();
    void Update();
    void Shutdown();
};
