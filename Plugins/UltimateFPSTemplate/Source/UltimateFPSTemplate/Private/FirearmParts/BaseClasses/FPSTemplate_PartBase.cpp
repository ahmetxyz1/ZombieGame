// Copyright 2021, Dakota Dawe, All rights reserved


#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "Components/FPSTemplate_PartComponent.h"
#include "Actors/FPSTemplateFirearm.h"

#include "Net/UnrealNetwork.h"

// Sets default values
AFPSTemplate_PartBase::AFPSTemplate_PartBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	NetUpdateFrequency = 30.0f;

	FirearmCollisionChannel = ECC_GameTraceChannel2;
	
	PartStats.Weight = 0.22f;
	PartStats.ErgonomicsChangePercentage = 0.0f;
	PartStats.RecoilChangePercentage = 0.0f;

	PartType = EPartType::Other;

	bIsAimable = false;
	AimSocket = FName("S_Aim");

	bHasRenderTarget = false;
}

void AFPSTemplate_PartBase::SetupPartMesh()
{
	TArray<UMeshComponent*> ActorMeshComponents;
	GetComponents<UMeshComponent>(ActorMeshComponents);
	for (UMeshComponent* MeshComponent : ActorMeshComponents)
	{
		if (IsValid(MeshComponent) && MeshComponent->ComponentHasTag(FName("FPSPart")))
		{
			PartMesh = MeshComponent;
			PartMesh->SetCollisionResponseToChannel(FirearmCollisionChannel, ECR_Ignore);
			break;
		}
	}
}

// Called when the game starts or when spawned
void AFPSTemplate_PartBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		GetComponents<UFPSTemplate_PartComponent>(PartComponents);
	}
}

void AFPSTemplate_PartBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	SetupPartMesh();
}

void AFPSTemplate_PartBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSTemplate_PartBase, PartComponents);
	DOREPLIFETIME(AFPSTemplate_PartBase, CurrentOffset);
}

bool AFPSTemplate_PartBase::Server_Use_Validate()
{
	return true;
}

void AFPSTemplate_PartBase::Server_Use_Implementation()
{
	
}

void AFPSTemplate_PartBase::OnPartRemoved_Implementation(UFPSTemplate_PartComponent* PartComponent)
{
	Destroy();
	PartsUpdated();
}

FFirearmPartStats AFPSTemplate_PartBase::GetPartStats()
{
	FFirearmPartStats ReturnPartStats = PartStats;
	for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent && IsValid(PartComponent->GetPart()))
		{
			PartComponent->GetPart()->CacheParts();
			ReturnPartStats += PartComponent->GetPart()->GetPartStats();
		}
	}
	return ReturnPartStats;
}

AFPSTemplateFirearm* AFPSTemplate_PartBase::GetOwningFirearm()
{
	AActor* PartOwner = GetOwner();
	for (uint8 i = 0; i < MAX_PartStack; ++i)
	{
		if (PartOwner)
		{
			if (AFPSTemplateFirearm* Firearm = Cast<AFPSTemplateFirearm>(PartOwner))
			{
				return Firearm;
			}

			PartOwner = PartOwner->GetOwner();
		}
	}
	return nullptr;
}

UFPSTemplate_CharacterComponent* AFPSTemplate_PartBase::GetOwningCharacterComponent()
{
	AFPSTemplateFirearm* Firearm = GetOwningFirearm();
	if (Firearm)
	{
		if (UFPSTemplate_CharacterComponent* CharacterComponent = Firearm->GetCharacterComponent())
		{
			return CharacterComponent;
		}
	}
	return nullptr;
}

TArray<UFPSTemplate_PartComponent*> AFPSTemplate_PartBase::GetPartComponents()
{
	TArray<UFPSTemplate_PartComponent*> AllPartComponents = PartComponents;
	for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent && IsValid(PartComponent->GetPart()))
		{
			AllPartComponents.Append(PartComponent->GetPart()->GetPartComponents());
		}
	}
	return AllPartComponents;
}

void AFPSTemplate_PartBase::PartsUpdated()
{
	if (AFPSTemplateFirearm* Firearm = GetOwningFirearm())
	{
		Firearm->AddPartCache(OwningPartComponent);
		Firearm->PartsChanged();
	}
}

void AFPSTemplate_PartBase::SetMinMaxOffset(float Min, float Max)
{
	MinOffset = Min;
	MaxOffset = Max;
}

void AFPSTemplate_PartBase::OnRep_CurrentOffset()
{
	SetActorRelativeLocation(FVector(0.0f, CurrentOffset, 0.0f));
}

bool AFPSTemplate_PartBase::Server_SetOffset_Validate(float Offset)
{
	return true;
}

void AFPSTemplate_PartBase::Server_SetOffset_Implementation(float Offset)
{
	CurrentOffset = Offset;
	OnRep_CurrentOffset();
}

bool AFPSTemplate_PartBase::SetOffset(float Offset)
{
	bool MaxedOffset = false;
	CurrentOffset += Offset;
	if (CurrentOffset > MaxOffset)
	{
		CurrentOffset = MaxOffset;
		MaxedOffset = true;
	}
	if (CurrentOffset < MinOffset)
	{
		CurrentOffset = MinOffset;
		MaxedOffset = true;
	}
	SetActorRelativeLocation(FVector(0.0f, CurrentOffset, 0.0f));
	return MaxedOffset;
}

void AFPSTemplate_PartBase::FinishedMovingPart()
{
	if (!HasAuthority())
	{
		Server_SetOffset(CurrentOffset);
	}
}

void AFPSTemplate_PartBase::Use_Implementation()
{
	OnUse();
}

TArray<UFPSTemplate_PartComponent*> AFPSTemplate_PartBase::GetSightComponents()
{
	TArray<UFPSTemplate_PartComponent*> SightComponents;
	for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent && IsValid(PartComponent->GetPart()))
		{
			if (PartComponent->GetPart()->IsAimable())
			{
				SightComponents.Add(PartComponent);
			}
			SightComponents.Append(PartComponent->GetPart()->GetSightComponents());
		}
	}
	return SightComponents;
}

void AFPSTemplate_PartBase::EnableAiming()
{
	if (!GetAimSocketTransform().GetLocation().Equals(FVector::ZeroVector))
	{
		bIsAimable = true;
	}
}

FTransform AFPSTemplate_PartBase::GetAimSocketTransform() const
{
	return IsValid(PartMesh) ? PartMesh->GetSocketTransform(AimSocket) : FTransform();
}
