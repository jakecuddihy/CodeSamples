// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASWeapon;
class USHealthComponent;

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    void MoveForward(float Value);
    void MoveRight(float Value);

    void BeginADS();
    void EndADS();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* CameraComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpringArmComponent* SpringArmComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USHealthComponent* HealthComp;

    bool bShouldADS;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
    bool bDead;

    UPROPERTY(EditDefaultsOnly, Category = "Player")
    float ADSFOV;

    float DefaultFOV;

    UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
    float ADSSpeed;

    UPROPERTY(Replicated)
    ASWeapon* CurrentWeapon;

    UPROPERTY(VisibleDefaultsOnly, Category = "Player")
    FName WeaponAttachSocketName;

    UPROPERTY(EditDefaultsOnly, Category = "Player")
    TSubclassOf<ASWeapon> StarterWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	FTransform WeaponTransform;

    UFUNCTION()
    void OnHealthChanged(USHealthComponent* OwningHealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    virtual FVector GetPawnViewLocation() const override;

    UFUNCTION(BlueprintCallable, Category = "Player")
    void StartFire();

    UFUNCTION(BlueprintCallable, Category = "Player")
    void StopFire();

};
