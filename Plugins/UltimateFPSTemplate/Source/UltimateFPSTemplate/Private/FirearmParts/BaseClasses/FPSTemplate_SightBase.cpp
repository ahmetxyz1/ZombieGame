// Copyright 2021, Dakota Dawe, All rights reserved

#include "FirearmParts/BaseClasses/FPSTemplate_SightBase.h"
#include "Actors/FPSTemplateFirearm.h"

#include "Materials/MaterialInstanceDynamic.h"

AFPSTemplate_SightBase::AFPSTemplate_SightBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	PartStats.Weight = 0.1f;
	PartStats.ErgonomicsChangePercentage = -2.0f;
	CameraFOVZoom = 10.0f;
	CameraFOVZoomSpeed = 10.0f;
	CameraDistance = 0.0f;
	bUsedFixedCameraDistance = false;

	bIsAimable = true;

	PartType = EPartType::Sight;

	ReticleIndex = 0;
	ReticleBrightnessSettings = {1.0f};
	ReticleDefaultBrightnessIndex = 0;
	ReticleBrightnessIndex = 0;
	ReticleMaterialIndex = 1;
}

// Called when the game starts or when spawned
void AFPSTemplate_SightBase::BeginPlay()
{
	Super::BeginPlay();
	SetReticle(ReticleIndex);
}

void AFPSTemplate_SightBase::PostInitProperties()
{
	Super::PostInitProperties();
	
	ReticleBrightnessIndex = ReticleDefaultBrightnessIndex;

	for (FReticleMaterial& Reticle : Reticles)
	{
		if (Reticle.ReticleMaterial)
		{
			Reticle.DynamicReticleMaterial = UMaterialInstanceDynamic::Create(Reticle.ReticleMaterial, this);
		}
	}

	SetReticleBrightness(ReticleDefaultBrightnessIndex);

	if (IsValid(PartMesh))
	{
		PartMesh->SetCollisionResponseToChannel(FirearmCollisionChannel, ECR_Ignore);
	}
}

void AFPSTemplate_SightBase::SetReticleBrightness(uint8 Index)
{
	if (Index < ReticleBrightnessSettings.Num())
	{
		for (FReticleMaterial& Reticle : Reticles)
		{
			if (Reticle.DynamicReticleMaterial)
			{
				Reticle.DynamicReticleMaterial->SetScalarParameterValue(FName("ReticleBrightness"), ReticleBrightnessSettings[Index]);
			}
		}
	}
}

void AFPSTemplate_SightBase::CycleReticle()
{
	if (Reticles.Num())
	{
		if (++ReticleIndex >= Reticles.Num())
		{
			ReticleIndex = 0;
		}
		SetReticle(ReticleIndex);
	}
}

void AFPSTemplate_SightBase::SetReticle(uint8 Index)
{
	if (IsValid(PartMesh) && Index < Reticles.Num() && Reticles[Index].DynamicReticleMaterial)
	{
		ReticleIndex = Index;
		PartMesh->SetMaterial(ReticleMaterialIndex, Reticles[ReticleIndex].DynamicReticleMaterial);
	}
}

void AFPSTemplate_SightBase::IncreaseBrightness()
{
	if (++ReticleBrightnessIndex < ReticleBrightnessSettings.Num())
	{
		SetReticleBrightness(ReticleBrightnessIndex);
	}
	else
	{
		ReticleBrightnessIndex = ReticleBrightnessSettings.Num() - 1;
	}
}

void AFPSTemplate_SightBase::DecreaseBrightness()
{
	if (ReticleBrightnessIndex - 1 >= 0)
	{
		--ReticleBrightnessIndex;
		SetReticleBrightness(ReticleBrightnessIndex);
	}
	else
	{
		ReticleBrightnessIndex = 0;
	}
}