#include "gui.h"
#include <cstdlib>
#include <string>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_impl_win32.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <cstring>
#include <cmath>
#include <cstdio>
#include "Configuration.h"
#include "../Game/SDK/Engine_classes.hpp"
#include <Windows.h>
#include <shellapi.h>
#include "player.h"
#include "console.h"
#include "esp.h"
#include "aimbot.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wideParameter, LPARAM longParameter);

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static bool hiddenMouse = false;
    if (gui::open)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
            return true;

        if (uMsg == WM_MOUSEMOVE || uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP ||
            uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP || uMsg == WM_MBUTTONDOWN ||
            uMsg == WM_MBUTTONUP || uMsg == WM_MOUSEWHEEL)
            return true;
    }

    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        gui::is_running = false;
        return 0;
    }

    return CallWindowProc(gui::oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT __stdcall gui::hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if (!gui::initDx)
    {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&gui::pDevice)))
        {
            gui::pDevice->GetImmediateContext(&gui::pContext);

            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            gui::window = sd.OutputWindow;

            ID3D11Texture2D* pBackBuffer;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            gui::pDevice->CreateRenderTargetView(pBackBuffer, NULL, &gui::mainRenderTargetView);
            pBackBuffer->Release();

            gui::oWndProc = (WNDPROC)SetWindowLongPtr(gui::window, GWLP_WNDPROC, (LONG_PTR)WndProc);

            gui::CreateImGui();
            gui::initDx = true;
        }
        else
            return gui::oPresent(pSwapChain, SyncInterval, Flags);
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (gui::open)
        gui::Render();

    UpdateESP();

    Aimbot::DrawFOVCircle();

    ImGui::Render();

    gui::pContext->OMSetRenderTargets(1, &gui::mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return gui::oPresent(pSwapChain, SyncInterval, Flags);
}

void gui::SetupImGuiStyle() noexcept {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.98f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.25f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.20f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.25f, 0.45f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.40f, 0.30f, 0.60f, 0.67f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.08f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.12f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.08f, 0.10f, 0.75f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.14f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.25f, 0.45f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.30f, 0.60f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.40f, 0.70f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.35f, 0.80f, 0.78f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.45f, 0.90f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.18f, 0.40f, 0.75f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.25f, 0.55f, 0.86f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.45f, 0.30f, 0.70f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.25f, 0.18f, 0.40f, 0.76f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.25f, 0.55f, 0.86f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.45f, 0.30f, 0.70f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.25f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.60f, 0.40f, 0.90f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.70f, 0.50f, 1.00f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.30f, 0.25f, 0.45f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.40f, 0.30f, 0.60f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.50f, 0.40f, 0.70f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.10f, 0.25f, 0.86f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.30f, 0.20f, 0.45f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.18f, 0.40f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.08f, 0.15f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.10f, 0.25f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.40f, 0.30f, 0.60f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(8.0f, 4.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing = 21.0f;
    style.ScrollbarSize = 16.0f;
    style.GrabMinSize = 12.0f;
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
    style.WindowRounding = 8.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 4.0f;
    style.LogSliderDeadzone = 4.0f;
    style.TabRounding = 4.0f;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
    style.DisplaySafeAreaPadding = ImVec2(3.0f, 3.0f);
}

void gui::CreateImGui() noexcept {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ::ImGui::GetIO();
    io.IniFilename = NULL;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    SetupImGuiStyle();

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(pDevice, pContext);
}

void gui::DestroyImGui() noexcept {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void gui::RenderAimbotTab() {
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.8f, 0.6f, 1.0f, 1.0f), "Aimbot Settings");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Checkbox("Enable Aimbot", &AimConfig::aimbot);
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), AimConfig::aimbot ? "ACTIVE" : "INACTIVE");

    if (!AimConfig::aimbot) return;

    ImGui::Spacing();
    ImGui::Columns(2, "AimbotColumns", false);

    ImGui::Checkbox("Silent Aim", &AimConfig::silentAim);
    ImGui::Checkbox("Custom Crosshair", &AimConfig::customCrosshair);
    ImGui::Checkbox("Visible Check", &AimConfig::visibleCheck);
    ImGui::Checkbox("Smooth Aim", &AimConfig::smoothAim);
    ImGui::Checkbox("Draw FOV", &AimConfig::drawFov);
    ImGui::Checkbox("Prediction", &AimConfig::prediction);
    ImGui::Checkbox("Mouse Aim", &AimConfig::mouseAim);
    ImGui::Checkbox("Magic Bullet", &AimConfig::magicBullet);

    ImGui::NextColumn();

    if (AimConfig::smoothAim) {
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderFloat("##Smoothness", &AimConfig::smoothness, 1.0f, 50.0f, "Smooth: %.1f");
    }

    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##FOV", &AimConfig::fov, 1.0f, 180.0f, "FOV: %.0f°");

    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##Distance", &AimConfig::maxDistance, 1.0f, 1000.0f, "Range: %.0f");

    ImGui::Columns(1);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Aim Key Binding");
    ImGui::Spacing();

    static bool waitingForKey = false;
    static bool keyBindDelay = false;
    static float keyBindTimer = 0.0f;
    static bool lastKeyStates[256] = { false };

    auto GetKeyName = [](int vk) -> std::string {
        if (vk == VK_LBUTTON) return "Left Mouse";
        if (vk == VK_RBUTTON) return "Right Mouse";
        if (vk == VK_MBUTTON) return "Middle Mouse";
        if (vk == VK_XBUTTON1) return "Mouse 4";
        if (vk == VK_XBUTTON2) return "Mouse 5";
        if (vk == VK_SHIFT) return "Shift";
        if (vk == VK_CONTROL) return "Ctrl";
        if (vk == VK_MENU) return "Alt";
        if (vk == VK_SPACE) return "Space";
        if (vk == VK_RETURN) return "Enter";
        if (vk == VK_CAPITAL) return "Caps Lock";
        if (vk == VK_ESCAPE) return "Escape";
        if (vk >= VK_F1 && vk <= VK_F12) {
            char buf[8];
            sprintf_s(buf, "F%d", vk - VK_F1 + 1);
            return buf;
        }
        if ((vk >= '0' && vk <= '9') || (vk >= 'A' && vk <= 'Z')) return std::string(1, (char)vk);
        char keyName[256];
        if (GetKeyNameTextA(MapVirtualKeyA(vk, MAPVK_VK_TO_VSC) << 16, keyName, 256) > 0) return std::string(keyName);
        return "Unknown";
        };

    std::string currentKeyName = GetKeyName(AimConfig::aimKey);
    ImGui::Text("Current Key: ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.8f, 0.6f, 1.0f, 1.0f), "%s", currentKeyName.c_str());

    ImGui::Spacing();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    if (waitingForKey) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        if (ImGui::Button("PRESS ANY KEY...", ImVec2(-1, 35))) {
            waitingForKey = false;
            keyBindDelay = false;
            keyBindTimer = 0.0f;
            for (int i = 0; i < 256; i++) lastKeyStates[i] = false;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), " (Click to cancel)");

        if (keyBindDelay) {
            keyBindTimer -= ImGui::GetIO().DeltaTime;
            if (keyBindTimer <= 0.0f) {
                keyBindDelay = false;
                keyBindTimer = 0.0f;
                for (int i = 0; i < 256; i++) lastKeyStates[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
            }
        }
        else {
            for (int vk = 1; vk < 256; vk++) {
                bool currentState = (GetAsyncKeyState(vk) & 0x8000) != 0;
                if (currentState && !lastKeyStates[vk]) {
                    if (vk == VK_LBUTTON && ImGui::IsItemHovered()) {
                        lastKeyStates[vk] = currentState;
                        continue;
                    }
                    AimConfig::aimKey = vk;
                    waitingForKey = false;
                    keyBindDelay = false;
                    keyBindTimer = 0.0f;
                    for (int i = 0; i < 256; i++) lastKeyStates[i] = false;
                    break;
                }
                lastKeyStates[vk] = currentState;
            }
        }
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.2f, 0.5f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.3f, 0.6f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.4f, 0.7f, 1.0f));
        if (ImGui::Button("SET AIM KEY", ImVec2(-1, 35))) {
            waitingForKey = true;
            keyBindDelay = true;
            keyBindTimer = 0.2f;
            for (int i = 0; i < 256; i++) lastKeyStates[i] = false;
        }
        ImGui::PopStyleColor(3);
    }
    ImGui::PopStyleVar();

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 0.6f, 1.0f), "Status: %s",
        (GetAsyncKeyState(AimConfig::aimKey) & 0x8000) ? "PRESSED" : "READY");
}


void gui::RenderExploitsTab() {
    if (ImGui::BeginChild("ExploitTabs", ImVec2(0, 400), true)) {
        if (ImGui::BeginTabBar("ExploitSubTabs")) {

            if (ImGui::BeginTabItem("Player")) {
                ImGui::TextColored(ImVec4(0.8f, 0.6f, 1.0f, 1.0f), "Player Exploits");
                ImGui::Separator();

                ImGui::Checkbox("Change FOV", &PlayerConfig::fovChange);
                if (PlayerConfig::fovChange) {
                    ImGui::SliderFloat("FOV Amount", &PlayerConfig::fovChangeAmount, 80.0f, 170.0f, "%.1f");
                }

                ImGui::Checkbox("Revive Self", &PlayerConfig::selfRevive);
                ImGui::Checkbox("Instant Revive", &PlayerConfig::instantRevive);

                ImGui::Text("Name Change:");
                static char nameBuffer[4096] = "";
                ImGui::InputText("##NameChange", nameBuffer, sizeof(nameBuffer));
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    if (strlen(nameBuffer) > 0) {
                        PlayerConfig::name = std::string(nameBuffer);
                        changeName();
                        nameBuffer[0] = '\0';
                    }
                }

                ImGui::Checkbox("Spin Bot", &PlayerConfig::spinBotEnabled);
                if (PlayerConfig::spinBotEnabled) {
                    ImGui::SliderFloat("Spin Bot Speed", &PlayerConfig::spinBotSpeed, 5.f, 100.f, "%.1f");
                }

                ImGui::Checkbox("TP Enemies", &PlayerConfig::tpEnemies);
                if (PlayerConfig::tpEnemies) {
                    ImGui::SliderFloat("TP Distance", &PlayerConfig::tpDistance, 5.f, 400.f, "%.1f");
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Weapon")) {
                ImGui::TextColored(ImVec4(0.8f, 0.6f, 1.0f, 1.0f), "Weapon Exploits");
                ImGui::Separator();

                ImGui::Checkbox("No Charge", &WeaponConfig::noCharge);
                ImGui::Checkbox("No Recoil", &WeaponConfig::noRecoil);
                ImGui::Checkbox("No Spread", &WeaponConfig::noSpread);
                ImGui::Checkbox("No Reload", &WeaponConfig::noReload);
                ImGui::Checkbox("Damage Changer", &WeaponConfig::damageEnable);

                if (WeaponConfig::damageEnable) {
                    ImGui::SliderFloat("Damage Multiplier", &WeaponConfig::damageMulitplier, 1.f, 75.f, "%.1f");
                }

                ImGui::Checkbox("Rapid Fire", &WeaponConfig::fireRate);

                if (WeaponConfig::fireRate) {
                    ImGui::SliderFloat("Fire Rate Multiplier", &WeaponConfig::fireRateMulitplier, 1.f, 75.f, "%.1f");
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Vehicle")) {
                ImGui::TextColored(ImVec4(0.8f, 0.6f, 1.0f, 1.0f), "Vehicle Exploits");
                ImGui::Separator();

                ImGui::Checkbox("Vehicle Fly", &VehicleConfig::vehicleFly);
                ImGui::Checkbox("No Collisions", &VehicleConfig::noVehicleCollisions);

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
        ImGui::EndChild();
    }
}

void gui::RenderESPTab() {
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.8f, 0.6f, 1.0f, 1.0f), "ESP Settings");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Checkbox("Enable ESP", &ESPConfig::enabled);
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), ESPConfig::enabled ? "ACTIVE" : "INACTIVE");

    if (!ESPConfig::enabled) return;

    ImGui::Spacing();
    ImGui::Columns(2, "ESPColumns", false);

    ImGui::Checkbox("2D Box", &ESPConfig::showBoxes);
    ImGui::Checkbox("Player Names", &ESPConfig::showNames);
    ImGui::Checkbox("Distance", &ESPConfig::showDistance);
    ImGui::Checkbox("Player HP", &ESPConfig::showHP);
    ImGui::Checkbox("Platform", &ESPConfig::showPlatform);
    ImGui::Checkbox("Skeleton", &ESPConfig::showBones);
    ImGui::Checkbox("Weapon", &ESPConfig::showWeapons);
    ImGui::Checkbox("Head Circle", &ESPConfig::headCircle);

    ImGui::NextColumn();

    ImGui::Checkbox("Filled Box", &ESPConfig::filledBox);
    ImGui::Checkbox("Snaplines", &ESPConfig::showSnaplines);
    ImGui::Checkbox("Box Visibility", &ESPConfig::boxVisibilityCheck);
    if (ESPConfig::showBones) {
        ImGui::Checkbox("Bone Visibility", &ESPConfig::boneVisibilityCheck);
    }

    ImGui::Columns(1);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Visual Settings");
    ImGui::Spacing();

    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##MaxDistance", &ESPConfig::maxDistance, 5.0f, 1000.0f, "Max Distance: %.0f");

    if (ESPConfig::showBoxes || ESPConfig::filledBox || ESPConfig::showSnaplines || ESPConfig::showBones) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Colors");
        ImGui::Spacing();

        ImGui::Columns(2, "ColorColumns", false);

        if (ESPConfig::showBoxes) {
            float boxColor[4] = { ESPConfig::boxColorR / 255.0f, ESPConfig::boxColorG / 255.0f, ESPConfig::boxColorB / 255.0f, ESPConfig::boxColorA / 255.0f };
            ImGui::ColorEdit4("Box##Color", boxColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
            ESPConfig::boxColorR = static_cast<int>(boxColor[0] * 255);
            ESPConfig::boxColorG = static_cast<int>(boxColor[1] * 255);
            ESPConfig::boxColorB = static_cast<int>(boxColor[2] * 255);
            ESPConfig::boxColorA = static_cast<int>(boxColor[3] * 255);
            ImGui::SameLine();
            ImGui::Text("Box Color");

            ImGui::SetNextItemWidth(-1);
            ImGui::SliderFloat("##BoxThick", &ESPConfig::boxThickness, 0.5f, 25.0f, "Thick: %.1f");
        }

        if (ESPConfig::filledBox) {
            float filledColor[4] = { ESPConfig::filledBoxColorR / 255.0f, ESPConfig::filledBoxColorG / 255.0f, ESPConfig::filledBoxColorB / 255.0f, ESPConfig::filledBoxColorA / 255.0f };
            ImGui::ColorEdit4("Filled##Color", filledColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
            ESPConfig::filledBoxColorR = static_cast<int>(filledColor[0] * 255);
            ESPConfig::filledBoxColorG = static_cast<int>(filledColor[1] * 255);
            ESPConfig::filledBoxColorB = static_cast<int>(filledColor[2] * 255);
            ESPConfig::filledBoxColorA = static_cast<int>(filledColor[3] * 255);
            ImGui::SameLine();
            ImGui::Text("Filled Color");
        }

        ImGui::NextColumn();

        if (ESPConfig::showSnaplines) {
            float snaplineColor[4] = { ESPConfig::snaplineColorR / 255.0f, ESPConfig::snaplineColorG / 255.0f, ESPConfig::snaplineColorB / 255.0f, ESPConfig::snaplineColorA / 255.0f };
            ImGui::ColorEdit4("Line##Color", snaplineColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
            ESPConfig::snaplineColorR = static_cast<int>(snaplineColor[0] * 255);
            ESPConfig::snaplineColorG = static_cast<int>(snaplineColor[1] * 255);
            ESPConfig::snaplineColorB = static_cast<int>(snaplineColor[2] * 255);
            ESPConfig::snaplineColorA = static_cast<int>(snaplineColor[3] * 255);
            ImGui::SameLine();
            ImGui::Text("Snapline Color");

            ImGui::SetNextItemWidth(-1);
            ImGui::SliderFloat("##LineThick", &ESPConfig::snaplineThickness, 0.5f, 15.0f, "Thick: %.1f");

            const char* positions[] = { "Bottom", "Middle", "Top" };
            ImGui::SetNextItemWidth(-1);
            ImGui::Combo("##LinePos", &ESPConfig::snaplinePosition, positions, IM_ARRAYSIZE(positions));
            ImGui::SameLine();
            ImGui::Text("Position");
        }

        if (ESPConfig::showBones) {
            float boneColor[3] = { ESPConfig::boneColorR / 255.0f, ESPConfig::boneColorG / 255.0f, ESPConfig::boneColorB / 255.0f };
            ImGui::ColorEdit3("Bone##Color", boneColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            ESPConfig::boneColorR = static_cast<int>(boneColor[0] * 255);
            ESPConfig::boneColorG = static_cast<int>(boneColor[1] * 255);
            ESPConfig::boneColorB = static_cast<int>(boneColor[2] * 255);
            ImGui::SameLine();
            ImGui::Text("Bone Color");

            float boneHiddenColor[3] = { ESPConfig::boneColorHiddenR / 255.0f, ESPConfig::boneColorHiddenG / 255.0f, ESPConfig::boneColorHiddenB / 255.0f };
            ImGui::ColorEdit3("Hidden##Color", boneHiddenColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            ESPConfig::boneColorHiddenR = static_cast<int>(boneHiddenColor[0] * 255);
            ESPConfig::boneColorHiddenG = static_cast<int>(boneHiddenColor[1] * 255);
            ESPConfig::boneColorHiddenB = static_cast<int>(boneHiddenColor[2] * 255);
            ImGui::SameLine();
            ImGui::Text("Hidden Color");

            ImGui::SetNextItemWidth(-1);
            ImGui::SliderFloat("##BoneThick", &ESPConfig::boneThickness, 0.5f, 15.0f, "Thick: %.1f");
        }

        ImGui::Columns(1);
    }
}



void gui::RenderUEConsoleTab() {
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.8f, 0.6f, 1.0f, 1.0f), "Unreal Engine Console");
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Unlock UE Console")) Console::unlockConsole();


    ImGui::Text("UE Command:");
    static char cmdBuffer[4096] = "";
    ImGui::InputText("##Command", cmdBuffer, sizeof(cmdBuffer));
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        if (strlen(cmdBuffer) > 0) {
            changeName();
            cmdBuffer[0] = '\0';
        }
    }
}



static ImVec2 menuPos = ImVec2(50, 50);
static bool draggingMenu = false;
static ImVec2 dragOffset = ImVec2(0, 0);

void gui::Render() noexcept {
    ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT), ImGuiCond_FirstUseEver);

    if (draggingMenu) {
        ImGui::SetNextWindowPos(menuPos);
    }
    else {
        ImGui::SetNextWindowPos(menuPos, ImGuiCond_FirstUseEver);
    }

    ImGui::Begin("HA - 6.31 MENU", &gui::open,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_AlwaysAutoResize);

    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    if (!draggingMenu && ImGui::IsMouseHoveringRect(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + 30)) &&
        ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        draggingMenu = true;
        ImVec2 mousePos = ImGui::GetMousePos();
        dragOffset = ImVec2(mousePos.x - windowPos.x, mousePos.y - windowPos.y);
    }

    if (draggingMenu) {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            ImVec2 mousePos = ImGui::GetMousePos();
            menuPos = ImVec2(mousePos.x - dragOffset.x, mousePos.y - dragOffset.y);
            ImGui::SetWindowPos(menuPos);
        }
        else {
            draggingMenu = false;
        }
    }

    ImGui::BeginChild("Header", ImVec2(0, 50), true);
    ImGui::SetCursorPos(ImVec2(20, 15));
    ImGui::TextColored(ImVec4(0.8f, 0.6f, 1.0f, 1.0f), "HA - 6.31 MENU");
    ImGui::SameLine(ImGui::GetWindowWidth() - 120);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "v2.0");
    ImGui::EndChild();

    ImGui::SetCursorPosY(60);
    ImGui::BeginChild("Content", ImVec2(0, 0), true);

    if (ImGui::BeginTabBar("MainTabs", ImGuiTabBarFlags_FittingPolicyScroll)) {
        ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.15f, 0.10f, 0.25f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.25f, 0.18f, 0.40f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(0.20f, 0.15f, 0.35f, 1.0f));

        if (ImGui::BeginTabItem("HOME")) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Welcome to HA Cheat Menu");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Made by Hexa & Axiom");
            ImGui::Spacing();

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.25f, 0.55f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.45f, 0.35f, 0.65f, 0.9f));
            if (ImGui::Button("JOIN DISCORD", ImVec2(200, 40))) {
                ShellExecute(0, 0, L"https://discord.gg/dqMysEVf2j", 0, 0, SW_SHOW);
            }
            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "Status: ");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "INJECTED");

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Use the tabs above to configure features");

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("AIMBOT")) {
            RenderAimbotTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("ESP")) {
            RenderESPTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("EXPLOITS")) {
            RenderExploitsTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("CONSOLE")) {
            RenderUEConsoleTab();
            ImGui::EndTabItem();
        }

        ImGui::PopStyleColor(3);
        ImGui::EndTabBar();
    }

    ImGui::EndChild();
    ImGui::End();

    if (gui::open)
    {

        ImGuiIO& io = ImGui::GetIO();

        io.WantCaptureMouse = true;

        io.MouseDrawCursor = false;

        ImVec2 mousePos = io.MousePos;
        float crossSize = 12.0f;
        float thickness = 2.0f;

        ImGui::GetForegroundDrawList()->AddLine(
            ImVec2(mousePos.x - crossSize, mousePos.y),
            ImVec2(mousePos.x + crossSize, mousePos.y),
            IM_COL32(0, 0, 0, 255), thickness + 1.0f);
        ImGui::GetForegroundDrawList()->AddLine(
            ImVec2(mousePos.x, mousePos.y - crossSize),
            ImVec2(mousePos.x, mousePos.y + crossSize),
            IM_COL32(0, 0, 0, 255), thickness + 1.0f);

        ImGui::GetForegroundDrawList()->AddLine(
            ImVec2(mousePos.x - crossSize, mousePos.y),
            ImVec2(mousePos.x + crossSize, mousePos.y),
            IM_COL32(255, 255, 255, 255), thickness);
        ImGui::GetForegroundDrawList()->AddLine(
            ImVec2(mousePos.x, mousePos.y - crossSize),
            ImVec2(mousePos.x, mousePos.y + crossSize),
            IM_COL32(255, 255, 255, 255), thickness);
    }
}