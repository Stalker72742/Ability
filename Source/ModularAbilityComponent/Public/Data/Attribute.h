// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
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

USTRUCT(BlueprintType, Blueprintable)
struct FAttributeContainer
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Params")
    FGameplayTag AttributeTag;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(BaseStruct="Attribute"), Category="Config")
    FInstancedStruct Data;
    
    FAttributeContainer(const FGameplayTag& InAttributeTag)
    {
        AttributeTag = InAttributeTag;
        Data = FInstancedStruct();
    }
    
    FAttributeContainer()
    {
        AttributeTag = FGameplayTag();
        Data = FInstancedStruct();
    }
    
    friend uint32 GetTypeHash(const FAttributeContainer& InContainer)
    {
        return GetTypeHash(InContainer.AttributeTag);
    }
    
    template<typename T>
    T* GetAs()
    {
        static_assert(TIsDerivedFrom<T, FAttribute>::Value, "T must derive from FAttribute");
        return const_cast<T*>(Data.GetPtr<T>());
    }

    template<typename T>
    const T* GetAs() const
    {
        static_assert(TIsDerivedFrom<T, FAttribute>::Value, "T must derive from FAttribute");
        return Data.GetPtr<T>();
    }
    
    bool operator==(const FAttributeContainer& Other) const { return AttributeTag == Other.AttributeTag; }
    bool operator==(const FGameplayTag& Other) const { return AttributeTag == Other; }
    bool operator!=(const FAttributeContainer& Other) const { return AttributeTag != Other.AttributeTag; }
    
    bool IsValid() const { return AttributeTag.IsValid(); }
};

USTRUCT(BlueprintType)
struct FFloatAttribute : public FAttribute
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Params")
    float CurrentValue {100.0f};

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Params")
    FVector2D MinMaxValue {0.0f, 100.0f};

    FFloatAttribute()
    {
        
    }
    
    void operator-=(const float& Value)
    {
        CurrentValue -= Value;
    }
};