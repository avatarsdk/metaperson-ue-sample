// Copyright Epic Games, Inc. All Rights Reserved.

#include "Metaperson2GameMode.h"
#include "Metaperson2Character.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/HUD.h"

AMetaperson2GameMode::AMetaperson2GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	static ConstructorHelpers::FClassFinder<AHUD> HUDBpClass(TEXT("/Game/ThirdPerson/Blueprints/BP_HUD")); 
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
		HUDClass = HUDBpClass.Class;
	}
}
