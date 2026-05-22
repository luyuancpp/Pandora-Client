// Copyright Xuanming. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "XuanmingGameState.generated.h"

/**
 * 玄冥 GameState - 同步到所有客户端的全局游戏状态
 */
UCLASS()
class XUANMING_API AXuanmingGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AXuanmingGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 比赛剩余时间（秒），服务器权威，同步到所有客户端 */
	UPROPERTY(ReplicatedUsing = OnRep_MatchTimeRemaining, BlueprintReadOnly, Category = "Xuanming")
	float MatchTimeRemaining = 600.0f;

	UFUNCTION()
	void OnRep_MatchTimeRemaining();
};
