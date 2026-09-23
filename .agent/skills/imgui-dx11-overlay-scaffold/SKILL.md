---
name: imgui-dx11-overlay-scaffold
description: Build high-performance Dear ImGui overlays for DirectX 11 with dark-mode phosphor Pip-Boy styling, custom widgets, and input blocking.
triggers:
  - style imgui fallout
  - imgui wndproc hook
  - fallout 4 gui menu
  - directx 11 imgui trainer
---

# ImGui DX11 Overlay Scaffold Skill

## Purpose
Scaffolds a production-ready, dark phosphor green Pip-Boy themed interface rendered via Dear ImGui over DirectX 11, with clean Win32 WndProc input dispatching.

## UI Design Guidelines

### 1. Theme Configuration (Dark Carbon / Phosphor)
```cpp
void ApplyPipBoyTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding    = 4.0f;
    style.FrameRounding     = 2.0f;
    style.PopupRounding     = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding      = 2.0f;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 1.0f;

    // Palette: Pitch Carbon Matte + Nuclear Phosphor Accent
    colors[ImGuiCol_WindowBg]             = ImVec4(0.05f, 0.05f, 0.07f, 0.95f);
    colors[ImGuiCol_Border]               = ImVec4(0.12f, 0.40f, 0.20f, 0.80f);
    colors[ImGuiCol_Text]                 = ImVec4(0.92f, 0.95f, 0.92f, 1.00f);
    colors[ImGuiCol_Header]               = ImVec4(0.08f, 0.25f, 0.12f, 0.80f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(0.12f, 0.40f, 0.18f, 0.80f);
    colors[ImGuiCol_Button]               = ImVec4(0.08f, 0.20f, 0.10f, 0.85f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.14f, 0.38f, 0.18f, 1.00f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.18f, 0.55f, 0.24f, 1.00f);
    colors[ImGuiCol_CheckMark]            = ImVec4(0.20f, 0.95f, 0.35f, 1.00f);
    colors[ImGuiCol_SliderGrab]           = ImVec4(0.18f, 0.65f, 0.28f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.22f, 0.95f, 0.38f, 1.00f);
}
```

### 2. Input Isolation

When `bMenuOpen` is toggled:
- Intercept keys in `WndProc`. If `VK_INSERT` is pressed, toggle `bMenuOpen = !bMenuOpen`.
- When `bMenuOpen == true`:
  - Call `ImGui_ImplWin32_WndProcHandler`.
  - Set `ImGui::GetIO().WantCaptureMouse = true` and `WantCaptureKeyboard = true`.
  - Show the Windows cursor: `ShowCursor(TRUE)`.
  - Consume mouse and keyboard events to prevent the character from attacking or turning while interacting with sliders.
- When `bMenuOpen == false`:
  - `ShowCursor(FALSE)`.
  - Pass all inputs directly to the engine's original `WndProc`.
