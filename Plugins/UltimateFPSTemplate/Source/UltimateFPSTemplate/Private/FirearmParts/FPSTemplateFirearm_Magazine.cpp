//Copyright 2021, Dakota Dawe, All rights reserved

#include "FirearmParts/FPSTemplateFirearm_Magazine.h"

#include "Actors/FPSTemplateFirearm.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AFPSTemplateFirearm_Magazine::AFPSTemplateFirearm_Magazine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MagazineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = MagazineMesh;
	MagazineMesh->SetVisibility(false);

	bReplicates = true;
	
	MagazineCapacity = 30;
	CurrentAmmo = MagazineCapacity;

	MagazineState = EMagazineState::NotInUse;
}

// Called when the game starts or when spawned
void AFPSTemplateFirearm_Magazine::BeginPlay()
{
	Super::BeginPlay();
	CurrentAmmo = MagazineCapacity;
}

void AFPSTemplateFirearm_Magazine::PostInitProperties()
{
	Super::PostInitProperties();
}

void AFPSTemplateFirearm_Magazine::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSTemplateFirearm_Magazine, MagazineState);
	DOREPLIFETIME(AFPSTemplateFirearm_Magazine, CurrentAmmo);
}

TSubclassOf<AFPSTemplateProjectile> AFPSTemplateFirearm_Magazine::ConsumeProjectile()
{
	if (CurrentAmmo - 1 >= 0)
	{
		--CurrentAmmo;
		return ProjectileClass;
	}
	return nullptr;
}

void AFPSTemplateFirearm_Magazine::DropMagazine()
{
	MagazineMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	SimulatePhysics(true);
}

void AFPSTemplateFirearm_Magazine::OnRep_MagazineState()
{
	switch (MagazineState)
	{
		case EMagazineState::InUse : SetVisibility(true); break;
		case EMagazineState::OnGround : SetVisibility(true); break;
		case EMagazineState::NotInUse : SetVisibility(false); break;
	}
}

void AFPSTemplateFirearm_Magazine::SimulatePhysics(bool Simulate)
{
	MagazineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MagazineMesh->SetSimulatePhysics(Simulate);
}

void AFPSTemplateFirearm_Magazine::SetVisibility(bool Visible)
{
	MagazineMesh->SetVisibility(Visible);
}

void AFPSTemplateFirearm_Magazine::SetMagazineState(EMagazineState NewMagazineState)
{
	MagazineState = NewMagazineState;
	
	switch (MagazineState)
	{
	case EMagazineState::InUse : SetVisibility(true); break;
	case EMagazineState::OnGround : SetVisibility(true); break;
	case EMagazineState::NotInUse : SetVisibility(false); break;
	}
}
