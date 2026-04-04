// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/AbilityComponent.h"

#include "Data/Effects/EffectData.h"
#include "Data/Misc/GameplayTags.h"
#include "Engine/DamageEvents.h"

UAbilityComponent::UAbilityComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
    bAutoActivate = true;
    
    SetIsReplicatedByDefault(true);
}

void UAbilityComponent::ApplyDamage(float InDamage)
{
    auto health = GetAttribute(HealthAttribute);
    if (!health)
    {
        return;
    }
    
    auto healthData = health->GetAs<FFloatAttribute>();
    if (healthData)
    {
        *healthData -= InDamage;
        
        if (healthData->CurrentValue <= 0.0f)
        {
            healthData->CurrentValue = 0.0f;
            OnDeath.Broadcast();
        }
        
        OnAttributeChanged.Broadcast(*health);
    }
}

void UAbilityComponent::Activate(bool bReset)
{
    Super::Activate(bReset);
    
}

void UAbilityComponent::Deactivate()
{
    Super::Deactivate();
}

void UAbilityComponent::HandleDamageEvent(float InDamageAmount, struct FDamageEvent const& InDamageEvent,
    class AController* InEventInstigator, AActor* InDamageCauser)
{
    if (!IsAlive() || InDamageAmount <= 0.0f)
    {
        return;
    }

    if (InDamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        FPointDamageEvent* const PointDamageEvent = (FPointDamageEvent*) &InDamageEvent;
        
        ApplyDamage(InDamageAmount);
        OnTakeDamage.Broadcast(PointDamageEvent->HitInfo);
    }
    else if (InDamageEvent.IsOfType(FRadialDamageEvent::ClassID))
    {
        FRadialDamageEvent* const RadialDamageEvent = (FRadialDamageEvent*) &InDamageEvent;
        
        ApplyDamage(InDamageAmount);
        OnTakeDamage.Broadcast(RadialDamageEvent->ComponentHits[0]);
    } 
    else
    {
        ApplyDamage(InDamageAmount);
    }
}

void UAbilityComponent::AddState(FGameplayTag InState)
{
    States.AddTag(InState);
}

void UAbilityComponent::RemoveState(FGameplayTag InState)
{
    States.RemoveTag(InState);
}

bool UAbilityComponent::IsAlive() const
{
    if (auto attribute = Attributes.Find(FAttributeContainer(HealthAttribute)))
    {
        if (auto health = attribute->GetAs<FFloatAttribute>())
        {
            return health->CurrentValue > 0.0f;
        }
    }
    
    return true;
}

FAttributeContainer* UAbilityComponent::GetAttribute(FGameplayTag InAttributeTag) const
{
    if (auto attribute = Attributes.Find(InAttributeTag))
    {
        return const_cast<FAttributeContainer*>(attribute);
    }
    
    return nullptr;
}
