// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSTemplateDataTypes.h"

#include "FPSTemplate_PartBase.generated.h"

class UFPSTemplate_PartComponent;
class AFPSTemplateFirearm;
class UMeshComponent;

#define MAX_PartStack 10

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_PartBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplate_PartBase();

protected:
	UMeshComponent* PartMesh;
	virtual void SetupPartMesh();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	FFirearmPartStats PartStats;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	FFirearmPartData PartData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	EPartType PartType;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default", meta = (EditCondition = "PartType == EPartType::Stock"))
	float StockLengthOfPull;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TEnumAsByte<ECollisionChannel> FirearmCollisionChannel;
	
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Aim")
	bool bIsAimable;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Aim")
	FName AimSocket;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "FPSTemplate | Parts")
	TArray<UFPSTemplate_PartComponent*> PartComponents;

	UFPSTemplate_PartComponent* OwningPartComponent;

	float MinOffset;
	float MaxOffset;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentOffset)
	float CurrentOffset;
	UFUNCTION()
	void OnRep_CurrentOffset();
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetOffset(float Offset);
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Events")
	void OnUse();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Use();
	virtual void Server_Use_Implementation();

	bool bHasRenderTarget;
	
public:
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Stats")
	virtual FFirearmPartStats GetPartStats();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Stats")
	FFirearmPartData GetPartData() const { return PartData; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	EPartType GetPartType() const { return PartType; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	AFPSTemplateFirearm* GetOwningFirearm();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	class UFPSTemplate_CharacterComponent* GetOwningCharacterComponent();

	TArray<UFPSTemplate_PartComponent*> GetPartComponents();

	void PartsUpdated();
	virtual void CacheParts() {}

	void SetMinMaxOffset(float Min, float Max);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Default")
	bool SetOffset(float Offset);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Default")
	void FinishedMovingPart();
	
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	float GetPartOffset() const { return CurrentOffset; }

	UFUNCTION(BlueprintNativeEvent, Category = "FPSTemplate | Events")
	void OnPartRemoved(class UFPSTemplate_PartComponent* PartComponent);

	UFUNCTION(BlueprintNativeEvent, Category = "FPSTemplate | Use")
	void Use();
	virtual void Use_Implementation();
	
	TArray<UFPSTemplate_PartComponent*> GetSightComponents();

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Aiming")
	bool IsAimable() const { return bIsAimable; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Aiming")
	void DisableAiming() { bIsAimable = false;}
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Aiming")
	void EnableAiming();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Aiming")
	virtual FTransform GetAimSocketTransform() const;
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Aiming")
	float GetStockLengthOfPull() const { return StockLengthOfPull; }

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Render")
	bool HasRenderTarget() const { return bHasRenderTarget; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Render")
	virtual void DisableRenderTarget(bool Disable) {}

	void SetOwningPartComponent(UFPSTemplate_PartComponent* PartComponent) { OwningPartComponent = PartComponent; }
};
