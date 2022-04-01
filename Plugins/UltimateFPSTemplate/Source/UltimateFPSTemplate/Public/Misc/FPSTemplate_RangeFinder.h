// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_SightMagnifiedRTBase.h"
#include "FPSTemplateDataTypes.h"
#include "FPSTemplate_RangeFinder.generated.h"

class UTextRenderComponent;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_RangeFinder : public AFPSTemplate_SightMagnifiedRTBase
{
	GENERATED_BODY()
public:
	AFPSTemplate_RangeFinder();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | RangeFinder")
	FName RangeFinderLaserSocket;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | RangeFinder")
	float MaxDistance;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | RangeFinder")
	float MinDistance;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | RangeFinder")
	EMeasurementType UnitType;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | RangeFinder")
	bool bAppendUnitType;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | RangeFinder", meta = (EditCondition = "bAppendUnitType"))
	bool bUseUpper;
	
	UPROPERTY(BlueprintReadWrite, Category = "FPSTemplate")
	UTextRenderComponent* TextRenderRange;

	FString CachedRange;

	float UnitTypeConverter;
	
	virtual void BeginPlay() override;
	virtual void SetupPartMesh() override;
	
	void RestoreRange();
	void AppendUnitType(FString& CurrentString);

public:
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | RangeFinder")
	void SetRangeByValue(float Range);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | RangeFinder")
	void SetRangeByString(FString RangeText);

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | RangeFinder")
	float Measure();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | RangeFinder")
	void ClearMeasure();

	virtual void DisableRenderTargets(bool Disable) override;
};
