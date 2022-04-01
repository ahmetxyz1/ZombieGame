//Copyright 2021, Dakota Dawe, All rights reserved
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SceneManagement.h"
#include "FPSTemplateProjectile.generated.h"

class AWindDirectionalSource;
class UProjectileMovementComponent;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplateProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplateProjectile();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FPSTemplate")
	class UStaticMeshComponent* Mesh;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate")
	class USphereComponent* CollisionComponent;
	
	UPROPERTY(EditAnywhere, Category = "FPSTemplate | Debug")
	bool DrawDebugBoxOnImpact;
	UPROPERTY(EditAnywhere, Category = "FPSTemplate | Debug")
	float DebugBoxSize;

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Physics")
	class UCurveFloat* DragCurve;//The drag curve used for this projectile (air resistance)
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Physics")
	bool AffectedByWind;//Whether or not this bullet is affected by wind
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Physics")
	AWindDirectionalSource* WindSource;
	FWindData WindData;//store our wind data to allow for wind to push projectile on tick

	FVector LastPosition;

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	float VelocityFPS;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	uint16 BulletWeightGrains;

	class UProjectileMovementComponent* ProjectileMovementComponent;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	float CalculateDrag();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Events")
	void OnProjectileFired(const class AFPSTemplateFirearm* Firearm);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Events")
	void OnProjectileImpact(const FHitResult& HitResult);

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	UProjectileMovementComponent* GetProjectileMovement() const {return ProjectileMovementComponent;}
	//Get the wind source that is affecting our projectile
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	AWindDirectionalSource* GetWindSource() const {return WindSource;}

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Ammo")
	float CalculateImpactForce() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Ammo")
	float GetProjectilePower() const { return FMath::Sqrt(VelocityFPS * BulletWeightGrains); }
};
