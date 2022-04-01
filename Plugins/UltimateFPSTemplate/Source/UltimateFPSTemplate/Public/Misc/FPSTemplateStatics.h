// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FPSTemplateDataTypes.h"
#include "Engine/EngineTypes.h"
#include "FPSTemplateStatics.generated.h"

#define FPSLog(Text, ...)\
{\
	UE_LOG(LogTemp, Warning, Text, __VA_ARGS__);\
}

#define FPSLogPrint(Text, ...)\
{\
	GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor::Red, FString::Printf(Text, __VA_ARGS__));\
}

#define FPSLogNPrint(Text, ...)\
{\
	GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor::Red, FString::Printf(Text, __VA_ARGS__));\
	UE_LOG(LogTemp, Warning, Text, __VA_ARGS__);\
}

class UFPSTemplate_PhysicalMaterial;
class UPrimitiveComponent;
class AFPSTemplateFirearm;

UCLASS()
class ULTIMATEFPSTEMPLATE_API UFPSTemplateStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Projectile")
	static void SpawnProjectileImpactEffects(const AActor* WorldActor, const FHitResult& HitResult, float DecalScaleMultiplier = 1.0f);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	static void SpawnFootstepImpactEffects(const AActor* WorldActor, const FHitResult& HitResult, float DecalScaleMultiplier = 1.0f);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Other")
	static void SpawnEffects(const AActor* WorldActor, const FHitResult& HitResult, const FImpactEffects& ImpactEffects, float DecalScaleMultiplier = 1.0f);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Other")
	static void SpawnEmptyCaseSound(const AActor* WorldActor, const FHitResult& HitResult);
	
	// Applies MOA to the muzzles rotation for accuracy. 1 MOA = 1 inch at 100 yards
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	static FRotator SetMuzzleMOA(FRotator MuzzleRotation, float MOA = 1.0f);
	// Gets the estimated zero for magnified scopes based on the sight and muzzle
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	static FRotator GetEstimatedMuzzleToScopeZero(const FTransform& MuzzleTransform, const FTransform& SightTransform, const float RangeMeters);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	static bool Ragdoll(UPrimitiveComponent* MeshComponent);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	static void RagdollWithImpact(UPrimitiveComponent* MeshComponent, FVector ImpactLocation = FVector::ZeroVector, float ImpactForce = 0.0f);

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Projectile")
	static FProjectileTransform CreateProjectileTransform(const FTransform& MuzzleTransform);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Projectile")
	static FTransform CreateTransformForProjectile(const FProjectileTransform& ProjectileTransform);
	
	static FFirearm GetFirearmStruct(AFPSTemplateFirearm* Firearm);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Serialize")
	static FString SerializeFirearm(AFPSTemplateFirearm* Firearm, FString& Error);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Serialize")
	static FFirearm DeserializeFirearm(const FString& JsonString, FString& Error);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Serialize")
	static AFPSTemplateFirearm* ConstructFirearm(AActor* WorldActor, FFirearm FirearmStruct, FString& Error);

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Serialize")
	static bool LoadTextFromFile(const FString& FileName, FString& OutString);

	// Rounds a float to the 2nd decimal place and returns in string format
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Math")
	static FString RoundFloat2Pos(float Value);
	// Rounds a float to the 3rd decimal place and returns in string format
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Math")
	static FString RoundFloat3Pos(float Value);
};
