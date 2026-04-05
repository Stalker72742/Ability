// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/AbilityComponent.h"

#include "Data/Effects/EffectData.h"
#include "Data/Misc/GameplayTags.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"

UAbilityComponent::UAbilityComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
    bAutoActivate = true;
    
    SetIsReplicatedByDefault(true);
}

void UAbilityComponent::OnRep_Attributes()
{
    OnAttributesReplicated.Broadcast(Attributes);
}

void UAbilityComponent::ApplyDamage(float InDamage)
{
    FAttributeContainer health;
    const bool result = GetAttribute(HealthAttribute, health);
    if (!result)
    {
        return;
    }

    if (auto healthData = health.GetAs<FFloatAttribute>())
    {
        *healthData -= InDamage;
        
        SetAttributeCurrentValue(HealthAttribute, healthData->CurrentValue);
        
        if (healthData->CurrentValue <= 0.0f)
        {
            healthData->CurrentValue = 0.0f;
            OnDeath.Broadcast();
        }
        
        OnAttributeChanged.Broadcast(health);
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

void UAbilityComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UAbilityComponent, Attributes);
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

void UAbilityComponent::SetAttributeCurrentValue(FGameplayTag InAttributeTag, float InValue)
{
    for (FAttributeContainer& Attribute : Attributes)
    {
        if (Attribute.AttributeTag == InAttributeTag)
        {
            if (auto attribute = Attribute.GetAs<FFloatAttribute>())
            {
                attribute->CurrentValue = FMath::Clamp(InValue, attribute->MinMaxValue.X, attribute->MinMaxValue.Y);
            }
            
            return;
        }
    }
}

bool UAbilityComponent::IsAlive() const
{
    for (const FAttributeContainer& Attribute : Attributes)
    {
        if (Attribute.AttributeTag == HealthAttribute)
        {
            if (const auto health = Attribute.GetAs<FFloatAttribute>())
            {
                return health->CurrentValue > 0.0f;
            }
        }
    }
    
    return true;
}

bool UAbilityComponent::GetAttribute(FGameplayTag InAttributeTag, FAttributeContainer& OutAttribute) const
{
    for (const FAttributeContainer& Attribute : Attributes)
    {
        if (Attribute.AttributeTag == InAttributeTag)
        {
            OutAttribute = Attribute;
            return true;
        }
    }
    
    return false;
}
