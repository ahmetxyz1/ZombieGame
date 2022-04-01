// Copyright 2021, Dakota Dawe, All rights reserved


#include "FirearmParts/BaseClasses/FPSTemplate_MagnifierBase.h"

#include "Net/UnrealNetwork.h"

AFPSTemplate_MagnifierBase::AFPSTemplate_MagnifierBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PartType = EPartType::Magnifier;
}

void AFPSTemplate_MagnifierBase::OnRep_FlippedOut()
{
	OnUse();
}

void AFPSTemplate_MagnifierBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AFPSTemplate_MagnifierBase, bFlippedOut, COND_SkipOwner);
}

bool AFPSTemplate_MagnifierBase::Server_Flip_Validate(bool bFlip)
{
	return true;
}

void AFPSTemplate_MagnifierBase::Server_Flip_Implementation(bool bFlip)
{
	Use();
}

void AFPSTemplate_MagnifierBase::Use_Implementation()
{
	bFlippedOut = !bFlippedOut;
	OnUse();
	
	if (!HasAuthority())
	{
		Server_Flip(bFlippedOut);
	}
}
