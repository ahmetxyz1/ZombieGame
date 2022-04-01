//Copyright 2021, Dakota Dawe, All rights reserved

#include "Actors/FPSTemplateCharacter.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Components/FPSTemplate_CharacterComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

AFPSTemplateCharacter::AFPSTemplateCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	GetMesh()->bCastHiddenShadow = true;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	
	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(GetCapsuleComponent());
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	CharacterComponent = CreateDefaultSubobject<UFPSTemplate_CharacterComponent>(TEXT("CharacterComponent"));

	GetCharacterMovement()->MaxAcceleration = 1365.0f;
	GetCharacterMovement()->BrakingFriction = 1.0f;
	GetCharacterMovement()->MaxWalkSpeed = 250.0f;

	SprintSpeed = 600.0f;
}

void AFPSTemplateCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	
}

void AFPSTemplateCharacter::PostInitProperties()
{
	Super::PostInitProperties();
	DefaultWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
}

void AFPSTemplateCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSTemplateCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSTemplateCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AFPSTemplateCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFPSTemplateCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AFPSTemplateCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFPSTemplateCharacter::LookUpAtRate);
}

void AFPSTemplateCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AFPSTemplateCharacter, bIsSprinting, COND_SkipOwner);
}

void AFPSTemplateCharacter::StartSprinting()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	bIsSprinting = true;

	if (!HasAuthority())
	{
		Server_SetSprinting(bIsSprinting);
	}
}

void AFPSTemplateCharacter::StopSprinting()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	bIsSprinting = false;
	if (!HasAuthority())
	{
		Server_SetSprinting(bIsSprinting);
	}
}

void AFPSTemplateCharacter::OnRep_IsSprinting()
{
	if (bIsSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	}
}

bool AFPSTemplateCharacter::Server_SetSprinting_Validate(bool IsSprinting)
{
	return true;
}

void AFPSTemplateCharacter::Server_SetSprinting_Implementation(bool IsSprinting)
{
	bIsSprinting = IsSprinting;
	OnRep_IsSprinting();
}

void AFPSTemplateCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFPSTemplateCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFPSTemplateCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	if (Rate != 0.0f)
	{
		Rate *= CharacterComponent->GetMagnificationSensitivity();
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void AFPSTemplateCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	if (Rate != 0.0f)
	{
		Rate *= CharacterComponent->GetMagnificationSensitivity();
		AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
}