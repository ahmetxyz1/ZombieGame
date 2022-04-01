//Copyright 2021, Dakota Dawe, All rights reserved
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FPSTemplateDataTypes.h"

#include "FPSTemplateAnimInstance.generated.h"

class AFPSTemplateFirearm;
class UFPSTemplate_CharacterComponent;
class UAnimSequence;

UCLASS()
class ULTIMATEFPSTEMPLATE_API UFPSTemplateAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
		
public:
	UFPSTemplateAnimInstance();

	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Default")
	UFPSTemplate_CharacterComponent* CharacterComponent;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Firearm")
	int32 AnimationIndex;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Firearm")
	AActor* AimingActor;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Firearm")
	AFPSTemplateFirearm* Firearm;
	AFPSTemplateFirearm* OldFirearm;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPSTemplate | Default")
	class UAnimSequence* LeftHandPose;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Default")
	bool bValidLeftHandPose;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Firearm")
	float MakeLeftHandFollowAlpha;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Toggles")
	bool bUseProceduralSpine;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Toggles", meta = (EditCondition = "bUseProceduralSpine"))
	int32 SpineBoneCount;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	float AimInterpolationSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	float CycleSightsInterpolationSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	float RotationLagResetInterpolationSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	float MotionLagResetInterpolationSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FName RightHandBone;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FTransform RelativeToHandTransform;
	FTransform FinalRelativeHand;
	bool bInterpRelativeToHand;

	bool bFirstRun;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FTransform SightTransform;
	float SightDistance;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	float AimingAlpha;
	bool bIsAiming;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FRotator HeadAimingRotation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sway")
	bool bInvertRotationLag;

	void SetSightTransform();
	void SetRelativeToHand();
	void InterpRelativeToHand(float DeltaSeconds);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | LeftHandIK")
	FTransform LeftHandIKTransform;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | LeftHandIK")
	float LeftHandIKAlpha;
	void SetLeftHandIK();

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Actions")
	float RotationAlpha;
	
	bool bInterpAiming;
	bool bInterpCameraZoom;
	void InterpCameraZoom(float DeltaSeconds);
	float DefaultCameraFOV;

	void InterpAimingAlpha(float DeltaSeconds);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Lag")
	FTransform RotationLagTransform;
	FTransform UnmodifiedRotationLagTransform;
	FRotator OldRotation;
	void SetRotationLag(float DeltaSeconds);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Lag")
	FTransform MovementLagTransform;
	FTransform UnmodifiedMovementLagTransform;
	FVector OldMovement;
	void SetMovementLag(float DeltaSeconds);
	FVector OldCharacterLocation;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FFirearmStats FirearmStats;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FRotator SpineRotation;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Recoil")
	FTransform RecoilTransform;
	FTransform FinalRecoilTransform;

	void RecoilInterpToZero(float DeltaSeconds);
	void RecoilInterpTo(float DeltaSeconds);

	ELeaning CurrentLean;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FRotator LeanRotation;
	bool bInterpLeaning;
	void InterpLeaning(float DeltaSeconds);
	
	float CurveTimer;
	float VelocityMultiplier;
	void HandleMovementSway(float DeltaSeconds);
	void HandleSprinting();
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FTransform SwayTransform;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FTransform SprintPoseTransform;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float SprintAlpha;

	float SwayMultiplier;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float CharacterVelocity;

	bool bCustomizingFirearm;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FTransform WeaponCustomizingTransform;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float WeaponCustomizingAlpha;

	bool bCanAim;

	bool bInterpPortPose;
	EPortPose PortPose;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float PortPoseAlpha;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Poses")
	FTransform CurrentPose;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Poses")
	FVector ShortStockPose;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Poses")
	float ShortStockPoseAlpha;

	FTransform DefaultRelativeToHand;

	bool bFreeLook;

	void InterpPortPose(float DeltaSeconds);
	FRotator OldFreeLookRotation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FRotator FreeLookRotation;

	FRotator FreeLookReleaseRotation;

	FRotator SpineToInterpTo;
	
public:
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Aiming")
	void SetIsAiming(bool IsAiming);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Aiming")
	void CycledSights();

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void SetLeaning(ELeaning Lean);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void SetAnimationIndex(int32 NewAnimationIndex) { AnimationIndex = NewAnimationIndex; }
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Actions")
	void SetIsReloading(bool IsReloading, float BlendAlpha = 0.35f);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Actions")
	void SetCanAim(bool CanAim);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Actions")
	void PerformRecoil(float Multiplier = 1.0f);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Actions")
	void ChangingFireMode(bool IsChanging);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Actions")
	void ChamberingRound(bool IsChambering);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Actions")
	void EnableLeftHandIK(bool Enable);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Actions")
	void SetLeftHandFollow(bool bMakeFollow) { MakeLeftHandFollowAlpha = bMakeFollow; }
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Default")
	void StopMontages(float BlendOutTime);
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Extra")
	void SetCustomization(bool CustomizeFirearm);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Extra")
	void SetPortPose(EPortPose Pose);
	void SetPortPoseBlend(EPortPose Pose, float Alpha);
	bool HandleFirearmCollision(EPortPose Pose, float Alpha);

	void SetCharacterComponent(UFPSTemplate_CharacterComponent* INCharacterComponent) { CharacterComponent = INCharacterComponent;}
};
