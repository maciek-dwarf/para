#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PDProjectileImpactInterface.generated.h"

USTRUCT(BlueprintType)
struct FPDProjectileImpactData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<AActor> ProjectileActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	uint8 AmmoType = 0; // Stored as byte to keep this interface independent from projectile header

	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	float Magnitude = 0.0f; // Damage or healing amount depending on ammo

	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	FHitResult Hit;
};

UINTERFACE(BlueprintType)
class TEST_PARADARK_API UPDProjectileImpactInterface : public UInterface
{
	GENERATED_BODY()
};

class TEST_PARADARK_API IPDProjectileImpactInterface
{
	GENERATED_BODY()

public:
	// Implement on any actor that wants to handle projectile effects in an encapsulated way
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Projectile")
	void HandleProjectileImpact(const FPDProjectileImpactData& Data);
};

