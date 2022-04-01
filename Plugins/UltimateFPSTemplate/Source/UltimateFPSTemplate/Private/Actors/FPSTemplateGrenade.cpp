// Copyright 2021, Dakota Dawe, All rights reserved


#include "Actors/FPSTemplateGrenade.h"

#include "Misc/FPSTemplateStatics.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

// Sets default values
AFPSTemplateGrenade::AFPSTemplateGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("SphereComponent"));
	//CollisionComponent->InitSphereRadius(1.6f);
	CollisionComponent->InitCapsuleSize(4.8f, 6.8f);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	CollisionComponent->SetSimulatePhysics(false);
	CollisionComponent->SetLinearDamping(0.35f);
	CollisionComponent->SetAngularDamping(1.0f);
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	RootComponent = CollisionComponent;

	PoseCollision = ECC_GameTraceChannel2;

	DudDestroyTime = 5.0f;
	ServerSyncIntervalPerSecond = 10.0f;
	
	MaxBounces = 2;
	CurrentBounces = 0;
	FuseTime = 4.5f;

	bReplicates = true;

	bExplodeOnImpact = false;
	bIsArmed = false;

	ClientGrenade = nullptr;
	bClientGrenadeInterp = true;
	bSyncGrenadeInterp = false;

	bIsVisibleGrenade = true;
}

// Called when the game starts or when spawned
void AFPSTemplateGrenade::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{
		//SetActorHiddenInGame(true);
		SetActorTickEnabled(false);
		SetActorTickInterval(1 / 60);
	}
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->OnComponentHit.AddDynamic(this, &AFPSTemplateGrenade::OnComponentHit);
}


void AFPSTemplateGrenade::PostInitProperties()
{
	Super::PostInitProperties();
	CollisionComponent->SetCollisionResponseToChannel(PoseCollision, ECR_Ignore);
}

void AFPSTemplateGrenade::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	InterpToNewLocation();
}

void AFPSTemplateGrenade::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!HasAuthority() && IsValid(this))
	{
		SuperExplode();
	}
	Super::EndPlay(EndPlayReason);
}

void AFPSTemplateGrenade::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	++CurrentBounces;
	if (bExplodeOnImpact && CurrentBounces >= MaxBounces)
	{
		CollisionComponent->SetGenerateOverlapEvents(false);
		CollisionComponent->OnComponentHit.RemoveAll(this);
		SetLifeSpan(DudDestroyTime);
	}
	
	if (bIsArmed)
	{
		SuperExplode();
		CollisionComponent->SetGenerateOverlapEvents(false);
		CollisionComponent->OnComponentHit.RemoveAll(this);
	}
	else
	{
		Impact(GetVelocity().Size());
	}
	
	if (GetWorldTimerManager().IsTimerActive(TArmTime))
	{
		UE_LOG(LogTemp, Warning, TEXT("Stopping Timer"));
		GetWorldTimerManager().ClearTimer(TArmTime);
	}
}

void AFPSTemplateGrenade::SuperExplode()
{
	if (!bIsVisibleGrenade)
	{
		return;
	}

	Explode();
}

void AFPSTemplateGrenade::Explode_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Explode_Implementation"));
}

void AFPSTemplateGrenade::Impact_Implementation(float Velocity)
{
	UE_LOG(LogTemp, Warning, TEXT("Impact_Implementation"));
}

void AFPSTemplateGrenade::EnablePhysics()
{
	CollisionComponent->SetSimulatePhysics(true);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AFPSTemplateGrenade::DisablePhysics()
{
	CollisionComponent->SetSimulatePhysics(false);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFPSTemplateGrenade::ArmGrenade()
{
	bIsArmed = true;
	FPSLog(TEXT("Armed"));
}

void AFPSTemplateGrenade::Multi_UpdateLocation_Implementation(FVector NewLocation, FRotator NewRotation)
{
	if (!HasAuthority())
	{
		bSyncGrenadeInterp = true;
		InterpToLocation = NewLocation;
		InterpToRotation = NewRotation;
		SetActorTickEnabled(true);
	}
}

void AFPSTemplateGrenade::Multi_ReleaseGrenade_Implementation(FVector Orientation, float Velocity)
{
	if (!HasAuthority() && GetVelocity().Size() < 10.0f)
	{
		ReleaseGrenade(Orientation, Velocity);
	}
}

void AFPSTemplateGrenade::SyncLocation()
{
	Multi_UpdateLocation(GetActorLocation(), GetActorRotation());
	if (GetVelocity().Equals(FVector::ZeroVector, 1.0f))
	{
		GetWorldTimerManager().ClearTimer(TSync);
	}
}

void AFPSTemplateGrenade::ReleaseGrenade(FVector Orientation, float Velocity, bool IsClientGrenade)
{
	EnablePhysics();
	CollisionComponent->SetPhysicsLinearVelocity(Orientation * Velocity);

	if (ServerSyncIntervalPerSecond > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TSync, this, &AFPSTemplateGrenade::SyncLocation, 1.0f / ServerSyncIntervalPerSecond, true);
	}
	else
	{
		SetActorTickEnabled(false);
	}
	Multi_ReleaseGrenade(Orientation, Velocity);
	
	if (bExplodeOnImpact)
	{
		if (ArmingTime > 0.0f)
		{
			GetWorldTimerManager().SetTimer(TArmTime, this, &AFPSTemplateGrenade::ArmGrenade, ArmingTime, false);
		}
		else
		{
			ArmGrenade();
		}
	}
	else if (!GetWorldTimerManager().IsTimerActive(TFuse) && !IsClientGrenade)
	{
		StartFuse(FuseTime);
	}
}

void AFPSTemplateGrenade::CookGrenade()
{
	if (!bExplodeOnImpact && !GetWorldTimerManager().IsTimerActive(TFuse))
	{
		StartFuse(FuseTime);
	}
}

void AFPSTemplateGrenade::InterpToNewLocation()
{
	if (bClientGrenadeInterp)
	{
		if (IsValid(ClientGrenade))
		{
			ClientGrenade->SetActorLocation(UKismetMathLibrary::VInterpTo(ClientGrenade->GetActorLocation(), InterpToLocation, GetWorld()->GetDeltaSeconds(), 1.0f));
			ClientGrenade->SetActorRotation(UKismetMathLibrary::RInterpTo(ClientGrenade->GetActorRotation(), InterpToRotation, GetWorld()->GetDeltaSeconds(), 1.0f));
		}
	}
}

void AFPSTemplateGrenade::SetClientGrenade(AFPSTemplateGrenade* Grenade)
{
	if (IsValid(Grenade))
	{
		Destroy();
		ClientGrenade = Grenade;
		ClientGrenade->SetActorHiddenInGame(false);
	
		FuseTime += (CreationTime - ClientGrenade->CreationTime) / 2.0f;
		ClientGrenade->StartFuse(FuseTime);
		
		SetActorHiddenInGame(true);
		bIsVisibleGrenade = false;
	}
}

FVector AFPSTemplateGrenade::GetSmokeGrenadeParticleLocation(float SpriteRadius)
{
	if (SpriteRadius <= 0.0f)
	{
		SpriteRadius = 1.0f;
	}
	FVector Location = GetActorLocation();
	Location.Z += SpriteRadius / 2.0f;
	return Location;
}

void AFPSTemplateGrenade::StartFuse(float Time)
{
	GetWorldTimerManager().SetTimer(TFuse, this, &AFPSTemplateGrenade::SuperExplode, Time, false);
}
