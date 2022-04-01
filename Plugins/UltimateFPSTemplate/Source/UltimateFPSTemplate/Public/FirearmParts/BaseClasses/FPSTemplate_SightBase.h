// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "FPSTemplate_SightBase.generated.h"

class UMaterialInstance;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_SightBase : public AFPSTemplate_PartBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplate_SightBase();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float CameraFOVZoom;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float CameraFOVZoomSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float CameraDistance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	bool bUsedFixedCameraDistance;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TArray<FReticleMaterial> Reticles;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TArray<float> ReticleBrightnessSettings;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	int32 ReticleDefaultBrightnessIndex;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	int32 ReticleMaterialIndex;
	
	uint8 ReticleIndex;
	uint8 ReticleBrightnessIndex;
	
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	
public:
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Sockets")
	FName& GetAimSocket() { return AimSocket; }

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	float GetCameraFOVZoom() const { return CameraFOVZoom; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	float GetCameraFOVZoomSpeed() const { return CameraFOVZoomSpeed; }

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Magnification")
	virtual float GetMagnification() const { return 1.0f; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Magnification")
	virtual void ZoomIn() {};
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Magnification")
	virtual void ZoomOut() {};

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Reticle")
	virtual void CycleReticle();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Reticle")
	virtual void SetReticle(uint8 Index);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Reticle")
	void IncreaseBrightness();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Reticle")
	void DecreaseBrightness();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Reticle")
	void SetReticleBrightness(uint8 Index);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Reticle")
	virtual void IncreaseVerticalZero() {}
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Reticle")
	virtual void DecreaseVerticalZero() {}

	float GetCameraDistance() const { return CameraDistance; }
	bool UseFixedCameraDistance() const { return bUsedFixedCameraDistance; }
};
