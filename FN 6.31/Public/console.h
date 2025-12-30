#pragma once
#include "../Game/SDK/Engine_classes.hpp"

namespace Console {
    inline SDK::UEngine* Engine = nullptr;
    inline SDK::UWorld* World = nullptr;

    inline bool Unlocked = false;
    inline bool engineFound = false;
    inline bool worldFound = false;

    SDK::UEngine* findEngine() noexcept;
    SDK::UWorld* findWorld() noexcept;
    void unlockConsole() noexcept;
    void command(SDK::FString command) noexcept;
}