// Copyright Xuanming. All Rights Reserved.

#include "XuanmingPlayerController.h"
#include "XuanmingCharacter.h"
#include "XuanmingWeapon.h"
#include "XuanmingPlayerViewModel.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "MVVMSubsystem.h"
#include "View/MVVMView.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"

AXuanmingPlayerController::AXuanmingPlayerController()
{
}

void AXuanmingPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("[Xuanming] PlayerController BeginPlay (IsLocal=%d)"), IsLocalController());

	// PIE 单机 / ListenServer 模式 BeginPlay 时 LocalPlayer 已就绪, 这里能成功注册
	// 真 DS+Client 模式 BeginPlay 太早 GetLocalPlayer() 返回 null, 走 AcknowledgePossession 兜底
	TryRegisterIMC(TEXT("BeginPlay"));
	TryCreateHUD(TEXT("BeginPlay"));

	// FPS 输入模式: 鼠标锁视口 + 隐藏光标
	// 不设的话 PIE 默认 GameAndUI: 鼠标可在视口外漂, 首次点击会"瞬移"光标回视口导致镜头猛转
	if (IsLocalController())
	{
		ApplyFPSInputMode();
	}
}

void AXuanmingPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HUDWidget)
	{
		HUDWidget->RemoveFromParent();
		HUDWidget = nullptr;
	}
	PlayerViewModel = nullptr;
	bViewModelInjected = false;
	Super::EndPlay(EndPlayReason);
}

void AXuanmingPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	// HUD 创建可能因 LocalPlayer 时机问题被推迟, 每帧重试一次直到成功
	if (IsLocalController())
	{
		if (!HUDWidget)
		{
			TryCreateHUD(TEXT("PlayerTick"));
		}
		else if (!bViewModelInjected)
		{
			TryInjectViewModel(TEXT("PlayerTick"));
		}
		PushStateToViewModel();
	}
}

void AXuanmingPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
	// 客户端收到 server 通知 possess 完成 = LocalPlayer 一定就绪了
	// DS+Client 模式 IMC / HUD 真正在这里注册成功
	UE_LOG(LogTemp, Warning, TEXT("[Xuanming] PlayerController AcknowledgePossession Pawn=%s"),
		*GetNameSafe(P));
	TryRegisterIMC(TEXT("AcknowledgePossession"));
	TryCreateHUD(TEXT("AcknowledgePossession"));
	ApplyFPSInputMode(); // DS+Client 模式下 BeginPlay 可能比 possess 早，这里再兜底一次
}

void AXuanmingPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	// ListenServer 房主路径 (server 端的本地 PC), AcknowledgePossession 不会调
	if (IsLocalController())
	{
		TryRegisterIMC(TEXT("OnPossess"));
		TryCreateHUD(TEXT("OnPossess"));
		ApplyFPSInputMode();
	}
}

void AXuanmingPlayerController::ApplyFPSInputMode()
{
	// FPS 输入模式 (幂等可重复调用):
	// 关键: 不能用 SetConsumeCaptureMouseDown(true), 那会让"第一次点击"被当成
	//       mouse capture 重置事件, 导致鼠标 delta 灌进 Look 输入 (镜头瞬间朝下脚下)
	// 关键: HUDWidget 已 AddToViewport, 但默认 IsFocusable=true 会抢 focus,
	//       必须显式 SetIsFocusable(false), 否则首次点击 = focus 从 UMG 还给 viewport,
	//       这一还原过程会触发 mouse delta -> 镜头乱转
	UE_LOG(LogTemp, Warning, TEXT("[Xuanming] FPS InputMode applied"));

	if (HUDWidget)
	{
		HUDWidget->SetIsFocusable(false);
	}

	FInputModeGameOnly InputMode;
	// SetConsumeCaptureMouseDown 默认 false, 不要改 - 否则首次点击触发 viewport
	// 重新捕获鼠标, 期间的鼠标 delta 会灌进 Look
	SetInputMode(InputMode);

	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;

	// 主动让 game viewport 抓住鼠标焦点 + 锁定光标到视口中心
	// 没这步的话, 第一次点击时引擎会做 "光标归位 + capture", 期间 mouse delta 会被 Look 吃掉
	if (UGameViewportClient* VC = GetWorld() ? GetWorld()->GetGameViewport() : nullptr)
	{
		VC->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
		VC->SetMouseLockMode(EMouseLockMode::LockAlways);
		VC->SetHideCursorDuringCapture(true);
		// 触发一次 SetUserFocus, 把焦点立刻给 viewport, 不等到首次点击
		FSlateApplication::Get().SetAllUserFocusToGameViewport();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Xuanming] FPS InputMode: GameViewport=null"));
	}

	FlushPressedKeys();
	// 兜底: 吞掉接下来 N 帧 Look 输入.
	// 实测 UE 5.7.4 把首次 capture 异常 delta 分多帧灌入 (帧1小, 帧2 |Axis|≈30, 帧3 ≈8),
	// 一次性吞前几帧最稳. 5 帧 @60fps ≈ 0.08s, 玩家无感.
	LookInputConsumeFrames = LookConsumeFramesAfterModeSwitch;
}

void AXuanmingPlayerController::TryRegisterIMC(const TCHAR* From)
{
	if (bIMCRegistered)
	{
		return;
	}
	if (!IsLocalController() || !DefaultMappingContext)
	{
		return;
	}
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Xuanming] IMC 注册推迟(%s): LocalPlayer 未就绪"), From);
		return;
	}
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP);
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Xuanming] IMC 注册失败(%s): EnhancedInputSubsystem 未就绪"), From);
		return;
	}
	Subsystem->AddMappingContext(DefaultMappingContext, 0);
	bIMCRegistered = true;
	UE_LOG(LogTemp, Warning, TEXT("[Xuanming] IMC 注册成功(%s): %s"),
		From, *DefaultMappingContext->GetName());
}

void AXuanmingPlayerController::TryCreateHUD(const TCHAR* From)
{
	// 仅本地客户端 (DS 不画 HUD); 已创建则跳过
	if (!IsLocalController() || HUDWidget != nullptr)
	{
		return;
	}
	if (!HUDWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Xuanming] HUD 创建跳过(%s): HUDWidgetClass 未指派 (BP_XuanmingPlayerController.HUDWidgetClass)"),
			From);
		return;
	}
	// LocalPlayer 没就绪先推迟, 后续时机会再调
	if (!GetLocalPlayer())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Xuanming] HUD 创建推迟(%s): LocalPlayer 未就绪"), From);
		return;
	}

	HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
	if (!HUDWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[Xuanming] HUD 创建失败(%s): CreateWidget 返回 null"), From);
		return;
	}
	HUDWidget->AddToViewport(0);
	UE_LOG(LogTemp, Warning, TEXT("[Xuanming] HUD 创建成功(%s): %s"),
		From, *HUDWidgetClass->GetName());

	// 立即创建 ViewModel 并尝试注入 (WBP MVVM View 在 NativeConstruct 时已就绪)
	if (!PlayerViewModel)
	{
		PlayerViewModel = NewObject<UXuanmingPlayerViewModel>(this);
		UE_LOG(LogTemp, Warning, TEXT("[Xuanming] PlayerViewModel 创建完成"));
	}
	TryInjectViewModel(From);
}

bool AXuanmingPlayerController::TryInjectViewModel(const TCHAR* From)
{
	if (bViewModelInjected)
	{
		return true;
	}
	if (!HUDWidget || !PlayerViewModel)
	{
		return false;
	}

	// MVVMSubsystem 是 EngineSubsystem, 取 View ��需要 GameInstance
	UMVVMView* View = UMVVMSubsystem::GetViewFromUserWidget(HUDWidget);
	if (!View)
	{
		// WBP 没在 MVVM 面板配过任何 ViewModel/Binding 就拿不到 View
		// 这是用户操作步骤未完成的预期错误, 提示一次然后停止重试
		UE_LOG(LogTemp, Warning,
			TEXT("[Xuanming] ViewModel 注入跳过(%s): WBP_PlayerHUD 未配 MVVM Bindings (Window->View Bindings)"),
			From);
		bViewModelInjected = true; // 标记成"已尝试", 避免每帧刷屏
		return false;
	}

	// ViewModel 名字必须和 WBP MVVM 面板里 Add Viewmodel 时填的 Name 一致
	const bool bOk = View->SetViewModel(PlayerViewModelName, PlayerViewModel);
	if (!bOk)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Xuanming] ViewModel 注入失败(%s): SetViewModel(%s) 返回 false. 检查 WBP 里 ViewModel Name 是否匹配, Creation Type 是否为 Manual"),
			From, *PlayerViewModelName.ToString());
		bViewModelInjected = true;
		return false;
	}

	bViewModelInjected = true;
	UE_LOG(LogTemp, Warning, TEXT("[Xuanming] ViewModel 注入成功(%s): %s"),
		From, *PlayerViewModelName.ToString());
	return true;
}

void AXuanmingPlayerController::PushStateToViewModel()
{
	if (!PlayerViewModel)
	{
		return;
	}

	AXuanmingCharacter* Ch = Cast<AXuanmingCharacter>(GetPawn());
	if (!Ch)
	{
		return;
	}

	// SetPropertyValue 内部会比较新旧值, 相同则不广播 -- 即使每帧调用,
	// MVVM Bindings 仍只在数据真正变化时执行. 这就是 MVVM 相对函数 Binding 的核心优势.
	PlayerViewModel->SetHealth(Ch->Health);
	PlayerViewModel->SetMaxHealth(Ch->MaxHealth);

	if (AXuanmingWeapon* W = Ch->CurrentWeapon)
	{
		PlayerViewModel->SetCurrentAmmo(W->CurrentAmmo);
		PlayerViewModel->SetMagazineSize(W->MagazineSize);
	}
}
