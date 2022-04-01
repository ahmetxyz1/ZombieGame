//Copyright 2021, Dakota Dawe, All rights reserved
#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "FPSTemplateDataTypes.h"
#include "Camera/CameraShakeBase.h"
#include "FPSTemplateProjectile.h"
#include "Interfaces/FPSTemplate_AimInterface.h"
#include "FPSTemplateFirearm.generated.h"

class UFPSTemplate_CharacterComponent;
class AFPSTemplate_Muzzle;
class UFPSTemplate_PartComponent;
class UAnimationAsset;
class UAnimMontage;
class UCurveVector;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplateFirearm : public AActor, public IFPSTemplate_AimInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplateFirearm();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CharacterComponent)
	UFPSTemplate_CharacterComponent* CharacterComponent;
	UFUNCTION()
	void OnRep_CharacterComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FPSTemplate")
	USkeletalMeshComponent* FirearmMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	int32 FirearmIndex;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	FFirearmStats DefaultFirearmStats;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	TArray<EFirearmFireMode> FireModes;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	float FireRateRPM;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	int32 BurstCount;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	bool bUseLeftHandIK;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TEnumAsByte<ECollisionChannel> PoseCollision;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float CameraFOVZoom;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float CameraFOVZoomSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Fire")
	float TimerAutoFireRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName FirearmGripSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName MuzzleSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName AimSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName LeftHandIKSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FVector ShoulderStockOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FRotator HeadAimRotation;
	
	UPROPERTY(ReplicatedUsing = OnRep_FireMode, BlueprintReadOnly, Category = "FPSTemplate | Firearm")
	EFirearmFireMode FireMode;
	int32 FireModeIndex;
	FTimerHandle TFAHandle;
	uint8 BurstFireCount;
	UFUNCTION()
	void OnRep_FireMode();
	
	UPROPERTY(ReplicatedUsing = OnRep_FirearmHidden, BlueprintReadOnly, Category = "FPSTemplate | Firearm")
	bool bFirearmHidden;
	UFUNCTION()
	void OnRep_FirearmHidden();

	UPROPERTY(ReplicatedUsing = OnRep_AttachedToSocket)
	FName AttachedToSocket;
	UFUNCTION()
	void OnRep_AttachedToSocket();

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	float ShortStockPoseDistance;

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform FirstPersonSprintPose;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform FirstPersonHighPortPose;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform FirstPersonLowPortPose;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform ThirdPersonSprintPose;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform ThirdPersonHighPortPose;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform ThirdPersonLowPortPose;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	UCurveVector* MovementSwayCurve;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	bool ControlMovementSwayByStats;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	bool ScaleCameraShakeByStats;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	TSubclassOf<UCameraShakeBase> FireCameraShake;

	UPROPERTY(Replicated)
	FFirearmStats FirearmStats;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "FPSTemplate | PartComponents")
	TArray<UFPSTemplate_PartComponent*> PartComponents;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "FPSTemplate | PartComponents")
	TArray<UFPSTemplate_PartComponent*> SightComponents;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Animation")
	float DefaultAimSwayMultiplier;
	float AimSwayMultiplier;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentSightComponent)
	UFPSTemplate_PartComponent* CurrentSightComponent;
	UFUNCTION()
	void OnRep_CurrentSightComponent();
	int32 SightComponentIndex;
	
	UPROPERTY(Replicated)
	UFPSTemplate_PartComponent* BarrelComponent;
	UPROPERTY(Replicated)
	UFPSTemplate_PartComponent* HandguardComponent;
	UPROPERTY(Replicated)
	UFPSTemplate_PartComponent* StockComponent;
	UPROPERTY(Replicated)
	TArray<UFPSTemplate_PartComponent*> MagnifierComponents;
	UPROPERTY(Replicated)
	TArray<UFPSTemplate_PartComponent*> LightsLaserComponents;
	UPROPERTY(Replicated)
	TArray<UFPSTemplate_PartComponent*> ComponentsWithRenderTargets;
	// FOR COMPLETE FIREARMS (NO PART BUILDS)
	UPROPERTY(Replicated)
	UFPSTemplate_PartComponent* ForwardGripComponent;
	UPROPERTY(Replicated)
	UFPSTemplate_PartComponent* MuzzleComponent;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void OnRep_Owner() override;
	virtual void OnRep_AttachmentReplication() override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_CycleSights(UFPSTemplate_PartComponent* SightComponent);

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void PerformProceduralRecoil(float Multiplier = 1.0f, bool PlayCameraShake = true);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetFireMode(EFirearmFireMode NewFireMode);

	//UFPSTemplate_PartComponent* GetSightComponent();

	void HandleSightComponents();

	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Events")
	void OnFireModeChanged();
	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Events")
	void OnPartsChanged();
	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Events")
	void OnAttachedToSocket(const FName& Socket);

	bool bCanFire;
	bool bCanReFire;
	float TimeSinceLastShot;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "FPSTemplate | Ammo")
	TSubclassOf<AFPSTemplateProjectile> ProjectileToSpawn;

	bool bCanCycleSights;

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetHidden(bool Hide);
	
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_AttachToSocket(const FName& Socket);
	
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FPSTemplate | Implementable")
	void Fire();
	virtual void Fire_Implementation() { UE_LOG(LogTemp, Warning, TEXT("Fire Implementation Example")); }
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FPSTemplate | Implementable")
	void StopFire();
	virtual void StopFire_Implementation() { UE_LOG(LogTemp, Warning, TEXT("StopFire Implementation Example")); }
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FPSTemplate | Implementable")
	void Reload();
	virtual void Reload_Implementation() { UE_LOG(LogTemp, Warning, TEXT("Reload Implementation Example")); }
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FPSTemplate | Implementable")
	void ReloadDropMagazine();
	virtual void ReloadDropMagazine_Implementation() { UE_LOG(LogTemp, Warning, TEXT("ReloadDropMagazine Implementation Example")); }
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	virtual void CycleFireMode();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Firearm")
	bool IsFirearmDisabled() const { return bCanFire; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Firearm")
	FHitResult MuzzleTrace(float Distance, ECollisionChannel CollisionChannel, bool& bMadeBlockingHit, bool bDrawDebugLine);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Firearm")
	TArray<FHitResult> MuzzleTraceMulti(float Distance, ECollisionChannel CollisionChannel, bool& bMadeBlockingHit, bool bDrawDebugLine);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void HideFirearm(bool Hide);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void AttachToSocket(const FName Socket);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void DestroyAllParts();

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	AFPSTemplate_SightBase* GetCurrentSight() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	FVector& GetStockOffset() { return ShoulderStockOffset; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Attachments")
	TArray<UFPSTemplate_PartComponent*> GetPartComponents() const;

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Projectile")
	FProjectileTransform GetMuzzleSocketProjectileTransform();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Projectile")
	FTransform GetMuzzleSocketTransform();
	// MOA = Minute of angle. 1 MOA = 1 inch of shift at 100 yards
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Projectile")
	FProjectileTransform GetMuzzleProjectileSocketTransform(float RangeMeters, float MOA = 1.0f);
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void EnableSightCycling() { bCanCycleSights = true; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void DisableSightCycling() { bCanCycleSights = false; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	virtual FTransform GetLeftHandIKTransform();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void CycleSights();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void RefreshCurrentSight();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	UCurveVector* GetMovementSwayCurve() const { return MovementSwayCurve; }
	bool UseStatsForMovementSway() const { return ControlMovementSwayByStats; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	float GetStockLengthOfPull();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	float GetAimSwayMultiplier() const { return AimSwayMultiplier; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void SetAimSwayMultiplier(float NewMultiplier) { NewMultiplier >= 0.0f ? AimSwayMultiplier = NewMultiplier : AimSwayMultiplier = DefaultAimSwayMultiplier; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void ResetAimSwayMultiplier() { AimSwayMultiplier = DefaultAimSwayMultiplier; }
	
	virtual FTransform GetSightSocketTransform() override;
	virtual FTransform GetDefaultSightSocketTransform() override;
	virtual bool IsFirearm() const override { return true; }
	virtual int32 GetAnimationIndex() const { return FirearmIndex; }
	virtual float GetInterpolationMultiplier() override;
	virtual AFPSTemplate_SightBase* GetActiveSight() override { return GetCurrentSight(); }
	virtual void DisableRenderTargets(bool Disable) override { DisableAllRenderTargets(Disable); }
	virtual FName GetGripSocketName() const override { return GetFirearmGripSocket(); }
	virtual FRotator GetHeadRotation() const override { return HeadAimRotation; }

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Stats")
	void UpdateFirearmStats();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Stats")
	FFirearmStats& GetFirearmStats() { return FirearmStats; }

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	int32 GetFirearmIndex() const { return FirearmIndex; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	USkeletalMeshComponent* GetMesh() const { return FirearmMesh; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	UFPSTemplate_CharacterComponent* GetCharacterComponent();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	EFirearmFireMode GetFireMode() const { return FireMode; };
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	bool IsSuppressed() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	virtual float GetCameraFOVZoom() const override;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	virtual float GetCameraFOVZoomSpeed() const override;

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	void CycleMagnifier();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	void UseLightLaser(ELightLaser Toggle);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	void CycleLaserColor();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	void CycleReticle();
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	void IncreaseReticleBrightness(bool Increase);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	float GetOpticMagnification() const;
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	void IncreaseVerticalZero(bool Increase);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	AFPSTemplate_Muzzle* GetMuzzleDevice() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	AActor* GetMuzzleActor() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	TArray<AFPSTemplate_PartBase*> GetFirearmParts() const;

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Ammo")
	TSubclassOf<AFPSTemplateProjectile>& GetInUseProjectile() { return ProjectileToSpawn; }
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Optics")
	void DisableAllRenderTargets(bool Disable);
	
	void PartsChanged();

	FTransform& GetSprintPose();
	FTransform& GetHighPortPose();
	FTransform& GetLowPortPose();
	FVector GetShortStockPose() const;
	float GetShortStockDistance() const { return ShortStockPoseDistance; }
	virtual void ZoomOptic(bool bZoom) override;
	virtual float GetCurrentMagnification() const override { return GetOpticMagnification(); }

	bool UseLeftHandIK() const { return bUseLeftHandIK; }

	const FName& GetAimSocket() const { return AimSocket; }

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Init")
	void SetCharacterComponent(UFPSTemplate_CharacterComponent* INCharacterComponent);

	void AddPartCache(UFPSTemplate_PartComponent* PartComponent);
	
	class UAnimSequence* GetGripAnimation()const;

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Sockets")
	const FName& GetFirearmGripSocket() const { return FirearmGripSocket; }
};