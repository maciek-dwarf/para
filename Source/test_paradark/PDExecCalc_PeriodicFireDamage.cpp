#include "PDExecCalc_PeriodicFireDamage.h"

#include "PDAttributeSet.h"
#include "PDCombatCharacter.h"
#include "PDDamageTypes.h"
#include "PDGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"

UPDExecCalc_PeriodicFireDamage::UPDExecCalc_PeriodicFireDamage()
{
}

void UPDExecCalc_PeriodicFireDamage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const float DamagePerTickBase = Spec.GetSetByCallerMagnitude(TAG_PD_Data_PeriodicFireDamage, false, 0.f);

	if (DamagePerTickBase <= 0.f)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	AActor* Avatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;
	if (APDCombatCharacter* Target = Cast<APDCombatCharacter>(Avatar))
	{
		FPDIncomingDamage Incoming;
		Incoming.DamageType = EPDDamageType::Fire;
		Incoming.BaseAmount = DamagePerTickBase;
		Target->ApplyIncomingDamage(Incoming);
		return;
	}

	// Fallback if burn runs on a non-PDCombatCharacter: apply raw health delta (no resist pipeline).
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UPDAttributeSet::GetHealthAttribute(),
		EGameplayModOp::Additive,
		-DamagePerTickBase));
}
