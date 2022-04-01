// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "FPSTemplate_LightLaserBase.generated.h"

class USpotLightComponent;
class UDecalComponent;
class UStaticMeshComponent;
class UMaterialInstance;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_LightLaserBase : public AFPSTemplate_PartBase
{
	GENERATED_BODY()
	
public:
	AFPSTemplate_LightLaserBase();

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "FPSTemplate | Default")
	USceneComponent* Root;
	UPROPERTY(VisibleDefaultsOnly, Category = "FPSTemplate | Default")
	UStaticMeshComponent* LaserMesh;
	UPROPERTY(VisibleDefaultsOnly, Category = "FPSTemplate | Default")
	USpotLightComponent* SpotLightComponent;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	UDecalComponent* LaserDecalComponent;

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	bool bIsLight;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	bool bIsLaser;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TEnumAsByte<ECollisionChannel> CollisionChannel;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	float MaxLaserDistance;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TArray<UMaterialInstance*> LaserMaterials;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TArray<UMaterialInstance*> LaserDotMaterials;
	
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Sockets")
	FName LaserSocket;
	
	UPROPERTY(ReplicatedUsing = OnRep_LightOn)
	bool bLightOn;
	UFUNCTION()
	void OnRep_LightOn();
	UPROPERTY(ReplicatedUsing = OnRep_LaserOn)
    bool bLaserOn;
	UFUNCTION()
	void OnRep_LaserOn();

	UPROPERTY(ReplicatedUsing = OnRep_LaserColorIndex)
	uint8 LaserColorIndex;
	UFUNCTION()
	void OnRep_LaserColorIndex();

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_ToggleLight();
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_ToggleLaser();
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_CycleLaserColor(uint8 Index);

	virtual void PostInitProperties() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void SetupPartMesh() override;
	
public:
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Toggle")
	void ToggleLight();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Toggle")
	void ToggleLaser();

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Toggle")
	void CycleLaserColor();

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Toggle")
	void SetLaserColor(uint8 Index);
};
