// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PDProjectileImpactInterface.h"
#include "PDProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UDamageType;

UENUM(BlueprintType)
enum class EPDProjectileAmmoType : uint8
{
	Regular UMETA(DisplayName = "Regular"),
	Fire UMETA(DisplayName = "Fire"),
	Water UMETA(DisplayName = "Water"),
	Healing UMETA(DisplayName = "Healing"),
};

UCLASS()
class TEST_PARADARK_API APDProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APDProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	EPDProjectileAmmoType GetAmmoType() const { return AmmoType; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	TObjectPtr<UProjectileMovementComponent> MovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Ammo")
	EPDProjectileAmmoType AmmoType = EPDProjectileAmmoType::Regular;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Tuning", meta = (ClampMin = "0"))
	float RegularDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Tuning", meta = (ClampMin = "0"))
	float FireImpactDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Tuning", meta = (ClampMin = "0"))
	float HealingAmount = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Tuning")
	TSubclassOf<UDamageType> RegularDamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Tuning")
	TSubclassOf<UDamageType> FireDamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Tuning")
	TSubclassOf<UDamageType> WaterDamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Tuning", meta = (ClampMin = "0"))
	float InitialLifeSeconds = 5.0f;

	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

};
