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
}

void AXuanmingPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	// ListenServer 房主路径 (server 端的本地 PC), AcknowledgePossession 不会调
	if (IsLocalController())
	{
		TryRegisterIMC(TEXT("OnPossess"));
		TryCreateHUD(TEXT("OnPossess"));
	}
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
