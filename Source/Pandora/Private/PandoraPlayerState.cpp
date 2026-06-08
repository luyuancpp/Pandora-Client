// Copyright Pandora. All Rights Reserved.

#include "PandoraPlayerState.h"
#include "AbilitySystemComponent.h"
#include "PandoraAttributeSet.h"
#include "Net/UnrealNetwork.h"

APandoraPlayerState::APandoraPlayerState()
{
	// ASC 挂在 PlayerState 上, 玩家死亡重生后属性数据不丢
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// Mixed 模式: GE 全部复制, 但 GameplayCues / Tags 只复制给 owning client + server,
	// 适合 PvP FPS, 比 Full 节省带宽, 比 Minimal 多了对自己技能的反馈
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UPandoraAttributeSet>(TEXT("AttributeSet"));

	// PlayerState NetUpdateFrequency 默认 1Hz, GAS 同步属性需要更高
	NetUpdateFrequency = 100.f;
}

void APandoraPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APandoraPlayerState, Kills);
	DOREPLIFETIME(APandoraPlayerState, Deaths);
	DOREPLIFETIME(APandoraPlayerState, TeamId);
}