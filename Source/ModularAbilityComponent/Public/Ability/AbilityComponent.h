// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "AbilityComponent.generated.h"

class UEffectObject;
class UEffectData;
struct FEffectModifier;
struct FDamageEvent;

// ---------------------------------------------------------------------------
// Delegates
// ---------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, InNewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChanged, float, InNewStamina);

// ---------------------------------------------------------------------------
// UAbilityComponent
// ---------------------------------------------------------------------------

/**
 * @brief Actor component that manages health, stamina, gameplay states and effects.
 *        Damage is fed in from the owning Actor's overridden TakeDamage via HandleDamageEvent.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MODULARABILITYCOMPONENT_API UAbilityComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UAbilityComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

    // --- Config fields ---

    /** @brief Initial gameplay state tags applied on activation. */
    UPROPERTY(EditDefaultsOnly, Category="Config")
    FGameplayTagContainer DefaultStates;

    /** @brief Starting current health. */
    UPROPERTY(EditDefaultsOnly, Category="Config")
    float StartingHealth = 100.0f;

    /** @brief Maximum health. */
    UPROPERTY(EditDefaultsOnly, Category="Config")
    float MaxHealth = 100.0f;

    /** @brief Starting current stamina. */
    UPROPERTY(EditDefaultsOnly, Category="Config")
    float StartingStamina = 100.0f;

    /** @brief Maximum stamina. */
    UPROPERTY(EditDefaultsOnly, Category="Config")
    float MaxStamina = 100.0f;

protected:

    // --- Runtime fields ---

    /** @brief Current gameplay state tags. */
    UPROPERTY()
    FGameplayTagContainer States;

    /** @brief Current health value. */
    float CurrentHealth = 100.0f;

    /** @brief Current stamina value. */
    float CurrentStamina = 100.0f;

    /** @brief Active effects ticked every frame. */
    UPROPERTY()
    TArray<TObjectPtr<UEffectObject>> Effects;

    /** @brief True after OnDeath has been broadcast — prevents double-fire. */
    uint8 bIsDead : 1;

protected:

    // --- Service / helper functions ---

    /**
     * @brief Core damage processing called by HandleDamageEvent.
     * @param InAmount  Actual damage amount (post-modifiers from TakeDamage).
     * @param InBone    Hit bone name; NAME_None for radial/generic damage.
     */
    void ProcessDamage(float InAmount, FName InBone = NAME_None);

    /**
     * @brief Broadcasts OnDeath if health just hit zero. Guarded by bIsDead.
     */
    void CheckDeathCondition();

    /**
     * @brief Removes all inactive effects from the array.
     */
    void PurgeExpiredEffects();

public:

    // --- Unreal Framework overrides ---

    virtual void TickComponent(float InDeltaTime, ELevelTick InTickType, FActorComponentTickFunction* InThisTickFunction) override;

public:

    // --- Parent class overrides ---

    virtual void Activate(bool bReset = false) override;
    virtual void Deactivate() override;

public:

    // --- Public functions ---

    /**
     * @brief Single entry point for all damage. Call this from your Actor's TakeDamage override.
     *        Internally casts FDamageEvent to FPointDamageEvent / FRadialDamageEvent to extract bone info.
     * @param InAmount      Damage amount returned by Super::TakeDamage.
     * @param InDamageEvent Original damage event passed into TakeDamage.
     */
    UFUNCTION(BlueprintCallable, Category="Ability|Health")
    virtual void HandleDamageEvent(float InAmount, const FDamageEvent& InDamageEvent);

    /**
     * @brief Directly applies damage bypassing the TakeDamage pipeline (e.g. server-side logic).
     * @param InBone    Bone to damage. NAME_None = flat health damage.
     * @param InDamage  Damage amount.
     */
    UFUNCTION(BlueprintCallable, Category="Ability|Health")
    virtual void ApplyDamageToBone(FName InBone, float InDamage);

    /**
     * @brief Creates and adds an effect from a data asset.
     * @param InEffectData  Data asset describing the effect.
     * @return              The created effect instance, or nullptr on failure.
     */
    UFUNCTION(BlueprintCallable, Category="Ability|Effects")
    virtual UEffectObject* AddEffectFromData(UEffectData* InEffectData);

    /**
     * @brief Adds an already-constructed effect instance.
     * @param InEffect  Effect to add.
     */
    UFUNCTION(BlueprintCallable, Category="Ability|Effects")
    virtual void AddEffect(UEffectObject* InEffect);

    /**
     * @brief Removes and deactivates an active effect.
     * @param InEffect  Effect to remove.
     */
    UFUNCTION(BlueprintCallable, Category="Ability|Effects")
    virtual void RemoveEffect(UEffectObject* InEffect);

    /**
     * @brief Adds a gameplay state tag.
     * @param InState  Tag to add.
     */
    UFUNCTION(BlueprintCallable, Category="Ability|States")
    virtual void AddState(FGameplayTag InState);

    /**
     * @brief Removes a gameplay state tag.
     * @param InState  Tag to remove.
     */
    UFUNCTION(BlueprintCallable, Category="Ability|States")
    virtual void RemoveState(FGameplayTag InState);

    /**
     * @brief Returns true if the pawn has enough stamina and is alive.
     * @param InStaminaCost  Required stamina amount.
     */
    UFUNCTION(BlueprintCallable, Category="Ability|Stamina")
    virtual bool CanExecuteStaminaAction(float InStaminaCost) const;

    /**
     * @brief Tries to spend stamina. Returns false if not enough.
     * @param InStaminaCost  Stamina to spend.
     */
    UFUNCTION(BlueprintCallable, Category="Ability|Stamina")
    virtual bool WasteStamina(float InStaminaCost);

    /**
     * @brief Applies a single effect modifier to the matching attribute.
     *        Called internally by UEffectObject; can also be called manually.
     * @param InModifier  Modifier descriptor.
     */
    UFUNCTION(BlueprintCallable, Category="Ability|Effects")
    virtual void ApplyEffectModifier(const FEffectModifier& InModifier);

public:

    // --- Const getters ---

    /** @brief Returns current health. */
    UFUNCTION(BlueprintPure, Category="Ability|Health")
    FORCEINLINE float GetHealth() const { return CurrentHealth; }

    /** @brief Returns max health. */
    UFUNCTION(BlueprintPure, Category="Ability|Health")
    FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

    /** @brief Returns current stamina. */
    UFUNCTION(BlueprintPure, Category="Ability|Stamina")
    FORCEINLINE float GetStamina() const { return CurrentStamina; }

    /** @brief Returns max stamina. */
    UFUNCTION(BlueprintPure, Category="Ability|Stamina")
    FORCEINLINE float GetMaxStamina() const { return MaxStamina; }

    /** @brief Returns true if the pawn is alive. */
    UFUNCTION(BlueprintPure, Category="Ability|Health")
    FORCEINLINE bool IsAlive() const { return !bIsDead; }

    /** @brief Returns the current gameplay state tag container. */
    UFUNCTION(BlueprintPure, Category="Ability|States")
    FORCEINLINE FGameplayTagContainer GetStates() const { return States; }

    /**
     * @brief Returns true if the owner currently has the given state tag.
     * @param InState  Tag to check.
     */
    UFUNCTION(BlueprintPure, Category="Ability|States")
    bool HasState(FGameplayTag InState) const;

    /**
     * @brief Finds the first active effect whose data asset is of the given class.
     * @param InDataClass  UEffectData subclass to search for.
     * @return             Matching effect or nullptr.
     */
    UFUNCTION(BlueprintPure, Category="Ability|Effects")
    UEffectObject* FindEffectByDataClass(TSubclassOf<UEffectData> InDataClass) const;

    /** @brief Returns a copy of the active effects array. */
    UFUNCTION(BlueprintPure, Category="Ability|Effects")
    FORCEINLINE TArray<UEffectObject*> GetActiveEffects() const { return TArray<UEffectObject*>(Effects); }

public:

    // --- Event dispatchers ---

    /** @brief Fired once when the pawn's health reaches zero. */
    UPROPERTY(BlueprintAssignable, Category="Events")
    FOnDeath OnDeath;

    /** @brief Fired whenever the health value changes. */
    UPROPERTY(BlueprintAssignable, Category="Events")
    FOnHealthChanged OnHealthChanged;

    /** @brief Fired whenever the stamina value changes. */
    UPROPERTY(BlueprintAssignable, Category="Events")
    FOnStaminaChanged OnStaminaChanged;
};