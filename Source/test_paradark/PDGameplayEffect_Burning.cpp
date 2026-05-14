#include "PDGameplayEffect_Burning.h"

#include "PDExecCalc_PeriodicFireDamage.h"
#include "PDGameplayTags.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

UPDGameplayEffect_Burning::UPDGameplayEffect_Burning()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat DurationMag;
	DurationMag.DataTag = TAG_PD_Data_BurnDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(DurationMag);

	Period = 0.5f;

	FGameplayEffectExecutionDefinition ExecDef;
	ExecDef.CalculationClass = UPDExecCalc_PeriodicFireDamage::StaticClass();
	Executions.Add(ExecDef);

	// Grant State.Burning to the target while this effect is active.
	InheritableOwnedTagsContainer.Added.AddTag(TAG_PD_State_Burning);

	// Block applying burn while target is wet (deprecation warning expected in UE 5.7).
	ApplicationTagRequirements.IgnoreTags.AddTag(TAG_PD_State_Wet);
}
