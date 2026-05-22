// Copyright Xuanming. All Rights Reserved.

#include "XuanmingCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

AXuanmingCharacter::AXuanmingCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 第一人称相机，挂在头部
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	// 默认移动参数（后续可在蓝图覆盖）
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->JumpZVelocity = 420.0f;

	// 启用网络同步
	bReplicates = true;
	SetReplicateMovement(true);
}

void AXuanmingCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AXuanmingCharacter, Health);
}

void AXuanmingCharacter::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
}

void AXuanmingCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AXuanmingCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	// 只有服务器处理伤害，结果通过 Replication 同步到客户端
	if (!HasAuthority())
	{
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.0f)
	{
		Health = FMath::Max(0.0f, Health - ActualDamage);
		if (Health <= 0.0f)
		{
			HandleDeath();
		}
	}
	return ActualDamage;
}

void AXuanmingCharacter::OnRep_Health()
{
	// 客户端血量变化时触发，可刷新 HUD
}

void AXuanmingCharacter::HandleDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("[Xuanming] Character died: %s"), *GetName());
	// 后续：通知 GameMode、播放死亡动画、延迟重生
}
