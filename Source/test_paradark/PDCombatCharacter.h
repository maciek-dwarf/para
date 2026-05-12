#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GameplayEffectTypes.h"
#include "PDDamageTypes.h"
#include "PDProjectileImpactInterface.h"
#include "PDCombatCharacter.generated.h"

class UPDAbilitySystemComponent;
class UPDAttributeSet;
class UGameplayEffect;

UCLASS(Blueprintable)
class TEST_PARADARK_API APDCombatCharacter : public ACharacter, public IAbilitySystemInterface, public IPDProjectileImpactInterface
{
	GENERATED_BODY()

public:
	APDCombatCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	UFUNCTION(BlueprintCallable, Category = "PD|GAS")
	UPDAbilitySystemComponent* GetPDAbilitySystemComponent() const { return AbilitySystemComponent; }

	UFUNCTION(BlueprintCallable, Category = "PD|GAS")
	const UPDAttributeSet* GetPDAttributeSet() const { return AttributeSet; }

	/** Single entry point for typed damage/heal; applies resistances for non-heal types. */
	UFUNCTION(BlueprintCallable, Category = "PD|Damage")
	void ApplyIncomingDamage(const FPDIncomingDamage& Incoming);

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	virtual void HandleProjectileImpact_Implementation(const FPDProjectileImpactData& Data) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PD|GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPDAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UPDAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "PD|GAS")
	TSubclassOf<UGameplayEffect> InstantHealthDeltaEffect;

	UPROPERTY(EditDefaultsOnly, Category = "PD|GAS")
	TSubclassOf<UGameplayEffect> BurnEffect;

	UPROPERTY(EditDefaultsOnly, Category = "PD|GAS")
	TSubclassOf<UGameplayEffect> WetEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PD|Combat", meta = (ClampMin = "0.1"))
	float BurnDurationSeconds = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PD|Combat", meta = (ClampMin = "0"))
	float BurnDamagePerTick = 2.f;

	/** Resistance in [0,1]: finalDamage = base * (1 - resistance). Heal bypasses resistances. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PD|Damage", meta = (ClampMin = "0", ClampMax = "1"))
	float ResistancePhysical = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PD|Damage", meta = (ClampMin = "0", ClampMax = "1"))
	float ResistanceFire = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PD|Damage", meta = (ClampMin = "0", ClampMax = "1"))
	float ResistanceWater = 0.f;

	FActiveGameplayEffectHandle ActiveBurnHandle;

	void InitAbilityActorInfoIfNeeded();
	void ApplyInstantHealthDelta(float Delta);
	FActiveGameplayEffectHandle TryApplyBurn();
	void ExtinguishBurnAndApplyWet();

	float GetResistanceForDamageType(EPDDamageType Type) const;
};
