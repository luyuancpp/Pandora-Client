// Copyright Xuanming. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "XuanmingPlayerState.generated.h"

class UAbilitySystemComponent;
class UXuanmingAttributeSet;

/**
 * 玄冥 PlayerState - 每个玩家的同步数据（击杀、死亡、分数）+ GAS 数据宿主
 *
 * GAS 设计: 玩家角色的 ASC 和 AttributeSet 挂在 PlayerState 上,
 * 这样角色死亡重生后属性数据不丢, 也方便记分板/旁观.
 * 对于 AI/小怪走 Character 自挂 ASC 的简化方案 (不在 M1.5 范围).
 */
UCLASS()
class XUANMING_API AXuanmingPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AXuanmingPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	UXuanmingAttributeSet* GetAttributeSet() const { return AttributeSet; }

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Xuanming|Stats")
	int32 Kills = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Xuanming|Stats")
	int32 Deaths = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Xuanming|Stats")
	int32 TeamId = 0;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Xuanming|GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UXuanmingAttributeSet> AttributeSet;
};
