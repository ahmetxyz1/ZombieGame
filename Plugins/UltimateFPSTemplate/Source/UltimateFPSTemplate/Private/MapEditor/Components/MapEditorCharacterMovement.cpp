// Copyright 2021, Dakota Dawe, All rights reserved


#include "MapEditor/Components/MapEditorCharacterMovement.h"

#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerController.h"

UMapEditorCharacterMovement::UMapEditorCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bInMovementMode = false;
	MovementMode = EMovementMode::MOVE_Flying;
	DefaultLandMovementMode = EMovementMode::MOVE_Flying;
	DefaultWaterMovementMode = EMovementMode::MOVE_Flying;
	BrakingFrictionFactor = 80.0f;
}

void UMapEditorCharacterMovement::BeginPlay()
{
	Super::BeginPlay();

	Init();
}

void UMapEditorCharacterMovement::Init()
{
	EnterMovementMode(bInMovementMode);
}

void UMapEditorCharacterMovement::EnterMovementMode(bool Enter)
{
	bInMovementMode = Enter;

	if (PawnOwner)
	{
		if (APlayerController* PC = PawnOwner->GetController<APlayerController>())
		{
			if (bInMovementMode)
			{
				PC->SetShowMouseCursor(false);
			}
			else
			{
				FVector2D CenterOfScreen = FVector2D(20, 20);
				if (GEngine && GEngine->GameViewport)
				{
					GEngine->GameViewport->GetViewportSize(CenterOfScreen);
					CenterOfScreen.X /= 2;
					CenterOfScreen.Y /= 2;
				}
				PC->SetShowMouseCursor(true);
				PC->SetMouseLocation(CenterOfScreen.X, CenterOfScreen.Y);
			}
		}
	}
}

void UMapEditorCharacterMovement::MoveForward(float Value)
{
	if (!bInMovementMode || Value == 0.0f) return;
	const FVector Movement = PawnOwner->GetActorForwardVector() * Value;
	//AddInputVector(Movement);
	PawnOwner->AddMovementInput(Movement);
}

void UMapEditorCharacterMovement::MoveRight(float Value)
{
	if (!bInMovementMode || Value == 0.0f) return;
	const FVector Movement = PawnOwner->GetActorRightVector() * Value;
	AddInputVector(Movement);
}

void UMapEditorCharacterMovement::MoveUp(float Value)
{
	if (!bInMovementMode || Value == 0.0f) return;
	FVector CurrentLocation = GetActorLocation();
	CurrentLocation.Z += 10.0f * Value;
	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CurrentLocation);
	const FVector Movement = UKismetMathLibrary::GetForwardVector(LookAtRotation);
	AddInputVector(Movement);
}

void UMapEditorCharacterMovement::LookUp(float Value)
{
	if (!bInMovementMode || !PawnOwner || Value == 0.0f) return;
	PawnOwner->AddControllerPitchInput(Value);
}

void UMapEditorCharacterMovement::Turn(float Value)
{
	if (!bInMovementMode || !PawnOwner || Value == 0.0f) return;
	PawnOwner->AddControllerYawInput(Value);
}