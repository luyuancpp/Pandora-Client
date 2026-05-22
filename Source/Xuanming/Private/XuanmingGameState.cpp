// Copyright Xuanming. All Rights Reserved.

#include "XuanmingGameState.h"
#include "Net/UnrealNetwork.h"

AXuanmingGameState::AXuanmingGameState()
{
	bReplicates = true;
}

void AXuanmingGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AXuanmingGameState, MatchTimeRemaining);
}

void AXuanmingGameState::OnRep_MatchTimeRemaining()
{
	// 客户端收到时间更新时触发，可用于刷新 UI
}
