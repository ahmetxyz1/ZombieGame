// Copyright 2021, Dakota Dawe, All rights reserved


#include "FirearmParts/FPSTemplate_ForwardGrip.h"
#include "FirearmParts/FPSTemplate_Handguard.h"
#include "Actors/FPSTemplateFirearm.h"

AFPSTemplate_ForwardGrip::AFPSTemplate_ForwardGrip()
{

	PartStats.Weight = 0.06f;
	PartStats.ErgonomicsChangePercentage = 10.0f;
	PartStats.RecoilChangePercentage = -8.0f;

	HandGripSocket = FName("S_LeftHandIK");

	PartType = EPartType::ForwardGrip;
}

void AFPSTemplate_ForwardGrip::BeginPlay()
{
	Super::BeginPlay();
	
}

FTransform AFPSTemplate_ForwardGrip::GetGripTransform() const
{
	return IsValid(PartMesh) ? PartMesh->GetSocketTransform(HandGripSocket) : FTransform();
}