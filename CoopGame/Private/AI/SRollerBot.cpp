// Fill out your copyright notice in the Description page of Project Settings.

#include "SRollerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "SHealthComponent.h"
#include "SCharacter.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"

// Console variable for displaying roller bot debug info
static int32 DebugRollerBotDrawing = 0;
FAutoConsoleVariableRef CVARDebugRollerBotDrawing(
    TEXT("COOP.DebugRollerBot"),
    DebugRollerBotDrawing,
    TEXT("Draw debug lines for roller bots"),
    ECVF_Cheat);

// Set default values
ASRollerBot::ASRollerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Add health component and bind damage event
    HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
    HealthComp->OnHealthChanged.AddDynamic(this, &ASRollerBot::HandleTakeDamage);

	// Create sphere collider component
    SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComp;

	// Sphere component collision and physics settings
	SphereComp->SetSimulatePhysics(true);
    SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SphereComp->SetCollisionResponseToAllChannels(ECR_Block);
	SphereComp->SetCanEverAffectNavigation(false);
    SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	


	// Attach static mesh to sphere comp
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	// Set default values
    UseVelocityChange = false;
    MovementForce = 1000;
    RequiredDistanceToTarget = 100;
    ExplosionRadius = 250;
    ExplosionDamage = 60;
    SelfDamageInterval = 0.25f;
	PathRefreshRateMax = 5.0f;
	PathRefreshRateMin = 0.2f;
	KillDistance = 250.0f;
	Besttarget = nullptr;
	ColliderSize = 100.0f;
}

void ASRollerBot::OnConstruction(const FTransform& Transform)
{
	// Set default sphere size
	SphereComp->SetSphereRadius(ColliderSize);
}

// Called when the game starts or when spawned
void ASRollerBot::BeginPlay()
{
	Super::BeginPlay();

	// Get next path point if we are the server
    if (Role == ROLE_Authority)
    {
        NextPathPoint = GetNextPathPoint();
    }

	// Set dynamic material instance to mesh component's first material
	DynamicMaterialInstance = MeshComp->CreateDynamicMaterialInstance(0, MeshComp->GetMaterial(0));
}

// Handle taking damage from other actors
void ASRollerBot::HandleTakeDamage(USHealthComponent* OwningHealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (DebugRollerBotDrawing)
	{
		UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());
	}

	// If DMI is valid, set flash material parameter
    if (DynamicMaterialInstance)
    {
		DynamicMaterialInstance->SetScalarParameterValue("LastTimeDamaged", GetWorld()->TimeSeconds);
    }

	// If health is at or below 0 die
    if (Health <= 0.0f) { SelfDestruct(); }
}

void ASRollerBot::SelfDestruct()
{
	// Dead bool check
    if (bDead) { return; }
    bDead = true;

	// Spawn death particle and sound FX
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
    UGameplayStatics::SpawnSoundAtLocation(this, ExplosionSound, GetActorLocation());

	// Hide sphere comp and children, stop physics and collision
    SphereComp->SetVisibility(false, true);
	SphereComp->SetSimulatePhysics(false);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Check if we are the server
    if (Role = ROLE_Authority)
    {
		// Create ignored actors list and add this bot to it
        TArray<AActor*> IgnoredActors;
        IgnoredActors.Add(this);

		// Create radial explosion damage
        UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

        if (DebugRollerBotDrawing)
        {
			// Draw debug explosion sphere
            DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 5, 0, 1.0f);
        }

		// Set default life span so the bot is destroyed
        SetLifeSpan(2.0f);
    }
}

// Use UE4 nav mesh to find a new point on path to enemy
FVector ASRollerBot::GetNextPathPoint()
{
	// Set default local values
    Besttarget = nullptr;
    float NearestTargetDistance = FLT_MAX;

	// Loop through all pawns in world
    for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
    {
		// Store test pawn and check if valid and not friendly
        APawn* TestPawn = It->Get();

        if (TestPawn == nullptr || USHealthComponent::IsFriendly(TestPawn, this))
        {
            continue;
        }

		// Store test pawn health component, check if valid and not dead
        USHealthComponent* TestPawnHealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));

        if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0.0f)
        {
			// Get distance from self to test pawn
            float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();

			// If test pawn is closer than last target
            if (NearestTargetDistance > Distance)
            {
                Besttarget = TestPawn;
                NearestTargetDistance = Distance;
            }
        }
    }

	// Check if we found a target
    if (Besttarget)
    {
		// Query nav mesh to find path to enemy
        UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), Besttarget);

		// Start timer to refresh path in case we run into a corner
        GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ASRollerBot::RefreshPath, PathRefreshRateMin, false);

		// If the path is valid, get the first point in the path
        if (NavPath && NavPath->PathPoints.Num() > 1)
        {
            return NavPath->PathPoints[1];
        }
    }

	// If we didn't find a path return self location
    return GetActorLocation();
}

void ASRollerBot::DamageSelf()
{
    UGameplayStatics::ApplyDamage(this, 20.0f, GetInstigatorController(), this, nullptr);
}

void ASRollerBot::RefreshPath()
{
    NextPathPoint = GetNextPathPoint();
}

// Called every frame
void ASRollerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check that we are server and bot is alive
    if (Role == ROLE_Authority && !bDead)
    {
		// Get distance from self to next path point
        float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		// Check if distance to next path point is less than threshold
        if (DistanceToTarget <= RequiredDistanceToTarget)
        {
			// Get a new path point
            NextPathPoint = GetNextPathPoint();

            if (DebugRollerBotDrawing)
            {
				// Draw debug text for target reached
                DrawDebugString(GetWorld(), GetActorLocation(), "TargetReached!", this, FColor::White, 1.0f);
            }
        }
        else
        {
			// Create vector in direction of next path point
            FVector ForceDirection = NextPathPoint - GetActorLocation();
            ForceDirection.Normalize();
            ForceDirection *= MovementForce;

			// Add force to self in direction of next path point
            SphereComp->AddForce(ForceDirection, NAME_None, UseVelocityChange);

            if (DebugRollerBotDrawing)
            {
				// Draw debug direction arrow to next path point
                DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Blue, false, 0.0f, 0, 2.0f);
            }
        }

		if (Besttarget)
		{
			float Distance = (Besttarget->GetActorLocation() - GetActorLocation()).Size();

			if (Distance <= KillDistance)
			{
				KillDistanceReached(Besttarget);
			}
		}

        if (DebugRollerBotDrawing)
        {
			// Draw debug sphere at the location of the next path point
            DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Cyan, false, 0.0f, 1.0f, 1.0f);
        }
    }
}

void ASRollerBot::KillDistanceReached(AActor* OtherActor)
{
	// Check if we are not already exploding or dead
    if (!bStartedSelfDestruct && !bDead)
    {
		// Cast overlaped actor to character
        ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);

		// Check that other actor is valid and not friendly
        if (PlayerPawn && !USHealthComponent::IsFriendly(OtherActor, this))
        {
			// Check that we are on the server
            if (Role == ROLE_Authority)
            {
				// Start the self destruct countdown timer
                GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASRollerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
            }

            bStartedSelfDestruct = true;
            UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
        }
    }
}