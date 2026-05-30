// Copyright Xuanming. All Rights Reserved.

#include "XuanmingWeapon.h"
#include "XuanmingCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
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

	LastFireTime = GetWorld()->GetTimeSeconds();

	// 本地立刻递减弹药用于 UI 预测（服务器会同步真值回来覆盖）
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
	if (!CanFire())
	{
		return;
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
	CurrentAmmo = FMath::Max(0, CurrentAmmo - 1);

	const FVector End = EyeLocation + AimDirection.GetSafeNormal() * Range;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(XuanmingWeapon), true, this);
	Params.AddIgnoredActor(GetOwner());
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, End, ECC_Visibility, Params);

	if (bHit && Hit.GetActor())
	{
		FPointDamageEvent DamageEvent(Damage, Hit, AimDirection, nullptr);
		AController* InstigatorController = nullptr;
		if (AXuanmingCharacter* OwnerChar = GetOwnerCharacter())
		{
			InstigatorController = OwnerChar->GetController();
		}
		Hit.GetActor()->TakeDamage(Damage, DamageEvent, InstigatorController, this);
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
