// Copyright Xuanming. All Rights Reserved.

#include "XuanmingGA_FrostCurse.h"
#include "XuanmingCharacter.h"
#include "XuanmingFrostEffects.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

UXuanmingGA_FrostCurse::UXuanmingGA_FrostCurse()
{
	// ServerOnly: 命中判定全在服务器, 不做预测, 简单可靠 (M1.5 优先正确性 > 响应感)
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 默认 Cost / Cooldown 指向 C++ 基类, BP 子类可覆盖
	CostGameplayEffectClass = UGE_FrostCost::StaticClass();
	CooldownGameplayEffectClass = UGE_FrostCooldown::StaticClass();

	// 默认减速 GE
	SlowEffectClass = UGE_FrostSlow::StaticClass();
}

void UXuanmingGA_FrostCurse::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// CommitAbility 一次性 check + apply Cost + Cooldown, 任一不过就 EndAbility
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility*/ true, /*bWasCancelled*/ true);
		return;
	}

	AXuanmingCharacter* Caster = ActorInfo ? Cast<AXuanmingCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UAbilitySystemComponent* CasterASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	UWorld* World = Caster ? Caster->GetWorld() : nullptr;

	if (!Caster || !CasterASC || !World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// ServerOnly 策略: 这里只可能在 server 跑
	const FVector Start = Caster->GetEyeLocation();
	const FVector Dir = Caster->GetEyeRotation().Vector();
	const FVector End = Start + Dir * Range;

	FCollisionQueryParams QP(SCENE_QUERY_STAT(FrostCurse), false, Caster);
	QP.bReturnPhysicalMaterial = false;

	FHitResult Hit;
	// ECC_Pawn: FPS 子弹/技能必须用这个通道, ECC_Visibility 默认对 Pawn 是 Ignore (CLAUDE.md 已知坑)
	const bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, QP);

	#if ENABLE_DRAW_DEBUG
	DrawDebugLine(World, Start, bHit ? Hit.ImpactPoint : End,
		bHit ? FColor::Cyan : FColor::Blue, false, 1.0f, 0, 1.5f);
	#endif

	if (bHit)
	{
		if (APawn* TargetPawn = Cast<APawn>(Hit.GetActor()))
		{
			if (TargetPawn != Caster && SlowEffectClass)
			{
				// 给目标 ASC 应用减速 GE (走 GAS 标准复制路径, 所有 client 自动同步)
				FGameplayEffectContextHandle Ctx = CasterASC->MakeEffectContext();
				Ctx.AddSourceObject(Caster);
				const FGameplayEffectSpecHandle SpecHandle =
					CasterASC->MakeOutgoingSpec(SlowEffectClass, /*Level*/ 1.f, Ctx);
				if (SpecHandle.IsValid())
				{
					CasterASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(),
						UAbilitySystemComponent::GetAbilitySystemComponentFromActor(TargetPawn));
				}
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility*/ true, /*bWasCancelled*/ false);
}
