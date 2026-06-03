// Copyright Xuanming. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "XuanmingGA_FrostCurse.generated.h"

class UGameplayEffect;

/**
 * 玄冥冰咒 (Frost Curse) - 仙道 vs 现代 FPS 反差差异化的核心示例技能.
 *
 * 流程:
 *   按 Q 触发 -> ASC TryActivateAbilityByClass ->
 *   server 端 ActivateAbility 检查 Mana >= 25 + Cooldown 未激活 ->
 *   消耗 Mana (CostGE) + 起 Cooldown (CooldownGE) ->
 *   从 Avatar 眼睛位置 LineTrace (ECC_Pawn, 与 Weapon 一致, 见 CLAUDE.md 已知坑) ->
 *   命中 Pawn 时给目标 ApplyGameplayEffect(GE_FrostSlow), 走 GAS 复制到所有 client.
 *
 * NetExecutionPolicy = ServerOnly: 简单稳妥, 命中判定权威, 无需预测.
 * 后续可改 LocalPredicted 提高响应感, 但需要严格 Confirm/Reject 路径.
 */
UCLASS()
class XUANMING_API UXuanmingGA_FrostCurse : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UXuanmingGA_FrostCurse();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	/** 命中后施加给目标的减速 GE (默认 GE_FrostSlow, BP 子类可换) */
	UPROPERTY(EditDefaultsOnly, Category = "Xuanming|FrostCurse")
	TSubclassOf<UGameplayEffect> SlowEffectClass;

	/** 射程 (cm), 默认 2500 (≈ 25m, 短于步枪 100m) */
	UPROPERTY(EditDefaultsOnly, Category = "Xuanming|FrostCurse")
	float Range = 2500.f;

	/**
	 * 调试用: 允许把减速 GE 施加到自己身上.
	 * - false (默认/生产): LineTrace 命中且目标 ≠ Caster 才 apply
	 * - true (M1.5 调试): LineTrace 没命中或命中自己, 也把 GE apply 给 Caster
	 *
	 * 用法: 在 BP_GA_FrostCurse 的 Class Defaults 里勾上, 按 Q 不用瞄人就能验证
	 * GE 链路通不通. 验证完取消勾选. 不要 Shipping 打开 (玩家滥用).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Xuanming|FrostCurse|Debug")
	bool bAllowSelfTarget = false;
};
