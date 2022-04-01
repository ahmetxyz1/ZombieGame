// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"

#include "FPSTemplateDataTypes.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "FPSTemplate_PhysicalMaterial.generated.h"

class USoundBase;

UCLASS(Blueprintable, BlueprintType)
class ULTIMATEFPSTEMPLATE_API UFPSTemplate_PhysicalMaterial : public UPhysicalMaterial
{
	GENERATED_BODY()
public:
	UFPSTemplate_PhysicalMaterial();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	FImpactEffects ProjectileImpactEffects;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Character)
	FImpactEffects FootstepImpactEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Grenade)
	FImpactEffects GrenadeImpactEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Other)
	FImpactEffects ImpactEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Other)
	USoundBase* EmptyCaseImpactSound;
};
