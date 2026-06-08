// Copyright Pandora. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PandoraGameMode.generated.h"

/**
 * Pandora GameMode - 服务器权威的游戏规则
 * 只在 Dedicated Server 上存在
 */
UCLASS()
class PANDORA_API APandoraGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APandoraGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** 当前在线玩家数 */
	UPROPERTY(BlueprintReadOnly, Category = "Pandora")
	int32 NumConnectedPlayers = 0;
};