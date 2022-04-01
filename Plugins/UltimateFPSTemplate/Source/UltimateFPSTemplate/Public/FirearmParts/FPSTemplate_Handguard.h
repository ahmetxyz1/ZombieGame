// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "FPSTemplate_Handguard.generated.h"

class AFPSTemplate_ForwardGrip;
class AFPSTemplateFirearm;
class AFPSTemplateFirearm_Sight;
class UAnimSequence;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_Handguard : public AFPSTemplate_PartBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplate_Handguard();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName ForwardGripSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName HandGripSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FString SightSocket;

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	UAnimSequence* GripAnimation;
	
	UPROPERTY(Replicated)
	AFPSTemplate_ForwardGrip* ForwardGrip;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Attachments")
	AFPSTemplate_ForwardGrip* GetForwardGrip() const { return ForwardGrip; }
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	UAnimSequence* GetGripAnimation() const;
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	FTransform GetGripTransform() const;

	virtual FTransform GetAimSocketTransform() const override;

	virtual void CacheParts() override;
};