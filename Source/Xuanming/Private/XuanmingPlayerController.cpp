// Copyright Xuanming. All Rights Reserved.

#include "XuanmingPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

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
}

void AXuanmingPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
	// 客户端收到 server 通知 possess 完成 = LocalPlayer 一定就绪了
	// DS+Client 模式 IMC 真正在这里注册成功
	UE_LOG(LogTemp, Warning, TEXT("[Xuanming] PlayerController AcknowledgePossession Pawn=%s"),
		*GetNameSafe(P));
	TryRegisterIMC(TEXT("AcknowledgePossession"));
}

void AXuanmingPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	// ListenServer 房主路径 (server 端的本地 PC), AcknowledgePossession 不会调
	if (IsLocalController())
	{
		TryRegisterIMC(TEXT("OnPossess"));
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
