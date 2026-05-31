// Copyright Xuanming. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "XuanmingHUD.generated.h"

/**
 * 玄冥 HUD - 占位类
 *
 * M1.4 起 HUD 改由 UMG 接管 (WBP_PlayerHUD, 由 PlayerController CreateWidget),
 * 此 C++ 类保留只为兼容 GameMode::HUDClass 的 AHUD 默认指派.
 * 未来若要加 debug-only 的 Canvas 调试绘制 (例如 cvar 切换), 可在 DrawHUD 里加.
 */
UCLASS()
class XUANMING_API AXuanmingHUD : public AHUD
{
	GENERATED_BODY()

public:
	AXuanmingHUD();

	virtual void DrawHUD() override;
};
