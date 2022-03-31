// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZombieGameGameMode.h"
#include "ZombieGameHUD.h"
#include "ZombieGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

AZombieGameGameMode::AZombieGameGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AZombieGameHUD::StaticClass();
}
