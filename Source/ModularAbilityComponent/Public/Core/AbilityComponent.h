// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Data/Attribute.h"
#include "AbilityComponent.generated.h"

class UEffectObject;
class UEffectData;
struct FEffectModifier;
struct FDamageEvent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, InNewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChanged, float, InNewStamina);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeChanged, const FAttributeContainer&, InAttribute);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeDamage, const FHitResult&, InHitResult);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MODULARABILITYCOMPONENT_API UAbilityComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UAbilityComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
    
    UPROPERTY(EditAnywhere, Category="Config")
    TSet<FAttributeContainer> Attributes;

protected:
    
    UPROPERTY(EditAnywhere, Category="Params")
    FGameplayTagContainer States;
    
protected:
    
    virtual void ApplyDamage(float InDamage);

public:
    
    virtual void Activate(bool bReset = false) override;
    virtual void Deactivate() override;

public:
    
    /**
     * @brief Single entry point for all damage. Call this from your Actor's TakeDamage override.
     *        Internally casts FDamageEvent to FPointDamageEvent / FRadialDamageEvent to extract bone info.
     */
    UFUNCTION(BlueprintCallable, Category="Ability|Health")
    virtual void HandleDamageEvent(float InDamageAmount, FDamageEvent const& InDamageEvent, class AController* InEventInstigator, 
        AActor* InDamageCauser);
    
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

public:
    
    /** @brief Returns true if the pawn is alive. */
    UFUNCTION(BlueprintPure, Category="Ability|Health")
    FORCEINLINE bool IsAlive() const;

    /** @brief Returns the current gameplay state tag container. */
    UFUNCTION(BlueprintPure, Category="Ability|States")
    FORCEINLINE FGameplayTagContainer GetStates() const { return States; }

    /**
     * @brief Returns true if the owner currently has the given state tag.
     * @param InState  Tag to check.
     */
    UFUNCTION(BlueprintPure, Category="Ability|States")
    bool HasState(FGameplayTag InState) const { return States.HasTag(InState); }
    
    FAttributeContainer* GetAttribute(FGameplayTag InAttributeTag) const;

public:
    
    /** @brief Fired once when the pawn's health reaches zero. */
    UPROPERTY(BlueprintAssignable, Category="Events")
    FOnDeath OnDeath;
    
    UPROPERTY(BlueprintAssignable, Category="Events")
    FAttributeChanged OnAttributeChanged;
    
    UPROPERTY(BlueprintAssignable, Category="Events")
    FTakeDamage OnTakeDamage;
};