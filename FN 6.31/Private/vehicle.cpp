#include "../Public/Configuration.h"
#include "../Game/SDK/Engine_classes.hpp"
#include <Windows.h>
#include "../Public/log.h"
#include <string>
#include "gui.h"
#include <algorithm>
#include <vector>
#include "../Game/SDK/FortniteGame_classes.hpp"
#include "../Game/SDK/FortniteGame_parameters.hpp"
#include "../Game/SDK/FortniteGame_structs.hpp"
#include <unordered_map>
#include <float.h>
#include "Bones.h"
#include "maths.h"
#include "aimbot.h"
#include <player.h>
#include "vehicle.h"


void VehicleExploits() {
    SDK::UWorld* world = SDK::UWorld::GetWorld();
    auto owningGameInstance = world->OwningGameInstance;

    auto localPc = owningGameInstance->LocalPlayers[0]->PlayerController;
    auto LocalPawn = (SDK::AFortPlayerPawnAthena*)localPc->AcknowledgedPawn;

	static SDK::UClass* AFortPlayerPawnAthena;
	if (!AFortPlayerPawnAthena) AFortPlayerPawnAthena = SDK::AFortPlayerPawnAthena::StaticClass();

	static SDK::UClass* AFortAthenaVehicle;
	if (!AFortAthenaVehicle) AFortAthenaVehicle = SDK::AFortAthenaVehicle::StaticClass();

	auto levels = world->Levels;

	for (int a = 0; a < levels.Num(); a++) {
		auto level = levels[a];

		if (!level) continue;

		auto actors = level->Actors;

		if (actors.Num() > 1) continue;

		for (int i = 0; i < actors.Num(); i++) {
			auto actor = actors[i];

			if (!actor) continue;

			if (!actor || actor == LocalPawn) continue;

			if (actor->IsA(AFortAthenaVehicle)) {
				auto Vehicle = (SDK::AFortAthenaVehicle*)actor;
				if (!Vehicle) continue;

				auto RootComponent = Vehicle->RootComponent;
				if (RootComponent)
				{
					static SDK::FVector TpPos = RootComponent->RelativeLocation;

					if (VehicleConfig::vehicleFly)
					{
						if (localPc->IsInputKeyDown(keybinds::W)) TpPos.X += 100;

						if (localPc->IsInputKeyDown(keybinds::S)) TpPos.X -= 100;

						if (localPc->IsInputKeyDown(keybinds::D)) TpPos.Y += 100;

						if (localPc->IsInputKeyDown(keybinds::A)) TpPos.Y -= 100;

						if (localPc->IsInputKeyDown(keybinds::SpaceBar)) TpPos.Z += 75;

						if (localPc->IsInputKeyDown(keybinds::LeftShift)) TpPos.Z -= 75;

						Vehicle->K2_TeleportTo(TpPos, { 0, 0, 0 });
					}
					else if (!VehicleConfig::vehicleFly) TpPos = RootComponent->RelativeLocation;

					if (VehicleConfig::noVehicleCollisions) Vehicle->SetActorEnableCollision(false);
					
					else if (!VehicleConfig::noVehicleCollisions) Vehicle->SetActorEnableCollision(true);
				}
			}
		}
	}
}

void Vehicle::Tick() {
	VehicleExploits();
}