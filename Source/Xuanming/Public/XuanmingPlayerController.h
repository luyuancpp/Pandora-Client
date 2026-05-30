// Copyright Xuanming. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "XuanmingPlayerController.generated.h"

class UInputMappingContext;

/**
 * 玄冥 PlayerController - 玩家的输入和 UI 入口
 * 客户端和服务器各持有一份本玩家的 PC
 */
UCLASS()
class XUANMING_API AXuanmingPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AXuanmingPlayerController();

	/** 玩家默认输入映射上下文（在 BP 里指派 IMC 资产） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Xuanming|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

protected:
	virtual void BeginPlay() override;
	/** 客户端 LocalPlayer 真正就绪后调用 (DS+Client 模式下最可靠的 IMC 注册时机) */
	virtual void AcknowledgePossession(APawn* P) override;
	/** 重启已有 LocalPlayer 时也走 IMC 注册 (兼容 ListenServer + 重新 possess) */
	virtual void OnPossess(APawn* InPawn) override;

private:
	/** 把 DefaultMappingContext 加到 LocalPlayer 的 EnhancedInputSubsystem, 幂等 */
	void TryRegisterIMC(const TCHAR* From);
	bool bIMCRegistered = false;
};
