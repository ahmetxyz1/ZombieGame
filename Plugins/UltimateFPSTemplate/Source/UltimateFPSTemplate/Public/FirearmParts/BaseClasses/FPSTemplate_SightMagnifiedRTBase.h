// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"

#include "FirearmParts/BaseClasses/FPSTemplate_SightBase.h"
#include "Interfaces/FPSTemplate_AimInterface.h"
#include "FPSTemplate_SightMagnifiedRTBase.generated.h"

class UMaterialInterface;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_SightMagnifiedRTBase : public AFPSTemplate_SightBase, public IFPSTemplate_AimInterface
{
	GENERATED_BODY()

public:
	AFPSTemplate_SightMagnifiedRTBase();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	class USceneComponent* RootSceneComponent;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	class USceneCaptureComponent2D* SceneCapture;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	float ScopeRefreshRate;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TArray<float> Magnifications;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	bool bIsFirstFocalPlane;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	bool bDisableWhenNotAiming;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default", meta = (EditCondition = "!bDisableWhenNotAiming"))
	float NotAimingRefreshRate;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default", meta = (EditCondition = "bDisableWhenNotAiming"))
	bool bClearScopeWithColor;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default", meta = (EditCondition = "bDisableWhenNotAiming && bClearScopeWithColor"))
	FLinearColor ScopeClearedColor;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default", meta = (EditCondition = "bDisableWhenNotAiming && !bClearScopeWithColor"))
	bool bClearScopeWithMaterial;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default", meta = (EditCondition = "bDisableWhenNotAiming && !bClearScopeWithColor && bClearScopeWithMaterial"))
	UMaterialInterface* ClearedScopeMaterial;
	UMaterialInterface* OriginalScopeMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Aim")
	int32 AnimationIndex;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Aim")
	float AimInterpolationMultiplier;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Aim")
	FName GripSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aim")
	FRotator HeadAimRotation;
	
	uint8 MagnificationIndex;
	float CurrentMagnification;

protected:
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void Tick(float DeltaSeconds) override;
	
	void SetFOVAngle();

	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Events")
	void OpticZoomedIn(float OldMagnification);

	void HandleFirstFocalPlaneZoom(float MagnificationDifference);
	void HandleRenderTarget();

public:
	virtual float GetMagnification() const override { return CurrentMagnification; }
	virtual void ZoomIn() override;
	virtual void ZoomOut() override;
	virtual void IncreaseVerticalZero() override;
	virtual void DecreaseVerticalZero() override;
	virtual void DisableRenderTarget(bool Disable) override;
	
	virtual FTransform GetDefaultSightSocketTransform() override { return GetSightSocketTransform(); }
	virtual FTransform GetSightSocketTransform() override { return GetAimSocketTransform(); }
	virtual int32 GetAnimationIndex() const override { return AnimationIndex; }
	virtual float GetInterpolationMultiplier() override { return AimInterpolationMultiplier; }
	virtual AFPSTemplate_SightBase* GetActiveSight() override { return this; }
	virtual void DisableRenderTargets(bool Disable) override { DisableRenderTarget(Disable); }
	virtual FRotator GetHeadRotation() const override { return HeadAimRotation; }
	virtual FName GetGripSocketName() const override { return GripSocket; }
	virtual void ZoomOptic(bool bZoom) override;
	virtual float GetCurrentMagnification() const override { return GetMagnification(); }
	virtual float GetCameraFOVZoom() const { return CameraFOVZoom; }
	virtual float GetCameraFOVZoomSpeed() const { return CameraFOVZoomSpeed; }
};
