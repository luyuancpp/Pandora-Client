// Copyright Xuanming. All Rights Reserved.

#include "XuanmingCharacter.h"
#include "XuanmingWeapon.h"
#include "XuanmingPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DamageEvents.h"

AXuanmingCharacter::AXuanmingCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 第一人称相机
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	// 默认移动参数
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->JumpZVelocity = 420.0f;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;

	// 蹲伏支持
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// 网络
	bReplicates = true;
	SetReplicateMovement(true);
}

void AXuanmingCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AXuanmingCharacter, Health);
	DOREPLIFETIME(AXuanmingCharacter, CurrentWeapon);
}

void AXuanmingCharacter::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;

	if (HasAuthority())
	{
		SpawnDefaultWeapon();
	}
}

void AXuanmingCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AXuanmingCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		return;
	}

	if (IA_Move)
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AXuanmingCharacter::Input_Move);
	}
	if (IA_Look)
	{
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AXuanmingCharacter::Input_Look);
	}
	if (IA_Jump)
	{
		EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &AXuanmingCharacter::Input_Jump_Started);
		EIC->BindAction(IA_Jump, ETriggerEvent::Completed, this, &AXuanmingCharacter::Input_Jump_Completed);
	}
	if (IA_Fire)
	{
		EIC->BindAction(IA_Fire, ETriggerEvent::Started, this, &AXuanmingCharacter::Input_Fire_Started);
		EIC->BindAction(IA_Fire, ETriggerEvent::Completed, this, &AXuanmingCharacter::Input_Fire_Completed);
	}
	if (IA_Crouch)
	{
		EIC->BindAction(IA_Crouch, ETriggerEvent::Started, this, &AXuanmingCharacter::Input_Crouch_Toggled);
	}
}

void AXuanmingCharacter::Input_Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller == nullptr || Axis.IsNearlyZero())
	{
		return;
	}
	const FRotator Rot = Controller->GetControlRotation();
	const FRotator YawRot(0.f, Rot.Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right, Axis.X);
}

void AXuanmingCharacter::Input_Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	// 兜底吞掉切 InputMode 后接下来 N 帧 Look 输入, 避免 mouse capture 重置时
	// 累积的大 delta 把镜头瞬间拉到脚下/天空. 详见 PC::ApplyFPSInputMode 注释.
	// 实测 UE 5.7.4 把异常 delta 分多帧灌入, 单帧吞不够.
	if (AXuanmingPlayerController* PC = Cast<AXuanmingPlayerController>(Controller))
	{
		if (PC->LookInputConsumeFrames > 0)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[Xuanming][Look] CONSUMED frame=%d Axis=(%.3f, %.3f)"),
				PC->LookInputConsumeFrames, Axis.X, Axis.Y);
			PC->LookInputConsumeFrames--;
			return;
		}
	}

	// 诊断日志: 只打 |delta| > 5 的, 避免刷屏. 正常鼠标转头 delta 在 0.1~3 之间.
	// 如果看到 |delta| > 10 -> 就是首次 capture 异常 delta 漏过来了
	if (FMath::Abs(Axis.X) > 5.0f || FMath::Abs(Axis.Y) > 5.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Xuanming][Look] BIG DELTA: Axis=(%.3f, %.3f) -> YawDelta=%.3f PitchDelta=%.3f"),
			Axis.X, Axis.Y, Axis.X, -Axis.Y);
	}

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(-Axis.Y); // 反转 Y 轴，符合 FPS 习惯
}

void AXuanmingCharacter::Input_Jump_Started(const FInputActionValue& Value)
{
	Jump();
}

void AXuanmingCharacter::Input_Jump_Completed(const FInputActionValue& Value)
{
	StopJumping();
}

void AXuanmingCharacter::Input_Fire_Started(const FInputActionValue& Value)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void AXuanmingCharacter::Input_Fire_Completed(const FInputActionValue& Value)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void AXuanmingCharacter::Input_Crouch_Toggled(const FInputActionValue& Value)
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

float AXuanmingCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Warning,
		TEXT("[Xuanming][Damage] %s 收到伤害=%.1f from=%s causer=%s HasAuth=%d Health(before)=%.1f"),
		*GetName(), DamageAmount,
		*GetNameSafe(EventInstigator), *GetNameSafe(DamageCauser),
		HasAuthority() ? 1 : 0, Health);

	if (!HasAuthority())
	{
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.0f)
	{
		Health = FMath::Max(0.0f, Health - ActualDamage);
		UE_LOG(LogTemp, Warning,
			TEXT("[Xuanming][Damage] %s Health(after)=%.1f (实际扣血=%.1f)"),
			*GetName(), Health, ActualDamage);
		if (Health <= 0.0f)
		{
			HandleDeath();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Xuanming][Damage] %s Super::TakeDamage 返回 0 (伤害被吞 - DamageType/Resistance?)"),
			*GetName());
	}
	return ActualDamage;
}

void AXuanmingCharacter::OnRep_Health()
{
}

FVector AXuanmingCharacter::GetEyeLocation() const
{
	return FirstPersonCamera ? FirstPersonCamera->GetComponentLocation() : GetActorLocation();
}

FRotator AXuanmingCharacter::GetEyeRotation() const
{
	if (AController* C = GetController())
	{
		return C->GetControlRotation();
	}
	return FirstPersonCamera ? FirstPersonCamera->GetComponentRotation() : GetActorRotation();
}

void AXuanmingCharacter::SpawnDefaultWeapon()
{
	if (!HasAuthority() || !DefaultWeaponClass)
	{
		return;
	}
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	CurrentWeapon = GetWorld()->SpawnActor<AXuanmingWeapon>(DefaultWeaponClass,
		GetActorLocation(), GetActorRotation(), Params);
	if (CurrentWeapon)
	{
		CurrentWeapon->AttachToComponent(GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			TEXT("WeaponSocket")); // 需要在角色 Mesh 上加一个 WeaponSocket
	}
}

void AXuanmingCharacter::HandleDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("[Xuanming] Character died: %s"), *GetName());
	// TODO: 通知 GameMode、播放死亡动画、延迟重生
}

void AXuanmingCharacter::XmDamageSelf(float Amount)
{
	UE_LOG(LogTemp, Warning, TEXT("[Xuanming][DamageSelf] 控制台触发 self-damage Amount=%.1f IsLocal=%d HasAuth=%d"),
		Amount, IsLocallyControlled() ? 1 : 0, HasAuthority() ? 1 : 0);
	// 走 ServerRPC, 让 server 真实扣血 (走标准 TakeDamage 链路, 验证 MVVM 推送)
	Server_DamageSelf(Amount);
}

void AXuanmingCharacter::Server_DamageSelf_Implementation(float Amount)
{
	// 在 server 上调 TakeDamage, 走完整伤害链路
	FDamageEvent DummyEvent;
	TakeDamage(Amount, DummyEvent, nullptr, this);
}
