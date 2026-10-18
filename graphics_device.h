#include "libs/imgui/imgui.h"
#include "libs/imgui/imgui_impl_dx12.h"

#include <d3d12.h>

bool CreateDeviceD3D(HWND);
void CleanupDeviceD3D();
ImGui_ImplDX12_InitInfo* GetImGuiInitInfo();
void RenderFrame(ImDrawData* drawData);
