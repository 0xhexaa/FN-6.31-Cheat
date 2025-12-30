    #pragma once
    #include <string>
    #include <Windows.h>
    #include "../Game/SDK/Engine_classes.hpp"

    namespace keybinds
    {
        inline SDK::FKey RightClick;
        inline SDK::FKey LeftClick;
        inline SDK::FKey Insert;
        inline SDK::FKey Tab;

        inline SDK::FKey F1;
        inline SDK::FKey F2;
        inline SDK::FKey F3;
        inline SDK::FKey F4;
        inline SDK::FKey F5;
        inline SDK::FKey F6;
        inline SDK::FKey F7;
        inline SDK::FKey F8;
        inline SDK::FKey F9;

        inline SDK::FKey W;
        inline SDK::FKey A;
        inline SDK::FKey S;
        inline SDK::FKey D;
        inline SDK::FKey SpaceBar;
        inline SDK::FKey LeftShift;
    }

    namespace Configuration {
        inline bool UEConsoleUnlock = false;
        inline std::wstring UEConsoleOpenBtn = L"F2";
        inline bool UEConsoleEnabled = false;
        inline bool UEConsoleCheats = true;
        inline bool GuiMenu = true;
        inline bool Debug = true;
        inline constexpr int GuiKey = VK_INSERT;
    }

    namespace PlayerConfig {
        inline bool fovChange = false;
        inline float fovChangeAmount = 140.f;
        inline std::string name = "";
        inline bool selfRevive = false;
        inline bool instantRevive = false;
        inline bool tpEnemies = false;
        inline float tpDistance = 50.f;
        inline bool spinBotEnabled = false;
        inline float spinBotSpeed = 50.0f;
        inline bool ziplineFly = false;
    }

    namespace AimConfig {
        inline bool aimbot = true;
        inline bool customCrosshair = false;
        inline bool silentAim = false;
        inline bool visibleCheck = true;
        inline bool smoothAim = false;
        inline float smoothness = 5.0f;
        inline float fov = 52.0f;
        inline float fovCircleScale = 0.5f;
        inline int aimKey = VK_RBUTTON;
        inline int shootKey = VK_LBUTTON;
        inline bool drawFov = true;
        inline float maxDistance = 500.0f;
        inline bool prediction = false;
        inline bool mouseAim = false;
        inline bool useControllerInput = false;
        inline bool magicBullet = false;
        inline int targetSelection = 0;
        inline bool showDebug = false;
        inline bool showFOVCircle = AimConfig::drawFov;
    }

    namespace ESPConfig {
        inline bool enabled = true;
        inline bool showBoxes = true;
        inline bool showNames = true;
        inline bool showDistance = true;
        inline bool showHP = true;
        inline bool showBones = true;
        inline bool showPlatform = true;
        inline bool boxVisibilityCheck = true;
        inline float maxDistance = 250.0f;
        inline float boxThickness = 2.0f;
        inline int boxColorR = 0;
        inline int boxColorG = 255;
        inline int boxColorB = 0;
        inline int boxColorA = 255;
        inline float boneThickness = 1.5f;
        inline int boneColorR = 0;
        inline int boneColorG = 255;
        inline int boneColorB = 0;
        inline int boneColorHiddenR = 255;
        inline int boneColorHiddenG = 0;
        inline int boneColorHiddenB = 0;
        inline bool boneVisibilityCheck = true;
        inline bool useHealthOffset = false;
        inline uintptr_t healthOffset = 0;
        inline float maxHealth = 100.0f;
        inline bool filledBox = true;
        inline int filledBoxColorR = 0;
        inline int filledBoxColorG = 0;
        inline int filledBoxColorB = 0;
        inline int filledBoxColorA = 50;
        inline bool showSnaplines = true;
        inline int snaplineColorR = 255;
        inline int snaplineColorG = 255;
        inline int snaplineColorB = 255;
        inline int snaplineColorA = 255;
        inline float snaplineThickness = 1.0f;
        inline int snaplinePosition = 0;
        inline bool showWeapons = true;
	    inline bool headCircle = true;
    }

    namespace WeaponConfig {
        inline bool noRecoil = false;
        inline bool noCharge = false;
        inline bool noSpread = false;
        inline bool noReload = false;
        inline bool damageEnable = false;
        inline float damageMulitplier = 25.f;
        inline bool fireRate = false;
        inline float fireRateMulitplier = 25.f;
    }

    namespace VehicleConfig {
        inline bool noVehicleCollisions = false;
        inline bool vehicleFly = false;
    }