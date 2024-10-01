// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

// On health changed event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USHealthComponent*, HealthComponent, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly, Category = "HealthComponent")
    float Health;	

    UFUNCTION()
    void OnRep_Health(float OldHealth);

    UFUNCTION()
    void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthComponent")
    float DefaultHealth;

    bool bDead;

public:

    USHealthComponent();

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHealthChangedSignature OnHealthChanged;

    UFUNCTION(BlueprintCallable, Category = "HealthComponent")
    void Heal(float HealAmount);

    float GetHealth() const;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealthComponent")
    uint8 TeamNum;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HealthComponent")
    static bool IsFriendly(AActor* ActorA, AActor* ActorB);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HealthComponent")
	float GetDefaultHealth();

};
