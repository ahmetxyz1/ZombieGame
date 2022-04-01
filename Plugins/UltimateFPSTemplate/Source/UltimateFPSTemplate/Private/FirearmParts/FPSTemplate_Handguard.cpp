// Copyright 2021, Dakota Dawe, All rights reserved


#include "FirearmParts/FPSTemplate_Handguard.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Components/FPSTemplate_PartComponent.h"
#include "FirearmParts/FPSTemplate_ForwardGrip.h"

#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AFPSTemplate_Handguard::AFPSTemplate_Handguard()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PartStats.Weight = 1.0f;
	PartStats.ErgonomicsChangePercentage = 5.0f;

	PartType = EPartType::Handguard;
	
	ForwardGripSocket = FName("S_ForwardGrip");
	HandGripSocket = FName("S_LeftHandIK");
	SightSocket = FString("S_Sight");
}

// Called when the game starts or when spawned
void AFPSTemplate_Handguard::BeginPlay()
{
	Super::BeginPlay();
}

void AFPSTemplate_Handguard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSTemplate_Handguard, ForwardGrip);
}

UAnimSequence* AFPSTemplate_Handguard::GetGripAnimation() const
{
	if (IsValid(ForwardGrip))
	{
		return ForwardGrip->GetGripAnimation();
	}
	return GripAnimation;
}

FTransform AFPSTemplate_Handguard::GetGripTransform() const
{
	if (IsValid(ForwardGrip))
	{
		return ForwardGrip->GetGripTransform();
	}
	
	return IsValid(PartMesh) ? PartMesh->GetSocketTransform(HandGripSocket) : FTransform();
}

FTransform AFPSTemplate_Handguard::GetAimSocketTransform() const
{
	return IsValid(PartMesh) ? PartMesh->GetSocketTransform(AimSocket) : FTransform();
}

void AFPSTemplate_Handguard::CacheParts()
{
	for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent)
		{
			if (IsValid(PartComponent->GetPart()))
			{
				AFPSTemplate_ForwardGrip* Grip = PartComponent->GetPart<AFPSTemplate_ForwardGrip>();
				if (IsValid(Grip))
				{
					ForwardGrip = Grip;
				}
				else
				{
					for (UFPSTemplate_PartComponent* PartPartComponent : PartComponent->GetPart()->GetPartComponents())
					{
						if (PartPartComponent)
						{
							Grip = PartPartComponent->GetPart<AFPSTemplate_ForwardGrip>();
							if (IsValid(Grip))
							{
								ForwardGrip = Grip;
							}
						}
					}
				}
			}
		}
	}
}
