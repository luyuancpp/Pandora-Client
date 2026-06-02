// Copyright Xuanming. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "XuanmingCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class AXuanmingWeapon;
class UAbilitySystemComponent;
class UXuanmingAttributeSet;
class UGameplayAbility;
struct FInputActionValue;

/**
 * 玄冥 Character - FPS 玩家角色
 * 包含输入处理、相机、武器持有、服务器权威血量与伤害
 */
UCLASS()
class XUANMING_API AXuanmingCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AXuanmingCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	// === GAS ===
	/** PlayerState 上的 ASC, server/client 都能拿到 */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** server 端调: 玩家被 possess 时初始化 ASC ActorInfo + Give 启动技能 */
	virtual void PossessedBy(AController* NewController) override;

	/** client 端调: PlayerState 复制过来后初始化 ASC ActorInfo (避免 ASC 接口指针空) */
	virtual void OnRep_PlayerState() override;

	/** 启动时给玩家的技能列表 (在 BP 默认值里指派, 例如 GA_FrostCurse) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|GAS")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	/** AttributeSet 推 Health 到 Character.Health 后, 检测到归零回调死亡 */
	void OnHealthDepleted();

	// === 输入 IA 资产（在 BP 里指派）===
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|Input")
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|Input")
	TObjectPtr<UInputAction> IA_Fire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|Input")
	TObjectPtr<UInputAction> IA_Crouch;

	/** 冰咒技能输入 (Q 键, 在 IMC 里映射到 IA_FrostCurse) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|Input")
	TObjectPtr<UInputAction> IA_FrostCurse;

	// === 状态 ===
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Xuanming|Combat")
	float Health = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|Combat")
	float MaxHealth = 100.0f;

	// === 武器 ===
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|Weapon")
	TSubclassOf<AXuanmingWeapon> DefaultWeaponClass;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Xuanming|Weapon")
	TObjectPtr<AXuanmingWeapon> CurrentWeapon;

	UFUNCTION()
	void OnRep_Health();

	/** 由武器查询：相机/眼睛位置，用于射线起点 */
	UFUNCTION(BlueprintCallable, Category = "Xuanming|Combat")
	FVector GetEyeLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Xuanming|Combat")
	FRotator GetEyeRotation() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Xuanming|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	// === 输入处理函数 ===
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_Jump_Started(const FInputActionValue& Value);
	void Input_Jump_Completed(const FInputActionValue& Value);
	void Input_Fire_Started(const FInputActionValue& Value);
	void Input_Fire_Completed(const FInputActionValue& Value);
	void Input_Crouch_Toggled(const FInputActionValue& Value);
	void Input_FrostCurse_Started(const FInputActionValue& Value);

	/** 初始化 ASC ActorInfo + (仅 server) Give 启动技能, 幂等 */
	void InitAbilitySystem();
	bool bAbilitiesGiven = false;

	/** 服务器端生成默认武器并附加到角色 */
	void SpawnDefaultWeapon();

	/** 死亡处理（仅服务器） */
	virtual void HandleDeath();

public:
	/**
	 * 调试用自伤命令: PIE 控制台直接输入 "XmDamageSelf 50" 即可对自己造成 50 伤害.
	 * Exec 关键字让函数自动注册为控制台命令.
	 * 仅本地客户端调用, 内部走 ServerRPC 让 server 真实扣血.
	 */
	UFUNCTION(Exec)
	void XmDamageSelf(float Amount = 25.f);

private:
	UFUNCTION(Server, Reliable)
	void Server_DamageSelf(float Amount);
};
