// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MapEditorCharacterMovement.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMATEFPSTEMPLATE_API UMapEditorCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:
	UMapEditorCharacterMovement(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	bool bInMovementMode;

	virtual void BeginPlay() override;
	
public:
	UFUNCTION(BlueprintCallable, Category = "MapEditor | PawnMovement")
	void MoveForward(float Value = 1.0f);
	UFUNCTION(BlueprintCallable, Category = "MapEditor | PawnMovement")
	void MoveRight(float Value = 1.0f);
	UFUNCTION(BlueprintCallable, Category = "MapEditor | PawnMovement")
	void MoveUp(float Value = 1.0f);
	
	UFUNCTION(BlueprintCallable, Category = "MapEditor | PawnMovement")
	void LookUp(float Value = 1.0f);
	UFUNCTION(BlueprintCallable, Category = "MapEditor | PawnMovement")
	void Turn(float Value = 1.0f);
	
	UFUNCTION(BlueprintCallable, Category = "MapEditor | PawnMovement")
	void EnterMovementMode(bool Enter);
	UFUNCTION(BlueprintPure, Category = "MapEditor | PawnMovement")
	bool InMovementMode() const { return bInMovementMode; }

	UFUNCTION(BlueprintCallable, Category = "MapEditor | Init")
	void Init();
};
