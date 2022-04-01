// Copyright 2021, Dakota Dawe, All rights reserved


#include "Misc/FPSTemplateStatics.h"
#include "Misc/FPSTemplate_PhysicalMaterial.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Components/FPSTemplate_PartComponent.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Particles/ParticleSystem.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void UFPSTemplateStatics::SpawnProjectileImpactEffects(const AActor* WorldActor, const FHitResult& HitResult, float DecalScaleMultiplier)
{
	if (UFPSTemplate_PhysicalMaterial* PhysMat = Cast<UFPSTemplate_PhysicalMaterial>(HitResult.PhysMaterial))
	{
		SpawnEffects(WorldActor, HitResult, PhysMat->ProjectileImpactEffects, DecalScaleMultiplier);
	}
}

void UFPSTemplateStatics::SpawnFootstepImpactEffects(const AActor* WorldActor, const FHitResult& HitResult,
	float DecalScaleMultiplier)
{
	if (UFPSTemplate_PhysicalMaterial* PhysMat = Cast<UFPSTemplate_PhysicalMaterial>(HitResult.PhysMaterial))
	{
		SpawnEffects(WorldActor, HitResult, PhysMat->FootstepImpactEffect, DecalScaleMultiplier);
	}
}

void UFPSTemplateStatics::SpawnEffects(const AActor* WorldActor, const FHitResult& HitResult,
                                       const FImpactEffects& ImpactEffects, float DecalScaleMultiplier)
{
	if (WorldActor)
	{
		if (UWorld* World = WorldActor->GetWorld())
		{
			if (UFPSTemplate_PhysicalMaterial* PhysMat = Cast<UFPSTemplate_PhysicalMaterial>(HitResult.PhysMaterial))
			{
				if (USoundBase* ImpactSound = ImpactEffects.ImpactSound)
				{
					UGameplayStatics::SpawnSoundAtLocation(World, ImpactSound, HitResult.Location);
				}
				
				if (UFXSystemAsset* ImpactEffect = ImpactEffects.ImpactEffect)
				{
					FRotator Rotation = FRotator::ZeroRotator;
					if (!ImpactEffects.bUseParticlesRotation)
					{
						Rotation = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
					}
					if (UNiagaraSystem* NEffect = Cast<UNiagaraSystem>(ImpactEffect))
					{
						UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, NEffect, HitResult.Location, Rotation);
					}
					else if (UParticleSystem* PEffect = Cast<UParticleSystem>(ImpactEffect))
					{
						UGameplayStatics::SpawnEmitterAtLocation(World, PEffect, HitResult.Location, Rotation);
					}
				}
				
				if (UMaterialInstance* ImpactDecal = ImpactEffects.ImpactDecal)
				{
					if (USceneComponent* HitComponent = HitResult.GetComponent())
					{
						FRotator Rotation = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
						Rotation.Pitch += 180.0f;
						FVector DecalSize = ImpactEffects.DecalSize * DecalScaleMultiplier;
						float DecalLifetime = ImpactEffects.DecalLifeTime;
						
						UGameplayStatics::SpawnDecalAttached(ImpactDecal, DecalSize, HitComponent, NAME_None,
							HitResult.Location, Rotation, EAttachLocation::KeepWorldPosition, DecalLifetime);
					}
				}
			}
		}
	}
}

void UFPSTemplateStatics::SpawnEmptyCaseSound(const AActor* WorldActor, const FHitResult& HitResult)
{
	if (WorldActor)
	{
		if (UWorld* World = WorldActor->GetWorld())
		{
			if (UFPSTemplate_PhysicalMaterial* PhysMat = Cast<UFPSTemplate_PhysicalMaterial>(HitResult.PhysMaterial))
			{
				if (PhysMat->EmptyCaseImpactSound)
				{
					UGameplayStatics::SpawnSoundAtLocation(World, PhysMat->EmptyCaseImpactSound, HitResult.Location);
				}
			}
		}
	}
}

FRotator UFPSTemplateStatics::SetMuzzleMOA(FRotator MuzzleRotation, float MOA)
{
	MOA /= 100.0f;
	float MOAChange = (FMath::RandRange(-MOA / 2.0f, MOA / 2.0f) * 2.54f); // 1 inch at 100 yards
	MuzzleRotation.Yaw += MOAChange;
	MOAChange = (FMath::RandRange(-MOA / 2.0f, MOA / 2.0f) * 2.54f);
	MuzzleRotation.Pitch += MOAChange * 0.6f; // reduce vertical MOA shift for consistency

	return MuzzleRotation;
}

FRotator UFPSTemplateStatics::GetEstimatedMuzzleToScopeZero(const FTransform& MuzzleTransform,
	const FTransform& SightTransform, const float RangeMeters)
{
	FVector EndLocation = SightTransform.GetLocation() + SightTransform.GetRotation().Vector() * RangeMeters;
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleTransform.GetLocation(), EndLocation);
	LookAtRotation.Pitch += 0.04f; // Increase vertical projectile launch angle (thanks gravity)
	return LookAtRotation;
}

bool UFPSTemplateStatics::Ragdoll(UPrimitiveComponent* MeshComponent)
{
	if (Cast<USkeletalMeshComponent>(MeshComponent))
	{
		if (ACharacter* Character = Cast<ACharacter>(MeshComponent->GetOwner()))
		{
			Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetSimulatePhysics(true);
		return true;
	}
	return false;
}

void UFPSTemplateStatics::RagdollWithImpact(UPrimitiveComponent* MeshComponent, FVector ImpactLocation, float ImpactForce)
{
	if (Ragdoll(MeshComponent) && ImpactLocation != FVector::ZeroVector && ImpactForce != 0.0f)
	{
		ImpactForce *= MeshComponent->GetMass();
		MeshComponent->AddRadialImpulse(ImpactLocation, 100.0f, ImpactForce, ERadialImpulseFalloff::RIF_Linear);
	}
}

FProjectileTransform UFPSTemplateStatics::CreateProjectileTransform(const FTransform& MuzzleTransform)
{
	return FProjectileTransform(MuzzleTransform);
}

FTransform UFPSTemplateStatics::CreateTransformForProjectile(const FProjectileTransform& ProjectileTransform)
{
	return FProjectileTransform::GetTransformFromProjectile(ProjectileTransform);
}

FFirearm UFPSTemplateStatics::GetFirearmStruct(AFPSTemplateFirearm* Firearm)
{
	if (Firearm)
	{
		FFirearm FirearmStruct;
		FirearmStruct.FirearmClass = Firearm->GetClass();
		for (UFPSTemplate_PartComponent* PartComponent : Firearm->GetPartComponents())
		{// && PartComponent->GetPart()->GetOwner() == Firearm
			if (PartComponent && IsValid(PartComponent->GetPart()))
			{
				FFirearmPartList PartList;
				PartList.ComponentName = PartComponent->GetName();
				PartList.Part = PartComponent->GetPart()->GetClass();
				PartList.PartOffset = PartComponent->GetPart()->GetPartOffset();
				if (PartComponent->GetPart()->GetOwner() == Firearm)
				{
					PartList.ParentFirearm = PartComponent->GetPart()->GetOwner()->GetClass();
				}
				else
				{
					PartList.Parent = PartComponent->GetPart()->GetOwner()->GetClass();
				}
				FirearmStruct.PartsList.Add(PartList);
			}
		}
		return FirearmStruct;
	}
	return FFirearm();
}

FString UFPSTemplateStatics::SerializeFirearm(AFPSTemplateFirearm* Firearm, FString& Error)
{
	if (!IsValid(Firearm))
	{
		Error = "Firearm INVALID";
		return "";
	}
	FString SerializedString;
	if (Firearm)
	{
		FFirearm FirearmStruct = GetFirearmStruct(Firearm);
		/*FirearmStruct.FirearmClass = Firearm->GetClass();
		for (UFPSTemplate_PartComponent* PartComponent : Firearm->GetPartComponents())
		{// && PartComponent->GetPart()->GetOwner() == Firearm
			if (PartComponent && IsValid(PartComponent->GetPart()))
			{
				FFirearmPartList PartList;
				PartList.ComponentName = PartComponent->GetName();
				PartList.Part = PartComponent->GetPart()->GetClass();
				PartList.Parent = PartComponent->GetPart()->GetOwner()->GetClass();
				FirearmStruct.PartsList.Add(PartList);

				/*for (UFPSTemplate_PartComponent* PartPartComponent : PartComponent->GetPart()->GetPartComponents())
				{
					if (PartPartComponent && IsValid(PartPartComponent->GetPart()))
					{
						
					}
				}#1#
			}
		}*/
		if (!FJsonObjectConverter::UStructToJsonObjectString(FirearmStruct, SerializedString))
		{
			Error = "Could Not Serialize String";
			return "";
		}
	}

	//FPlatformMisc::ClipboardCopy(ToCStr(SerializedString));
	return SerializedString;
}

FFirearm UFPSTemplateStatics::DeserializeFirearm(const FString& JsonString, FString& Error)
{
	FFirearm FirearmStruct;
	if (FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &FirearmStruct, 0, 0))
	{
		FPSLog(TEXT("SUCCESS DESERIALIZED"));
	}
	else
	{
		Error = "Could NOT Deserialize Json String";
	}

	return FirearmStruct;
}

AFPSTemplateFirearm* UFPSTemplateStatics::ConstructFirearm(AActor* WorldActor, FFirearm FirearmStruct, FString& Error)
{
	if (!IsValid(WorldActor))
	{
		Error = "WorldActor INVALID";
		return nullptr;
	}
	if (!WorldActor->HasAuthority())
	{
		Error = "NO AUTHORITY";
		return nullptr;
	}
	
	UWorld* World = WorldActor->GetWorld();
	if (!World)
	{
		Error = "Could NOT Get World";
		return nullptr;
	}

	if (AFPSTemplateFirearm* Firearm = World->SpawnActor<AFPSTemplateFirearm>(FirearmStruct.FirearmClass))
	{
		for (FFirearmPartList& PartList : FirearmStruct.PartsList)
		{
			PartList.bHasBeenCreated = true;
			if (PartList.ParentFirearm)
			{
				for (UFPSTemplate_PartComponent* PartComponent : Firearm->GetPartComponents())
				{
					if (PartComponent && PartComponent->GetName().Equals(PartList.ComponentName))
					{
						if (PartComponent->AddPart(PartList.Part) && IsValid(PartComponent->GetPart()))
						{
							PartComponent->GetPart()->SetOffset(PartList.PartOffset);
						}
					}
				}
			}
			else
			{
				for (UFPSTemplate_PartComponent* PartComponent : Firearm->GetPartComponents())
				{
					if (PartComponent && PartComponent->GetName().Equals(PartList.ComponentName))
					{
						//FPSLog(TEXT("Matching Part: %s	 To Part Component: %s"), *PartList.Part->GetName(), *PartComponent->GetName());
						if (PartComponent->AddPart(PartList.Part) && IsValid(PartComponent->GetPart()))
						{
							PartComponent->GetPart()->SetOffset(PartList.PartOffset);
						}
					}
				}
			}
		}
		return Firearm;
	}

	return nullptr;
}

bool UFPSTemplateStatics::LoadTextFromFile(const FString& FileName, FString& OutString)
{
	return FFileHelper::LoadFileToString(OutString, *(FPaths::ProjectPluginsDir() + FileName));
}

FString UFPSTemplateStatics::RoundFloat2Pos(float Value)
{
	return FString::Printf(TEXT("%.2f"), Value);
}

FString UFPSTemplateStatics::RoundFloat3Pos(float Value)
{
return FString::Printf(TEXT("%.3f"), Value);
}
