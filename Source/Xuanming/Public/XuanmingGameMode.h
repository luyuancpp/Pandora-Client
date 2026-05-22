// Copyright Xuanming. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "XuanmingGameMode.generated.h"

/**
 * 玄冥 GameMode - 服务器权威的游戏规则
 * 只在 Dedicated Server 上存在
 */
UCLASS()
class XUANMING_API AXuanmingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AXuanmingGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** 当前在线玩家数 */
	UPROPERTY(BlueprintReadOnly, Category = "Xuanming")
	int32 NumConnectedPlayers = 0;
};
