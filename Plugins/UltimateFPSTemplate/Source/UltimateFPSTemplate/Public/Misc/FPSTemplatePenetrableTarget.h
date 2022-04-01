// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSTemplateDataTypes.h"
#include "FPSTemplatePenetrableTarget.generated.h"

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplatePenetrableTarget : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplatePenetrableTarget();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	int32 MaxHoleCount;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	int32 MaterialIndex;

	class UMaterialInstanceDynamic* MaterialInstance;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	TArray<FVector> HoleLocations;
	uint8 HoleIndex;

public:
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Hole")
	void AddHole(FVector ImpactLocation);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Hole")
	bool DidImpactHitHole(FVector ImpactLocation, float Tolerance = 1.0f);
};
