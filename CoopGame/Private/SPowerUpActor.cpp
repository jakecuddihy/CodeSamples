// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerUpActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASPowerUpActor::ASPowerUpActor()
{
    PowerUpInterval = 0.0f;
    TotalTicks = 0;

    bIsPowerUpActive = false;

    SetReplicates(true);
}

void ASPowerUpActor::OnTickPowerup()
{
    TicksProcessed++;

    OnPowerUpTicked();

    if (TicksProcessed >= TotalTicks)
    {
        OnExpired();

        bIsPowerUpActive = false;
        OnRep_PowerUpActive();

        GetWorldTimerManager().ClearTimer(TimerHandle_PowerUpTick);
    }
}

void ASPowerUpActor::OnRep_PowerUpActive()
{
    OnPowerUpStateChanged(bIsPowerUpActive);
}

void ASPowerUpActor::ActivatePowerUp(AActor* ActivateFor)
{
    OnActivated(ActivateFor);

    bIsPowerUpActive = true;
    OnRep_PowerUpActive();

    if (PowerUpInterval > 0.0f)
    {
        GetWorldTimerManager().SetTimer(TimerHandle_PowerUpTick, this, &ASPowerUpActor::OnTickPowerup, PowerUpInterval, true);
    }
    else
    {
        OnTickPowerup();
    }
}


void ASPowerUpActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASPowerUpActor, bIsPowerUpActive);
}