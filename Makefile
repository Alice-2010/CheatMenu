cpp_sources = main.cpp graphics_device.cpp memory.cpp ui.cpp cheats/noclip.cpp cheats/godmode.cpp libs/imgui/imgui.cpp libs/imgui/imgui_impl_dx12.cpp libs/imgui/imgui_impl_win32.cpp libs/imgui/imgui_draw.cpp libs/imgui/imgui_tables.cpp libs/imgui/imgui_widgets.cpp

link_libs = kernel32.lib bcrypt.lib d3d12.lib dxgi.lib user32.lib gdi32.lib

.PHONY: build
build:
	@echo "Building project..."
	@mkdir -p build
	@cl /O2 /GA /Fe:build/AliceCheatMenu.exe /Fo:build/ $(cpp_sources) /Ilibs/imgui /link $(link_libs)
