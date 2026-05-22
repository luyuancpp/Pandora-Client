// Copyright Xuanming. All Rights Reserved.

#include "XuanmingPlayerState.h"
#include "Net/UnrealNetwork.h"

AXuanmingPlayerState::AXuanmingPlayerState()
{
}

void AXuanmingPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AXuanmingPlayerState, Kills);
	DOREPLIFETIME(AXuanmingPlayerState, Deaths);
	DOREPLIFETIME(AXuanmingPlayerState, TeamId);
}
