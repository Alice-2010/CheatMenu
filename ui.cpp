#include "ui.h"
#include "graphics_device.h"
#include "cheats/noclip.h"
#include "libs/imgui/imgui.h"
#include "libs/imgui/imgui_impl_win32.h"

void UI::Initialize(HWND hwnd)
{
    ImGui_ImplWin32_EnableDpiAwareness();
    IMGUI_CHECKVERSION();
    this->m_hwnd = hwnd;
    this->m_pContext = ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(GetImGuiInitInfo());

    this->m_vCheats["Noclip"] = new Noclip(this->mem, "73B5DC089D2A9678A2BD519647986F23E5C705ADDDA4981029774CA179625B68");
}

void UI::Update()
{
    // Frame setup
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Cheat Menu");
    if (ImGui::Button("Connect to Game"))
        this->mem->ConnectToGame();
    if (ImGui::Checkbox("Enable NoClip", &this->m_bNoclipEnabled))
        this->m_vCheats["Noclip"]->SetIsEnabled(this->m_bNoclipEnabled);
    ImGui::End();

    // Rendering
    ImGui::Render();
    RenderFrame(ImGui::GetDrawData());

    for (auto& cheat : this->m_vCheats)
    {
        cheat.second->Run();
    }
}

void UI::Shutdown()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext(this->m_pContext);
    this->mem->CloseMemoryHandles();
}
