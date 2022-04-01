// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "FPSTemplate_ForwardGrip.generated.h"

class UAnimSequence;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_ForwardGrip : public AFPSTemplate_PartBase
{
	GENERATED_BODY()
public:
	AFPSTemplate_ForwardGrip();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Sockets")
	FName HandGripSocket;

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	UAnimSequence* GripAnimation;

	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	FTransform GetGripTransform() const;

	UAnimSequence* GetGripAnimation() const { return GripAnimation; }
};