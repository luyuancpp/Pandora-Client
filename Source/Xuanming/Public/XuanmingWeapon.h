// Copyright Xuanming. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "XuanmingWeapon.generated.h"

class UStaticMeshComponent;
class AXuanmingCharacter;

/**
 * 玄冥武器基类 - 服务器权威 hitscan 射击
 *
 * 流程：
 *   客户端按下开火 → StartFire() →
 *   本地预测一发（特效），同时 Server_Fire RPC →
 *   服务器做线检测、扣血、广播 Multicast_PlayFireFX
 */
UCLASS()
class XUANMING_API AXuanmingWeapon : public AActor
{
	GENERATED_BODY()

public:
	AXuanmingWeapon();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 武器外观（M1 阶段用 placeholder static mesh，接正式美术资产时可换 SkeletalMesh 或保留 + 加骨骼组件） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Xuanming|Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	/** 单发伤害 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|Weapon|Stats")
	float Damage = 25.0f;

	/** 射程（cm） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|Weapon|Stats")
	float Range = 10000.0f;

	/** 射速（发/分钟） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|Weapon|Stats")
	float RoundsPerMinute = 600.0f;

	/** 弹匣容量 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|Weapon|Stats")
	int32 MagazineSize = 30;

	/** 当前弹匣余量（服务器权威） */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Xuanming|Weapon|Stats")
	int32 CurrentAmmo = 30;

	/** 准星扩散角度（度），0 为完全无散布 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|Weapon|Stats")
	float BulletSpreadDegrees = 1.0f;

	/** 由 Character 调用：开始/停止开火 */
	UFUNCTION(BlueprintCallable, Category = "Xuanming|Weapon")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Xuanming|Weapon")
	void StopFire();

protected:
	virtual void BeginPlay() override;

	/** 本地按下开火后调用，处理本地预测 + RPC */
	void HandleFire();

	/** 服务器实际执行射击与伤害 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Fire(const FVector& EyeLocation, const FVector& AimDirection);

	/** 广播开火特效到所有客户端（旁观者也能看到） */
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayFireFX(const FVector& HitLocation, bool bHit);

	bool CanFire() const;

	/** 计算两次开火的最小间隔（秒） */
	float GetFireInterval() const { return 60.0f / FMath::Max(1.0f, RoundsPerMinute); }

	/** 取得拥有者角色（由 Owner 转型） */
	AXuanmingCharacter* GetOwnerCharacter() const;

private:
	FTimerHandle FireTimerHandle;
	float LastFireTime = -1000.0f;
	bool bWantsToFire = false;
};
