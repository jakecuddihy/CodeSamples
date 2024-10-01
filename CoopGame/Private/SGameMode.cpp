// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"
#include "TimerManager.h"
#include "SHealthComponent.h"
#include "SPlayerState.h"
#include "SGameState.h"

static int32 DebugGameModeDrawing = 0;
FAutoConsoleVariableRef CVARDebugGameModeDrawing(
    TEXT("COOP.DebugGameModeDrawing"),
    DebugGameModeDrawing,
    TEXT("Print game mode status information."),
    ECVF_Cheat);

ASGameMode::ASGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	TickInterval = 0.3f;
	PrimaryActorTick.TickInterval = TickInterval;
;
    GameStateClass = ASGameState::StaticClass();
    PlayerStateClass = ASPlayerState::StaticClass();

	TimeBetweenWaves = 2.0f;
	StartingWave = 0;
	BotSpawnRate = 0.1f;
	BaseWaveSize = 5;
	WaveIncreaseExponent = 3;
}

void ASGameMode::StartPlay()
{
	Super::StartPlay();
	CurrentWave = StartingWave;
	PrepNextWave();
}

void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CheckWaveState();
	CheckForLivePlayers();
}

void ASGameMode::PrepNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);
	UE_LOG(LogTemp, Log, TEXT("Prepping next wave."));

	SetWaveState(EWaveState::WaitingToStart);

	RespawnDeadPlayers();
}

void ASGameMode::StartWave()
{
    CurrentWave++;
    UE_LOG(LogTemp, Log, TEXT("Wave %d started."), CurrentWave);

    NumberOfBotsToSpawn = FMath::Pow(CurrentWave, WaveIncreaseExponent) + BaseWaveSize;
    UE_LOG(LogTemp, Log, TEXT("%d bots to spawn."), NumberOfBotsToSpawn);

    GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, BotSpawnRate, true, 0.0f);

    SetWaveState(EWaveState::WaveInProgress);
}

void ASGameMode::CheckWaveState()
{
    bool bIsPreparingWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	// Wave isn't over yet, don't bother checking if all bots are dead
    if (NumberOfBotsToSpawn > 0 || bIsPreparingWave) { return; }

    bool bIsAnyBotAlive = false;

    for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
    {
        APawn* TestPawn = It->Get();

        if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
        {
            continue;
        }

        USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
        if (HealthComp && HealthComp->GetHealth() > 0.0f)
        {
            bIsAnyBotAlive = true;
            break;
        }
    }

    if (!bIsAnyBotAlive)
    {
        SetWaveState(EWaveState::WaveComplete);
        PrepNextWave();
    }
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	UE_LOG(LogTemp, Log, TEXT("Wave ended."));

	//SetWaveState(EWaveState::WaitingToComplete);

	SetWaveState(EWaveState::WaveComplete);
	PrepNextWave();
}

void ASGameMode::CheckForLivePlayers()
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();

        if (PC && PC->GetPawn())
        {
            APawn* MyPawn = PC->GetPawn();
            USHealthComponent* HealthComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));

            if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f)
            {
                // A player is still alive
                return;
            }
        }
    }

    // No player alive
    GameOver();
}

void ASGameMode::RespawnDeadPlayers()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();

		if (PC && PC->GetPawn() == nullptr)
		{

			UE_LOG(LogTemp, Log, TEXT("Respawned player."));
			RestartPlayer(PC);
		}
	}
}

void ASGameMode::GameOver()
{
	UE_LOG(LogTemp, Log, TEXT("Game over, players died."));
    //EndWave();

    SetWaveState(EWaveState::GameOver);
}

void ASGameMode::SetWaveState(EWaveState NewState)
{
    ASGameState* GS = GetGameState<ASGameState>();
    if (ensureAlways(GS))
    {
        GS->SetWaveState(NewState);
    }
}

void ASGameMode::SpawnBotTimerElapsed()
{
    SpawnNewBot();

    NumberOfBotsToSpawn--;

    if (NumberOfBotsToSpawn <= 0)
    {
		SetWaveState(EWaveState::WaitingToComplete);
		GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
    }
}

void ASGameMode::SetCurrentWave(int32 NewWave)
{
	CurrentWave = NewWave;
}

int32 ASGameMode::GetCurrentWave()
{
	return CurrentWave;
}