// Copyright Pandora. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PandoraPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UPandoraPlayerViewModel;

/**
 * Pandora PlayerController - 玩家的输入和 UI 入口
 * 客户端和服务器各持有一份本玩家的 PC
 */
UCLASS()
class PANDORA_API APandoraPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APandoraPlayerController();

	/** 玩家默认输入映射上下文（在 BP 里指派 IMC 资产） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pandora|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** 玩家 HUD Widget 类（在 BP 里指派 WBP_PlayerHUD） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pandora|UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	/** 运行时的 HUD 实例（仅本地客户端） */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Pandora|UI")
	TObjectPtr<UUserWidget> HUDWidget;

	/** 运行时的 HUD ViewModel (MVVM 数据源, 仅本地客户端) */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Pandora|UI")
	TObjectPtr<UPandoraPlayerViewModel> PlayerViewModel;

	/**
	 * WBP 里 MVVM 面板配置的 ViewModel 名字 (Add Viewmodel 时填写).
	 * 必须和 WBP 里 ViewModel 的 Name 字段完全一致, 默认 "PlayerViewModel".
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pandora|UI")
	FName PlayerViewModelName = TEXT("PlayerViewModel");

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PlayerTick(float DeltaTime) override;
	/** 客户端 LocalPlayer 真正就绪后调用 (DS+Client 模式下最可靠的 IMC 注册时机) */
	virtual void AcknowledgePossession(APawn* P) override;
	/** 重启已有 LocalPlayer 时也走 IMC 注册 (兼容 ListenServer + 重新 possess) */
	virtual void OnPossess(APawn* InPawn) override;

private:
	/** 把 DefaultMappingContext 加到 LocalPlayer 的 EnhancedInputSubsystem, 幂等 */
	void TryRegisterIMC(const TCHAR* From);
	bool bIMCRegistered = false;

	/** CreateWidget(HUDWidgetClass) + AddToViewport + 注入 ViewModel, 幂等. 仅本地客户端. */
	void TryCreateHUD(const TCHAR* From);

	/** 把 PlayerViewModel 推到 HUDWidget 的 MVVM View, 幂等返回 true 表示成功 */
	bool TryInjectViewModel(const TCHAR* From);
	bool bViewModelInjected = false;

	/** 每帧把 Pawn/Weapon 状态推到 ViewModel (Setter 内部比较新旧值, 不变就不广播) */
	void PushStateToViewModel();

	/** 设 GameOnly 输入模式 + 隐藏鼠标. 幂等, 多时机兜底. */
	void ApplyFPSInputMode();

public:
	/**
	 * 兜底吞掉切 InputMode 后接下来的 N 帧 Look 输入.
	 * 根因方案 (ApplyFPSInputMode 里的 capture 配置) 不能完全消除首次 mouse delta,
	 * 实测 UE 5.7.4 会把异常 delta 分多帧灌入 (第 1 帧小, 第 2 帧 |Axis|≈30, 第 3 帧 ≈8),
	 * 所以一次性吞前几帧最稳.
	 * Character::Input_Look 第一行检查这个值, > 0 就跳过并自减.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Pandora|Input")
	int32 LookInputConsumeFrames = 0;

	/** 切 InputMode 后吞掉的 Look 帧数. 默认 5 帧 (~0.08s @ 60fps), 玩家无感. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pandora|Input")
	int32 LookConsumeFramesAfterModeSwitch = 5;
};
