// Copyright 2021, Dakota Dawe, All rights reserved


#include "FirearmParts/FPSTemplate_Muzzle.h"
#include "FirearmParts/FPSTemplate_Barrel.h"
#include "Components/FPSTemplate_PartComponent.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Misc/FPSTemplateStatics.h"

#include "Net/UnrealNetwork.h"

AFPSTemplate_Muzzle::AFPSTemplate_Muzzle()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIsSuppressor = false;
	
	MuzzleSocket = FName("S_Muzzle");
	
	PartStats.Weight = 1.6f;
	PartStats.RecoilChangePercentage = -10.0f;

	PartType = EPartType::MuzzleDevice;
}

void AFPSTemplate_Muzzle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSTemplate_Muzzle, MuzzleAttachment);
}

FTransform AFPSTemplate_Muzzle::GetMuzzleSocketTransform()
{
	if (IsValid(MuzzleAttachment) && IsValid(MuzzleAttachment->PartMesh))
	{
		return MuzzleAttachment->PartMesh->GetSocketTransform(MuzzleSocket);
	}
	return IsValid(PartMesh) ? PartMesh->GetSocketTransform(MuzzleSocket) : FTransform();
}

bool AFPSTemplate_Muzzle::DoesMuzzleSocketExist()
{
	return IsValid(PartMesh) ? PartMesh->DoesSocketExist(MuzzleSocket) : false;
}

bool AFPSTemplate_Muzzle::IsSuppressor() const
{
	if (IsValid(MuzzleAttachment))
	{
		return MuzzleAttachment->bIsSuppressor;
	}
	return bIsSuppressor;
}

void AFPSTemplate_Muzzle::CacheParts()
{
	for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent)
		{
			AFPSTemplate_Muzzle* Muzzle = PartComponent->GetPart<AFPSTemplate_Muzzle>();
			if (IsValid(Muzzle))
			{
				MuzzleAttachment = Muzzle;
			}
		}
	}
}
