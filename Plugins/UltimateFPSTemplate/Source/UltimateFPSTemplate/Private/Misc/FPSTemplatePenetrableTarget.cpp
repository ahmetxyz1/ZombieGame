// Copyright 2021, Dakota Dawe, All rights reserved


#include "Misc/FPSTemplatePenetrableTarget.h"

// Sets default values
AFPSTemplatePenetrableTarget::AFPSTemplatePenetrableTarget()
{
	PrimaryActorTick.bCanEverTick = false;

	MaxHoleCount = 8;
	MaterialIndex = 0;
	MaterialInstance = nullptr;
}

// Called when the game starts or when spawned
void AFPSTemplatePenetrableTarget::BeginPlay()
{
	Super::BeginPlay();

	UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
	if (StaticMeshComponent)
	{
		MaterialInstance = StaticMeshComponent->CreateDynamicMaterialInstance(MaterialIndex);
		StaticMeshComponent->SetMaterial(MaterialIndex, MaterialInstance);
	}
	else if (USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(GetComponentByClass(USkeletalMeshComponent::StaticClass())))
	{
		MaterialInstance = SkeletalMeshComponent->CreateDynamicMaterialInstance(MaterialIndex);
		SkeletalMeshComponent->SetMaterial(MaterialIndex, MaterialInstance);
	}

	for (uint8 i = 0; i < MaxHoleCount; ++i)
	{
		HoleLocations.Add(FVector::ZeroVector);
	}
}

void AFPSTemplatePenetrableTarget::AddHole(FVector ImpactLocation)
{
	if (MaterialInstance)
	{
		FString ParamName = FString("Impact" + FString::FromInt(HoleIndex));
		MaterialInstance->SetVectorParameterValue(FName(ParamName), ImpactLocation);
		if (HoleIndex < HoleLocations.Num())
		{
			HoleLocations[HoleIndex] = ImpactLocation;
		}
		if (++HoleIndex > MaxHoleCount - 1)
		{
			HoleIndex = 0;
		}
	}
}

bool AFPSTemplatePenetrableTarget::DidImpactHitHole(FVector ImpactLocation, float Tolerance)
{
	for (FVector& HoleLocation : HoleLocations)
	{
		if (FVector::Distance(HoleLocation, ImpactLocation) <= Tolerance)
		{
			return true;
		}
	}
	return false;
}
