// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameState.h"
#include "Net/UnrealNetwork.h"

static int32 DebugGameStateDrawing = 0;
FAutoConsoleVariableRef CVARDebugGameStateDrawing(
    TEXT("COOP.DebugGameStateDrawing"),
    DebugGameStateDrawing,
    TEXT("Print game state information"),
    ECVF_Cheat);

void ASGameState::OnRep_WaveState(EWaveState OldState)
{
    WaveStateChanged(WaveState, OldState);
}

void ASGameState::SetWaveState(EWaveState NewWaveState)
{
    if (Role == ROLE_Authority)
    {
         EWaveState OldState = WaveState;
         WaveState = NewWaveState;
        // Manually call on server because on rep only happens on clients
         OnRep_WaveState(OldState);
    }
}

void ASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASGameState, WaveState);
}