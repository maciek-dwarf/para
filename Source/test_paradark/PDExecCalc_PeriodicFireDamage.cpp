#include "PDExecCalc_PeriodicFireDamage.h"

#include "PDAttributeSet.h"
#include "PDGameplayTags.h"
#include "GameplayEffectTypes.h"

UPDExecCalc_PeriodicFireDamage::UPDExecCalc_PeriodicFireDamage()
{
}

void UPDExecCalc_PeriodicFireDamage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const float DamagePerTick = Spec.GetSetByCallerMagnitude(TAG_PD_Data_PeriodicFireDamage, false, 0.f);

	if (DamagePerTick <= 0.f)
	{
		return;
	}

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UPDAttributeSet::GetHealthAttribute(),
		EGameplayModOp::Additive,
		-DamagePerTick));
}
