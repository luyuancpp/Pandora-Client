// Copyright Xuanming. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "XuanmingCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

/**
 * 玄冥 Character - FPS 玩家角色
 * 包含基础同步血量、第一/第三人称相机、武器挂点（后续接入）
 */
UCLASS()
class XUANMING_API AXuanmingCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AXuanmingCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	/** 血量，服务器权威 */
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Xuanming|Combat")
	float Health = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|Combat")
	float MaxHealth = 100.0f;

	UFUNCTION()
	void OnRep_Health();

protected:
	virtual void BeginPlay() override;

	/** 第一人称相机 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Xuanming|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	/** 死亡处理（仅服务器） */
	virtual void HandleDeath();
};
