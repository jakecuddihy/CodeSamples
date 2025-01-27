// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "SGameMode.h"

// Console variable for displaying health debug info
static int32 DebugHealthCompDrawing = 0;
FAutoConsoleVariableRef CVARDebugHealthCompDrawing(
	TEXT("COOP.DebugHealthCompDrawing"),
	DebugHealthCompDrawing,
	TEXT("Draw debug info for health components"),
	ECVF_Cheat);

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
    DefaultHealth = 100;
    bDead = false;
    TeamNum = 255;

    SetIsReplicated(true);
}


void USHealthComponent::Heal(float HealAmount)
{
    if (HealAmount < 0.0f || Health <= 0.0f)
    {
        return;
    }

    Health = FMath::Clamp(Health + HealAmount, 0.0f, DefaultHealth);

	if (DebugHealthCompDrawing)
	{
		UE_LOG(LogTemp, Log, TEXT("Health changed: %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount));
	}

    OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}

float USHealthComponent::GetDefaultHealth()
{
	return DefaultHealth;
}

float USHealthComponent::GetHealth() const
{
    return Health;
}

bool USHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
    if (ActorA == nullptr || ActorB == nullptr)
    {
        return true;
    }
    USHealthComponent* HealthCompA = Cast<USHealthComponent>(ActorA->GetComponentByClass(USHealthComponent::StaticClass()));

    USHealthComponent* HealthCompB = Cast<USHealthComponent>(ActorB->GetComponentByClass(USHealthComponent::StaticClass()));

    if (HealthCompA == nullptr || HealthCompB == nullptr)
    {
        return true;
    }

    return HealthCompA->TeamNum == HealthCompB->TeamNum;
}

// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

    if (GetOwnerRole() == ROLE_Authority)
    {
        AActor* MyOwner = GetOwner();
        if (MyOwner)
        {
            MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
        }
    }
	
     Health = DefaultHealth;
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
    float Damage = Health - OldHealth;
    OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    if (Damage <= 0.0f || bDead)
    {
        return;
    }

    if (IsFriendly(DamagedActor, DamageCauser) && DamageCauser != DamagedActor)
    {
        return;
    }

    // Update health clamped
    Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	if (DebugHealthCompDrawing)
	{
		UE_LOG(LogTemp, Log, TEXT("Health changed: %s"), *FString::SanitizeFloat(Health));
	}

    bDead = Health <= 0.0f;

    OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

    if (bDead)
    {
        ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());

        if (GM)
        {
            GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
        }
    }

}


void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(USHealthComponent, Health);
}