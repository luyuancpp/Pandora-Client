// Copyright Pandora. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PandoraGameState.generated.h"

/**
 * Pandora GameState - 同步到所有客户端的全局游戏状态
 */
UCLASS()
class PANDORA_API APandoraGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	APandoraGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 比赛剩余时间（秒），服务器权威，同步到所有客户端 */
	UPROPERTY(ReplicatedUsing = OnRep_MatchTimeRemaining, BlueprintReadOnly, Category = "Pandora")
	float MatchTimeRemaining = 600.0f;

	UFUNCTION()
	void OnRep_MatchTimeRemaining();
};