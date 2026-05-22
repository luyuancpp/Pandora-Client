// Copyright Xuanming. All Rights Reserved.

#include "XuanmingPlayerController.h"

AXuanmingPlayerController::AXuanmingPlayerController()
{
}

void AXuanmingPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("[Xuanming] PlayerController BeginPlay (IsLocal=%d)"), IsLocalController());
}

void AXuanmingPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	// 后续在这里绑定 EnhancedInput
}
