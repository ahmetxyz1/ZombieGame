//Copyright 2021, Dakota Dawe, All rights reserved

#include "FPSTemplateAnimInstance.h"
#include "Actors/FPSTemplateFirearm.h"
#include "FirearmParts/BaseClasses/FPSTemplate_SightBase.h"
#include "Components/FPSTemplate_CharacterComponent.h"

#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Curves/CurveVector.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Misc/FPSTemplateStatics.h"

UFPSTemplateAnimInstance::UFPSTemplateAnimInstance()
{
	AimInterpolationSpeed = 20.0f;
	CycleSightsInterpolationSpeed = 20.0f;
	RotationLagResetInterpolationSpeed = 20.0f;
	MotionLagResetInterpolationSpeed = 20.0f;

	SpineBoneCount = 1;
	
	bInterpRelativeToHand = false;
	bFirstRun = true;
	bIsAiming = false;
	AimingAlpha = 0.0f;
	LeftHandIKAlpha = 1.0f;
	RotationAlpha = false;
	bInterpAiming = false;
	RightHandBone = FName("hand_r");

	CurrentLean = ELeaning::None;

	HeadAimingRotation = FRotator(45.0f, 0.0f, 0.0f);

	SightDistance = 0.0f;
	CurveTimer = 1.0f;
	bCustomizingFirearm = false;
	bCanAim = true;
	bInterpPortPose = false;
	bFreeLook = false;
	ShortStockPoseAlpha = 0.0f;
	bUseProceduralSpine = true;
	bValidLeftHandPose = false;
	bInvertRotationLag = false;

	MakeLeftHandFollowAlpha = 1.0f;

	SwayMultiplier = 1.0f;
}

void UFPSTemplateAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	
	if (const AActor* OwningActor = GetOwningActor())
	{
		CharacterComponent = Cast<UFPSTemplate_CharacterComponent>(OwningActor->GetComponentByClass(TSubclassOf<UFPSTemplate_CharacterComponent>()));
	}
}

void UFPSTemplateAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!CharacterComponent || bCustomizingFirearm) { return; }
	
	AimingActor = CharacterComponent->GetAimingActor();

	if (AimingActor)
	{
		SetRelativeToHand();

		Firearm = Cast<AFPSTemplateFirearm>(AimingActor);
		if (Firearm)
		{
			LeftHandPose = Firearm->GetGripAnimation();
			if (LeftHandPose)
			{
				bValidLeftHandPose = true;
			}
			else
			{
				bValidLeftHandPose = false;
			}
			
			if (Firearm->UseLeftHandIK())
			{
				SetLeftHandIK();
			}
			else
			{
				LeftHandIKAlpha = 0.0f;
			}
		}

		if (PortPose == EPortPose::None)
		{
			InterpRelativeToHand(DeltaSeconds);
		}
		
		if (bInterpAiming)
		{
			InterpAimingAlpha(DeltaSeconds);
		}
		
		if (CharacterComponent->IsLocallyControlled())
		{
			if (bInterpCameraZoom)
			{
				InterpCameraZoom(DeltaSeconds);
			}
			SetRotationLag(DeltaSeconds);
			if (CharacterComponent->GetMovementComponent()->Velocity.Size() || !MovementLagTransform.Equals(FTransform()))
			{
				SetMovementLag(DeltaSeconds);
			}

			HandleMovementSway(DeltaSeconds);
		}

		HandleSprinting();

		if (!FinalRecoilTransform.GetRotation().Rotator().Equals(FRotator::ZeroRotator) && !FinalRecoilTransform.GetLocation().Equals(FVector::ZeroVector))
		{
			RecoilInterpToZero(DeltaSeconds);
			RecoilInterpTo(DeltaSeconds);
		}

		if (bInterpPortPose)
		{
			InterpPortPose(DeltaSeconds);
		}
		
		FVector Velocity = CharacterComponent->GetMovementComponent()->Velocity;
		Velocity.Z = 0.0f;
		CharacterVelocity = Velocity.Size();
	}

	if (bUseProceduralSpine)
	{
		if (!CharacterComponent->IsLocallyControlled())
		{
			SpineToInterpTo = CharacterComponent->GetBaseAimRotation();
			SpineToInterpTo.Roll = SpineToInterpTo.Pitch * -1.0f;
			SpineToInterpTo.Pitch = 0.0f;
			SpineToInterpTo.Yaw = 0.0f;
			SpineToInterpTo.Roll /= SpineBoneCount;
			SpineRotation = UKismetMathLibrary::RInterpTo(SpineRotation, SpineToInterpTo, DeltaSeconds, 10.0f);
		}
		else
		{
			SpineToInterpTo = CharacterComponent->GetControlRotation();
			SpineToInterpTo.Roll = SpineToInterpTo.Pitch * -1.0f;
			SpineToInterpTo.Pitch = 0.0f;
			SpineToInterpTo.Yaw = 0.0f;
			SpineToInterpTo.Roll /= SpineBoneCount;
			SpineRotation = SpineToInterpTo;
		}

		if (bInterpLeaning)
		{
			InterpLeaning(DeltaSeconds);
		}
	}
}

void UFPSTemplateAnimInstance::InterpRelativeToHand(float DeltaSeconds)
{
	// Change InterpSpeed to weight of firearm
	if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
	{
		AnimationIndex = AimInterface->GetAnimationIndex();
	
		float InterpSpeed = AimInterpolationSpeed;
		float Multiplier = AimInterface->GetInterpolationMultiplier();
		Multiplier = UKismetMathLibrary::NormalizeToRange(Multiplier, -40.0f, 150.0f);
		Multiplier = FMath::Clamp(Multiplier, 0.0f, 1.0f);
		InterpSpeed *= Multiplier;
	
		// Change InterpSpeed to be modified by firearm in hand
		RelativeToHandTransform = UKismetMathLibrary::TInterpTo(RelativeToHandTransform, FinalRelativeHand, DeltaSeconds, InterpSpeed);
	
		float HandToSightDistance = 0.0f;
		HandToSightDistance = FinalRelativeHand.GetLocation().X;
		bool UseFixedCameraDistance = false;
		if (CharacterComponent->IsLocallyControlled())
		{
			if (AFPSTemplate_SightBase* Sight = AimInterface->GetActiveSight())
			{
				UseFixedCameraDistance = Sight->UseFixedCameraDistance();
				if (UseFixedCameraDistance)
				{
					HandToSightDistance = Sight->GetCameraDistance();
				}
				else
				{
					HandToSightDistance -= Sight->GetCameraDistance();
				}
			}
		}

		if (!UseFixedCameraDistance && IsValid(Firearm))
		{
			HandToSightDistance -= Firearm->GetStockLengthOfPull() / 2.0f;
		}
		SightDistance = UKismetMathLibrary::FInterpTo(SightDistance, HandToSightDistance * -1.0f, DeltaSeconds, InterpSpeed);
		SetSightTransform();

		if (RelativeToHandTransform.Equals(FinalRelativeHand))
		{
			bInterpRelativeToHand = false;
			SightDistance = HandToSightDistance * -1.0f;
			SetSightTransform();
		}
	}
	else
	{
		AnimationIndex = 0;
	}
}

void UFPSTemplateAnimInstance::SetSightTransform()
{
	FTransform CameraTransform = CharacterComponent->GetInUseMesh()->GetSocketTransform(CharacterComponent->GetCameraSocket(), RTS_ParentBoneSpace);
	const FTransform NewTransform = CharacterComponent->GetCameraComponent()->GetSocketTransform(NAME_None, RTS_ParentBoneSpace);
	FRotator NewRot = NewTransform.GetRotation().Rotator();
	NewRot.Roll += -90.0f;
	NewRot.Yaw += 90.0f;
	CameraTransform.SetRotation(NewRot.Quaternion());
	
	FVector CameraVector = CameraTransform.GetLocation();
	CameraVector.Y += SightDistance + 10.0f;

	if (!CharacterComponent->IsLocallyControlled() && IsValid(Firearm))
	{
		FVector StockOffset = FVector(-8.0f, 0.0f, 2.0f);
		StockOffset = Firearm->GetStockOffset() * -1.0f;
		StockOffset.Y += Firearm->GetStockLengthOfPull();
		
		CameraVector.X += StockOffset.Z * -1.0f;
		CameraVector.Z += StockOffset.X;
		CameraVector.Y += StockOffset.Y;
	}

	CameraTransform.SetLocation(CameraVector);
	
	/*CameraTransform = CharacterComponent->GetInUseMesh()->GetSocketTransform(CharacterComponent->GetCameraSocket(), RTS_Component);
	
	FVector TestVector = CameraTransform.GetLocation();
	//TestVector.Y += 10.0f;
	
	FTransform TestTrans = CharacterComponent->GetInUseMesh()->GetSocketTransform(CharacterComponent->GetCameraSocket(), RTS_World);
	TestVector = TestTrans.GetLocation() + TestTrans.Rotator().Vector() * 10.0f;
	FTransform ToBeRelative = CharacterComponent->GetInUseMesh()->GetSocketTransform(CharacterComponent->GetCameraSocket(), RTS_World);
	FTransform NewTrans = UKismetMathLibrary::MakeRelativeTransform(TestTrans, ToBeRelative);
	FVector Location = NewTrans.GetLocation();
	Location.Z += 160.0f;
	Location.Y += 30.0f;
	CameraTransform.SetLocation(Location);
	
	FRotator Rot = FRotator::ZeroRotator;
	Rot.Yaw += 90.0f;
	//Rot -= CameraTransform.Rotator();
	
	CameraTransform.SetRotation(Rot.Quaternion());*/
	SightTransform = CameraTransform;
}

void UFPSTemplateAnimInstance::SetRelativeToHand()
{
	if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
	{
		HeadAimingRotation = AimInterface->GetHeadRotation();
		//HeadAimingRotation = FRotator(45.0f, 0.0f, 0.0f);
		FTransform SightSocketTransform = FTransform();
		if (SprintAlpha > 0.0f)
		{
			SightSocketTransform = AimInterface->GetDefaultSightSocketTransform();
		}
		else
		{
			SightSocketTransform = AimInterface->GetSightSocketTransform();
		}
		const FTransform Hand_RTransform = CharacterComponent->GetInUseMesh()->GetSocketTransform(RightHandBone);
		FinalRelativeHand = UKismetMathLibrary::MakeRelativeTransform(SightSocketTransform, Hand_RTransform);
		
		FVector TestLoc = SightSocketTransform.GetLocation();
		TestLoc.Z += 80.0f;
		SightSocketTransform.SetLocation(TestLoc);
		
		const FTransform DefaultTransform = AimInterface->GetDefaultSightSocketTransform();
		DefaultRelativeToHand = UKismetMathLibrary::MakeRelativeTransform(DefaultTransform, Hand_RTransform);
		bInterpRelativeToHand = true;
	}
}

void UFPSTemplateAnimInstance::SetLeftHandIK()
{
	if (IsValid(Firearm))
	{
		const FTransform LeftHandIK = Firearm->GetLeftHandIKTransform();
		if (LeftHandIK.GetLocation().Equals(FVector::ZeroVector)) { LeftHandIKAlpha = 0.0f; return; }
		FVector OutPosition;
		FRotator OutRotation;

		CharacterComponent->GetInUseMesh()->TransformToBoneSpace(RightHandBone, LeftHandIK.GetLocation(), LeftHandIK.Rotator(), OutPosition, OutRotation);

		LeftHandIKTransform.SetLocation(OutPosition);
		LeftHandIKTransform.SetRotation(OutRotation.Quaternion());
		LeftHandIKTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
}

void UFPSTemplateAnimInstance::InterpPortPose(float DeltaSeconds)
{
	if (Firearm)
	{
		FTransform InterpTo = SightTransform;
		switch (PortPose)
		{
		case EPortPose::High: InterpTo = Firearm->GetHighPortPose(); break;
		case EPortPose::Low: InterpTo = Firearm->GetLowPortPose(); break;
		default: InterpTo = FTransform();
		}
	
		CurrentPose = UKismetMathLibrary::TInterpTo(CurrentPose, InterpTo, DeltaSeconds, 8.0f);
		
		if (CurrentPose.Equals(InterpTo, 12.0f) && PortPose == EPortPose::None)
		{
			PortPoseAlpha = 0.0f;
		}
		
		if (CurrentPose.Equals(InterpTo))
		{
			bInterpPortPose = false;
		}
	}
}

void UFPSTemplateAnimInstance::SetPortPose(EPortPose Pose)
{
	if (PortPose != Pose)
	{
		PortPose = Pose;
		if (PortPose != EPortPose::None)
		{
			CurrentPose = FTransform();
			SetIsAiming(false);
		}

		RelativeToHandTransform = DefaultRelativeToHand;
		PortPoseAlpha = 1.0f;
		bInterpPortPose = true;
	}
}

void UFPSTemplateAnimInstance::SetPortPoseBlend(EPortPose Pose, float Alpha)
{
	if (Firearm)
	{
		PortPose = Pose;
		PortPoseAlpha = Alpha;
	
		if (Pose == EPortPose::None)
		{
			return;
		}

		SetIsAiming(false);

		/*Alpha = FMath::Clamp(Alpha, 0.01f, 1.0f);
		FVector ShortPose = Firearm->GetShortStockPose() * Alpha;

		if (FVector::Distance(ShortStockPose, ShortPose) < Firearm->GetShortStockDistance() / 2.0f)
		{
			PortPose = EPortPose::None;
		}*/
	
		RelativeToHandTransform = DefaultRelativeToHand;
		bInterpPortPose = true;
	}
}

bool UFPSTemplateAnimInstance::HandleFirearmCollision(EPortPose Pose, float Alpha)
{
	if (Firearm)
	{
		if (PortPose == EPortPose::None)
		{
			FVector ShortPose = Firearm->GetShortStockPose();
			//Alpha = FMath::Clamp(Alpha, 0.01f, 1.0f);
			ShortStockPose = UKismetMathLibrary::VInterpTo(ShortStockPose, ShortPose * Alpha, GetWorld()->DeltaTimeSeconds, 10.0f);
			float Diff = FVector::Distance(ShortPose, ShortStockPose);
			
			ShortStockPoseAlpha = 1.0f;
			if (ShortStockPose.Equals(ShortPose, 5.0f))
			{
				ShortStockPoseAlpha = 0.0f;
				ShortStockPose = FVector::ZeroVector;
				SetPortPoseBlend(Pose, Alpha);
				return true;
			}
			return false;
		}
		else
		{
			ShortStockPoseAlpha = 0.0f;
			ShortStockPose = FVector::ZeroVector;
			SetPortPoseBlend(Pose, Alpha);
			return true;
		}
	}
	return false;
}

void UFPSTemplateAnimInstance::EnableLeftHandIK(bool Enable)
{
	if (Enable)
	{
		LeftHandIKAlpha = 1.0f;
	}
	else
	{
		LeftHandIKAlpha = 0.0f;
	}
}

void UFPSTemplateAnimInstance::InterpCameraZoom(float DeltaSeconds)
{
	float CurrentFOV = CharacterComponent->GetCameraComponent()->FieldOfView;
	float TargetFOV = CharacterComponent->GetDefaultFOV();
	float InterpSpeed = 10.0f;
	if (bIsAiming && IsValid(AimingActor))
	{
		if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
		{
			TargetFOV -= AimInterface->GetCameraFOVZoom();
			InterpSpeed = AimInterface->GetCameraFOVZoomSpeed();
		}
	}
	CurrentFOV = UKismetMathLibrary::FInterpTo(CurrentFOV, TargetFOV, DeltaSeconds, InterpSpeed);
	CharacterComponent->GetCameraComponent()->SetFieldOfView(CurrentFOV);
	if (CurrentFOV == TargetFOV)
	{
		bInterpCameraZoom = false;
	}
}

void UFPSTemplateAnimInstance::InterpAimingAlpha(float DeltaSeconds)
{
	if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
	{
		// Change InterpSpeed to weight of firearm
		float InterpSpeed = AimInterpolationSpeed;
		float Multiplier = AimInterface->GetInterpolationMultiplier();
		Multiplier = UKismetMathLibrary::NormalizeToRange(Multiplier, -40.0f, 150.0f);
		//Multiplier = FMath::Clamp(Multiplier, 0.0f, 1.35f);// EXPOSE THIS TO FIREARM LATER
		InterpSpeed *= Multiplier;

		AimingAlpha = UKismetMathLibrary::FInterpTo(AimingAlpha, bIsAiming, DeltaSeconds, InterpSpeed);
		
		if ((bIsAiming && AimingAlpha >= 1.0f) || (!bIsAiming && AimingAlpha <= 0.0f))
		{
			bInterpAiming = false;
		}
	}
}

void UFPSTemplateAnimInstance::SetRotationLag(float DeltaSeconds)
{
	if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
	{
		float InterpSpeed = RotationLagResetInterpolationSpeed;
		float Multiplier = AimInterface->GetInterpolationMultiplier();
		Multiplier = UKismetMathLibrary::NormalizeToRange(Multiplier, -40.0f, 150.0f);
		//Multiplier = FMath::Clamp(Multiplier, 0.0f, 1.0f);
		InterpSpeed *= Multiplier;
	
		const FRotator CurrentRotation = CharacterComponent->GetControlRotation();
		FRotator Rotation = UKismetMathLibrary::RInterpTo(UnmodifiedRotationLagTransform.Rotator(), CurrentRotation - OldRotation, DeltaSeconds, InterpSpeed);
		UnmodifiedRotationLagTransform.SetRotation(Rotation.Quaternion());

		float FirearmWeightMultiplier = 1.0f;
		if (IsValid(Firearm))
		{
			// Modify on HELD weapon weight	
			FirearmWeightMultiplier = UKismetMathLibrary::NormalizeToRange(Firearm->GetFirearmStats().Weight, 0.0f, 25.0f);
			FirearmWeightMultiplier = FirearmWeightMultiplier * (0.3f / Multiplier);
		}
		
		Rotation *= 3.0f;
		Rotation *= FirearmWeightMultiplier;

		float InvertMultiplier = -1.0f;
		Rotation.Roll = Rotation.Pitch;
		Rotation.Pitch = 0.0f;
		if (bInvertRotationLag)
		{
			InvertMultiplier = 1.0f;
		}
		Rotation.Yaw = FMath::Clamp(Rotation.Yaw, -7.0f, 7.0f) * InvertMultiplier;	
		Rotation.Roll = FMath::Clamp(Rotation.Roll, -3.0f, 3.0f) * -InvertMultiplier;

		RotationLagTransform.SetRotation(Rotation.Quaternion());
		RotationLagTransform.SetLocation(FVector(Rotation.Yaw / 4.0f, 0.0f, Rotation.Roll / 1.5));

		OldRotation = CurrentRotation;
	}
}

void UFPSTemplateAnimInstance::SetMovementLag(float DeltaSeconds)
{
	if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
	{
		const FVector Velocity = CharacterComponent->GetMovementComponent()->Velocity;
		float FowardSpeed = FVector::DotProduct(Velocity, CharacterComponent->GetActorForwardVector());
		float RightSpeed = FVector::DotProduct(Velocity, CharacterComponent->GetActorRightVector());
		float VerticalSpeed = Velocity.Z;
		FowardSpeed = UKismetMathLibrary::NormalizeToRange(FowardSpeed, 0.0f, 5.0f);
		RightSpeed = UKismetMathLibrary::NormalizeToRange(RightSpeed, 0.0f, 75.0f);
		VerticalSpeed = UKismetMathLibrary::NormalizeToRange(VerticalSpeed, 0.0f, 75.0f);
	
		float InterpSpeed = MotionLagResetInterpolationSpeed;
		float Multiplier = AimInterface->GetInterpolationMultiplier();
		Multiplier = UKismetMathLibrary::NormalizeToRange(Multiplier, -40.0f, 150.0f);
		Multiplier = FMath::Clamp(Multiplier, 0.0f, 1.0f);
		InterpSpeed *= Multiplier;
		InterpSpeed *= 0.1f;

		FRotator NewRot = MovementLagTransform.GetRotation().Rotator();
		NewRot.Pitch = UKismetMathLibrary::FInterpTo(NewRot.Pitch, RightSpeed, DeltaSeconds, 10.0f);
		NewRot.Roll = UKismetMathLibrary::FInterpTo(NewRot.Roll, VerticalSpeed, DeltaSeconds, 10.0f);
	
		MovementLagTransform.SetRotation(NewRot.Quaternion());
	}
}

void UFPSTemplateAnimInstance::InterpLeaning(float DeltaSeconds)
{
	float LeanAngle = 0.0f;
	const float NewLeanAngle = CharacterComponent->GetLeanAngle() / 2.0f;
	float CurrentLeanAngle = LeanRotation.Pitch;
	switch (CurrentLean)
	{
		case ELeaning::None : break;
		case ELeaning::Left : LeanAngle = NewLeanAngle * -1.0f; break;
		case ELeaning::Right : LeanAngle = NewLeanAngle; break;
	}

	CurrentLeanAngle = UKismetMathLibrary::FInterpTo(CurrentLeanAngle, LeanAngle, DeltaSeconds, 10.0f);
	
	LeanRotation.Pitch = CurrentLeanAngle;
	if (CurrentLeanAngle == NewLeanAngle || CurrentLeanAngle == NewLeanAngle * -1.0f)
	{
		bInterpLeaning = false;
	}
}

void UFPSTemplateAnimInstance::HandleMovementSway(float DeltaSeconds)
{	// REWRITE FOR USE WITH AIMING ACTOR AS WELL???
	if (IsValid(Firearm))
	{
		if (UCurveVector* Curve = Firearm->GetMovementSwayCurve())
		{
			const float OldVelocityMultiplier = VelocityMultiplier;
			VelocityMultiplier = UKismetMathLibrary::NormalizeToRange(CharacterVelocity, 0.0f, CharacterComponent->GetSprintSpeed());
			if (VelocityMultiplier < OldVelocityMultiplier)
			{
				VelocityMultiplier = UKismetMathLibrary::FInterpTo(OldVelocityMultiplier, VelocityMultiplier, DeltaSeconds, 3.2f);
			}
			if (VelocityMultiplier < 0.25f)
			{
				VelocityMultiplier = 0.25f;
			}
			float Multiplier = 1.1f;
			if (Firearm->UseStatsForMovementSway())
			{
				FFirearmStats Stats = Firearm->GetFirearmStats();
				Multiplier = Stats.Weight * (Stats.Weight * 3.0f / (Stats.Ergonomics * 1.5f)) + 1.0f;
				Multiplier = FMath::Clamp(Multiplier, 0.5f, 1.0f);
			}

			SwayMultiplier = UKismetMathLibrary::FInterpTo(SwayMultiplier, Firearm->GetAimSwayMultiplier(), DeltaSeconds, 2.0f);
			
			CurveTimer += (DeltaSeconds * VelocityMultiplier);
			FVector Graph = Curve->GetVectorValue(CurveTimer);
			Graph *= VelocityMultiplier * Multiplier;
			FRotator Rotation = FRotator(Graph.Y, Graph.X, Graph.Z);
			SwayTransform.SetLocation(Graph * SwayMultiplier);
			SwayTransform.SetRotation(Rotation.Quaternion() * SwayMultiplier);
		}
	}
}

void UFPSTemplateAnimInstance::HandleSprinting()
{
	if (CharacterComponent->GetIsSprinting() && CharacterVelocity > CharacterComponent->GetSprintSpeed() / 2.0f)
	{
		SprintAlpha = 1.0f;
		SetIsAiming(false);
		if (IsValid(Firearm))
		{
			SprintPoseTransform = Firearm->GetSprintPose();
		}
	}
	else
	{
		if (CharacterComponent->GetIsAiming())
		{
			SetIsAiming(true);
		}
		SprintAlpha = 0.0f;
	}
}

void UFPSTemplateAnimInstance::SetIsAiming(bool IsAiming)
{
	if ((IsAiming && !bCanAim) || PortPoseAlpha || SprintAlpha)
	{
		bIsAiming = false;
		bInterpAiming = true;
		return;
	}

	if (bIsAiming != IsAiming)
	{
		bIsAiming = IsAiming;
		bInterpAiming = true;
		bInterpCameraZoom = true;
	}
}

void UFPSTemplateAnimInstance::SetLeaning(ELeaning Lean)
{
	CurrentLean = Lean;
	bInterpLeaning = true;
}

void UFPSTemplateAnimInstance::SetIsReloading(bool IsReloading, float BlendAlpha)
{
	if (IsReloading)
	{
		RotationAlpha = BlendAlpha;
		LeftHandIKAlpha = 0.0f;
	}
	else
	{
		RotationAlpha = 0.0f;
		LeftHandIKAlpha = 1.0f;
	}
}

void UFPSTemplateAnimInstance::SetCanAim(bool CanAim)
{
	bCanAim = CanAim;
}

void UFPSTemplateAnimInstance::ChangingFireMode(bool IsChanging)
{
	if (IsChanging)
	{
		RotationAlpha = 0.55f;
	}
	else
	{
		RotationAlpha = 0.0f;
	}
}

void UFPSTemplateAnimInstance::ChamberingRound(bool IsChambering)
{
	if (IsChambering)
	{
		RotationAlpha = 0.55f;
		LeftHandIKAlpha = 0.0f;
	}
	else
	{
		RotationAlpha = 0.55f;
		LeftHandIKAlpha = 1.0f;
	}
}

void UFPSTemplateAnimInstance::CycledSights()
{
	SetRelativeToHand();
	bInterpCameraZoom = true;
}

void UFPSTemplateAnimInstance::StopMontages(float BlendOutTime)
{
	Montage_Stop(BlendOutTime);
}

void UFPSTemplateAnimInstance::SetCustomization(bool CustomizeFirearm)
{
	bCustomizingFirearm = CustomizeFirearm;
	if (bCustomizingFirearm)
	{
		LeftHandIKAlpha = 0.0f;
		AimingAlpha = 0.0f;
		PortPoseAlpha = 0.0f;
	}
	else
	{
		LeftHandIKAlpha = 1.0f;
	}
}

void UFPSTemplateAnimInstance::RecoilInterpToZero(float DeltaSeconds)
{
	FinalRecoilTransform = UKismetMathLibrary::TInterpTo(FinalRecoilTransform, FTransform(), DeltaSeconds, 8.0f); // def = 6
}

void UFPSTemplateAnimInstance::RecoilInterpTo(float DeltaSeconds)
{
	float Push = RecoilTransform.GetLocation().Y;
	FFloatSpringState SpringState;
	Push = UKismetMathLibrary::FloatSpringInterp(Push, FinalRecoilTransform.GetLocation().Y, SpringState,
		1.0f, 1.0f, DeltaSeconds, 1.0f);
	
	RecoilTransform = UKismetMathLibrary::TInterpTo(RecoilTransform, FinalRecoilTransform, DeltaSeconds, 25.0f);// def = 15
	FVector Vec = RecoilTransform.GetLocation();
	Vec.Y = Push;
	RecoilTransform.SetLocation(Vec);
}

void UFPSTemplateAnimInstance::PerformRecoil(float Multiplier)
{
	if (Firearm)
	{
		float TotalRecoil = 0.0f;
		Multiplier = FMath::Clamp(Multiplier, 0.01f, 4.0f);
		float Weight = 1.0f;
		if (Firearm->GetFirearmStats().Weight > 0.0f)
		{
			Weight = Firearm->GetFirearmStats().Weight;
		}
		float RecoilVerticalMultiplier = Firearm->GetFirearmStats().VerticalRecoil * (1 / (Weight / 2.0f));
		RecoilVerticalMultiplier = UKismetMathLibrary::NormalizeToRange(RecoilVerticalMultiplier, 0.0f, 160.0f);
		RecoilVerticalMultiplier *= Multiplier;
		TotalRecoil += RecoilVerticalMultiplier;
			
		float RecoilHorizontalMultiplier = Firearm->GetFirearmStats().HorizontalRecoil * (1 / (Weight / 2.0f));
		RecoilHorizontalMultiplier = UKismetMathLibrary::NormalizeToRange(RecoilHorizontalMultiplier, 0.0f, 260.0f);
		RecoilHorizontalMultiplier *= Multiplier;
		TotalRecoil += RecoilHorizontalMultiplier;
			
		FVector RecoilLoc = FinalRecoilTransform.GetLocation();
		RecoilLoc += FVector(0.0f, FMath::RandRange(-3.0f, -2.0f) * TotalRecoil, FMath::RandRange(0.0f, 0.5f) * TotalRecoil);

		FRotator RecoilRot = FinalRecoilTransform.GetRotation().Rotator();
		// Pitch = Roll,		Yaw = Yaw,		Roll = Pitch
		RecoilRot += FRotator(FMath::RandRange(-0.0f, 0.0f),
			FMath::RandRange(-1.0f, 2.5f) * RecoilHorizontalMultiplier,
			FMath::RandRange(-5.0f, -2.0f) * RecoilVerticalMultiplier);
		RecoilRot.Roll = FMath::Clamp(RecoilRot.Roll, -15.0f, 0.0f);

		FinalRecoilTransform.SetLocation(RecoilLoc);
		FinalRecoilTransform.SetRotation(RecoilRot.Quaternion());
	}
}