// Copyright Pandora. All Rights Reserved.

#include "PandoraGameMode.h"
#include "PandoraGameState.h"
#include "PandoraPlayerController.h"
#include "PandoraPlayerState.h"
#include "PandoraCharacter.h"
#include "PandoraHUD.h"

APandoraGameMode::APandoraGameMode()
{
	// 绑定默认类
	GameStateClass = APandoraGameState::StaticClass();
	PlayerControllerClass = APandoraPlayerController::StaticClass();
	PlayerStateClass = APandoraPlayerState::StaticClass();
	DefaultPawnClass = APandoraCharacter::StaticClass();
	HUDClass = APandoraHUD::StaticClass();

	// 服务器 tick 频率
	bUseSeamlessTravel = true;
}

void APandoraGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("[Pandora] GameMode BeginPlay on %s"),
		HasAuthority() ? TEXT("Server") : TEXT("Client"));
}

void APandoraGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	NumConnectedPlayers++;
	UE_LOG(LogTemp, Warning, TEXT("[Pandora] Player joined. Total: %d"), NumConnectedPlayers);
}

void APandoraGameMode::Logout(AController* Exiting)
{
	NumConnectedPlayers = FMath::Max(0, NumConnectedPlayers - 1);
	UE_LOG(LogTemp, Warning, TEXT("[Pandora] Player left. Remaining: %d"), NumConnectedPlayers);
	Super::Logout(Exiting);
}