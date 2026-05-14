// Fill out your copyright notice in the Description page of Project Settings.


#include "PDProjectile.h"

#include "PDDamageTypes.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

namespace PDProjectileDefaults
{
	// Component setup defaults (constructor-time; not designer-facing).
	// If you need designers to tune these per-variant, promote to UPROPERTY(EditDefaultsOnly).
	static constexpr float CollisionSphereRadius = 10.0f;
	static constexpr float InitialSpeed          = 3000.0f;
	static constexpr float MaxSpeed              = 3000.0f;
	static constexpr float ProjectileGravity     = 0.0f;
	// Used as the "no damage" magnitude on impact paths where the ammo carries no direct hit damage.
	static constexpr float NoImpactDamage        = 0.0f;
}

// Sets default values
APDProjectile::APDProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->InitSphereRadius(PDProjectileDefaults::CollisionSphereRadius);
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->SetGenerateOverlapEvents(false);
	RootComponent = CollisionComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	MovementComponent->InitialSpeed = PDProjectileDefaults::InitialSpeed;
	MovementComponent->MaxSpeed = PDProjectileDefaults::MaxSpeed;
	MovementComponent->bRotationFollowsVelocity = true;
	MovementComponent->bShouldBounce = false;
	MovementComponent->ProjectileGravityScale = PDProjectileDefaults::ProjectileGravity;

	CollisionComponent->OnComponentHit.AddDynamic(this, &APDProjectile::OnProjectileHit);

}

// Called when the game starts or when spawned
void APDProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(InitialLifeSeconds);
	UE_LOG(LogTemp, Log, TEXT("[PDProjectile] Spawned %s Ammo=%d Instigator=%s Life=%.2fs"),
		*GetNameSafe(this),
		static_cast<int32>(AmmoType),
		*GetNameSafe(GetInstigator()),
		InitialLifeSeconds);
}

void APDProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetInstigator())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[PDProjectile] Ignored hit. Self/invalid. Projectile=%s"), *GetNameSafe(this));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[PDProjectile] Hit. Projectile=%s Ammo=%d Other=%s Comp=%s Bone=%s Impact=%s"),
		*GetNameSafe(this),
		static_cast<int32>(AmmoType),
		*GetNameSafe(OtherActor),
		*GetNameSafe(OtherComp),
		*Hit.BoneName.ToString(),
		*Hit.ImpactPoint.ToString());

	// Encapsulated path: targets that implement the interface fully own effect logic (damage/resists/burn/heal).
	if (OtherActor->GetClass()->ImplementsInterface(UPDProjectileImpactInterface::StaticClass()))
	{
		FPDProjectileImpactData Data;
		Data.InstigatorActor = GetInstigator();
		Data.ProjectileActor = this;
		Data.AmmoType = static_cast<uint8>(AmmoType);

		switch (AmmoType)
		{
		case EPDProjectileAmmoType::Regular:
			Data.Magnitude = RegularDamage;
			Data.IncomingDamage.DamageType = EPDDamageType::Physical;
			Data.IncomingDamage.BaseAmount = RegularDamage;
			break;
		case EPDProjectileAmmoType::Fire:
			Data.Magnitude = FireImpactDamage;
			Data.IncomingDamage.DamageType = EPDDamageType::Fire;
			Data.IncomingDamage.BaseAmount = FireImpactDamage;
			break;
		case EPDProjectileAmmoType::Water:
			Data.Magnitude = PDProjectileDefaults::NoImpactDamage;
			Data.IncomingDamage.DamageType = EPDDamageType::Water;
			Data.IncomingDamage.BaseAmount = PDProjectileDefaults::NoImpactDamage;
			break;
		case EPDProjectileAmmoType::Healing:
			Data.Magnitude = HealingAmount;
			Data.IncomingDamage.DamageType = EPDDamageType::Heal;
			Data.IncomingDamage.BaseAmount = HealingAmount;
			break;
		default:
			Data.Magnitude = PDProjectileDefaults::NoImpactDamage;
			Data.IncomingDamage.DamageType = EPDDamageType::Physical;
			Data.IncomingDamage.BaseAmount = PDProjectileDefaults::NoImpactDamage;
			break;
		}

		Data.Hit = Hit;

		UE_LOG(LogTemp, Log, TEXT("[PDProjectile] Forwarding impact to interface. Target=%s type=%d base=%.2f"),
			*GetNameSafe(OtherActor),
			static_cast<int32>(Data.IncomingDamage.DamageType),
			Data.IncomingDamage.BaseAmount);
		IPDProjectileImpactInterface::Execute_HandleProjectileImpact(OtherActor, Data);
		Destroy();
		return;
	}

	// Fallback path: still does something even if target isn't wired yet.
	AController* InstigatorController = GetInstigatorController();

	switch (AmmoType)
	{
	case EPDProjectileAmmoType::Regular:
	{
		TSubclassOf<UDamageType> DT = RegularDamageType;
		if (!DT) { DT = UDamageType::StaticClass(); }
		UGameplayStatics::ApplyPointDamage(OtherActor, RegularDamage, GetVelocity().GetSafeNormal(), Hit, InstigatorController, this, DT);
		UE_LOG(LogTemp, Log, TEXT("[PDProjectile] Applied Regular damage=%.2f to %s (fallback)"), RegularDamage, *GetNameSafe(OtherActor));
		break;
	}
	case EPDProjectileAmmoType::Fire:
	{
		TSubclassOf<UDamageType> DT = FireDamageType;
		if (!DT) { DT = UDamageType::StaticClass(); }
		UGameplayStatics::ApplyPointDamage(OtherActor, FireImpactDamage, GetVelocity().GetSafeNormal(), Hit, InstigatorController, this, DT);
		UE_LOG(LogTemp, Log, TEXT("[PDProjectile] Applied Fire impact damage=%.2f to %s (fallback). Burn requires interface/component on target."),
			FireImpactDamage, *GetNameSafe(OtherActor));
		break;
	}
	case EPDProjectileAmmoType::Water:
	{
		TSubclassOf<UDamageType> DT = WaterDamageType;
		if (!DT) { DT = UDamageType::StaticClass(); }
		UGameplayStatics::ApplyPointDamage(OtherActor, PDProjectileDefaults::NoImpactDamage, GetVelocity().GetSafeNormal(), Hit, InstigatorController, this, DT);
		UE_LOG(LogTemp, Log, TEXT("[PDProjectile] Water hit %s (fallback). Extinguish requires interface/component on target."), *GetNameSafe(OtherActor));
		break;
	}
	case EPDProjectileAmmoType::Healing:
	{
		UE_LOG(LogTemp, Warning, TEXT("[PDProjectile] Healing hit %s but target has no impact interface. Implement UPDProjectileImpactInterface to receive healing."),
			*GetNameSafe(OtherActor));
		break;
	}
	default:
		UE_LOG(LogTemp, Warning, TEXT("[PDProjectile] Unknown ammo type %d"), static_cast<int32>(AmmoType));
		break;
	}

	Destroy();
}

