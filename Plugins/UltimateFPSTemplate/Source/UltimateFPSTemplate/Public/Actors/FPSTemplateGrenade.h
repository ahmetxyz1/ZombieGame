// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "FPSTemplateGrenade.generated.h"

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplateGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplateGrenade();

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "FPSTemplate")
	class UCapsuleComponent* CollisionComponent;

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	int32 MaxBounces;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	float FuseTime;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	bool bExplodeOnImpact;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	float ArmingTime;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	float DudDestroyTime;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	float ServerSyncIntervalPerSecond;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TEnumAsByte<ECollisionChannel> PoseCollision;
	
	FTimerHandle TFuse;
	FTimerHandle TArmTime;
	FTimerHandle TSync;
	uint8 CurrentBounces;

	bool bIsArmed;
	bool bIsVisibleGrenade;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void SuperExplode();
	
	UFUNCTION(BlueprintNativeEvent, Category = "FPSTemplate | Grenade")
	void Explode();
	virtual void Explode_Implementation();
	UFUNCTION(BlueprintNativeEvent, Category = "FPSTemplate | Grenade")
	void Impact(float Velocity);
	virtual void Impact_Implementation(float Velocity);

	void EnablePhysics();
	void DisablePhysics();
	
	void ArmGrenade();

	UFUNCTION(NetMulticast, Unreliable)
	void Multi_UpdateLocation(FVector NewLocation, FRotator NewRotation);
	UFUNCTION(NetMulticast, Unreliable)
	void Multi_ReleaseGrenade(FVector Orientation, float Velocity);

	AFPSTemplateGrenade* ClientGrenade;
	bool bClientGrenadeInterp;
	bool bSyncGrenadeInterp;
	
	void SyncLocation();

	FVector InterpToLocation;
	FRotator InterpToRotation;
	
public:
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Grenade")
	void ReleaseGrenade(FVector Orientation, float Velocity, bool IsClientGrenade = false);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Grenade")
	void CookGrenade();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Grenade")
	void InterpToNewLocation();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Grenade")
	void SetClientGrenade(AFPSTemplateGrenade* Grenade);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | SmokeGrenade")
	FVector GetSmokeGrenadeParticleLocation(float SpriteRadius);
	

	void StartFuse(float Time);
};