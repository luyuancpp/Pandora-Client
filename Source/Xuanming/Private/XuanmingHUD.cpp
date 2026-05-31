// Copyright Xuanming. All Rights Reserved.

#include "XuanmingHUD.h"

AXuanmingHUD::AXuanmingHUD()
{
}

void AXuanmingHUD::DrawHUD()
{
	Super::DrawHUD();
	// UMG 接管 (WBP_PlayerHUD), 这里不再做 Canvas 绘制.
	// 详见 Tools/CreateHUDWidgets.py 与 PlayerController 的 TryCreateHUD.
}
