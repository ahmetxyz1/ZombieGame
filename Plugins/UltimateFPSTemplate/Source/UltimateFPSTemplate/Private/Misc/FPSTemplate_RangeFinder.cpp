// Copyright 2021, Dakota Dawe, All rights reserved

#include "Misc/FPSTemplate_RangeFinder.h"

#include "Components/TextRenderComponent.h"

AFPSTemplate_RangeFinder::AFPSTemplate_RangeFinder()
{
	Magnifications = {6.0f};

	MaxDistance = 999.0f;
	MinDistance = 8.0f;
	UnitType = EMeasurementType::Metric;
	bAppendUnitType = true;
	bUseUpper = true;

	CachedRange = FString("");
	UnitTypeConverter = 100.0f;
}

void AFPSTemplate_RangeFinder::BeginPlay()
{
	Super::BeginPlay();
	if (UnitType == EMeasurementType::Imperial)
	{
		UnitTypeConverter = 91.44f;
	}
	SetRangeByValue(0.0f);
}

void AFPSTemplate_RangeFinder::SetupPartMesh()
{
	Super::SetupPartMesh();
	
	TArray<UTextRenderComponent*> ActorTextComponents;
	GetComponents<UTextRenderComponent>(ActorTextComponents);
	for (UTextRenderComponent* TextComponent : ActorTextComponents)
	{
		if (IsValid(TextComponent) && TextComponent->ComponentHasTag(FName("FPSPart")))
		{
			TextRenderRange = TextComponent;
			ClearMeasure();
			break;
		}
	}
}

void AFPSTemplate_RangeFinder::SetRangeByValue(float Range)
{
	SetRangeByString(FString::SanitizeFloat(Range));
}

void AFPSTemplate_RangeFinder::SetRangeByString(FString RangeText)
{
	if (IsValid(TextRenderRange) && !RangeText.IsEmpty())
	{
		int32 DotIndex = -1;
		if (RangeText.FindChar('.', DotIndex))
		{
			FString NewString = RangeText.Left(DotIndex + 2);
			if (bAppendUnitType)
			{
				AppendUnitType(NewString);
			}
			TextRenderRange->SetText(FText::FromString(NewString));
		}
	}
}

void AFPSTemplate_RangeFinder::AppendUnitType(FString& CurrentString)
{
	FString AppendingString = FString(" M");
	if (UnitType == EMeasurementType::Imperial)
	{
		AppendingString[1] = 'Y';
	}
	if (!bUseUpper)
	{
		AppendingString = AppendingString.ToLower();
	}
	CurrentString.Append(AppendingString);
}

float AFPSTemplate_RangeFinder::Measure()
{
	if (IsValid(PartMesh) && PartMesh->DoesSocketExist(RangeFinderLaserSocket))
	{	// Perform Line Trace for Distance
		FVector Start = PartMesh->GetSocketLocation(RangeFinderLaserSocket);
		FVector End = Start + PartMesh->GetSocketRotation(RangeFinderLaserSocket).Vector() * (MaxDistance * UnitTypeConverter);
		
		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
		{
			if (HitResult.Distance > MinDistance * UnitTypeConverter)
			{
				SetRangeByValue(HitResult.Distance / UnitTypeConverter);
				return HitResult.Distance / UnitTypeConverter;
			}
		}
	}
	
	return -1.0f;
}

void AFPSTemplate_RangeFinder::ClearMeasure()
{
	if (IsValid(TextRenderRange))
	{
		CachedRange = TextRenderRange->Text.ToString();
		TextRenderRange->SetText(FText());
	}
}

void AFPSTemplate_RangeFinder::RestoreRange()
{
	if (IsValid(TextRenderRange))
	{
		SetRangeByString(CachedRange);
	}
}

void AFPSTemplate_RangeFinder::DisableRenderTargets(bool bDisable)
{
	Super::DisableRenderTarget(bDisable);
	if (bDisable)
	{
		ClearMeasure();
	}
	else
	{
		RestoreRange();
	}
}
