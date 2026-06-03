// Copyright Xuanming. All Rights Reserved.

#include "XuanmingFrostEffects.h"
#include "XuanmingAttributeSet.h"

// 说明：UE 5.7 在 UObject 构造函数里通过 FindOrAddComponent 添加
// UTargetTagsGameplayEffectComponent 会触发 fatal（内部 NewObject 传 NAME_None
// 被严格模式拒绝，详见 Pitfalls.md）。所以这里 C++ 只配 Duration + Modifier，
// **Granted/Asset Tags 一律由 BP 子类（BP_GE_FrostSlow / BP_GE_FrostCooldown）
// 在编辑器里配置**——这也是 Epic 5.4+ 推荐做法，策划/新人改标签不必动 C++。

// ============================================================================
// GE_FrostSlow: Duration 3s, MoveSpeed *= 0.5
// Tag 'State.Debuff.Slow' 由 BP 子类在 Details -> Components -> Target Tags 里配
//
// 注意 Multiplicitive 的诡异语义 (Epic 拼写错也没修):
//   NewValue = BaseValue * (1 + Σ Multiplicitive Modifiers)
// 想让 MoveSpeed × 0.5 必须传 -0.5 (1 + (-0.5) = 0.5), 传 0.5 实际是 ×1.5 加速.
// 见 Pitfalls.md "GAS Multiplicitive Modifier 是 (1+Σ) 不是 Σ".
// ============================================================================
UGE_FrostSlow::UGE_FrostSlow()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(3.0f));

	FGameplayModifierInfo Mod;
	Mod.Attribute = UXuanmingAttributeSet::GetMoveSpeedAttribute();
	Mod.ModifierOp = EGameplayModOp::Multiplicitive;
	// -0.5 → MoveSpeed * (1 + -0.5) = MoveSpeed * 0.5
	Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-0.5f));
	Modifiers.Add(Mod);
}

// ============================================================================
// GE_FrostCost: Instant, Mana -25
// ============================================================================
UGE_FrostCost::UGE_FrostCost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo Mod;
	Mod.Attribute = UXuanmingAttributeSet::GetManaAttribute();
	Mod.ModifierOp = EGameplayModOp::Additive;
	Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-25.0f));
	Modifiers.Add(Mod);
}

// ============================================================================
// GE_FrostCooldown: Duration 5s
// Tag 'Cooldown.FrostCurse' 由 BP 子类在 Details -> Components -> Target Tags 里配
// ============================================================================
UGE_FrostCooldown::UGE_FrostCooldown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(5.0f));
}
