// Copyright 2021, Dakota Dawe, All rights reserved


#include "FirearmParts/FPSTemplate_Barrel.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Misc/FPSTemplateStatics.h"
#include "FirearmParts/FPSTemplate_Muzzle.h"
#include "Components/FPSTemplate_PartComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values
AFPSTemplate_Barrel::AFPSTemplate_Barrel()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MuzzleSocket = FName("S_Muzzle");
	PartStats.Weight = 0.7f;
	PartStats.ErgonomicsChangePercentage = -5.0f;
	PartStats.RecoilChangePercentage = -2.0f;

	PartType = EPartType::Barrel;
}

// Called when the game starts or when spawned
void AFPSTemplate_Barrel::BeginPlay()
{
	Super::BeginPlay();

}

void AFPSTemplate_Barrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSTemplate_Barrel, MuzzleDevice);
}

FTransform AFPSTemplate_Barrel::GetMuzzleSocketTransform() const
{
	if (IsValid(MuzzleDevice))
	{
		return MuzzleDevice->GetMuzzleSocketTransform();
	}
	return IsValid(PartMesh) ? PartMesh->GetSocketTransform(MuzzleSocket) : FTransform();
}

bool AFPSTemplate_Barrel::DoesMuzzleSocketExist() const
{
	return IsValid(PartMesh) ? PartMesh->DoesSocketExist(MuzzleSocket) : false;
}

AFPSTemplate_Muzzle* AFPSTemplate_Barrel::GetMuzzleDevice()
{
	return Cast<AFPSTemplate_Muzzle>(GetMuzzleDeviceActor());
}

AActor* AFPSTemplate_Barrel::GetMuzzleDeviceActor()
{
	if (IsValid(MuzzleDevice))
	{
		return MuzzleDevice->GetMuzzleAttachment();
	}
	return this;
}

void AFPSTemplate_Barrel::CacheParts()
{
	for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent)
		{
			AFPSTemplate_Muzzle* Muzzle = PartComponent->GetPart<AFPSTemplate_Muzzle>();
			if (IsValid(Muzzle))
			{
				MuzzleDevice = Muzzle;
			}
		}
	}
}