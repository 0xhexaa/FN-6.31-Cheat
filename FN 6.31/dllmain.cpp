#define _CRT_SECURE_NO_WARNINGS

#include "gui.h"
#include "player.h"
#include <thread>
#include "Configuration.h"
#include "../Game/SDK/Engine_classes.hpp"
#include "kiero/kiero.h"
#include <windows.h>
#include <chrono>
#include "esp.h"
#include <string>
#include <tlhelp32.h>
#include "aimbot.h"
#include "weapon.h"
#include "hooks/hooks.h"
#include "vehicle.h"

static bool shouldUninject = false;
static bool hooksInitialized = false;
static bool minHookInitialized = false;

bool IsFortniteProcess() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    DWORD currentPID = GetCurrentProcessId();
    bool isFortnite = false;

    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (pe32.th32ProcessID == currentPID) {
                std::wstring processName = pe32.szExeFile;
                std::wstring targetName = L"FortniteClient-Win64-Shipping.exe";

                if (processName.find(targetName) != std::wstring::npos) {
                    isFortnite = true;
                    break;
                }

                std::wstring targetName2 = L"FortniteClient-Win64-Shipping_BE.exe";
                std::wstring targetName3 = L"FortniteLauncher.exe";
                std::wstring targetName4 = L"FortniteGame.exe";

                if (processName.find(targetName2) != std::wstring::npos ||
                    processName.find(targetName3) != std::wstring::npos ||
                    processName.find(targetName4) != std::wstring::npos) {
                    isFortnite = true;
                    break;
                }
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return isFortnite;
}

bool InitializeHooks()
{
    if (hooksInitialized) {
        return true;
    }

    if (!minHookInitialized) {
        if (!Hooks::Init()) {
            return false;
        }
        minHookInitialized = true;
    }

    if (kiero::init(kiero::RenderType::D3D11) != kiero::Status::Success) {
        return false;
    }

    if (kiero::bind(8, (void**)&gui::oPresent, gui::hkPresent) != kiero::Status::Success) {
        return false;
    }

    hooksInitialized = true;
    return true;
}

static void ApplyFeatures()
{

    SDK::UWorld* world = SDK::UWorld::GetWorld();
    if (!world || !world->OwningGameInstance || world->OwningGameInstance->LocalPlayers.Num() == 0) {
        return;
    }

    SDK::APlayerController* localController = world->OwningGameInstance->LocalPlayers[0]->PlayerController;
    if (!localController || !localController->Pawn) {
        return;
    }

    Aimbot::Tick();
    Player::Tick();
    Weapon::Tick();
    Vehicle::Tick();
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    HMODULE hModule = (HMODULE)lpParam;

    if (!IsFortniteProcess()) {
        return 0;
    }

    while (!shouldUninject)
    {
        if (InitializeHooks()) {
            break;
        }

        if (GetAsyncKeyState(VK_END) & 1)
        {
            shouldUninject = true;
            break;
        }
        Sleep(100);
    }

    while (!shouldUninject)
    {
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            gui::open = !gui::open;
        }

        if (GetAsyncKeyState(VK_END) & 1) {
            shouldUninject = true;
        }

        ApplyFeatures();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    gui::is_running = false;
    gui::open = false;
    Sleep(100);

    if (hooksInitialized) {
        kiero::shutdown();
    }

    if (minHookInitialized) {
        Hooks::Destroy();
    }

    Sleep(100);

    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}