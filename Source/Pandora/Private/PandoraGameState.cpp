// Copyright Pandora. All Rights Reserved.

#include "PandoraGameState.h"
#include "Net/UnrealNetwork.h"

APandoraGameState::APandoraGameState()
{
	bReplicates = true;
}

void APandoraGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APandoraGameState, MatchTimeRemaining);
}

void APandoraGameState::OnRep_MatchTimeRemaining()
{
	// 客户端收到时间更新时触发，可用于刷新 UI
}