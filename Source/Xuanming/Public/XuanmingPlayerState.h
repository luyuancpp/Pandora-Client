// Copyright Xuanming. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "XuanmingPlayerState.generated.h"

/**
 * 玄冥 PlayerState - 每个玩家的同步数据（击杀、死亡、分数）
 * 所有客户端都能看到所有 PlayerState（用于显示记分板）
 */
UCLASS()
class XUANMING_API AXuanmingPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AXuanmingPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Xuanming|Stats")
	int32 Kills = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Xuanming|Stats")
	int32 Deaths = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Xuanming|Stats")
	int32 TeamId = 0;
};
