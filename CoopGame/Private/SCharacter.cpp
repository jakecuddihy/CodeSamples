// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SWeapon.h"
#include "Components/CapsuleComponent.h"
#include "CoopGame.h"
#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the camera component and attach it to the root component
    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
    CameraComp->SetupAttachment(RootComponent);
	CameraComp->bUsePawnControlRotation = true;

	// Player capsule collider should ignore collisions with weapon
    GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	// Add a health component
    HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	
	// Set default values for aiming down sights
    ADSFOV = 40;
    ADSSpeed = 20;

    //WeaponAttachSocketName = "WeaponSocket";


}

void ASCharacter::OnConstruction(const FTransform& Transform)
{

}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
    Super::BeginPlay();  

    HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);

	// Save this value here to account for BP changes 
	DefaultFOV = CameraComp->FieldOfView;

	if (Role == ROLE_Authority)
	{
		// Set spawn parameters for the current weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (CurrentWeapon)
		{
			CurrentWeapon->AttachToComponent(CameraComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
			CurrentWeapon->SetOwner(this);

			CurrentWeapon->SetActorRelativeTransform(WeaponTransform);
		}
	}
}

void ASCharacter::MoveForward(float Value)
{
    AddMovementInput(GetActorForwardVector() * Value);
}

void ASCharacter::MoveRight(float Value)
{
    AddMovementInput(GetActorRightVector() * Value);
}

void ASCharacter::BeginADS()
{
    bShouldADS = true;
}

void ASCharacter::EndADS()
{
    bShouldADS = false;
}

void ASCharacter::StartFire()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StartFire();
    }
}

void ASCharacter::StopFire()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StopFire();
    }
}

void ASCharacter::OnHealthChanged(USHealthComponent* OwningHealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, 
    class AController* InstigatedBy, AActor* DamageCauser)
{
    if (Health <= 0.0f && !bDead)
    {
        bDead = true;
		StopFire();
        GetMovementComponent()->StopMovementImmediately();
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        DetachFromControllerPendingDestroy();

        SetLifeSpan(10.0f);
    }
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    float TargetFOV = bShouldADS ? ADSFOV : DefaultFOV;

    float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ADSSpeed);

    CameraComp->SetFieldOfView(NewFOV);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

    PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("LookRight", this, &ASCharacter::AddControllerYawInput);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &ASCharacter::BeginADS);
    PlayerInputComponent->BindAction("ADS", IE_Released, this, &ASCharacter::EndADS);
    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
    PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

}

FVector ASCharacter::GetPawnViewLocation() const
{
    if (CameraComp)
    {
        return CameraComp->GetComponentLocation();
    }

    return Super::GetPawnViewLocation();
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASCharacter, CurrentWeapon);
    DOREPLIFETIME(ASCharacter, bDead);
}