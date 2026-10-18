#include "ui.h"
#include "graphics_device.h"
#include "cheats/noclip.h"
#include "cheats/godmode.h"
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
    this->m_vCheats["Noclip"] = new Noclip(this->mem);
    this->m_vCheats["Godmode"] = new Godmode(this->mem);
}

void UI::ConnectToGame()
{
    this->mem->ConnectToGame();
    if (!this->mem->IsConnected())
        return;

    std::string sha256 = this->mem->GetGameSHA256();
    for (auto& cheat : this->m_vCheats) cheat.second->SetSha256(sha256);
}

void UI::Update()
{
    // Frame setup
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Cheat Menu");
    if (ImGui::Button("Connect to Game"))
        this->ConnectToGame();
    bool isConnected = this->mem->IsConnected();
    if (!isConnected) ImGui::BeginDisabled();
    ImGui::Checkbox("Enable NoClip", &this->m_vCheats["Noclip"]->m_bIsEnabled);
    ImGui::Checkbox("Enable Godmode", &this->m_vCheats["Godmode"]->m_bIsEnabled);
    if (!isConnected) ImGui::EndDisabled();
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
