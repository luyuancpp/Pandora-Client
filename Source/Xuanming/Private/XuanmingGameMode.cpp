// Copyright Xuanming. All Rights Reserved.

#include "XuanmingGameMode.h"
#include "XuanmingGameState.h"
#include "XuanmingPlayerController.h"
#include "XuanmingPlayerState.h"
#include "XuanmingCharacter.h"

AXuanmingGameMode::AXuanmingGameMode()
{
	// 绑定默认类
	GameStateClass = AXuanmingGameState::StaticClass();
	PlayerControllerClass = AXuanmingPlayerController::StaticClass();
	PlayerStateClass = AXuanmingPlayerState::StaticClass();
	DefaultPawnClass = AXuanmingCharacter::StaticClass();

	// 服务器 tick 频率
	bUseSeamlessTravel = true;
}

void AXuanmingGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("[Xuanming] GameMode BeginPlay on %s"),
		HasAuthority() ? TEXT("Server") : TEXT("Client"));
}

void AXuanmingGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	NumConnectedPlayers++;
	UE_LOG(LogTemp, Warning, TEXT("[Xuanming] Player joined. Total: %d"), NumConnectedPlayers);
}

void AXuanmingGameMode::Logout(AController* Exiting)
{
	NumConnectedPlayers = FMath::Max(0, NumConnectedPlayers - 1);
	UE_LOG(LogTemp, Warning, TEXT("[Xuanming] Player left. Remaining: %d"), NumConnectedPlayers);
	Super::Logout(Exiting);
}
