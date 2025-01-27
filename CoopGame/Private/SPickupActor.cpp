// Fill out your copyright notice in the Description page of Project Settings.

#include "SPickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "TimerManager.h"
#include "SPowerUpActor.h"
#include "SHealthComponent.h"

// Sets default values
ASPickupActor::ASPickupActor()
{
    SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    SphereComp->SetSphereRadius(75.0f);
    RootComponent = SphereComp;

    DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
    DecalComp->SetupAttachment(RootComponent);
    DecalComp->SetRelativeRotation(FRotator(90, 0.0f, 0.0f));
    DecalComp->DecalSize = FVector(64, 75, 75);

    CooldownDuration = 10.0f;

    SetReplicates(true);
}

// Called when the game starts or when spawned
void ASPickupActor::BeginPlay()
{
	Super::BeginPlay();

    if (Role == ROLE_Authority)
    {
        Respawn();
    }
}

void ASPickupActor::Respawn()
{
    if (PowerUpClass == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("PowerUp class is nullptr in %s."), *GetName());
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    PowerUpInstance = GetWorld()->SpawnActor<ASPowerUpActor>(PowerUpClass, GetTransform(), SpawnParams);
}

void ASPickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	USHealthComponent* TestPawnHealthComp = Cast<USHealthComponent>(OtherActor->GetComponentByClass(USHealthComponent::StaticClass()));

	if (!TestPawnHealthComp || TestPawnHealthComp->TeamNum != 0)
    {
        return;
    }

    if (/*Role == ROLE_Authority &&*/ PowerUpInstance)
    {
        PowerUpInstance->ActivatePowerUp(OtherActor);
        PowerUpInstance = nullptr;

        GetWorldTimerManager().SetTimer(TimerHandle_RespawnCooldown, this, &ASPickupActor::Respawn, CooldownDuration);
    }
}
