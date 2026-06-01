// Copyright Xuanming. All Rights Reserved.

#include "XuanmingWeapon.h"
#include "XuanmingCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

AXuanmingWeapon::AXuanmingWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = WeaponMesh;
}

void AXuanmingWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AXuanmingWeapon, CurrentAmmo);
}

void AXuanmingWeapon::BeginPlay()
{
	Super::BeginPlay();
	CurrentAmmo = MagazineSize;
}

AXuanmingCharacter* AXuanmingWeapon::GetOwnerCharacter() const
{
	return Cast<AXuanmingCharacter>(GetOwner());
}

bool AXuanmingWeapon::CanFire() const
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	return (Now - LastFireTime) >= GetFireInterval() && CurrentAmmo > 0;
}

void AXuanmingWeapon::StartFire()
{
	bWantsToFire = true;
	HandleFire();

	// 自动连发：按射速间隔重复
	const float Interval = GetFireInterval();
	if (Interval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(FireTimerHandle, this,
			&AXuanmingWeapon::HandleFire, Interval, true);
	}
}

void AXuanmingWeapon::StopFire()
{
	bWantsToFire = false;
	GetWorldTimerManager().ClearTimer(FireTimerHandle);
}

void AXuanmingWeapon::HandleFire()
{
	if (!bWantsToFire || !CanFire())
	{
		return;
	}

	AXuanmingCharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar)
	{
		return;
	}

	// 视线起点和方向
	const FVector EyeLoc = OwnerChar->GetEyeLocation();
	const FRotator EyeRot = OwnerChar->GetEyeRotation();
	FVector AimDir = EyeRot.Vector();

	// 散布
	if (BulletSpreadDegrees > 0.0f)
	{
		AimDir = FMath::VRandCone(AimDir, FMath::DegreesToRadians(BulletSpreadDegrees));
	}

	// 本地立刻递减弹药用于 UI 预测（服务器会同步真值回来覆盖）
	// 注意: 不要预先写 LastFireTime - PIE Standalone/ListenServer 下 client 和 server
	// 是同一个 weapon 实例, client 写了 LastFireTime=Now 后, 同帧 RPC 进入
	// Server_Fire_Implementation 检查 Now-LastFireTime=0 < Interval, CD reject!
	// LastFireTime 只能由 Server_Fire_Implementation 在真实 fire 时写.
	CurrentAmmo = FMath::Max(0, CurrentAmmo - 1);

	// 真实命中判定走服务器
	if (OwnerChar->IsLocallyControlled())
	{
		Server_Fire(EyeLoc, AimDir);
	}
}

bool AXuanmingWeapon::Server_Fire_Validate(const FVector& EyeLocation, const FVector& AimDirection)
{
	// 防作弊：校验方向是单位向量，不能为零
	return !AimDirection.IsNearlyZero();
}

void AXuanmingWeapon::Server_Fire_Implementation(const FVector& EyeLocation, const FVector& AimDirection)
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float Interval = GetFireInterval();
	const float Elapsed = Now - LastFireTime;
	const bool bCDOk = Elapsed >= Interval;
	const bool bAmmoOk = CurrentAmmo > 0;

	if (!bCDOk || !bAmmoOk)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Xuanming][Fire][Server] CanFire=false: CD=%d (Elapsed=%.3f >= %.3f) Ammo=%d (Cur=%d)"),
			bCDOk ? 1 : 0, Elapsed, Interval, bAmmoOk ? 1 : 0, CurrentAmmo);
		return;
	}

	LastFireTime = Now;
	CurrentAmmo = FMath::Max(0, CurrentAmmo - 1);

	const FVector End = EyeLocation + AimDirection.GetSafeNormal() * Range;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(XuanmingWeapon), true, this);
	Params.AddIgnoredActor(GetOwner());
	// 用 MultiTrace 检查所有命中物 (诊断: 看视线穿过了什么)
	TArray<FHitResult> AllHits;
	GetWorld()->LineTraceMultiByChannel(AllHits, EyeLocation, End, ECC_Pawn, Params);
	for (const FHitResult& H : AllHits)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Xuanming][Fire][Server] MultiTrace ECC_Pawn 命中: %s 组件=%s"),
			*GetNameSafe(H.GetActor()),
			*GetNameSafe(H.GetComponent()));
	}

	// 实际伤害判定: 改用 ECC_Pawn 而非 ECC_Visibility
	// ECC_Visibility 是相机/光照的视线遮挡通道, Capsule 上常被设为 Ignore;
	// FPS 子弹打人应该用 ECC_Pawn (Pawn collision profile 默认 Block).
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, End, ECC_Pawn, Params);

	UE_LOG(LogTemp, Warning,
		TEXT("[Xuanming][Fire][Server] Trace bHit=%d HitActor=%s Ammo剩=%d"),
		bHit ? 1 : 0,
		bHit ? *GetNameSafe(Hit.GetActor()) : TEXT("(none)"),
		CurrentAmmo);

	// 诊断: 打印 trace 起点/终点 + 视线方向, 帮定位为啥打不中 Dummy
	UE_LOG(LogTemp, Warning,
		TEXT("[Xuanming][Fire][Server] Trace 详情 EyeLoc=%s AimDir=%s End=%s"),
		*EyeLocation.ToCompactString(),
		*AimDirection.GetSafeNormal().ToCompactString(),
		*End.ToCompactString());

	// 诊断: 列出场景里所有 BP_XuanmingCharacter 的位置 (除了自己)
	for (TActorIterator<AXuanmingCharacter> It(GetWorld()); It; ++It)
	{
		AXuanmingCharacter* Ch = *It;
		if (Ch && Ch != GetOwner())
		{
			const FVector ChLoc = Ch->GetActorLocation();
			const float Dist = FVector::Dist(EyeLocation, ChLoc);
			const FVector ToTarget = (ChLoc - EyeLocation).GetSafeNormal();
			const float DotProd = FVector::DotProduct(AimDirection.GetSafeNormal(), ToTarget);
			UE_LOG(LogTemp, Warning,
				TEXT("[Xuanming][Fire][Server] 场景中 %s 位置=%s 距离=%.0f 视线点积=%.2f (1.0=正前方)"),
				*Ch->GetName(), *ChLoc.ToCompactString(), Dist, DotProd);
		}
	}

	if (bHit && Hit.GetActor())
	{
		FPointDamageEvent DamageEvent(Damage, Hit, AimDirection, nullptr);
		AController* InstigatorController = nullptr;
		if (AXuanmingCharacter* OwnerChar = GetOwnerCharacter())
		{
			InstigatorController = OwnerChar->GetController();
		}
		const float DamageDealt = Hit.GetActor()->TakeDamage(Damage, DamageEvent, InstigatorController, this);
		UE_LOG(LogTemp, Warning,
			TEXT("[Xuanming][Fire][Server] TakeDamage 返回=%.1f"), DamageDealt);
	}

	Multicast_PlayFireFX(bHit ? Hit.ImpactPoint : End, bHit);
}

void AXuanmingWeapon::Multicast_PlayFireFX_Implementation(const FVector& HitLocation, bool bHit)
{
	// TODO: 播放枪口火光、弹道光束、命中粒子
	// 现在只画调试线，方便联调
#if !UE_BUILD_SHIPPING
	if (AXuanmingCharacter* OwnerChar = GetOwnerCharacter())
	{
		const FVector Start = OwnerChar->GetEyeLocation();
		DrawDebugLine(GetWorld(), Start, HitLocation,
			bHit ? FColor::Red : FColor::Yellow, false, 0.5f, 0, 1.0f);
	}
#endif
}
