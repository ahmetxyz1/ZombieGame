// Copyright 2021, Dakota Dawe, All rights reserved


#include "FirearmParts/BaseClasses/FPSTemplate_SightMagnifiedRTBase.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Components/FPSTemplate_CharacterComponent.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInterface.h"

AFPSTemplate_SightMagnifiedRTBase::AFPSTemplate_SightMagnifiedRTBase()
{
	PrimaryActorTick.bCanEverTick = true;
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;
	
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
	SceneCapture->SetupAttachment(RootComponent);
	SceneCapture->bHiddenInGame = true;
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->bAlwaysPersistRenderingState = true;
	SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;

	ScopeRefreshRate = 60.0f;
	bIsFirstFocalPlane = false;

	Magnifications = {1.0f};
	MagnificationIndex = 0;

	bHasRenderTarget = true;
	bDisableWhenNotAiming = true;
	NotAimingRefreshRate = 5.0f;

	bClearScopeWithColor = true;
	ScopeClearedColor = FLinearColor::Black;

	AnimationIndex = -1;
	AimInterpolationMultiplier = 50.0f;
	GripSocket = FName("cc_FirearmGrip");
	HeadAimRotation = FRotator(0.0f, 0.0f, 0.0f);
}

void AFPSTemplate_SightMagnifiedRTBase::BeginPlay()
{
	Super::BeginPlay();
	
	DisableRenderTarget(true);
}

void AFPSTemplate_SightMagnifiedRTBase::PostInitProperties()
{
	Super::PostInitProperties();
	
	SceneCapture->SetHiddenInGame(false);
	
	if (Magnifications.Num())
	{
		CurrentMagnification = Magnifications[MagnificationIndex];
	}
	else
	{
		CurrentMagnification = 1.0f;
	}
	SetFOVAngle();
	
	SetActorTickInterval(1 / ScopeRefreshRate);
}

void AFPSTemplate_SightMagnifiedRTBase::HandleRenderTarget()
{
	if (PartType == EPartType::Magnifier)
	{
		if (UFPSTemplate_CharacterComponent* CharacterComponent = GetOwningCharacterComponent())
		{
			if (CharacterComponent->IsLocallyControlled())
			{
				DisableRenderTarget(false);
			}
		}
	}
	else
	{
		DisableRenderTarget(true);
	}
}

void AFPSTemplate_SightMagnifiedRTBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SceneCapture->CaptureScene();
}

void AFPSTemplate_SightMagnifiedRTBase::SetFOVAngle()
{
	SceneCapture->FOVAngle = 15.0f / CurrentMagnification;
}

void AFPSTemplate_SightMagnifiedRTBase::HandleFirstFocalPlaneZoom(float MagnificationDifference)
{
	for (FReticleMaterial& Reticle : Reticles)
	{
		if (Reticle.DynamicReticleMaterial)
		{
			float CurrentReticleSize;
			Reticle.DynamicReticleMaterial->GetScalarParameterValue(FName("ReticleSize"), CurrentReticleSize);
			if (CurrentReticleSize)
			{
				float NewReticleSize = CurrentReticleSize + (MagnificationDifference * 0.1f / 2.0f);
				Reticle.DynamicReticleMaterial->SetScalarParameterValue(FName("ReticleSize"), NewReticleSize);
			}
		}
	}
}

void AFPSTemplate_SightMagnifiedRTBase::ZoomIn()
{
	if (++MagnificationIndex < Magnifications.Num())
	{
		float OldMagnification = CurrentMagnification;
		CurrentMagnification = Magnifications[MagnificationIndex];

		if (bIsFirstFocalPlane)
		{
			HandleFirstFocalPlaneZoom(CurrentMagnification - OldMagnification);
		}
		
		for (FReticleMaterial& Reticle : Reticles)
		{
			if (Reticle.DynamicReticleMaterial)
			{
				float EyeboxRange;
				Reticle.DynamicReticleMaterial->GetScalarParameterValue(FName("EyeboxRange"), EyeboxRange);
				if (EyeboxRange)
				{
					EyeboxRange -= 150.0f;
					Reticle.DynamicReticleMaterial->SetScalarParameterValue(FName("EyeboxRange"), EyeboxRange);
				}
			}
		}
		
		OpticZoomedIn(OldMagnification);
		SetFOVAngle();
	}
	else
	{
		MagnificationIndex = Magnifications.Num() - 1;
	}
}

void AFPSTemplate_SightMagnifiedRTBase::ZoomOut()
{
	if (MagnificationIndex - 1 >= 0)
	{
		float OldMagnification = CurrentMagnification;
		--MagnificationIndex;
		CurrentMagnification = Magnifications[MagnificationIndex];

		if (bIsFirstFocalPlane)
		{
			HandleFirstFocalPlaneZoom(CurrentMagnification - OldMagnification);
		}
		
		for (FReticleMaterial& Reticle : Reticles)
		{
			if (Reticle.DynamicReticleMaterial)
			{
				float EyeboxRange;
				Reticle.DynamicReticleMaterial->GetScalarParameterValue(FName("EyeboxRange"), EyeboxRange);
				if (EyeboxRange)
				{
					EyeboxRange += 150.0f;
					Reticle.DynamicReticleMaterial->SetScalarParameterValue(FName("EyeboxRange"), EyeboxRange);
				}
			}
		}
		
		OpticZoomedIn(OldMagnification);
		SetFOVAngle();
	}
	else
	{
		MagnificationIndex = 0;
	}
}

void AFPSTemplate_SightMagnifiedRTBase::IncreaseVerticalZero()
{
	FRotator CurrentRot = SceneCapture->GetRelativeRotation();
	CurrentRot.Pitch -= 0.006f;
	SceneCapture->SetRelativeRotation(CurrentRot);
}

void AFPSTemplate_SightMagnifiedRTBase::DecreaseVerticalZero()
{
	FRotator CurrentRot = SceneCapture->GetRelativeRotation();
	CurrentRot.Pitch += 0.006f;
	SceneCapture->SetRelativeRotation(CurrentRot);
}

void AFPSTemplate_SightMagnifiedRTBase::DisableRenderTarget(bool Disable)
{
	if (!bDisableWhenNotAiming)
	{
		if (Disable)
		{
			SetActorTickInterval(1 / NotAimingRefreshRate);
		}
		else
		{
			SceneCapture->CaptureScene();
			SetActorTickInterval(1 / ScopeRefreshRate);
		}
	}
	else if (IsValid(PartMesh))
	{
		if (Disable)
		{
			bool bClearedWithMaterial = false;
			if (bClearScopeWithMaterial && ClearedScopeMaterial)
			{
				PartMesh->SetMaterial(ReticleMaterialIndex, ClearedScopeMaterial);
				bClearedWithMaterial = true;
			}
			if (bClearScopeWithColor || !bClearedWithMaterial)
			{
				UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), SceneCapture->TextureTarget, ScopeClearedColor);
			}
		}
		else
		{
			if (bClearScopeWithMaterial && ReticleIndex < Reticles.Num() && Reticles[ReticleIndex].DynamicReticleMaterial)
			{
				PartMesh->SetMaterial(ReticleMaterialIndex, Reticles[ReticleIndex].DynamicReticleMaterial);
			}
		}
		SetActorTickEnabled(!Disable);
	}
}

void AFPSTemplate_SightMagnifiedRTBase::ZoomOptic(bool bZoom)
{
	if (bZoom)
	{
		ZoomIn();
	}
	else
	{
		ZoomOut();
	}
}
