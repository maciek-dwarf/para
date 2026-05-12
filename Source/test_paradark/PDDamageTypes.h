#pragma once

#include "CoreMinimal.h"
#include "PDDamageTypes.generated.h"

/** Typed damage / heal channel used by ApplyIncomingDamage (resistances apply to non-heal types). */
UENUM(BlueprintType)
enum class EPDDamageType : uint8
{
	Physical UMETA(DisplayName = "Physical"),
	Fire UMETA(DisplayName = "Fire"),
	Water UMETA(DisplayName = "Water"),
	Heal UMETA(DisplayName = "Heal"),
};

/**
 * Single damage/heal payload built by weapons/projectiles.
 * BaseAmount is always non-negative: magnitude before resist (damage types) or raw heal (Heal).
 */
USTRUCT(BlueprintType)
struct FPDIncomingDamage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PD|Damage")
	EPDDamageType DamageType = EPDDamageType::Physical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PD|Damage", meta = (ClampMin = "0"))
	float BaseAmount = 0.f;
};
