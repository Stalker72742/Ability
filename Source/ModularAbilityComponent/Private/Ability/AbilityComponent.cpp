// Fill out your copyright notice in the Description page of Project Settings.

#include "Ability/AbilityComponent.h"

#include "Data/Effects/EffectData.h"
#include "Effects/EffectObject.h"
#include "Engine/DamageEvents.h"

static const FName TAG_ATTRIBUTE_HEALTH ("CharacterAttributes.Health");
static const FName TAG_ATTRIBUTE_STAMINA("CharacterAttributes.Stamina");

UAbilityComponent::UAbilityComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
    SetAutoActivate(true);
    SetIsReplicatedByDefault(true);

    bIsDead = false;
}

// ---------------------------------------------------------------------------
// Unreal Framework
// ---------------------------------------------------------------------------

void UAbilityComponent::TickComponent(float InDeltaTime, ELevelTick InTickType, FActorComponentTickFunction* InThisTickFunction)
{
    Super::TickComponent(InDeltaTime, InTickType, InThisTickFunction);

    for (UEffectObject* effect : Effects)
    {
        if (effect && effect->IsActive())
        {
            effect->Tick(InDeltaTime);
        }
    }

    PurgeExpiredEffects();
}

// ---------------------------------------------------------------------------
// Activate / Deactivate
// ---------------------------------------------------------------------------

void UAbilityComponent::Activate(bool bReset)
{
    Super::Activate(bReset);

    CurrentHealth  = FMath::Clamp(StartingHealth,  0.0f, MaxHealth);
    CurrentStamina = FMath::Clamp(StartingStamina, 0.0f, MaxStamina);
    bIsDead        = false;
    States         = DefaultStates;
}

void UAbilityComponent::Deactivate()
{
    Super::Deactivate();
}

// ---------------------------------------------------------------------------
// Damage
// ---------------------------------------------------------------------------

void UAbilityComponent::HandleDamageEvent(float InAmount, const FDamageEvent& InDamageEvent)
{
    if (bIsDead || InAmount <= 0.0f)
    {
        return;
    }

    FName boneName = NAME_None;

    if (InDamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        const FPointDamageEvent& pointEvent = static_cast<const FPointDamageEvent&>(InDamageEvent);
        boneName = pointEvent.HitInfo.BoneName;
    }

    ProcessDamage(InAmount, boneName);
}

void UAbilityComponent::ApplyDamageToBone(FName InBone, float InDamage)
{
    ProcessDamage(InDamage, InBone);
}

void UAbilityComponent::ProcessDamage(float InAmount, FName InBone)
{
    if (bIsDead || InAmount <= 0.0f)
    {
        return;
    }

    CurrentHealth = FMath::Clamp(CurrentHealth - InAmount, 0.0f, MaxHealth);
    OnHealthChanged.Broadcast(CurrentHealth);

    CheckDeathCondition();
}

void UAbilityComponent::CheckDeathCondition()
{
    if (bIsDead)
    {
        return;
    }

    if (CurrentHealth <= 0.0f)
    {
        bIsDead = true;
        OnDeath.Broadcast();
    }
}

// ---------------------------------------------------------------------------
// Effects
// ---------------------------------------------------------------------------

UEffectObject* UAbilityComponent::AddEffectFromData(UEffectData* InEffectData)
{
    if (!InEffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UAbilityComponent] AddEffectFromData: null data asset"));
        return nullptr;
    }

    UEffectObject* effect = UEffectObject::Create(this, InEffectData, this);
    if (effect)
    {
        AddEffect(effect);
    }

    return effect;
}

void UAbilityComponent::AddEffect(UEffectObject* InEffect)
{
    if (!InEffect)
    {
        return;
    }

    Effects.AddUnique(InEffect);
}

void UAbilityComponent::RemoveEffect(UEffectObject* InEffect)
{
    if (!InEffect)
    {
        return;
    }

    InEffect->ForceRemove();
    Effects.Remove(InEffect);
}

void UAbilityComponent::PurgeExpiredEffects()
{
    Effects.RemoveAll([](const TObjectPtr<UEffectObject>& effect)
    {
        return !effect || !effect->IsActive();
    });
}

void UAbilityComponent::ApplyEffectModifier(const FEffectModifier& InModifier)
{
    const FName tagName = InModifier.AttributeTag.GetTagName();

    if (tagName == TAG_ATTRIBUTE_HEALTH)
    {
        float newValue = CurrentHealth;

        switch (InModifier.Operation)
        {
            case EEffectOperation::Add:      newValue += InModifier.Magnitude; break;
            case EEffectOperation::Subtract: newValue -= InModifier.Magnitude; break;
            case EEffectOperation::Multiply: newValue *= InModifier.Magnitude; break;
            case EEffectOperation::Set:      newValue  = InModifier.Magnitude; break;
        }

        CurrentHealth = FMath::Clamp(newValue, 0.0f, MaxHealth);
        OnHealthChanged.Broadcast(CurrentHealth);
        CheckDeathCondition();
    }
    else if (tagName == TAG_ATTRIBUTE_STAMINA)
    {
        float newValue = CurrentStamina;

        switch (InModifier.Operation)
        {
            case EEffectOperation::Add:      newValue += InModifier.Magnitude; break;
            case EEffectOperation::Subtract: newValue -= InModifier.Magnitude; break;
            case EEffectOperation::Multiply: newValue *= InModifier.Magnitude; break;
            case EEffectOperation::Set:      newValue  = InModifier.Magnitude; break;
        }

        CurrentStamina = FMath::Clamp(newValue, 0.0f, MaxStamina);
        OnStaminaChanged.Broadcast(CurrentStamina);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[UAbilityComponent] ApplyEffectModifier: unknown attribute tag '%s'"), *tagName.ToString());
    }
}

// ---------------------------------------------------------------------------
// States
// ---------------------------------------------------------------------------

void UAbilityComponent::AddState(FGameplayTag InState)
{
    States.AddTag(InState);
}

void UAbilityComponent::RemoveState(FGameplayTag InState)
{
    States.RemoveTag(InState);
}

bool UAbilityComponent::HasState(FGameplayTag InState) const
{
    return States.HasTag(InState);
}

// ---------------------------------------------------------------------------
// Stamina
// ---------------------------------------------------------------------------

bool UAbilityComponent::CanExecuteStaminaAction(float InStaminaCost) const
{
    return !bIsDead && CurrentStamina >= InStaminaCost;
}

bool UAbilityComponent::WasteStamina(float InStaminaCost)
{
    if (!CanExecuteStaminaAction(InStaminaCost))
    {
        return false;
    }

    CurrentStamina = FMath::Clamp(CurrentStamina - InStaminaCost, 0.0f, MaxStamina);
    OnStaminaChanged.Broadcast(CurrentStamina);
    return true;
}

// ---------------------------------------------------------------------------
// Getters
// ---------------------------------------------------------------------------

UEffectObject* UAbilityComponent::FindEffectByDataClass(TSubclassOf<UEffectData> InDataClass) const
{
    for (UEffectObject* effect : Effects)
    {
        if (effect && effect->GetEffectData() && effect->GetEffectData()->IsA(InDataClass))
        {
            return effect;
        }
    }
    return nullptr;
}