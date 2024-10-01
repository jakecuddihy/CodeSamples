// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SRollerBot.generated.h"

class USHealthComponent;
class USphereComponent;
class USoundCue;

UCLASS()
class COOPGAME_API ASRollerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASRollerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


    UFUNCTION()
    void HandleTakeDamage(USHealthComponent* OwningHealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType,
        class AController* InstigatedBy, AActor* DamageCauser);

    void SelfDestruct();

    UPROPERTY(VisibleDefaultsOnly, Category = "Components")
    UStaticMeshComponent* MeshComp;
	

    UPROPERTY(VisibleDefaultsOnly, Category = "Components")
    USHealthComponent* HealthComp;

    UPROPERTY(VisibleDefaultsOnly, Category = "Components")
    USphereComponent* SphereComp;

    FVector GetNextPathPoint();
    FVector NextPathPoint;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float PathRefreshRateMax;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float PathRefreshRateMin;

	float PathRefreshRate;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    bool UseVelocityChange;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float RequiredDistanceToTarget;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float KillDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TrackerBot")
	float ColliderSize;

    UMaterialInstanceDynamic* DynamicMaterialInstance;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    UParticleSystem* ExplosionEffect;

    bool bDead;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float ExplosionDamage;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float ExplosionRadius;

    FTimerHandle TimerHandle_SelfDamage;
    FTimerHandle TimerHandle_RefreshPath;

    void DamageSelf();

    bool bStartedSelfDestruct;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    USoundCue* SelfDestructSound;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    USoundCue* ExplosionSound;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float SelfDamageInterval;

    void RefreshPath();

	void KillDistanceReached(AActor* OtherActor);

	AActor* Besttarget;

public:	
	// Called every frame
    virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

    //virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
