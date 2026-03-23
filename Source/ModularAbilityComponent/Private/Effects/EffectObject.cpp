// Fill out your copyright notice in the Description page of Project Settings.

#include "Effects/EffectObject.h"

#include "Ability/AbilityComponent.h"
#include "Data/Effects/EffectData.h"

UEffectObject::UEffectObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsActive = false;
}

UEffectObject* UEffectObject::Create(UObject* InOuter, UEffectData* InData, UAbilityComponent* InOwner)
{
	if (!InData || !InOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UEffectObject] Create called with null InData or InOwner"));
		return nullptr;
	}

	UEffectObject* instance = NewObject<UEffectObject>(InOuter);
	instance->EffectData  = InData;
	instance->OwnerComponent = InOwner;
	instance->bIsActive   = true;
	instance->ElapsedTime = 0.0f;
	instance->TimeSinceLastTick = 0.0f;

	if (InData->IsInstant())
	{
		instance->ApplyModifiers();
		instance->bIsActive = false;
	}

	return instance;
}

void UEffectObject::Tick(float InDeltaTime)
{
	if (!bIsActive || !EffectData)
	{
		return;
	}

	ElapsedTime       += InDeltaTime;
	TimeSinceLastTick += InDeltaTime;

	if (TimeSinceLastTick >= EffectData->GetTickInterval())
	{
		TimeSinceLastTick = 0.0f;
		ApplyModifiers();
	}

	if (!EffectData->IsInfinite() && ElapsedTime >= EffectData->GetDuration())
	{
		bIsActive = false;
		OnEffectExpired.Broadcast(this);
	}
}

void UEffectObject::ForceRemove()
{
	bIsActive = false;
}

void UEffectObject::ApplyModifiers()
{
	if (!OwnerComponent.IsValid())
	{
		return;
	}

	for (const FEffectModifier& modifier : EffectData->GetModifiers())
	{
		OwnerComponent->ApplyEffectModifier(modifier);
	}
}