// Copyright Xuanming. All Rights Reserved.

#include "XuanmingCharacter.h"
#include "XuanmingWeapon.h"
#include "XuanmingPlayerController.h"
#include "XuanmingPlayerState.h"
#include "XuanmingAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
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
	if (IA_FrostCurse)
	{
		EIC->BindAction(IA_FrostCurse, ETriggerEvent::Started, this, &AXuanmingCharacter::Input_FrostCurse_Started);
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
			PC->LookInputConsumeFrames--;
			return;
		}
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

void AXuanmingCharacter::Input_FrostCurse_Started(const FInputActionValue& Value)
{
	// 通过 ASC 触发技能, 内部走 server 端 ActivateAbility (GA 配的 NetExecutionPolicy 决定)
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		// TryActivateAbilitiesByClass 走 GA 类查找, 命中 StartupAbilities 里 Given 过的实例
		// 找不到等价 class API 时, 退化用 GameplayTag (M1.5 先保持 class 查���简洁)
		for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartupAbilities)
		{
			if (*AbilityClass && AbilityClass->GetDefaultObject<UGameplayAbility>()->GetClass()->GetName().Contains(TEXT("FrostCurse")))
			{
				ASC->TryActivateAbilityByClass(AbilityClass);
				return;
			}
		}
	}
}

float AXuanmingCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
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
	UE_LOG(LogTemp, Log, TEXT("[Xuanming] XmDamageSelf Amount=%.1f"), Amount);
	// 走 ServerRPC, 让 server 真实扣血 (走标准 TakeDamage 链路, 验证 MVVM 推送)
	Server_DamageSelf(Amount);
}

void AXuanmingCharacter::Server_DamageSelf_Implementation(float Amount)
{
	// 在 server 上调 TakeDamage, 走完整伤害链路
	FDamageEvent DummyEvent;
	TakeDamage(Amount, DummyEvent, nullptr, this);
}

// === GAS ===

UAbilitySystemComponent* AXuanmingCharacter::GetAbilitySystemComponent() const
{
	if (const AXuanmingPlayerState* PS = GetPlayerState<AXuanmingPlayerState>())
	{
		return PS->GetAbilitySystemComponent();
	}
	return nullptr;
}

void AXuanmingCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	// server 端: PlayerState 此时已就绪
	InitAbilitySystem();
}

void AXuanmingCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	// client 端: PlayerState 复制过来后才能拿到 ASC
	InitAbilitySystem();
}

void AXuanmingCharacter::InitAbilitySystem()
{
	AXuanmingPlayerState* PS = GetPlayerState<AXuanmingPlayerState>();
	if (!PS)
	{
		return;
	}
	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// OwnerActor=PS (属性宿主), AvatarActor=this (技能作用者)
	ASC->InitAbilityActorInfo(PS, this);

	// 仅 server 给技能, 给后通过 replication 同步到 client
	if (HasAuthority() && !bAbilitiesGiven)
	{
		for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartupAbilities)
		{
			if (*AbilityClass)
			{
				ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, /*Level*/ 1, /*InputID*/ INDEX_NONE, this));
			}
		}
		bAbilitiesGiven = true;
	}

	// 同步初始 Health 到旧字段, 让 ViewModel 推送有正确起点
	if (UXuanmingAttributeSet* AS = PS->GetAttributeSet())
	{
		Health = AS->GetHealth();
		MaxHealth = AS->GetMaxHealth();
	}
}

void AXuanmingCharacter::OnHealthDepleted()
{
	if (HasAuthority() && Health <= 0.f)
	{
		HandleDeath();
	}
}
