#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

UENUM(BlueprintType)
enum class EWaveState : uint8
{
    WaitingToStart,

    WaveInProgress,

    // No longer spawning bots. Now waiting for players to kill bots.
    WaitingToComplete,

    WaveComplete,

    GameOver,

};