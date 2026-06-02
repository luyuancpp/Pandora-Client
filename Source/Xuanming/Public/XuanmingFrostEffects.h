// Copyright Xuanming. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "XuanmingFrostEffects.generated.h"

/**
 * 冰咒减速效果 - 给目标加 3s 的 MoveSpeed Multiply 0.5
 * (在 BP 子类里调参更方便, 这个 C++ 基类只设默认值便于程序化测试)
 */
UCLASS()
class XUANMING_API UGE_FrostSlow : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_FrostSlow();
};

/** 冰咒法力消耗 - Instant, Mana -25 */
UCLASS()
class XUANMING_API UGE_FrostCost : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_FrostCost();
};

/** 冰咒冷却 - Duration 5s, 持续期间打 Cooldown.FrostCurse 标签 */
UCLASS()
class XUANMING_API UGE_FrostCooldown : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_FrostCooldown();
};
