// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "EffectData.generated.h"

/**
 * @brief How the effect modifies the target attribute value.
 */
UENUM(BlueprintType)
enum class EEffectOperation : uint8
{
    Add        UMETA(DisplayName = "Add"),
    Subtract   UMETA(DisplayName = "Subtract"),
    Multiply   UMETA(DisplayName = "Multiply"),
    Set        UMETA(DisplayName = "Set"),
};

/**
 * @brief Describes a single attribute modification inside an effect.
 */
USTRUCT(BlueprintType)
struct FEffectModifier
{
    GENERATED_BODY()

    /** @brief Gameplay tag of the attribute to modify (e.g. CharacterAttributes.Stamina). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modifier")
    FGameplayTag AttributeTag;

    /** @brief Math operation applied to the attribute value. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modifier")
    EEffectOperation Operation = EEffectOperation::Add;

    /** @brief Magnitude of the modification. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modifier")
    float Magnitude = 0.0f;
};

/**
 * @brief Data asset that fully describes one effect: its modifiers, duration and tick settings.
 *        A single asset can replace most simple effects — just configure the modifiers list.
 */
UCLASS(BlueprintType)
class MODULARABILITYCOMPONENT_API UEffectData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:

    UEffectData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

    // --- Config fields ---

    /** @brief Human-readable name shown in editor and logs. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config")
    FName EffectName = NAME_None;

    /** @brief Total duration in seconds. 0 means instant (applied once). Negative means infinite. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config")
    float Duration = 0.0f;

    /** @brief Interval between ticks in seconds. Used only when Duration != 0. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config")
    float TickInterval = 1.0f;

    /** @brief List of attribute modifications applied each tick (or once if instant). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config")
    TArray<FEffectModifier> Modifiers;

public:

    // --- Const getters ---

    /** @brief Returns effect display name. */
    FORCEINLINE FName GetEffectName() const { return EffectName; }

    /** @brief Returns total duration. 0 = instant, negative = infinite. */
    FORCEINLINE float GetDuration() const { return Duration; }

    /** @brief Returns tick interval. */
    FORCEINLINE float GetTickInterval() const { return TickInterval; }

    /** @brief Returns modifier list. */
    FORCEINLINE const TArray<FEffectModifier>& GetModifiers() const { return Modifiers; }

    /** @brief Returns true if the effect is applied instantly (no ticking). */
    FORCEINLINE bool IsInstant() const { return FMath::IsNearlyZero(Duration); }

    /** @brief Returns true if the effect lasts forever until manually removed. */
    FORCEINLINE bool IsInfinite() const { return Duration < 0.0f; }
};