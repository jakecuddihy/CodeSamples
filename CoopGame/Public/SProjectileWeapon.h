// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASProjectileWeapon : public ASWeapon
{
	GENERATED_BODY()

protected:

    virtual void StartFire() override;

    UPROPERTY(EditDefaultsOnly, Category = "ProjectileWeapon")
    TSubclassOf<AActor> ProjectileClass;
	
};
