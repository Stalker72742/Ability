// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Attribute.generated.h"

/**
 * @brief Base struct for all character attributes.
 */
USTRUCT(BlueprintType)
struct FAttribute
{
    GENERATED_BODY()
    virtual ~FAttribute() = default;
    FAttribute() {}
};

// ---------------------------------------------------------------------------
// Health
// ---------------------------------------------------------------------------

/**
 * @brief Simple flat health: current/max with alive check.
 */
USTRUCT(BlueprintType)
struct FHealth : public FAttribute
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Params")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Params")
    float MaxHealth = 100.0f;

    FHealth() {}

    virtual bool IsAlive() const
    {
        return CurrentHealth > 0.0f;
    }

    void ApplyDamage(float InAmount)
    {
        CurrentHealth = FMath::Clamp(CurrentHealth - InAmount, 0.0f, MaxHealth);
    }

    void ApplyHeal(float InAmount)
    {
        CurrentHealth = FMath::Clamp(CurrentHealth + InAmount, 0.0f, MaxHealth);
    }
};

// ---------------------------------------------------------------------------
// ModularHealth (per-bone)
// ---------------------------------------------------------------------------

/**
 * @brief Per-bone health map. IsAlive = all bones above 0.
 */
USTRUCT(BlueprintType)
struct FModularHealth : public FHealth
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Params")
    TMap<FName, FGameplayTag> BonesDictionary;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Params")
    TMap<FGameplayTag, float> BonesWithHealth;

    FModularHealth() {}

    virtual bool IsAlive() const override
    {
        for (const TPair<FGameplayTag, float>& pair : BonesWithHealth)
        {
            if (pair.Value <= 0.0f)
            {
                return false;
            }
        }
        return true;
    }

    void ApplyDamageToBone(float InAmount, FName InBone)
    {
        const FGameplayTag* boneTag = BonesDictionary.Find(InBone);
        if (!boneTag)
        {
            return;
        }

        float* current = BonesWithHealth.Find(*boneTag);
        if (!current)
        {
            return;
        }

        *current = FMath::Max(0.0f, *current - InAmount);
    }

    float GetBoneHealth(FName InBone) const
    {
        const FGameplayTag* boneTag = BonesDictionary.Find(InBone);
        if (!boneTag)
        {
            return 0.0f;
        }

        const float* value = BonesWithHealth.Find(*boneTag);
        return value ? *value : 0.0f;
    }
};

// ---------------------------------------------------------------------------
// Stamina
// ---------------------------------------------------------------------------

/**
 * @brief Simple stamina attribute with current/max values.
 */
USTRUCT(BlueprintType)
struct FStaminaAttribute : public FAttribute
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Params")
    float CurrentStamina = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Params")
    float MaxStamina = 100.0f;

    FStaminaAttribute() {}

    /**
     * @brief Returns true if there is enough stamina to spend InCost.
     */
    bool CanSpend(float InCost) const
    {
        return CurrentStamina >= InCost;
    }

    /**
     * @brief Spends InCost stamina. Returns false if not enough stamina.
     */
    bool Spend(float InCost)
    {
        if (!CanSpend(InCost))
        {
            return false;
        }
        CurrentStamina = FMath::Clamp(CurrentStamina - InCost, 0.0f, MaxStamina);
        return true;
    }

    void Restore(float InAmount)
    {
        CurrentStamina = FMath::Clamp(CurrentStamina + InAmount, 0.0f, MaxStamina);
    }
};