// Copyright Xuanming. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "XuanmingPlayerController.generated.h"

/**
 * 玄冥 PlayerController - 玩家的输入和 UI 入口
 * 客户端和服务器各持有一份本玩家的 PC
 */
UCLASS()
class XUANMING_API AXuanmingPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AXuanmingPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};
