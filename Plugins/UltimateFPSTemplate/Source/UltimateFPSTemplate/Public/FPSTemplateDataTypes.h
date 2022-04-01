//Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "Templates/SubclassOf.h"
#include "Engine/NetSerialization.h"

#include "FPSTemplateDataTypes.generated.h"

UENUM(BlueprintType)
enum class EMagazineState : uint8
{
	InUse		UMETA(DisplayName, "InUse"),
	NotInUse	UMETA(DisplayName, "NotInUse"),
	OnGround	UMETA(DisplayName, "OnGround")
};

UENUM(BlueprintType)
enum class ELightLaser : uint8
{
	Light	UMETA(DisplayName, "Light"),
	Laser	UMETA(DisplayName, "Laser"),
	Both	UMETA(DisplayName, "Both")
};

UENUM(BlueprintType)
enum class EFirearmFireMode : uint8
{
	Safe			UMETA(DisplayName, "Safe"),
	Semi			UMETA(DisplayName, "Semi"),
	Burst			UMETA(DisplayName, "Burst"),
	FullAuto		UMETA(DisplayName, "FullAuto"),
	BoltAction		UMETA(DisplayName, "BoltAction")
};

UENUM(BlueprintType)
enum class ELeaning : uint8
{
	None	UMETA(DisplayName, "None"),
	Left	UMETA(DisplayName, "Left"),
	Right	UMETA(DisplayName, "Right")
};

UENUM(BlueprintType)
enum class EPortPose : uint8
{
	None	UMETA(DisplayName, "None"),
	High	UMETA(DisplayName, "High"),
	Low		UMETA(DisplayName, "Low")
};

UENUM(BlueprintType)
enum class EPartType : uint8
{
	Sight			UMETA(DisplayName, "Sight"),
	Magnifier		UMETA(DisplayName, "Magnifier"),
	LightLaser		UMETA(DisplayName, "LightLaser"),
	Handguard		UMETA(DisplayName, "Handguard"),
	Barrel			UMETA(DisplayName, "Barrel"),
	MuzzleDevice	UMETA(DisplayName, "MuzzleDevice"),
	Stock			UMETA(DisplayName, "Stock"),
	PistolGrip		UMETA(DisplayName, "PistolGrip"),
	ForwardGrip		UMETA(DisplayName, "ForwardGrip"),
	Other			UMETA(DisplayName, "Other")
};

UENUM(BlueprintType)
enum class EFirearmInHand : uint8
{
	None		UMETA(DisplayName, "None"),
	M4			UMETA(DisplayName, "M4"),
	Test		UMETA(DisplayName, "Test"),
};

UENUM(BlueprintType)
enum class EMeasurementType : uint8
{
	Metric	UMETA(DisplayName, "Metric"),
	Imperial	UMETA(DisplayName, "Imperial")
};

class UMaterialInstance;
class UMaterialInstanceDynamic;
class UFXSystemAsset;
class USoundBase;

USTRUCT(BlueprintType)
struct FHoleMaterial
{
	GENERATED_BODY()
	UMaterialInstanceDynamic* MaterialInstance = nullptr;
	TArray<FVector> HoleLocations;
	uint8 HoleIndex = 0;
	uint8 MaxHoleCount = 8;
};

USTRUCT(BlueprintType)
struct FHoleMaterialSetting
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate")
	int32 MaxHoleCount = 8;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate")
	int32 MaterialIndex = 0;
};

USTRUCT(BlueprintType)
struct FFirearmPartStats
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float Weight = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float ErgonomicsChangePercentage = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float RecoilChangePercentage = 0.0f;

	FFirearmPartStats operator+ (const FFirearmPartStats& Stats) const
	{
		FFirearmPartStats NewStats;
		NewStats.Weight = this->Weight + Stats.Weight;
		NewStats.ErgonomicsChangePercentage = this->ErgonomicsChangePercentage + Stats.ErgonomicsChangePercentage;
		NewStats.RecoilChangePercentage = this->RecoilChangePercentage + Stats.RecoilChangePercentage;

		return NewStats;
	}
	
	FFirearmPartStats& operator+= (const FFirearmPartStats& Stats)
	{
		this->Weight += Stats.Weight;
		this->ErgonomicsChangePercentage += Stats.ErgonomicsChangePercentage;
		this->RecoilChangePercentage += Stats.RecoilChangePercentage;

		return *this;
	}
};

USTRUCT(BlueprintType)
struct FFirearmStats
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float Weight = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float Ergonomics = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float VerticalRecoil = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float HorizontalRecoil = 0.0f;
};

USTRUCT(BlueprintType)
struct FSightData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "FPSTemplate")
	class AFPSTemplate_SightBase* Sight = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "FPSTemplate")
	FName Socket = NAME_None;

	bool operator==(const FSightData& SightData) const
	{
		return this->Sight == SightData.Sight;
	}
};

USTRUCT(BlueprintType)
struct FFirearmPartData
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	TSubclassOf<class AFPSTemplate_PartBase> PartClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	class UTexture2D* PartImage = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	FString PartName;
};

USTRUCT(BlueprintType)
struct FReticleMaterial
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Reticle")
	UMaterialInstance* ReticleMaterial = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Reticle")
	UMaterialInstanceDynamic* DynamicReticleMaterial = nullptr;
};

USTRUCT(BlueprintType)
struct FImpactEffects
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	UFXSystemAsset* ImpactEffect = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	UMaterialInstance* ImpactDecal = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	bool bUseParticlesRotation = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	FVector DecalSize = FVector(2.5f, 2.5f, 2.5f);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float DecalLifeTime = 8.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	USoundBase* ImpactSound = nullptr;
};

USTRUCT(BlueprintType)
struct FFirearmPartList
{
	GENERATED_BODY()
	UPROPERTY()
	FString ComponentName;
	UPROPERTY()
	TSubclassOf<class AFPSTemplateFirearm> ParentFirearm;
	UPROPERTY()
	TSubclassOf<class AFPSTemplate_PartBase> Parent;
	UPROPERTY()
	TSubclassOf<class AFPSTemplate_PartBase> Part;
	UPROPERTY()
	float PartOffset = 0.0f;
	
	bool bHasBeenCreated = false;
};

USTRUCT(BlueprintType)
struct FFirearm
{
	GENERATED_BODY()
	UPROPERTY()
	TSubclassOf<class AFPSTemplateFirearm> FirearmClass;
	UPROPERTY()
	TArray<FFirearmPartList> PartsList;
};

USTRUCT(BlueprintType)
struct FProjectileTransform
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "FPSTemplate | Projectile")
	FVector_NetQuantize Location;
	UPROPERTY(BlueprintReadWrite, Category = "FPSTemplate | Projectile")
	FVector_NetQuantize Rotation;

	FProjectileTransform() {Location = FVector_NetQuantize(); Rotation = FVector_NetQuantize();}
	FProjectileTransform(const FVector& INLocation, const FRotator& INRotation)
	{
		Location = FVector_NetQuantize(INLocation);
		Rotation = FVector_NetQuantize(INRotation.Pitch, INRotation.Yaw, INRotation.Roll);
	}
	FProjectileTransform(const FTransform& INTransform)
	{
		Location = FVector_NetQuantize(INTransform.GetLocation());
		Rotation = FVector_NetQuantize(INTransform.Rotator().Pitch, INTransform.Rotator().Yaw, INTransform.Rotator().Roll);
	}
	
	static FTransform GetTransformFromProjectile(const FProjectileTransform& ProjectileTransform)
	{
		FVector_NetQuantize Rot = ProjectileTransform.Rotation;
		FVector Loc = ProjectileTransform.Location;
		return FTransform(FRotator(ProjectileTransform.Rotation.X, ProjectileTransform.Rotation.Y, ProjectileTransform.Rotation.Z).Quaternion(), ProjectileTransform.Location, FVector::OneVector);
	}
	FTransform GetTransformFromProjectile() const
	{
		return FTransform(FRotator(Rotation.X, Rotation.Y, Rotation.Z).Quaternion(), Location, FVector::OneVector);
	}
};