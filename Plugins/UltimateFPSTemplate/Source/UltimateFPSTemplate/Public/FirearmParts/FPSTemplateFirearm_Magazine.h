//Copyright 2021, Dakota Dawe, All rights reserved
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSTemplateDataTypes.h"
#include "Actors/FPSTemplateProjectile.h"
#include "FPSTemplateFirearm_Magazine.generated.h"

//class AFPSTemplateProjectile;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplateFirearm_Magazine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplateFirearm_Magazine();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FPSTemplate")
	class UStaticMeshComponent* MagazineMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	TSubclassOf<AFPSTemplateProjectile> ProjectileClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	int32 MagazineCapacity;

	UPROPERTY(Replicated)
	int32 CurrentAmmo;

	UPROPERTY(ReplicatedUsing = OnRep_MagazineState)
	EMagazineState MagazineState;
	UFUNCTION()
	void OnRep_MagazineState();

	bool bIsDropped;

	void SimulatePhysics(bool Simulate);
	void SetVisibility(bool Visible);
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;

public:
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Ammo")
	TSubclassOf<AFPSTemplateProjectile> ConsumeProjectile();
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Ammo")
	TSubclassOf<AFPSTemplateProjectile> GetProjectileClass() const { return ProjectileClass; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Ammo")
	bool HasAmmo() const { return CurrentAmmo > 0; };
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Ammo")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Physics")
	void DropMagazine();

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | State")
	void SetMagazineState(EMagazineState NewMagazineState);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | State")
	EMagazineState GetMagazineState() const { return MagazineState; }
};
