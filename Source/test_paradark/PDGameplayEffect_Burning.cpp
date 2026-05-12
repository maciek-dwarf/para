#include "PDGameplayEffect_Burning.h"

#include "PDExecCalc_PeriodicFireDamage.h"
#include "PDGameplayTags.h"
#include "GameplayEffectTypes.h"

UPDGameplayEffect_Burning::UPDGameplayEffect_Burning()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FGameplayEffectMagnitudeSetByCaller DurationMag;
	DurationMag.DataTag = TAG_PD_Data_BurnDuration;
	DurationMag.DefaultValue = 5.f;
	DurationMagnitude = FGameplayEffectModifierMagnitude(DurationMag);

	Period = 0.5f;
	bExecutePeriodicEffect = true;

	StackingType = EGameplayEffectStackingType::AggregateByTarget;
	StackCountMax = 1;
	StackDurationRefreshPolicy = EGameplayEffectStackingDurationPolicy::RefreshOnSuccessfulApplication;

	FGameplayEffectExecutionDefinition ExecDef;
	ExecDef.CalculationClass = UPDExecCalc_PeriodicFireDamage::StaticClass();
	Executions.Add(ExecDef);

	InheritableGameplayEffectTags.Added.TargetTags.AddTag(TAG_PD_State_Burning);

	ApplicationTagRequirements.IgnoreTags.AddTag(TAG_PD_State_Wet);
}
