// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FPSTemplateDataTypes.h"
#include "FPSTemplate_HoleComponent.generated.h"

class UMaterialInstanceDynamic;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULTIMATEFPSTEMPLATE_API UFPSTemplate_HoleComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFPSTemplate_HoleComponent();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TArray<FHoleMaterialSetting> MaterialSettings;

	TArray<FHoleMaterial> HoleMaterials;
	
	class UMaterialInstanceDynamic* MaterialInstance;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	TArray<FVector> HoleLocations;
	uint8 HoleIndex;
	
	int32 GetFaceIndex(const FHitResult& HitLocation);

	UFUNCTION()
	void SetupMaterials();
public:
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Hole")
	void AddHole(const FHitResult& HitResult);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Hole")
	bool DidImpactHitHole(const FHitResult& HitResult, float Tolerance = 1.0f);
};
