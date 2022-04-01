// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FPSTemplateDataTypes.h"
#include "Actors/FPSTemplateFirearm.h"
#include "FPSTemplate_CharacterComponent.generated.h"

class UFPSTemplateAnimInstance;
class USkeletalMeshComponent;
class UCameraComponent;
class AFPSTemplateFirearm;
class AFPSTemplate_RangeFinder;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULTIMATEFPSTEMPLATE_API UFPSTemplate_CharacterComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFPSTemplate_CharacterComponent();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Readables")
	UFPSTemplateAnimInstance* AnimationInstance;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Readables")
	UCameraComponent* FPCameraComponent;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Readables")
	USkeletalMeshComponent* FPMesh;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Readables")
	USkeletalMeshComponent* TPMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	bool bUseParentSocketForAiming;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	FName CameraSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	FName CameraSocketParentBone;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float SprintSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float MaxLookUpAngle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float MaxLookDownAngle;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate")
	float DefaultLeanAngle;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate")
	int32 MaxFirearmAttachmentAttempts;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate")
	float FirearmReAttachmentAttemptInterval;
	uint8 AttachmentAttempt;

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | FirearmCollision")
	bool bUseFirearmCollision;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | FirearmCollision", meta = (EditCondition = "bUseFirearmCollision"))
	bool bLimitFirearmCollisionTickRate;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | FirearmCollision", meta = (EditCondition = "bUseFirearmCollision && bLimitFirearmCollisionTickRate"))
	int32 FirearmCollisionTickRate;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | FirearmCollision")
	float FirearmCollisionDistanceCheck;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | FirearmCollision")
	TEnumAsByte<ECollisionChannel> FirearmCollisionChannel;
	bool bFirearmCollisionHitting;

	bool bHighPort;
	bool bLowPort;
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetPortPose(EPortPose NewPortPose);
	UPROPERTY(ReplicatedUsing = OnRep_PortPose)
	EPortPose PortPose;
	UFUNCTION()
	void OnRep_PortPose();
	
	UPROPERTY(Replicated)
	bool bIsSprinting;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetSprinting(bool IsSprinting);
	
	UPROPERTY(ReplicatedUsing = OnRep_IsAiming)
	bool bIsAiming;
	UFUNCTION()
	void OnRep_IsAiming();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentFirearm)
	AFPSTemplateFirearm* CurrentFirearm;
	UFUNCTION()
	void OnRep_CurrentFirearm();

	AFPSTemplateFirearm* TempFirearm;

	UPROPERTY(ReplicatedUsing = OnRep_AimingActor)
	AActor* AimingActor;
	UFUNCTION()
	void OnRep_AimingActor();

	UPROPERTY(ReplicatedUsing = OnRep_Lean)
	ELeaning CurrentLean;
	UFUNCTION()
	void OnRep_Lean();
	UPROPERTY(ReplicatedUsing = OnRep_Lean)
	float LeanAngle;
	bool bLeanLeftDown;
	bool bLeanRightDown;

	UPROPERTY(ReplicatedUsing = OnRep_LeftHandFollowIK)
	bool bLeftHandFollowIK;
	UFUNCTION()
	void OnRep_LeftHandFollowIK();
	UPROPERTY(ReplicatedUsing = OnRep_UseLeftHandIK)
	bool bUseLeftHandIK;
	UFUNCTION()
	void OnRep_UseLeftHandIK();
	
	APawn* OwningPawn;
	bool bIsInitialized;
	bool bLocallyControlled;
	float DefaultCameraFOV;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	const bool HasAuthority() { return GetOwner() ? GetOwner()->HasAuthority() : false; }
	
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetAiming(bool IsAiming);
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetLean(ELeaning Lean);
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SwitchFirearm(AFPSTemplateFirearm* Firearm, const FName& Socket, bool HideCurrentFirearm);
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_OnUnEquipCurrentFirearm();
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetLeanAngle(float NewLeanAngle);
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetLeftHandFollowIK(bool bFollow);
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetUseLeftHandIK(bool bUse);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | CharacterComponent")
	void Init(UCameraComponent* CameraComponent, USkeletalMeshComponent* FirstPersonMesh, USkeletalMeshComponent* ThirdPersonMesh);

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	USkeletalMeshComponent* GetInUseMesh();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	UFPSTemplateAnimInstance* GetAnimationInstance()const  { return AnimationInstance; }
	
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	UPawnMovementComponent* GetMovementComponent()const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	bool IsLocallyControlled()const { return bLocallyControlled; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	FRotator GetBaseAimRotation()const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	FRotator GetControlRotation()const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	const FName& GetCameraSocket()const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	UCameraComponent* GetCameraComponent()const { return FPCameraComponent; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	float GetLeanAngle()const { return LeanAngle; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void SetLeanAngle(float NewLeanAngle);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void ResetLeanAngle();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	float GetSprintSpeed()const { return SprintSpeed; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	bool GetIsAiming()const { return bIsAiming; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	FVector GetActorForwardVector()const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	FVector GetActorRightVector()const;
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void RagdollCharacter();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void RagdollCharacterWithForce(const FVector ImpactLocation, const float ImpactForce = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void StartAiming();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void StopAiming();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void SetSprinting(bool Sprinting);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Procedural")
	bool GetIsSprinting()const { return bIsSprinting; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void LeanLeft();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void LeanRight();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void StopLeanLeft();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void StopLeanRight();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	bool IsUsingFirearmCollision() const { return bUseFirearmCollision; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void SetFirearmCollision(bool EnableFirearmCollision) { bUseFirearmCollision = EnableFirearmCollision; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void SetAimingActor(AActor* Actor) { AimingActor = Actor; OnRep_AimingActor(); }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void SetLeftHandFollowIK(bool bFollow);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void SetUseLeftHandIK(bool bUse);

	float GetDefaultFOV()const { return DefaultCameraFOV; }

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void PlayCameraShake(TSubclassOf<UCameraShakeBase> CameraShake, float Scale = 1.0f)const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Firearm")
	AFPSTemplateFirearm* GetFirearm() const { return CurrentFirearm; }
	template <class T>
	T* GetFirearm() const { return Cast<T>(CurrentFirearm); }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Firearm")
	AActor* GetAimingActor() const { return CurrentFirearm ? Cast<AActor>(CurrentFirearm) : AimingActor; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	AFPSTemplateFirearm* AddFirearm(TSubclassOf<AFPSTemplateFirearm> FirearmClass, bool SetAsCurrent = true, const FName AttachToSocketName = NAME_None);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void AddExistingFirearm(AFPSTemplateFirearm* Firearm, bool SetAsCurrent = true, const FName AttachToSocketName = NAME_None);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	float GetMagnificationSensitivity() const;
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	float GetMagnificationSensitivityStartValue(float StartAtMagnification = 4.0f) const;
	UFUNCTION()
	void AttachItem(AActor* Actor, const FName& SocketName);
	// Returns Previously Held Firearm
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	AFPSTemplateFirearm* SwitchToFirearm(AFPSTemplateFirearm* Firearm, const FName FirearmAttachSocket = NAME_None, bool HideCurrentFirearm = true);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void OnUnEquipCurrentFirearm();
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | HighLowPort")
	void SetHighPortPose();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | HighLowPort")
	void SetLowPortPose();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | HighLowPort")
	void StopLowPortPose();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | HighLowPort")
	void StopHighPortPose();
};
