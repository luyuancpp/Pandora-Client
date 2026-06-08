// Copyright Pandora. All Rights Reserved.

#include "PandoraAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "PandoraCharacter.h"

UPandoraAttributeSet::UPandoraAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitMana(100.f);
	InitMaxMana(100.f);
	InitMoveSpeed(1.f);
	InitDamage(0.f);
}

void UPandoraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// REPNOTIFY_Always: 即使新值等于旧值也广播, GAS 推荐 (避免 prediction 回滚被吞)
	DOREPLIFETIME_CONDITION_NOTIFY(UPandoraAttributeSet, Health,    COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPandoraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPandoraAttributeSet, Mana,      COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPandoraAttributeSet, MaxMana,   COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPandoraAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
}

void UPandoraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 上下限钳制
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.1f, 5.f);
	}
}

void UPandoraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute& Attr = Data.EvaluatedData.Attribute;

	// Damage meta attribute -> 转成 Health 扣减
	if (Attr == GetDamageAttribute())
	{
		const float LocalDamage = GetDamage();
		SetDamage(0.f);
		if (LocalDamage > 0.f)
		{
			SetHealth(FMath::Clamp(GetHealth() - LocalDamage, 0.f, GetMaxHealth()));
		}
	}

	// MoveSpeed 改变时, 同步 CharacterMovement.MaxWalkSpeed
	if (Attr == GetMoveSpeedAttribute())
	{
		AActor* OwnerActor = nullptr;
		if (Data.Target.AbilityActorInfo.IsValid())
		{
			OwnerActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		}
		if (ACharacter* C = Cast<ACharacter>(OwnerActor))
		{
			if (UCharacterMovementComponent* Move = C->GetCharacterMovement())
			{
				// 600 是 Character 默认 MaxWalkSpeed (见 PandoraCharacter.cpp 构造函数)
				Move->MaxWalkSpeed = 600.f * GetMoveSpeed();
			}
		}
	}

	// Health 变化时镜像到 Character.Health (兼容旧 ViewModel/HUD 推送链路)
	if (Attr == GetHealthAttribute())
	{
		AActor* OwnerActor = nullptr;
		if (Data.Target.AbilityActorInfo.IsValid())
		{
			OwnerActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		}
		if (APandoraCharacter* XC = Cast<APandoraCharacter>(OwnerActor))
		{
			XC->Health = GetHealth();
			if (XC->Health <= 0.f)
			{
				// 旧链路: 走 Character 的死亡逻辑 (HandleDeath 在 TakeDamage 路径里, 这里手动触发一次)
				XC->OnHealthDepleted();
			}
		}
	}
}

void UPandoraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPandoraAttributeSet, Health, OldValue);
}

void UPandoraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPandoraAttributeSet, MaxHealth, OldValue);
}

void UPandoraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPandoraAttributeSet, Mana, OldValue);
}

void UPandoraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPandoraAttributeSet, MaxMana, OldValue);
}

void UPandoraAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPandoraAttributeSet, MoveSpeed, OldValue);
}