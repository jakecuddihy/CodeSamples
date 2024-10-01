// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

// Contains hitscan trace information
USTRUCT()
struct FHitScanTrace
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TEnumAsByte<EPhysicalSurface> SurfaceType;

    UPROPERTY()
    FVector_NetQuantize TraceEnd;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFiredSignature, EPhysicalSurface, SurfaceType, float, Damage);

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
    ASWeapon();

    virtual void StartFire();
    void StopFire();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFiredSignature OnFired;

protected:
    void PlayFireEffects(FVector TracerEndoint);

    void Fire();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerFire();

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* MeshComp;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    TSubclassOf<UDamageType> DamageType;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    FName MuzzleSocketName;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    FName TracerTargetName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    UParticleSystem* MuzzleEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    UParticleSystem* DefaultImpactEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    UParticleSystem* FleshImpactEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    UParticleSystem* TracerEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<UCameraShake> FireCameraShake;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float BaseDamage;

    FTimerHandle TimerHandle_TimeBetweenShots;

    float LastFiredTime;

    // RPM, bullets per minute fired by weapon
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float RateOfFire;

    // Derived from RateOfFire
    float TimeBetweenShots;
    /* Bullet spread in degrees*/
    UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin=0.0f))
    float BulletSpread;

    UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
    FHitScanTrace HitScanInfo;

    UFUNCTION()
    void OnRep_HitScanTrace();

    void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactLocation);
};