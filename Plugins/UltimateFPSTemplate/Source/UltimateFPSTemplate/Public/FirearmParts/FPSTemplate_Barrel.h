// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "FPSTemplate_Barrel.generated.h"

class AFPSTemplateProjectile;
class AFPSTemplate_Muzzle;
class AFPSTemplateFirearm;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_Barrel : public AFPSTemplate_PartBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplate_Barrel();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Sockets")
	FName MuzzleSocket;

	UPROPERTY(Replicated)
	AFPSTemplate_Muzzle* MuzzleDevice;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Ammo")
	FTransform GetMuzzleSocketTransform() const;

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	bool DoesMuzzleSocketExist() const;

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	AFPSTemplate_Muzzle* GetMuzzleDevice();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	AActor* GetMuzzleDeviceActor();

	virtual void CacheParts() override;
};