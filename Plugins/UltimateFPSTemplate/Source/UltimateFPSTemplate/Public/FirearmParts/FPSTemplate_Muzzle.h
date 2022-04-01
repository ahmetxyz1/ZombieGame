// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "FPSTemplate_Muzzle.generated.h"


UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_Muzzle : public AFPSTemplate_PartBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplate_Muzzle();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	bool bIsSuppressor;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName MuzzleSocket;

	UPROPERTY(Replicated)
	AFPSTemplate_Muzzle* MuzzleAttachment;
	
public:
	UFUNCTION(BlueprintCallable, Category = "SteelChallenge | Ammo")
	FTransform GetMuzzleSocketTransform();

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Default")
	bool DoesMuzzleSocketExist();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Default")
	bool IsSuppressor() const;
	UFUNCTION(BlueprintCallable, Category = "SteelChallenge | Ammo")
	AFPSTemplate_Muzzle* GetMuzzleAttachment() { return IsValid(MuzzleAttachment) ? MuzzleAttachment : this; }

	virtual void CacheParts() override;
};
