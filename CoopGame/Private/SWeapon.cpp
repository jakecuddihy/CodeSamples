// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "CoopGame.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"


static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
    TEXT("COOP.DebugWeapons"), 
    DebugWeaponDrawing, 
    TEXT("Draw debug lines for weapons"), 
    ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;

    MuzzleSocketName = "MuzzleSocket";
    TracerTargetName = "Target";
    BaseDamage = 20.0f;
    RateOfFire = 600;
    BulletSpread = 2.0f;

    SetReplicates(true);
    NetUpdateFrequency = 66.0f;
    MinNetUpdateFrequency = 33.0f;
}

void ASWeapon::BeginPlay()
{
    Super::BeginPlay();

    TimeBetweenShots = 60 / RateOfFire;
    LastFiredTime = -TimeBetweenShots;
}


void ASWeapon::OnRep_HitScanTrace()
{

    PlayFireEffects(HitScanInfo.TraceEnd);
    PlayImpactEffects(HitScanInfo.SurfaceType, HitScanInfo.TraceEnd);
}

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactLocation)
{
    UParticleSystem* SelectedEffect = nullptr;

    switch (SurfaceType)
    {
    case SURFACE_FLESHDEFAULT: // Flesh default
    case SURFACE_FLESHVULNERABLE:
        SelectedEffect = FleshImpactEffect;
        break;
    default:
        SelectedEffect = DefaultImpactEffect;
        break;
    }

    if (SelectedEffect)
    {
        FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

        FVector ShotDirection = ImpactLocation - MuzzleLocation;
        ShotDirection.Normalize();

        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactLocation, ShotDirection.Rotation());
    }
}

void ASWeapon::StartFire()
{
    float FirstDelay = FMath::Max(LastFiredTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

    GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::Fire()
{
    // Trace the world from pawn eyes to crosshair location

    if (Role < ROLE_Authority)
    {
        ServerFire();
    }

    AActor* MyOwner = GetOwner();

    if (MyOwner)
    {
        FVector EyeLocation;
        FRotator EyeRotation;
        MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

        FVector ShotDirection = EyeRotation.Vector();

        float HalfRad = FMath::DegreesToRadians(BulletSpread);
        ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

        FVector TraceEnd = EyeLocation + (ShotDirection * 10000);
        // Particle "Target " parameter
        FVector TracerEndoint = TraceEnd;

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(MyOwner);
        QueryParams.AddIgnoredActor(this);
        QueryParams.bTraceComplex = true;
        QueryParams.bReturnPhysicalMaterial = true;

        EPhysicalSurface SurfaceType = SurfaceType_Default;

        FHitResult Hit;
        if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
        {
            // Blocking hit, process damage

            AActor* HitActor = Hit.GetActor();


            // Using .get because of weak object pointer to prevent it from being removed in memory
            SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

            float NewDamage = BaseDamage;

            if (SurfaceType == SURFACE_FLESHVULNERABLE)
            {
                NewDamage *= 4.0f;
            }

            UGameplayStatics::ApplyPointDamage(HitActor, NewDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);

            PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

            TracerEndoint = Hit.ImpactPoint;

			OnFired.Broadcast(SurfaceType, NewDamage);
        }

        if (DebugWeaponDrawing > 0)
        {
            DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
        }

        PlayFireEffects(TracerEndoint);

        if (Role ==  ROLE_Authority)
        {
            HitScanInfo.TraceEnd = TracerEndoint;

            HitScanInfo.SurfaceType = SurfaceType;
        }

        LastFiredTime = GetWorld()->TimeSeconds;
    }
}

void ASWeapon::ServerFire_Implementation()
{
    Fire();
}

bool ASWeapon::ServerFire_Validate()
{
    return true;
}

void ASWeapon::PlayFireEffects(FVector TracerEndoint)
{
    if (MuzzleEffect)
    {
        UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
    }

    if (TracerEffect)
    {
        FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

        UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);

        if (TracerComp)
        {
            TracerComp->SetVectorParameter(TracerTargetName, TracerEndoint);
        }
    }

    APawn* MyOwner = Cast<APawn>(GetOwner());
    if (MyOwner)
    {
        APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
        if (PC)
        {
            PC->ClientPlayCameraShake(FireCameraShake);
        }
    }
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ASWeapon, HitScanInfo, COND_SkipOwner);
}