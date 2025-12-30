#include "../Public/console.h"
#include "../Public/Configuration.h"
#include "../Game/SDK/Engine_classes.hpp"
#include <Windows.h>
#include <iostream>
#include "../Public/log.h"
#include "gui.h"

SDK::UEngine* Console::findEngine() noexcept {
    Engine = SDK::UEngine::GetEngine();
    engineFound = (Engine != nullptr);
    return Engine;
}

SDK::UWorld* Console::findWorld() noexcept {
    World = SDK::UWorld::GetWorld();
    worldFound = (World != nullptr);
    return World;
}

void Console::unlockConsole() noexcept
{
    if (Console::Unlocked || !Configuration::UEConsoleEnabled) return;

    SDK::UEngine* Engine = SDK::UEngine::GetEngine();
    SDK::UWorld* World = SDK::UWorld::GetWorld();

    SDK::APlayerController* MyController = World->OwningGameInstance->LocalPlayers[0]->PlayerController;

    std::cout << Engine->ConsoleClass->GetFullName() << std::endl;

    for (int i = 0; i < SDK::UObject::GObjects->Num(); i++)
    {
        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);

        if (!Obj)
            continue;

        if (Obj->IsDefaultObject())
            continue;

        if (Obj->IsA(SDK::APawn::StaticClass()) || Obj->HasTypeFlag(SDK::EClassCastFlags::Pawn))
        {
            std::cout << Obj->GetFullName() << "\n";
        }
    }

    SDK::ULevel* Level = World->PersistentLevel;
    SDK::TArray<SDK::AActor*>& Actors = Level->Actors;

    for (SDK::AActor* Actor : Actors)
    {
        if (!Actor || !Actor->IsA(SDK::EClassCastFlags::Pawn) || !Actor->IsA(SDK::APawn::StaticClass()))
            continue;

        SDK::APawn* Pawn = static_cast<SDK::APawn*>(Actor);
    }

    SDK::UInputSettings::GetDefaultObj()->ConsoleKeys[0].KeyName = SDK::UKismetStringLibrary::Conv_StringToName(L"F2");

    SDK::UObject* NewObject = SDK::UGameplayStatics::SpawnObject(Engine->ConsoleClass, Engine->GameViewport);

    Engine->GameViewport->ViewportConsole = static_cast<SDK::UConsole*>(NewObject);

    Console::Unlocked = true;
}

void Console::command(SDK::FString command) noexcept {
    __try {

        SDK::UWorld* world = SDK::UWorld::GetWorld();
        if (!world || !world->OwningGameInstance) {
            return;
        }

        if (world->OwningGameInstance->LocalPlayers.Num() == 0) {
            return;
        }

        SDK::APlayerController* player = world->OwningGameInstance->LocalPlayers[0]->PlayerController;
        if (!player) {
            return;
        }

        SDK::APawn* pawn = player->Pawn;
        if (!pawn) {
            return;
        }

        const SDK::FString consoleCommand = SDK::FString(command);

        player->SendToConsole(consoleCommand);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {

        return;
    }
}