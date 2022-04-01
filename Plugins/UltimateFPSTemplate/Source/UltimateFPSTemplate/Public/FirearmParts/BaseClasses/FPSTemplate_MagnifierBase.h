// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_SightMagnifiedRTBase.h"
#include "FPSTemplate_MagnifierBase.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_MagnifierBase : public AFPSTemplate_SightMagnifiedRTBase
{
	GENERATED_BODY()
public:
	AFPSTemplate_MagnifierBase();

protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_FlippedOut, Category = "FPSTemplate | Magnifier")
	bool bFlippedOut;
	UFUNCTION()
	void OnRep_FlippedOut();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_Flip(bool bFlip);
	
public:
	virtual void Use_Implementation() override;
};
