// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SEnums.h"
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, VictimActor, AActor*, KillerActor, AController*, KillerController); // Killed actor, killer actor,

UCLASS()
class COOPGAME_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:

    // Hook for BP to spawn a single bot
    UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
    void SpawnNewBot();

    void SpawnBotTimerElapsed();

    void StartWave();

    void EndWave();

    // Set timer for next start wave
    void PrepNextWave();

    void CheckWaveState();

    void CheckForLivePlayers();

    void GameOver();

    void SetWaveState(EWaveState NewState);

    void RespawnDeadPlayers();

    FTimerHandle TimerHandle_BotSpawner;

    FTimerHandle TimerHandle_NextWaveStart;
	
    int32 NumberOfBotsToSpawn;

    int32 CurrentWave;

    UPROPERTY(EditDefaultsOnly, Category = "GameMode")
    float TimeBetweenWaves;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TickInterval;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float BotSpawnRate;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	int32 StartingWave;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	int32 WaveIncreaseExponent;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	int32 BaseWaveSize;

public:

    ASGameMode();

    virtual void StartPlay() override;

    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(BlueprintAssignable, Category = "GameMode")
    FOnActorKilled OnActorKilled;

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void SetCurrentWave(int32 NewWave);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode")
	int32 GetCurrentWave();
};
