// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerUpActor.generated.h"

UCLASS()
class COOPGAME_API ASPowerUpActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPowerUpActor();

protected:
    /* Time between powerup effects */
    UPROPERTY(EditDefaultsOnly, Category = "PowerUps")
    float PowerUpInterval;

    UPROPERTY(EditDefaultsOnly, Category = "PowerUps")
    int32 TotalTicks;

    FTimerHandle TimerHandle_PowerUpTick;

    UFUNCTION()
    void OnTickPowerup();

    int32 TicksProcessed;

    UPROPERTY(ReplicatedUsing=OnRep_PowerUpActive)
    bool bIsPowerUpActive;

    UFUNCTION()
    void OnRep_PowerUpActive();

    UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
    void OnPowerUpStateChanged(bool bNewIsActive);

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
    void OnActivated(AActor* ActivateFor);

    UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
    void OnExpired();

    UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
    void OnPowerUpTicked();

    void ActivatePowerUp(AActor* ActivateFor);

};
