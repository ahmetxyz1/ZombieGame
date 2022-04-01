// Copyright 2021, Dakota Dawe, All rights reserved


#include "Components/FPSTemplate_CharacterComponent.h"
#include "FPSTemplateAnimInstance.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Misc/FPSTemplateStatics.h"
#include "Interfaces/FPSTemplate_AimInterface.h"

#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

UFPSTemplate_CharacterComponent::UFPSTemplate_CharacterComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	bLimitFirearmCollisionTickRate = true;
	FirearmCollisionTickRate = 60;
	
	FirearmCollisionChannel = ECC_GameTraceChannel2;
	
	bIsInitialized = false;
	bLocallyControlled = false;
	
	SprintSpeed = 350.0f;
	MaxLookUpAngle = 80.0f;
	MaxLookDownAngle = 80.0f;
	DefaultLeanAngle = 35.0f;
	LeanAngle = DefaultLeanAngle;
	DefaultCameraFOV = 90.0f;
	
	CameraSocket = FName("Camera");
	CameraSocketParentBone = FName("cc_Camera");

	MaxFirearmAttachmentAttempts = 5;
	FirearmReAttachmentAttemptInterval = 0.5f;
	AttachmentAttempt = 0;

	bLeftHandFollowIK = true;
	bUseLeftHandIK = true;
}

void UFPSTemplate_CharacterComponent::BeginPlay()
{
	Super::BeginPlay();
	if (bUseFirearmCollision)
	{
		if (bLimitFirearmCollisionTickRate)
		{
			PrimaryComponentTick.TickInterval = 1.0f / FirearmCollisionTickRate;
		}
	}
	else
	{
		SetComponentTickEnabled(false);
	}
}

void UFPSTemplate_CharacterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFPSTemplate_CharacterComponent, CurrentFirearm);
	DOREPLIFETIME(UFPSTemplate_CharacterComponent, AimingActor);
	DOREPLIFETIME(UFPSTemplate_CharacterComponent, LeanAngle);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, bLeftHandFollowIK, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, bUseLeftHandIK, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, bIsAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, CurrentLean, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, bIsSprinting, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, PortPose, COND_SkipOwner);
}

void UFPSTemplate_CharacterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bUseFirearmCollision && FPCameraComponent && CurrentFirearm && AnimationInstance && PortPose == EPortPose::None)
	{
		FVector Start = FPCameraComponent->GetComponentLocation();
		FVector Muzzle = CurrentFirearm->GetMuzzleSocketTransform().GetLocation();
		FRotator Rotation = FPCameraComponent->GetComponentRotation();
		if (!bIsAiming)
		{
			Rotation.Yaw += 8.0f;
			Rotation.Pitch -= 2.0f;
		}
		FVector End = Start + Rotation.Vector() * 150.0f;
		float DistanceToMuzzle = FVector::Distance(Start, Muzzle);

		FHitResult HitResult;		
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(GetOwner());

		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, -1.0f, 0, 1.0f);
		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, FirearmCollisionChannel, Params))
		{
			float HitDistance = HitResult.Distance - DistanceToMuzzle;

			EPortPose Pose = EPortPose::High;
			float Normalized = 0.0f;
			float NeededCollisionDistance = 15.0f;
			if (HitDistance < NeededCollisionDistance)
			{
				HitDistance *= -1.0f;
				Normalized = UKismetMathLibrary::NormalizeToRange(HitDistance, NeededCollisionDistance * -1.0f, -5.0f);
				Normalized = FMath::Clamp(Normalized,0.0f, 1.0f);

				float Pitch = GetBaseAimRotation().Pitch;
				if (IsLocallyControlled())
				{
					if (Pitch < 0.0f)
					{
						Pose = EPortPose::Low;
					}
				}
				else
				{
					if (Pitch > 89.0f)
					{
						Pose = EPortPose::Low;
					}
				}
				bFirearmCollisionHitting = AnimationInstance->HandleFirearmCollision(Pose, Normalized);
			}
			else if (HitDistance > NeededCollisionDistance + 10.0f)
			{
				Normalized = 0.0f;
				Pose = EPortPose::None;
				bFirearmCollisionHitting = AnimationInstance->HandleFirearmCollision(Pose, Normalized);
				if (GetIsAiming())
				{
					AnimationInstance->SetIsAiming(true);
				}
			}
		}
		else
		{
			bFirearmCollisionHitting = AnimationInstance->HandleFirearmCollision(EPortPose::None, 0.0f);
		}
	}
}

void UFPSTemplate_CharacterComponent::Init(UCameraComponent* CameraComponent, USkeletalMeshComponent* FirstPersonMesh,
	USkeletalMeshComponent* ThirdPersonMesh)
{
	LeanAngle = DefaultLeanAngle;
	FPCameraComponent = CameraComponent;
	FPMesh = FirstPersonMesh;
	TPMesh = ThirdPersonMesh;

	OwningPawn = GetOwner<APawn>();

	if (OwningPawn)
	{
		if (OwningPawn->IsLocallyControlled())
		{
			AnimationInstance = Cast<UFPSTemplateAnimInstance>(FPMesh->GetAnimInstance());
			bLocallyControlled = true;
		}
		else
		{
			AnimationInstance = Cast<UFPSTemplateAnimInstance>(TPMesh->GetAnimInstance());
		}
	}

	if (AnimationInstance)
	{
		AnimationInstance->SetCharacterComponent(this);
	}

	if (FPCameraComponent)
	{
		DefaultCameraFOV = FPCameraComponent->FieldOfView;
		if (IsLocallyControlled())
		{
			FPCameraComponent->AttachToComponent(GetInUseMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, CameraSocket);
			GetInUseMesh()->SetOwnerNoSee(false);
			if (APlayerController* PC = GetOwner<APawn>()->GetController<APlayerController>())
			{
				if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
				{
					CameraManager->ViewPitchMax = MaxLookUpAngle;
					CameraManager->ViewPitchMin = MaxLookDownAngle * -1.0f;
				}
			}
		}
	}
	
	bIsInitialized = true;
}


bool UFPSTemplate_CharacterComponent::Server_SetPortPose_Validate(EPortPose NewPortPose)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SetPortPose_Implementation(EPortPose NewPortPose)
{
	PortPose = NewPortPose;
	OnRep_PortPose();
}

void UFPSTemplate_CharacterComponent::SetHighPortPose()
{
	if (!bFirearmCollisionHitting)
	{
		bHighPort = true;
		if (PortPose == EPortPose::Low)
		{
			PortPose = EPortPose::None;
		}
		else
		{
			PortPose = EPortPose::High;
		}
		
		OnRep_PortPose();
		if (!GetOwner()->HasAuthority())
		{
			Server_SetPortPose(PortPose);
		}
	}
}

void UFPSTemplate_CharacterComponent::SetLowPortPose()
{
	if (!bFirearmCollisionHitting)
	{
		bLowPort = true;
		if (PortPose == EPortPose::High)
		{
			PortPose = EPortPose::None;
		}
		else
		{
			PortPose = EPortPose::Low;
		}
		
		OnRep_PortPose();
		if (!GetOwner()->HasAuthority())
		{
			Server_SetPortPose(PortPose);
		}
	}
}

void UFPSTemplate_CharacterComponent::StopLowPortPose()
{
	if (!bFirearmCollisionHitting)
	{
		bLowPort = false;
		if (bHighPort)
		{
			PortPose = EPortPose::High;
		}
		else
		{
			PortPose = EPortPose::None;
		}
		
		OnRep_PortPose();
		if (!GetOwner()->HasAuthority())
		{
			Server_SetPortPose(PortPose);
		}
	}
}

void UFPSTemplate_CharacterComponent::StopHighPortPose()
{
	if (!bFirearmCollisionHitting)
	{
		bHighPort = false;
		if (bLowPort)
		{
			PortPose = EPortPose::Low;
		}
		else
		{
			PortPose = EPortPose::None;
		}
		
		OnRep_PortPose();
		if (!GetOwner()->HasAuthority())
		{
			Server_SetPortPose(PortPose);
		}
	}
}

void UFPSTemplate_CharacterComponent::OnRep_PortPose()
{
	if (AnimationInstance)
	{
		AnimationInstance->SetPortPose(PortPose);
	}
}

bool UFPSTemplate_CharacterComponent::Server_SetSprinting_Validate(bool IsSprinting)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SetSprinting_Implementation(bool IsSprinting)
{
	bIsSprinting = IsSprinting;
}

void UFPSTemplate_CharacterComponent::OnRep_IsAiming()
{
	if (AnimationInstance)
	{
		AnimationInstance->SetIsAiming(bIsAiming);
	}
}

void UFPSTemplate_CharacterComponent::OnRep_CurrentFirearm()
{
	if (IsValid(CurrentFirearm))
	{
		CurrentFirearm->AttachToSocket(CurrentFirearm->GetFirearmGripSocket());
		AttachItem(CurrentFirearm, CurrentFirearm->GetFirearmGripSocket());
		SetAimingActor(CurrentFirearm);
	}
}

void UFPSTemplate_CharacterComponent::AttachItem(AActor* Actor, const FName& SocketName)
{
	if (IsValid(Actor))
	{
		if (USkeletalMeshComponent* AttachToMesh = GetInUseMesh())
		{
			Actor->AttachToComponent(AttachToMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
			/*if (AFPSTemplateFirearm* Firearm = Cast<AFPSTemplateFirearm>(Actor))
			{
				Firearm->SetCharacterComponent(this);
				Firearm->CycleSights();
			}*/
			AttachmentAttempt = 0;
		}
		else
		{
			if (++AttachmentAttempt <= MaxFirearmAttachmentAttempts)
			{
				FTimerHandle TTempHandle;
				FTimerDelegate TimerDelegate;
				TimerDelegate.BindUFunction(this, FName("AttachItem"), Actor, SocketName);
				GetWorld()->GetTimerManager().SetTimer(TTempHandle, TimerDelegate, FirearmReAttachmentAttemptInterval, false);
			}
		}
	}
}

void UFPSTemplate_CharacterComponent::OnRep_AimingActor()
{
	if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
	{
		AttachItem(AimingActor, AimInterface->GetGripSocketName());
	}
}

void UFPSTemplate_CharacterComponent::OnRep_Lean()
{
	if (AnimationInstance)
	{
		AnimationInstance->SetLeaning(CurrentLean);
	}
}

void UFPSTemplate_CharacterComponent::OnRep_LeftHandFollowIK()
{
	if (AnimationInstance)
	{
		AnimationInstance->SetLeftHandFollow(bLeftHandFollowIK);
	}
}

void UFPSTemplate_CharacterComponent::OnRep_UseLeftHandIK()
{
	if (AnimationInstance)
	{
		AnimationInstance->EnableLeftHandIK(bUseLeftHandIK);
	}
}

USkeletalMeshComponent* UFPSTemplate_CharacterComponent::GetInUseMesh()
{
	if (OwningPawn)
	{
		if (OwningPawn->IsLocallyControlled())
		{
			return FPMesh;
		}
		return TPMesh;
	}

	return TPMesh;
}

UPawnMovementComponent* UFPSTemplate_CharacterComponent::GetMovementComponent() const
{
	if (OwningPawn)
	{
		return OwningPawn->GetMovementComponent();
	}
	return nullptr;
}

FRotator UFPSTemplate_CharacterComponent::GetBaseAimRotation() const
{
	if (OwningPawn)
	{
		return OwningPawn->GetBaseAimRotation();
	}
	return FRotator::ZeroRotator;
}

FRotator UFPSTemplate_CharacterComponent::GetControlRotation() const
{
	if (OwningPawn)
	{
		return OwningPawn->GetControlRotation();
	}
	return FRotator::ZeroRotator;
}

const FName& UFPSTemplate_CharacterComponent::GetCameraSocket() const
{
	if (bUseParentSocketForAiming)
	{
		return CameraSocketParentBone;
	}
	return CameraSocket;
}

bool UFPSTemplate_CharacterComponent::Server_SetLeanAngle_Validate(float NewLeanAngle)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SetLeanAngle_Implementation(float NewLeanAngle)
{
	SetLeanAngle(NewLeanAngle);
}

void UFPSTemplate_CharacterComponent::SetLeanAngle(float NewLeanAngle)
{
	LeanAngle = NewLeanAngle >= 0.0f ? LeanAngle = NewLeanAngle : LeanAngle = DefaultLeanAngle;
	OnRep_Lean();
	if (!HasAuthority())
	{
		Server_SetLeanAngle(LeanAngle);
	}
}

void UFPSTemplate_CharacterComponent::ResetLeanAngle()
{
	LeanAngle = DefaultLeanAngle;
	OnRep_Lean();
	if (!HasAuthority())
	{
		Server_SetLeanAngle(LeanAngle);
	}
}

void UFPSTemplate_CharacterComponent::SetSprinting(bool Sprinting)
{
	bIsSprinting = Sprinting;
	if (!GetOwner()->HasAuthority())
	{
		Server_SetSprinting(bIsSprinting);
	}
}

FVector UFPSTemplate_CharacterComponent::GetActorForwardVector() const
{
	if (OwningPawn)
	{
		return OwningPawn->GetActorForwardVector();
	}
	return FVector::ZeroVector;
}

FVector UFPSTemplate_CharacterComponent::GetActorRightVector() const
{
	if (OwningPawn)
	{
		return OwningPawn->GetActorRightVector();
	}
	return FVector::ZeroVector;
}

void UFPSTemplate_CharacterComponent::RagdollCharacter()
{
	UFPSTemplateStatics::Ragdoll(FPMesh);
	UFPSTemplateStatics::Ragdoll(TPMesh);
}

void UFPSTemplate_CharacterComponent::RagdollCharacterWithForce(const FVector ImpactLocation, const float ImpactForce)
{
	UFPSTemplateStatics::RagdollWithImpact(FPMesh, ImpactLocation, ImpactForce);
	UFPSTemplateStatics::RagdollWithImpact(TPMesh, ImpactLocation, ImpactForce);
}

void UFPSTemplate_CharacterComponent::LeanLeft()
{
	bLeanLeftDown = true;
	if (CurrentLean == ELeaning::Right)
	{
		CurrentLean = ELeaning::None;
	}
	else
	{
		CurrentLean = ELeaning::Left;
	}
	
	if (AnimationInstance)
	{
		AnimationInstance->SetLeaning(CurrentLean);
	}
	if (!GetOwner()->HasAuthority())
	{
		Server_SetLean(CurrentLean);
	}
}

void UFPSTemplate_CharacterComponent::LeanRight()
{
	bLeanRightDown = true;
	if (CurrentLean == ELeaning::Left)
	{
		CurrentLean = ELeaning::None;
	}
	else
	{
		CurrentLean = ELeaning::Right;
	}

	if (AnimationInstance)
	{
		AnimationInstance->SetLeaning(CurrentLean);
	}
	if (!GetOwner()->HasAuthority())
	{
		Server_SetLean(CurrentLean);
	}
}

void UFPSTemplate_CharacterComponent::StopLeanLeft()
{
	bLeanLeftDown = false;
	if (bLeanRightDown)
	{
		CurrentLean = ELeaning::Right;
	}
	else
	{
		CurrentLean = ELeaning::None;
	}
	
	if (AnimationInstance)
	{
		AnimationInstance->SetLeaning(CurrentLean);
	}
	if (!GetOwner()->HasAuthority())
	{
		Server_SetLean(CurrentLean);
	}
}

void UFPSTemplate_CharacterComponent::StopLeanRight()
{
	bLeanRightDown = false;
	if (bLeanLeftDown)
	{
		CurrentLean = ELeaning::Left;
	}
	else
	{
		CurrentLean = ELeaning::None;
	}
	
	if (AnimationInstance)
	{
		AnimationInstance->SetLeaning(CurrentLean);
	}
	if (!GetOwner()->HasAuthority())
	{
		Server_SetLean(CurrentLean);
	}
}

bool UFPSTemplate_CharacterComponent::Server_SetLean_Validate(ELeaning Lean)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SetLean_Implementation(ELeaning Lean)
{
	CurrentLean = Lean;
	OnRep_Lean();
}

bool UFPSTemplate_CharacterComponent::Server_SetLeftHandFollowIK_Validate(bool bFollow)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SetLeftHandFollowIK_Implementation(bool bFollow)
{
	SetLeftHandFollowIK(bFollow);
}

void UFPSTemplate_CharacterComponent::SetLeftHandFollowIK(bool bFollow)
{
	if (bLeftHandFollowIK != bFollow)
	{
		bLeftHandFollowIK = bFollow;
		OnRep_LeftHandFollowIK();
		if (!HasAuthority())
		{
			Server_SetLeftHandFollowIK(bLeftHandFollowIK);
		}
	}
}

bool UFPSTemplate_CharacterComponent::Server_SetUseLeftHandIK_Validate(bool bUse)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SetUseLeftHandIK_Implementation(bool bUse)
{
	SetUseLeftHandIK(bUse);
}

void UFPSTemplate_CharacterComponent::SetUseLeftHandIK(bool bUse)
{
	if (bUseLeftHandIK != bUse)
	{
		bUseLeftHandIK = bUse;
		OnRep_UseLeftHandIK();
		if (!HasAuthority())
		{
			Server_SetUseLeftHandIK(bUseLeftHandIK);
		}
	}
}

void UFPSTemplate_CharacterComponent::PlayCameraShake(TSubclassOf<UCameraShakeBase> CameraShake, float Scale) const
{
	if (OwningPawn && CameraShake)
	{
		if (APlayerController* PC = OwningPawn->GetController<APlayerController>())
		{
			PC->ClientStartCameraShake(CameraShake, Scale);
		}
	}
}

AFPSTemplateFirearm* UFPSTemplate_CharacterComponent::AddFirearm(TSubclassOf<AFPSTemplateFirearm> FirearmClass, bool SetAsCurrent, const FName AttachToSocketName)
{
	if (FirearmClass && HasAuthority())
	{
		FActorSpawnParameters Params;
		Params.Owner = GetOwner();
		if (AFPSTemplateFirearm* Firearm = GetWorld()->SpawnActor<AFPSTemplateFirearm>(FirearmClass, Params))
		{
			Firearm->SetCharacterComponent(this);
			if (SetAsCurrent)
			{
				CurrentFirearm = Firearm;
				OnRep_CurrentFirearm();
			}
			else if (AttachToSocketName != NAME_None)
			{
				Firearm->AttachToSocket(AttachToSocketName);
			}
			return Firearm;
		}
	}
	return nullptr;
}

void UFPSTemplate_CharacterComponent::AddExistingFirearm(AFPSTemplateFirearm* Firearm, bool SetAsCurrent, const FName AttachToSocketName)
{
	if (Firearm && HasAuthority())
	{
		Firearm->SetCharacterComponent(this);
		Firearm->SetOwner(GetOwner());
		if (SetAsCurrent)
		{
			CurrentFirearm = Firearm;
			OnRep_CurrentFirearm();
		}
		else if (AttachToSocketName != NAME_None)
		{
			Firearm->AttachToSocket(AttachToSocketName);
		}
	}
}

float UFPSTemplate_CharacterComponent::GetMagnificationSensitivity() const
{
	if (bIsAiming)
	{
		if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
		{
			return 1.0f / AimInterface->GetCurrentMagnification();
		}
	}
	return 1.0f;
}

float UFPSTemplate_CharacterComponent::GetMagnificationSensitivityStartValue(float StartAtMagnification) const
{
	if (bIsAiming)
	{
		if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
		{
			if (AimInterface->GetCurrentMagnification() > StartAtMagnification)
			{
				return 1.0f / CurrentFirearm->GetOpticMagnification();
			}
		}
	}
	return 1.0f;
}

bool UFPSTemplate_CharacterComponent::Server_SwitchFirearm_Validate(AFPSTemplateFirearm* Firearm, const FName& Socket, bool HideCurrentFirearm)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SwitchFirearm_Implementation(AFPSTemplateFirearm* Firearm, const FName& Socket, bool HideCurrentFirearm)
{
	SwitchToFirearm(Firearm, Socket, HideCurrentFirearm);
}

AFPSTemplateFirearm* UFPSTemplate_CharacterComponent::SwitchToFirearm(AFPSTemplateFirearm* Firearm, const FName FirearmAttachSocket, bool HideCurrentFirearm)
{
	if (IsValid(Firearm))
	{
		if (IsValid(CurrentFirearm) && IsLocallyControlled())
		{
			CurrentFirearm->DisableAllRenderTargets(true);
		}
		
		TempFirearm = CurrentFirearm;
		if (HideCurrentFirearm)
		{
			CurrentFirearm->HideFirearm(true);
		}
		Firearm->HideFirearm(false);
		CurrentFirearm = Firearm;
		
		if (IsLocallyControlled() && bIsAiming)
		{
			CurrentFirearm->DisableAllRenderTargets(false);
		}
		
		OnRep_CurrentFirearm();
		if (!HasAuthority())
		{
			Server_SwitchFirearm(Firearm, FirearmAttachSocket, HideCurrentFirearm);
		}
		if (IsValid(TempFirearm) && FirearmAttachSocket != NAME_None)
		{
			TempFirearm->AttachToSocket(FirearmAttachSocket);
		}
		return TempFirearm;
	}
	return CurrentFirearm;
}

bool UFPSTemplate_CharacterComponent::Server_OnUnEquipCurrentFirearm_Validate()
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_OnUnEquipCurrentFirearm_Implementation()
{
	CurrentFirearm = nullptr;
}

void UFPSTemplate_CharacterComponent::OnUnEquipCurrentFirearm()
{
	if (!HasAuthority())
	{
		Server_OnUnEquipCurrentFirearm();
	}
	if (IsValid(CurrentFirearm))
	{
		CurrentFirearm->DisableAllRenderTargets(true);
	}
	if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
	{
		AimInterface->DisableRenderTargets(true);
	}
	CurrentFirearm = nullptr;
	AimingActor = nullptr;
}

void UFPSTemplate_CharacterComponent::StartAiming()
{
	bIsAiming = true;
	if (bFirearmCollisionHitting)
	{
		return;
	}
	if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
	{
		AimInterface->DisableRenderTargets(false);
	}
	if (AnimationInstance)
	{
		AnimationInstance->SetIsAiming(bIsAiming);
	}
	if (!GetOwner()->HasAuthority())
	{
		Server_SetAiming(bIsAiming);
	}
}

void UFPSTemplate_CharacterComponent::StopAiming()
{
	bIsAiming = false;
	if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
	{
		AimInterface->DisableRenderTargets(true);
	}
	if (AnimationInstance)
	{
		AnimationInstance->SetIsAiming(bIsAiming);
	}
	if (!GetOwner()->HasAuthority())
	{
		Server_SetAiming(bIsAiming);
	}
}

bool UFPSTemplate_CharacterComponent::Server_SetAiming_Validate(bool IsAiming)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SetAiming_Implementation(bool IsAiming)
{
	bIsAiming = IsAiming;
	OnRep_IsAiming();
}
