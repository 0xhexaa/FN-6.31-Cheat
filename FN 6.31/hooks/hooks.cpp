#include "hooks.h"

#include "MinHook.h"

namespace Hooks {

    bool Init() {

        MH_STATUS Result = MH_Initialize();
        if (Result == MH_OK)
            return true;
        return false;
    }

    void Destroy() {
        MH_Uninitialize();
    }

    bool CreateHook(void* Target, void* Detour, void** Original) {
        MH_STATUS Result = MH_CreateHook(Target, Detour, Original);
        if (Result == MH_OK)
            return true;
        return false;
    }

    bool EnableHook(void* Target) {
        MH_STATUS Result = MH_EnableHook(Target);
        if (Result == MH_OK)
            return true;
        return false;
    }

    bool RemoveHook(void* Target) {
        MH_STATUS Result = MH_RemoveHook(Target);
        if (Result == MH_OK)
            return true;
        return false;
    }

    bool DisableHook(void* Target) {
        MH_STATUS Result = MH_DisableHook(Target);
        if (Result == MH_OK)
            return true;
        return false;
    }

}

