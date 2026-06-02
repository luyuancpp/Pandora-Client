// Copyright Xuanming. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "XuanmingAttributeSet.generated.h"

// GAS 标准宏: 一次性生成 GetXxxAttribute / GetXxx / SetXxx / InitXxx 四件套.
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 玄冥 AttributeSet - 玩家所有数值属性集合.
 *
 * 挂在 PlayerState 上 (跟 ASC 一起), 让 Character 复活后属性不丢.
 *
 * 属性表:
 *   Health / MaxHealth   - 生命值, GE 修改 Health, MaxHealth 由装备/buff 修改
 *   Mana / MaxMana       - 法力值, 释放技能消耗
 *   MoveSpeed            - 移动速度倍率 (1.0 = 100%), 冰咒减速 -> 0.5
 *   Damage               - 伤害 meta attribute (临时值, 在 PostGameplayEffectExecute
 *                          里转成 Health 扣减, 不直接复制)
 */
UCLASS()
class XUANMING_API UXuanmingAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UXuanmingAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	// === Health ===
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Xuanming|Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UXuanmingAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Xuanming|Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UXuanmingAttributeSet, MaxHealth)

	// === Mana ===
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Xuanming|Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UXuanmingAttributeSet, Mana)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Xuanming|Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UXuanmingAttributeSet, MaxMana)

	// === MoveSpeed (倍率) ===
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeed, Category = "Xuanming|Attributes")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UXuanmingAttributeSet, MoveSpeed)

	// === Damage (meta, 不复制) ===
	UPROPERTY(BlueprintReadOnly, Category = "Xuanming|Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UXuanmingAttributeSet, Damage)

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Mana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);
};
