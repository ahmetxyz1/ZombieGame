// Copyright 2021, Dakota Dawe, All rights reserved


#include "Components/FPSTemplate_HoleComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values for this component's properties
UFPSTemplate_HoleComponent::UFPSTemplate_HoleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaterialInstance = nullptr;
}

// Called when the game starts or when spawned
void UFPSTemplate_HoleComponent::BeginPlay()
{
	Super::BeginPlay();
	
	SetupMaterials();
}

void UFPSTemplate_HoleComponent::SetupMaterials()
{
	if (GetOwner())
	{
		HoleMaterials.Reserve(MaterialSettings.Num() - 1);
		UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(GetOwner()->GetComponentByClass(UStaticMeshComponent::StaticClass()));
		if (StaticMeshComponent)
		{
			for (FHoleMaterialSetting MaterialSetting : MaterialSettings)
			{
				FHoleMaterial HoleMaterial;
				
				HoleMaterial.MaterialInstance = StaticMeshComponent->CreateDynamicMaterialInstance(MaterialSetting.MaterialIndex);
				//StaticMeshComponent->SetMaterial(MaterialSetting.MaterialIndex, MaterialInstance);

				HoleMaterial.HoleLocations.Reserve(MaterialSetting.MaxHoleCount);
				for (uint8 i = 0; i < MaterialSetting.MaxHoleCount; ++i)
				{
					HoleMaterial.HoleLocations.Add(FVector::ZeroVector);
				}

				HoleMaterial.MaxHoleCount = MaterialSetting.MaxHoleCount;				
				HoleMaterials.Add(HoleMaterial);
			}
		}
	}
}

int32 UFPSTemplate_HoleComponent::GetFaceIndex(const FHitResult& HitLocation)
{
	FHitResult NewHitResult;
	FCollisionQueryParams Params;
	Params.bTraceComplex = true;
	Params.bReturnFaceIndex = true;
	
	if (GetWorld()->LineTraceSingleByChannel(NewHitResult, HitLocation.TraceStart, HitLocation.TraceEnd, ECC_Visibility, Params))
	{
		return NewHitResult.FaceIndex;
	}
	return -1;
}

void UFPSTemplate_HoleComponent::AddHole(const FHitResult& HitResult)
{
	//HitResult.FaceIndex
	int32 SectionIndex = 0;
	int32 FaceIndex = GetFaceIndex(HitResult);
	if (UMaterialInterface* MI = HitResult.Component->GetMaterialFromCollisionFaceIndex(FaceIndex, SectionIndex))
	{
		for (FHoleMaterial& Material : HoleMaterials)
		{
			if (Material.MaterialInstance && MI == Material.MaterialInstance)
			{
				FString ParamName = FString("Impact" + FString::FromInt(Material.HoleIndex));
				
				Material.MaterialInstance->SetVectorParameterValue(FName(ParamName), HitResult.Location);
				if (Material.HoleIndex < Material.HoleLocations.Num())
				{
					Material.HoleLocations[Material.HoleIndex] = HitResult.Location;
				}
				
				if (++Material.HoleIndex > Material.MaxHoleCount - 1)
				{
					Material.HoleIndex = 0;
				}
				
				break;
			}
		}
	}
}

bool UFPSTemplate_HoleComponent::DidImpactHitHole(const FHitResult& HitResult, float Tolerance)
{
	int32 SectionIndex = 0;
	int32 FaceIndex = GetFaceIndex(HitResult);
	if (UMaterialInterface* MI = HitResult.Component->GetMaterialFromCollisionFaceIndex(FaceIndex, SectionIndex))
	{
		for (FHoleMaterial& Material : HoleMaterials)
		{
			if (Material.MaterialInstance && MI == Material.MaterialInstance)
			{
				for (const FVector& Location : Material.HoleLocations)
				{
					if (FVector::Distance(Location, HitResult.Location) <= Tolerance)
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}
