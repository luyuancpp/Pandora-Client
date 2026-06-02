// Copyright Xuanming. All Rights Reserved.

#include "XuanmingPlayerState.h"
#include "AbilitySystemComponent.h"
#include "XuanmingAttributeSet.h"
#include "Net/UnrealNetwork.h"

AXuanmingPlayerState::AXuanmingPlayerState()
{
	// ASC 挂在 PlayerState 上, 玩家死亡重生后属性数据不丢
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// Mixed 模式: GE 全部复制, 但 GameplayCues / Tags 只复制给 owning client + server,
	// 适合 PvP FPS, 比 Full 节省带宽, 比 Minimal 多了对自己技能的反馈
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UXuanmingAttributeSet>(TEXT("AttributeSet"));

	// PlayerState NetUpdateFrequency 默认 1Hz, GAS 同步属性需要更高
	NetUpdateFrequency = 100.f;
}

void AXuanmingPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AXuanmingPlayerState, Kills);
	DOREPLIFETIME(AXuanmingPlayerState, Deaths);
	DOREPLIFETIME(AXuanmingPlayerState, TeamId);
}
