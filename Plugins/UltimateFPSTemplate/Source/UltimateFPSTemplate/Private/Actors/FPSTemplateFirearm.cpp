//Copyright 2021, Dakota Dawe, All rights reserved

#include "Actors/FPSTemplateFirearm.h"
#include "FPSTemplateAnimInstance.h"
#include "Misc/FPSTemplateStatics.h"
#include "FirearmParts/BaseClasses/FPSTemplate_SightBase.h"
#include "FirearmParts/FPSTemplate_Barrel.h"
#include "FirearmParts/FPSTemplate_Handguard.h"
#include "FirearmParts/FPSTemplate_ForwardGrip.h"
#include "FirearmParts/FPSTemplate_Muzzle.h"
#include "FirearmParts/BaseClasses/FPSTemplate_LightLaserBase.h"
#include "Components/FPSTemplate_PartComponent.h"
#include "FirearmParts/BaseClasses/FPSTemplate_MagnifierBase.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "Components/FPSTemplate_CharacterComponent.h"

#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

// Sets default values
AFPSTemplateFirearm::AFPSTemplateFirearm()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	FirearmMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	FirearmMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = FirearmMesh;
	PoseCollision = ECollisionChannel::ECC_GameTraceChannel2;
	
	bReplicates = true;

	bFirearmHidden = false;
	
	FirearmIndex = 0;

	FirearmGripSocket = FName("cc_FirearmGrip");
	MuzzleSocket = FName("S_Muzzle");
	LeftHandIKSocket = FName("S_LeftHandIK");
	AimSocket = FName("S_Aim");

	bUseLeftHandIK = false;

	DefaultFirearmStats.Weight = 7.0f;
	DefaultFirearmStats.Ergonomics = 50.0f;
	DefaultFirearmStats.VerticalRecoil = 110.0f;
	DefaultFirearmStats.HorizontalRecoil = 180.0f;

	CameraFOVZoom = 15.0f;
	CameraFOVZoomSpeed = 10.0f;

	ShoulderStockOffset = FVector(-8.0f, 0.0f, 2.0f);
	HeadAimRotation = FRotator(45.0f, 0.0f, 0.0f);

	ControlMovementSwayByStats = true;
	ScaleCameraShakeByStats = true;

	bCanFire = true;
	bCanReFire = true;
	bCanCycleSights = true;
	
	FireModeIndex = 0;
	BurstFireCount = 0;
	BurstCount = 3;

	FireRateRPM = 800.0f;

	TimeSinceLastShot = 0.0f;

	ShortStockPoseDistance = 18.0f;
	FirstPersonSprintPose.SetLocation(FVector(-5.0f, 0.0f, -10.0f));
	FirstPersonSprintPose.SetRotation(FRotator(-45.0f, 0.0f, 20.0f).Quaternion());
	FirstPersonHighPortPose.SetLocation(FVector(-10.0f, 0.0f, -10.0f));
	FirstPersonHighPortPose.SetRotation(FRotator(80.0f, 45.0f, 0.0f).Quaternion());
	FirstPersonLowPortPose.SetLocation(FVector(-10.0f, 0.0f, -10.0f));
	FirstPersonLowPortPose.SetRotation(FRotator(80.0f, -45.0f, 0.0f).Quaternion());

	ThirdPersonSprintPose.SetLocation(FVector(-5.0f, 0.0f, -10.0f));
	ThirdPersonSprintPose.SetRotation(FRotator(-45.0f, 0.0f, 20.0f).Quaternion());
	ThirdPersonHighPortPose.SetLocation(FVector(-10.0f, 0.0f, -3.0f));
	ThirdPersonHighPortPose.SetRotation(FRotator(80.0f, 45.0f, 0.0f).Quaternion());
	ThirdPersonLowPortPose.SetLocation(FVector(0.0f, 8.0f, 8.0f));
	ThirdPersonLowPortPose.SetRotation(FRotator(80.0f, -45.0f, 0.0f).Quaternion());

	DefaultAimSwayMultiplier = 2.0f;
	AimSwayMultiplier = DefaultAimSwayMultiplier;

	CharacterComponent = nullptr;

	AttachedToSocket = NAME_None;
}

void AFPSTemplateFirearm::OnRep_CharacterComponent()
{
	CycleSights();
	if (IsValid(this) && AttachedToSocket != NAME_None)
	{
		AttachToSocket(AttachedToSocket);
	}
}

// Called when the game starts or when spawned
void AFPSTemplateFirearm::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* OwningActor = GetOwner())
	{
		CharacterComponent = Cast<UFPSTemplate_CharacterComponent>(OwningActor->GetComponentByClass(TSubclassOf<UFPSTemplate_CharacterComponent>()));
	}
	
	if (HasAuthority())
	{
		GetComponents<UFPSTemplate_PartComponent>(PartComponents);
	}

	if (HasAuthority())
	{
		HandleSightComponents();
	}

	// Update initial Default Parts
	FTimerHandle TTemp;
	GetWorldTimerManager().SetTimer(TTemp, this, &AFPSTemplateFirearm::PartsChanged, 0.1f, false);
}

void AFPSTemplateFirearm::PostInitProperties()
{
	Super::PostInitProperties();
	AimSwayMultiplier = DefaultAimSwayMultiplier;
	
	FirearmStats = DefaultFirearmStats;
	if (FireModes.Num())
	{
		FireMode = FireModes[0];
	}
	TimerAutoFireRate = 60 / FireRateRPM;
	FirearmMesh->SetCollisionResponseToChannel(PoseCollision, ECollisionResponse::ECR_Ignore);
}

void AFPSTemplateFirearm::OnRep_Owner()
{
	if (const AActor* OwningActor = GetOwner())
	{
		CharacterComponent = Cast<UFPSTemplate_CharacterComponent>(OwningActor->GetComponentByClass(TSubclassOf<UFPSTemplate_CharacterComponent>()));
	}
	RefreshCurrentSight();
}

void AFPSTemplateFirearm::OnRep_AttachmentReplication()
{
	//Super::OnRep_AttachmentReplication();
}

void AFPSTemplateFirearm::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSTemplateFirearm, CharacterComponent);
	DOREPLIFETIME(AFPSTemplateFirearm, bFirearmHidden);
	DOREPLIFETIME(AFPSTemplateFirearm, ProjectileToSpawn);
	DOREPLIFETIME_CONDITION(AFPSTemplateFirearm, PartComponents, COND_OwnerOnly);
	DOREPLIFETIME(AFPSTemplateFirearm, BarrelComponent);
	DOREPLIFETIME(AFPSTemplateFirearm, HandguardComponent);
	DOREPLIFETIME_CONDITION(AFPSTemplateFirearm, SightComponents, COND_OwnerOnly);
	DOREPLIFETIME(AFPSTemplateFirearm, CurrentSightComponent);
	DOREPLIFETIME(AFPSTemplateFirearm, FirearmStats);
	DOREPLIFETIME_CONDITION(AFPSTemplateFirearm, FireMode, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AFPSTemplateFirearm, LightsLaserComponents, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFPSTemplateFirearm, MagnifierComponents, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFPSTemplateFirearm, ComponentsWithRenderTargets, COND_OwnerOnly);
	DOREPLIFETIME(AFPSTemplateFirearm, StockComponent);
	DOREPLIFETIME(AFPSTemplateFirearm, ForwardGripComponent);
	DOREPLIFETIME(AFPSTemplateFirearm, MuzzleComponent);
	DOREPLIFETIME(AFPSTemplateFirearm, AttachedToSocket);
}

float AFPSTemplateFirearm::GetStockLengthOfPull()
{
	if (StockComponent)
	{
		if (IsValid(StockComponent->GetPart()))
		{
			if (AFPSTemplate_PartBase* Part = StockComponent->GetPart())
			{
				if (GetCharacterComponent() && CharacterComponent->IsLocallyControlled())
				{
					return Part->GetPartOffset() + Part->GetStockLengthOfPull();
				}
				return Part->GetPartOffset();
			}
		}
	}
	return 0.0f;
}

float AFPSTemplateFirearm::GetInterpolationMultiplier()
{
	return GetFirearmStats().Ergonomics * (10.0f / (GetFirearmStats().Weight * 1.5f));
}

void AFPSTemplateFirearm::UpdateFirearmStats()
{
	if (HasAuthority())
	{
		FirearmStats = DefaultFirearmStats;
		FFirearmPartStats PartStats;
		for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
		{
			if (PartComponent && IsValid(PartComponent->GetPart()))
			{
				PartComponent->GetPart()->CacheParts();
				PartStats += PartComponent->GetPart()->GetPartStats();
			}
		}

		FirearmStats.Weight += PartStats.Weight;
		float PercentMultiplier = 1.0f + (PartStats.RecoilChangePercentage / 100.0f);
		FirearmStats.VerticalRecoil *= PercentMultiplier;
		FirearmStats.HorizontalRecoil *= PercentMultiplier;

		PercentMultiplier = 1.0f + (PartStats.ErgonomicsChangePercentage / 100.0f);
		FirearmStats.Ergonomics *= PercentMultiplier;
	}
}

void AFPSTemplateFirearm::HandleSightComponents()
{
	SightComponents.Empty();
	for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent && IsValid(PartComponent->GetPart()))
		{
			if (PartComponent->GetPart()->IsAimable())
			{
				SightComponents.Add(PartComponent);
			}
			for (UFPSTemplate_PartComponent* PartPartComponent : PartComponent->GetPart()->GetSightComponents())
			{
				if (PartPartComponent && IsValid(PartPartComponent->GetPart()))
				{
					if (PartPartComponent->GetPart()->IsAimable())
					{
						SightComponents.Add(PartPartComponent);
					}
				}
			}
		}
	}
	if (!CurrentSightComponent || (CurrentSightComponent && !IsValid(CurrentSightComponent->GetPart())))
	{
		CycleSights();
	}
}

UFPSTemplate_CharacterComponent* AFPSTemplateFirearm::GetCharacterComponent()
{
	if (!CharacterComponent)
	{
		if (IsValid(GetOwner()))
		{
			UActorComponent* Component = GetOwner()->GetComponentByClass(UFPSTemplate_CharacterComponent::StaticClass());
			CharacterComponent = Cast<UFPSTemplate_CharacterComponent>(Component);
			OnRep_CharacterComponent();
		}
	}
	return CharacterComponent;
}

bool AFPSTemplateFirearm::IsSuppressed() const
{
	AFPSTemplate_Muzzle* Muzzle = GetMuzzleDevice();
	if (IsValid(Muzzle))
	{
		return Muzzle->IsSuppressor();
	}
	return false;
}

float AFPSTemplateFirearm::GetCameraFOVZoom() const
{
	AFPSTemplate_SightBase* CurrentSight = GetCurrentSight();
	if (IsValid(CurrentSight))
	{
		return CurrentSight->GetCameraFOVZoom();
	}
	return CameraFOVZoom;
}

float AFPSTemplateFirearm::GetCameraFOVZoomSpeed() const
{
	AFPSTemplate_SightBase* CurrentSight = GetCurrentSight();
	if (IsValid(CurrentSight))
	{
		return CurrentSight->GetCameraFOVZoomSpeed();
	}
	return CameraFOVZoomSpeed;
}

void AFPSTemplateFirearm::CycleMagnifier()
{
	for (UFPSTemplate_PartComponent* PartComponent : MagnifierComponents)
	{
		if (PartComponent)
		{
			AFPSTemplate_MagnifierBase* Magnifier = PartComponent->GetPart<AFPSTemplate_MagnifierBase>();
			if (IsValid(Magnifier))
			{
				UE_LOG(LogTemp, Warning, TEXT("Cycle Magnifier: %s"), *Magnifier->GetName());
				Magnifier->Use();
			}
		}
	}
	/*for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent && PartComponent->GetPart())
		{
			if (PartComponent->GetPart()->GetPartType() == EPartType::FlipMount)
			{
				if(AFPSTemplate_FlipMount* FlipMount = PartComponent->GetPart<AFPSTemplate_FlipMount>())
				{
					FlipMount->Use();
					return;
				}
			}
			for (UFPSTemplate_PartComponent* PartPartComponent : PartComponent->GetPart()->GetPartComponents())
			{
				if(AFPSTemplate_FlipMount* FlipMount = PartPartComponent->GetPart<AFPSTemplate_FlipMount>())
				{
					FlipMount->Use();
					return;
				}
			}
		}
	}*/
}

void AFPSTemplateFirearm::UseLightLaser(ELightLaser Toggle)
{
	for (UFPSTemplate_PartComponent* PartComponent : LightsLaserComponents)
	{
		if (PartComponent)
		{
			AFPSTemplate_LightLaserBase* LightLaser = PartComponent->GetPart<AFPSTemplate_LightLaserBase>();
			if (IsValid(LightLaser))
			{
				switch (Toggle)
				{
					case ELightLaser::Light: LightLaser->ToggleLight(); break;
					case ELightLaser::Laser: LightLaser->ToggleLaser(); break;
					case ELightLaser::Both: LightLaser->ToggleLight(); LightLaser->ToggleLaser(); break;
				}
			}
		}
	}
	
	/*for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent && PartComponent->GetPart())
		{
			if (PartComponent->GetPart()->GetPartType() == EPartType::LightLaser)
			{
				if(AFPSTemplate_LightLaserBase* LightLaser = PartComponent->GetPart<AFPSTemplate_LightLaserBase>())
				{
					LightLaser->ToggleLight();
				}
			}
			for (UFPSTemplate_PartComponent* PartPartComponent : PartComponent->GetPart()->GetPartComponents())
			{
				if(AFPSTemplate_LightLaserBase* LightLaser = PartPartComponent->GetPart<AFPSTemplate_LightLaserBase>())
				{
					LightLaser->ToggleLight();
				}
			}
		}
	}*/
}

void AFPSTemplateFirearm::CycleLaserColor()
{
	for (UFPSTemplate_PartComponent* PartComponent : LightsLaserComponents)
	{
		if (PartComponent)
		{
			AFPSTemplate_LightLaserBase* LightLaser = PartComponent->GetPart<AFPSTemplate_LightLaserBase>();
			if (IsValid(LightLaser))
			{
				LightLaser->CycleLaserColor();
			}
		}
	}
}

void AFPSTemplateFirearm::CycleReticle()
{
	if (AFPSTemplate_SightBase* Sight = GetCurrentSight())
	{
		Sight->CycleReticle();
	}
}

void AFPSTemplateFirearm::ZoomOptic(bool bZoom)
{
	if (AFPSTemplate_SightBase* Sight = GetCurrentSight())
	{
		if (bZoom)
		{
			Sight->ZoomIn();
		}
		else
		{
			Sight->ZoomOut();
		}
	}
}

void AFPSTemplateFirearm::IncreaseReticleBrightness(bool Increase)
{
	if (AFPSTemplate_SightBase* Sight = GetCurrentSight())
	{
		if (Increase)
		{
			Sight->IncreaseBrightness();
		}
		else
		{
			Sight->DecreaseBrightness();
		}
	}
}

float AFPSTemplateFirearm::GetOpticMagnification() const
{
	/*if (AFPSTemplate_SightBase* Sight = GetCurrentSight())
	{
		return Sight->GetMagnification();
	}*/
	return IsValid(GetCurrentSight()) ? GetCurrentSight()->GetMagnification() : 1.0f;
}

void AFPSTemplateFirearm::IncreaseVerticalZero(bool Increase)
{
	if (AFPSTemplate_SightBase* Sight = GetCurrentSight())
	{
		if (Increase)
		{
			Sight->IncreaseVerticalZero();
		}
		else
		{
			Sight->DecreaseVerticalZero();
		}
	}
}

AFPSTemplate_Muzzle* AFPSTemplateFirearm::GetMuzzleDevice() const
{
	return Cast<AFPSTemplate_Muzzle>(GetMuzzleActor());
}

AActor* AFPSTemplateFirearm::GetMuzzleActor() const
{
	if (MuzzleComponent)
	{
		return MuzzleComponent->GetPart();
	}
	if (BarrelComponent)
	{
		AFPSTemplate_Barrel* Barrel = BarrelComponent->GetPart<AFPSTemplate_Barrel>();
		if (IsValid(Barrel))
		{
			return Barrel->GetMuzzleDeviceActor();
		}
	}
	return nullptr;
}

FHitResult AFPSTemplateFirearm::MuzzleTrace(float Distance, ECollisionChannel CollisionChannel, bool& bMadeBlockingHit, bool bDrawDebugLine)
{
	FTransform MuzzleTransform = FProjectileTransform::GetTransformFromProjectile(GetMuzzleSocketTransform());
	FVector End = MuzzleTransform.GetLocation() + MuzzleTransform.Rotator().Vector() * Distance;
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	
	bMadeBlockingHit = GetWorld()->LineTraceSingleByChannel(HitResult, MuzzleTransform.GetLocation(), End, CollisionChannel, Params);
	if (bDrawDebugLine)
	{
		DrawDebugLine(GetWorld(), MuzzleTransform.GetLocation(), End, FColor::Red, false, 3.0f, 0, 2.0f);
	}
	return HitResult;
}

TArray<FHitResult> AFPSTemplateFirearm::MuzzleTraceMulti(float Distance, ECollisionChannel CollisionChannel, bool& bMadeBlockingHit, bool bDrawDebugLine)
{
	FTransform MuzzleTransform = FProjectileTransform::GetTransformFromProjectile(GetMuzzleSocketTransform());
	FVector End = MuzzleTransform.GetLocation() + MuzzleTransform.Rotator().Vector() * Distance;
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	
	bMadeBlockingHit = GetWorld()->LineTraceMultiByChannel(HitResults, MuzzleTransform.GetLocation(), End, CollisionChannel, Params);
	if (bDrawDebugLine)
	{
		DrawDebugLine(GetWorld(), MuzzleTransform.GetLocation(), End, FColor::Red, false, 3.0f, 0, 2.0f);
	}
	return HitResults;
}

void AFPSTemplateFirearm::DisableAllRenderTargets(bool Disable)
{
	for (UFPSTemplate_PartComponent* PartComponent : ComponentsWithRenderTargets)
	{
		if (PartComponent && IsValid(PartComponent->GetPart()))
		{
			PartComponent->GetPart()->DisableRenderTarget(Disable);
		}
	}
	/*for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent && IsValid(PartComponent->GetPart()))
		{
			if (PartComponent->GetPart()->HasRenderTarget())
			{
				PartComponent->GetPart()->DisableRenderTarget(Disable);
			}
			for (UFPSTemplate_PartComponent* PartPartComponent : PartComponent->GetPart()->GetPartComponents())
			{
				if (PartPartComponent && IsValid(PartPartComponent->GetPart()))
				{
					if (PartPartComponent->GetPart()->HasRenderTarget())
					{
						PartPartComponent->GetPart()->DisableRenderTarget(Disable);
					}
				}
			}
		}
	}*/
}

void AFPSTemplateFirearm::PartsChanged()
{
	/*for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent && IsValid(PartComponent->GetPart()))
		{
			switch (PartComponent->GetPart()->GetPartType())
			{
				case EPartType::Barrel : BarrelComponent = PartComponent; break;
				case EPartType::Handguard :
					{
						HandguardComponent = PartComponent;
						if (CharacterComponent && CharacterComponent->GetAnimationInstance())
						{
							CharacterComponent->GetAnimationInstance()->SetGripType(HandguardComponent->GetPart<AFPSTemplate_Handguard>()->GetGripType());
						}
						break;
					}
				case EPartType::ForwardGrip :
					{
						ForwardGripComponent = PartComponent;
						if (CharacterComponent->GetAnimationInstance())
						{
							CharacterComponent->GetAnimationInstance()->SetGripType(ForwardGripComponent->GetPart<AFPSTemplate_ForwardGrip>()->GetGripType());
						}
						break;
					}
				case EPartType::Stock :
					{
						StockComponent = PartComponent;
						break;
					}
			case EPartType::FlipMount :
					{
						FlipMounts.Add(PartComponent);
						break;
					}
			case EPartType::LightLaser :
					{
						LightsLasers.Add(PartComponent);
						break;
					}
			}
		}
	}*/
	
	HandleSightComponents();
	UpdateFirearmStats();
	OnPartsChanged();
}

TArray<AFPSTemplate_PartBase*> AFPSTemplateFirearm::GetFirearmParts() const
{
	TArray<AFPSTemplate_PartBase*> Parts;
	for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent && IsValid(PartComponent->GetPart()))
		{
			Parts.Add(PartComponent->GetPart());
			for (UFPSTemplate_PartComponent* PartPartComponent : PartComponent->GetPart()->GetPartComponents())
			{
				if (PartPartComponent && IsValid(PartPartComponent->GetPart()))
				{
					Parts.Add(PartPartComponent->GetPart());
				}
			}
		}
	}
	return Parts;
}

FTransform& AFPSTemplateFirearm::GetSprintPose()
{
	if (GetCharacterComponent())
	{
		if (!CharacterComponent->IsLocallyControlled())
		{
			return ThirdPersonSprintPose; // THIRD PERSON HERE
		}
		else
		{
			return FirstPersonSprintPose;
		}
	}
	return FirstPersonSprintPose;
}

FTransform& AFPSTemplateFirearm::GetHighPortPose()
{
	if (GetCharacterComponent())
	{
		if (!CharacterComponent->IsLocallyControlled())
		{
			return ThirdPersonHighPortPose;
		}
		else
		{
			return FirstPersonHighPortPose;
		}
	}
	return FirstPersonHighPortPose;
}

FTransform& AFPSTemplateFirearm::GetLowPortPose()
{
	if (GetCharacterComponent())
	{
		if (!CharacterComponent->IsLocallyControlled())
		{
			return ThirdPersonLowPortPose;
		}
		else
		{
			return FirstPersonLowPortPose;
		}
	}
	return FirstPersonLowPortPose;
}

FVector AFPSTemplateFirearm::GetShortStockPose() const
{
	return FVector(0.0f, ShortStockPoseDistance * -1.0f, 0.0f);
}

void AFPSTemplateFirearm::SetCharacterComponent(UFPSTemplate_CharacterComponent* INCharacterComponent)
{
	CycleSights();
	CharacterComponent = INCharacterComponent;
}

void AFPSTemplateFirearm::AddPartCache(UFPSTemplate_PartComponent* PartComponent)
{
	if (PartComponent && IsValid(PartComponent->GetPart()))
	{
		//FPSLog(TEXT("Adding: %s"), *PartComponent->GetPart()->GetName())
		switch (PartComponent->GetPart()->GetPartType())
		{
		case EPartType::Barrel : BarrelComponent = PartComponent; break;
		case EPartType::Handguard :
			{
				HandguardComponent = PartComponent;
				break;
			}
		case EPartType::Stock :
			{
				StockComponent = PartComponent;
				break;
			}
		case EPartType::Magnifier :
			{
				MagnifierComponents.AddUnique(PartComponent);
				ComponentsWithRenderTargets.AddUnique(PartComponent);
				break;
			}
		case EPartType::LightLaser :
			{
				LightsLaserComponents.AddUnique(PartComponent);
				break;
			}
		case EPartType::ForwardGrip :
			{
				ForwardGripComponent = PartComponent;
				break;
			}
		case EPartType::MuzzleDevice :
			{
				MuzzleComponent = PartComponent;
				break;
			}
		default: if (PartComponent->GetPart()->HasRenderTarget())
			{
				ComponentsWithRenderTargets.AddUnique(PartComponent);
			}
		}
	}
}

UAnimSequence* AFPSTemplateFirearm::GetGripAnimation() const
{
	if (ForwardGripComponent) // FOR COMPLETE FIREARMS (NO PART BUILDS)
	{
		AFPSTemplate_ForwardGrip* ForwardGrip = ForwardGripComponent->GetPart<AFPSTemplate_ForwardGrip>();
		if (IsValid(ForwardGrip))
		{
			return ForwardGrip->GetGripAnimation();
		}
	}
	if (HandguardComponent)
	{
		AFPSTemplate_Handguard* Handguard = HandguardComponent->GetPart<AFPSTemplate_Handguard>();
		if (IsValid(Handguard))
		{
			return Handguard->GetGripAnimation();
		}
	}
	return nullptr;
}

void AFPSTemplateFirearm::OnRep_FireMode()
{
	if (GetCharacterComponent() && CharacterComponent->IsLocallyControlled())
	{
		return;
	}
	OnFireModeChanged();
}

void AFPSTemplateFirearm::OnRep_FirearmHidden()
{
	FirearmMesh->SetHiddenInGame(bFirearmHidden, true);
}

bool AFPSTemplateFirearm::Server_SetFireMode_Validate(EFirearmFireMode NewFireMode)
{
	return true;
}

void AFPSTemplateFirearm::Server_SetFireMode_Implementation(EFirearmFireMode NewFireMode)
{
	FireMode = NewFireMode;
	OnRep_FireMode();
}

void AFPSTemplateFirearm::CycleFireMode()
{
	if (FireModes.Num() > 1)
	{
		if (++FireModeIndex > FireModes.Num() - 1)
		{
			FireModeIndex = 0;
		}
		FireMode = FireModes[FireModeIndex];
		OnFireModeChanged();
		if (!HasAuthority())
		{
			Server_SetFireMode(FireMode);
		}
	}
}

void AFPSTemplateFirearm::PerformProceduralRecoil(float Multiplier, bool PlayCameraShake)
{
	if (GetCharacterComponent() && CharacterComponent->GetAnimationInstance())
	{
		CharacterComponent->GetAnimationInstance()->PerformRecoil(Multiplier);
		if (PlayCameraShake && FireCameraShake)
		{
			CharacterComponent->PlayCameraShake(FireCameraShake, Multiplier * 1.2f);
		}
	}
}

TArray<UFPSTemplate_PartComponent*> AFPSTemplateFirearm::GetPartComponents() const
{
	TArray<UFPSTemplate_PartComponent*> PartComponentsList = PartComponents;
	for (UFPSTemplate_PartComponent* Part : PartComponents)
	{
		if (Part && IsValid(Part->GetPart()))
		{
			PartComponentsList.Append(Part->GetPart()->GetPartComponents());
		}
	}
	return PartComponentsList;
}

FTransform AFPSTemplateFirearm::GetSightSocketTransform()
{
	if (CurrentSightComponent && IsValid(CurrentSightComponent->GetPart()))
	{
		return CurrentSightComponent->GetPart()->GetAimSocketTransform();
	}
	if (SightComponents.Num() && SightComponents[0] && IsValid(SightComponents[0]->GetPart()))
	{
		return SightComponents[0]->GetPart()->GetAimSocketTransform();
	}
	return FirearmMesh->GetSocketTransform(AimSocket);
}

FTransform AFPSTemplateFirearm::GetDefaultSightSocketTransform()
{
	return FirearmMesh->DoesSocketExist(AimSocket) ? FirearmMesh->GetSocketTransform(AimSocket) : GetSightSocketTransform();
}

FTransform AFPSTemplateFirearm::GetLeftHandIKTransform()
{
	if (ForwardGripComponent) // FOR COMPLETE FIREARMS (NO PART BUILDS)
	{
		AFPSTemplate_ForwardGrip* ForwardGrip = ForwardGripComponent->GetPart<AFPSTemplate_ForwardGrip>();
		if (IsValid(ForwardGrip))
		{
			return ForwardGrip->GetGripTransform();
		}
	}
	if (HandguardComponent)
	{
		AFPSTemplate_Handguard* Handguard = HandguardComponent->GetPart<AFPSTemplate_Handguard>();
		if (IsValid(Handguard))
		{
			return Handguard->GetGripTransform();
		}
	}
	return FirearmMesh->GetSocketTransform(LeftHandIKSocket);
}

FProjectileTransform AFPSTemplateFirearm::GetMuzzleSocketProjectileTransform()
{
	if (BarrelComponent)
	{
		if (AFPSTemplate_Barrel* Barrel = BarrelComponent->GetPart<AFPSTemplate_Barrel>())
		{
			if (IsValid(Barrel))
			{
				return Barrel->GetMuzzleSocketTransform();
			}
		}
	}
	return FirearmMesh->GetSocketTransform(MuzzleSocket);
}

FTransform AFPSTemplateFirearm::GetMuzzleSocketTransform()
{
	if (BarrelComponent)
	{
		if (AFPSTemplate_Barrel* Barrel = BarrelComponent->GetPart<AFPSTemplate_Barrel>())
		{
			if (IsValid(Barrel))
			{
				return Barrel->GetMuzzleSocketTransform();
			}
		}
	}
	return FirearmMesh->GetSocketTransform(MuzzleSocket);
}

FProjectileTransform AFPSTemplateFirearm::GetMuzzleProjectileSocketTransform(float RangeMeters, float MOA)
{
	RangeMeters *= 100.0f;
	if (RangeMeters > 10000)
	{
		RangeMeters = 10000;
	}
	else if (RangeMeters < 2500)
	{
		RangeMeters = 2500;
	}
	
	FTransform SightTransform = GetSightSocketTransform();
	FTransform MuzzleTransform = GetMuzzleSocketTransform();

	FRotator MuzzleRotation = UFPSTemplateStatics::GetEstimatedMuzzleToScopeZero(MuzzleTransform, SightTransform, RangeMeters);	
	MuzzleRotation = UFPSTemplateStatics::SetMuzzleMOA(MuzzleRotation, MOA);

	MuzzleTransform.SetRotation(MuzzleRotation.Quaternion());
	return MuzzleTransform;
}

void AFPSTemplateFirearm::OnRep_CurrentSightComponent()
{
	if (GetCharacterComponent() && CharacterComponent->GetAnimationInstance())
	{
		if (!CharacterComponent->IsLocallyControlled())
		{
			CharacterComponent->GetAnimationInstance()->CycledSights();
		}
	}
}

bool AFPSTemplateFirearm::Server_CycleSights_Validate(UFPSTemplate_PartComponent* SightComponent)
{
	return true;
}

void AFPSTemplateFirearm::Server_CycleSights_Implementation(UFPSTemplate_PartComponent* SightComponent)
{
	if (SightComponent)
	{
		CurrentSightComponent = SightComponent;
		OnRep_CurrentSightComponent();
	}
}

/*UFPSTemplate_PartComponent* AFPSTemplateFirearm::GetSightComponent()
{
	for (SightComponentIndex; SightComponentIndex < PartComponents.Num(); ++SightComponentIndex)
	{
		if (UFPSTemplate_PartComponent* PartComponent = SightComponents[SightComponentIndex])
		{
			if (PartComponent != CurrentSightComponent)
			{
				if (AFPSTemplate_PartBase* Part = PartComponent->GetPart())
				{
					if (Part->GetPartType() == EPartType::Sight)
					{
						CurrentSightComponent = PartComponent;
						return CurrentSightComponent;
					}
				}
			}
		}
	}
	return nullptr;
}*/

void AFPSTemplateFirearm::CycleSights()
{
	if (!bCanCycleSights)
	{
		return;
	}
	
	bool FoundValidSight = false;
	if (SightComponents.Num())
	{
		if (++SightComponentIndex >= SightComponents.Num())
		{
			SightComponentIndex = 0;
		}
		if (UFPSTemplate_PartComponent* SightComponent = SightComponents[SightComponentIndex])
		{
			CurrentSightComponent = SightComponent;
			FoundValidSight = true;
		}
	}
	if (FoundValidSight)
	{
		if (!HasAuthority())
		{
			Server_CycleSights(CurrentSightComponent);
		}
	}
	if (GetCharacterComponent())
	{
		if (UFPSTemplateAnimInstance* AnimInstance = CharacterComponent->GetAnimationInstance())
		{
			AnimInstance->CycledSights();
		}
	}
}

void AFPSTemplateFirearm::RefreshCurrentSight()
{
	if (!bCanCycleSights)
	{
		return;
	}
	
	if (CurrentSightComponent)
	{
		if (!HasAuthority())
		{
			Server_CycleSights(CurrentSightComponent);
		}
		if (GetCharacterComponent())
		{
			if (UFPSTemplateAnimInstance* AnimInstance = CharacterComponent->GetAnimationInstance())
			{
				AnimInstance->CycledSights();
			}
		}
	}
}

bool AFPSTemplateFirearm::Server_SetHidden_Validate(bool Hide)
{
	return true;
}

void AFPSTemplateFirearm::Server_SetHidden_Implementation(bool Hide)
{
	HideFirearm(Hide);
}

void AFPSTemplateFirearm::HideFirearm(bool Hide)
{
	if (Hide != bFirearmHidden)
	{
		bFirearmHidden = Hide;
		OnRep_FirearmHidden();
		if (!HasAuthority())
		{
			Server_SetHidden(Hide);
		}
	}
}

bool AFPSTemplateFirearm::Server_AttachToSocket_Validate(const FName& Socket)
{
	return true;
}

void AFPSTemplateFirearm::Server_AttachToSocket_Implementation(const FName& Socket)
{
	AttachToSocket(Socket);
}

void AFPSTemplateFirearm::OnRep_AttachedToSocket()
{
	if (GetCharacterComponent())
	{
		CharacterComponent->AttachItem(this, AttachedToSocket);
		OnAttachedToSocket(AttachedToSocket);
	}
}

void AFPSTemplateFirearm::AttachToSocket(const FName Socket)
{
	AttachedToSocket = Socket;
	if (Socket != NAME_None && GetCharacterComponent())
	{
		OnRep_AttachedToSocket();
		if (!HasAuthority())
		{
			Server_AttachToSocket(Socket);
		}
	}
}

void AFPSTemplateFirearm::DestroyAllParts()
{
	for (AFPSTemplate_PartBase* Part : GetFirearmParts())
	{
		if (IsValid(Part))
		{
			Part->Destroy();
		}
	}
}

AFPSTemplate_SightBase* AFPSTemplateFirearm::GetCurrentSight() const
{
	if (CurrentSightComponent)
	{
		return CurrentSightComponent->GetPart<AFPSTemplate_SightBase>();
	}
	return nullptr;
}
