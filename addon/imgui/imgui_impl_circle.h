// dear imgui: Platform Backend for Circle
// This needs to be used along with a Renderer (e.g. OpenGL3)
// (Info: Circle is a C++ bare metal programming environment for the Raspberry Pi. The VideoCoreIV found in the Raspberry Pi 1, 2 and 3 supports OpenGL ES 2.0 and OpenVG 1.1.)

// Implemented features:
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Clipboard support.
//  [X] Platform: Keyboard arrays indexed using SDL_SCANCODE_* codes, e.g. ImGui::IsKeyPressed(SDL_SCANCODE_SPACE).
// Missing features:
//  [ ] Platform: Gamepad support. Enabled with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.
//  [ ] Platform: ???

// You can copy and use unmodified imgui_impl_* files in your project. See sample/demo.cpp for an example of using this.
// If you are new to dear imgui, read the documentation at the top of imgui.cpp and at https://github.com/ocornut/imgui.

#pragma once
#include "imgui.h"      // IMGUI_IMPL_API

void CircleInit(void);
void CircleExit(void);
bool CircleTerminate(void);
void CircleSwapBuffers(void);

IMGUI_IMPL_API bool     ImGui_ImplCircle_Init(void);
IMGUI_IMPL_API void     ImGui_ImplCircle_Shutdown(void);
IMGUI_IMPL_API void     ImGui_ImplCircle_NewFrame(void);
IMGUI_IMPL_API bool     ImGui_ImplCircle_ProcessEvent(void);
