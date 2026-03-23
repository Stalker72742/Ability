// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "EffectObject.generated.h"

class UEffectData;
class UAbilityComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectExpired, UEffectObject*, InEffect);

/**
 * @brief Runtime instance of an effect.
 *        Holds its UEffectData config and drives the tick/expiry logic.
 *        Created via UEffectObject::Create() — never construct manually.
 */
UCLASS(BlueprintType)
class MODULARABILITYCOMPONENT_API UEffectObject : public UObject
{
    GENERATED_BODY()

public:

    UEffectObject(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

    // --- Config fields ---

    /** @brief Source data asset that describes this effect. */
    UPROPERTY()
    TObjectPtr<UEffectData> EffectData;

protected:

    // --- Runtime fields ---

    /** @brief Weak ref to the owning component. */
    UPROPERTY()
    TWeakObjectPtr<UAbilityComponent> OwnerComponent;

    /** @brief Elapsed time since the effect was applied. */
    float ElapsedTime = 0.0f;

    /** @brief Time since last tick was processed. */
    float TimeSinceLastTick = 0.0f;

    /** @brief True while the effect is active and ticking. */
    uint8 bIsActive : 1;

protected:

    // --- Service / helper functions ---

    /**
     * @brief Applies all modifiers from EffectData to the owner's attributes once.
     */
    void ApplyModifiers();

public:

    // --- Public functions ---

    /**
     * @brief Factory method. Creates and initialises a new effect instance.
     * @param InOuter       Outer object (typically the UAbilityComponent).
     * @param InData        Data asset describing the effect.
     * @param InOwner       The ability component that owns this effect.
     * @return              Ready-to-use effect instance.
     */
    UFUNCTION(BlueprintCallable, Category="Effect")
    static UEffectObject* Create(UObject* InOuter, UEffectData* InData, UAbilityComponent* InOwner);

    /**
     * @brief Called every frame/tick by the owning AbilityComponent.
     * @param InDeltaTime   Frame delta time in seconds.
     */
    void Tick(float InDeltaTime);

    /**
     * @brief Immediately deactivates the effect without broadcasting expiry.
     */
    void ForceRemove();

public:

    // --- Const getters ---

    /** @brief Returns the source data asset. */
    FORCEINLINE UEffectData* GetEffectData() const { return EffectData; }

    /** @brief Returns true if the effect is currently active. */
    FORCEINLINE bool IsActive() const { return bIsActive; }

public:

    // --- Event dispatchers ---

    /** @brief Fired when the effect expires naturally (duration elapsed). */
    UPROPERTY(BlueprintAssignable, Category="Events")
    FOnEffectExpired OnEffectExpired;
};