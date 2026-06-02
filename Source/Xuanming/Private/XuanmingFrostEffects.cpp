// Copyright Xuanming. All Rights Reserved.

#include "XuanmingFrostEffects.h"
#include "XuanmingAttributeSet.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

// ============================================================================
// GE_FrostSlow: Duration 3s, MoveSpeed *= 0.5
// ============================================================================
UGE_FrostSlow::UGE_FrostSlow()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(3.0f));

	FGameplayModifierInfo Mod;
	Mod.Attribute = UXuanmingAttributeSet::GetMoveSpeedAttribute();
	Mod.ModifierOp = EGameplayModOp::Multiplicitive;
	Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.5f));
	Modifiers.Add(Mod);

	// Tag: State.Debuff.Slow (在编辑器 Project Settings -> GameplayTags 里注册或自动加)
	UTargetTagsGameplayEffectComponent& TagsComp = FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
	FInheritedTagContainer Inherited;
	Inherited.Added.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Debuff.Slow"), false));
	TagsComp.SetAndApplyTargetTagChanges(Inherited);
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
// GE_FrostCooldown: Duration 5s, 加 Cooldown.FrostCurse 标签
// ============================================================================
UGE_FrostCooldown::UGE_FrostCooldown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(5.0f));

	UTargetTagsGameplayEffectComponent& TagsComp = FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
	FInheritedTagContainer Inherited;
	Inherited.Added.AddTag(FGameplayTag::RequestGameplayTag(FName("Cooldown.FrostCurse"), false));
	TagsComp.SetAndApplyTargetTagChanges(Inherited);
}
