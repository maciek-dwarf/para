#include "PDGameplayEffect_Burning.h"

#include "PDExecCalc_PeriodicFireDamage.h"
#include "PDGameplayTags.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

namespace PDBurnDefaults
{
	// How often the burn execution calculation fires while the GE is active.
	// Smaller = smoother DOT but more ticks; tuned for "feels responsive at default duration".
	static constexpr float TickPeriodSeconds = 0.5f;
}

UPDGameplayEffect_Burning::UPDGameplayEffect_Burning()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat DurationMag;
	DurationMag.DataTag = TAG_PD_Data_BurnDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(DurationMag);

	Period = PDBurnDefaults::TickPeriodSeconds;

	FGameplayEffectExecutionDefinition ExecDef;
	ExecDef.CalculationClass = UPDExecCalc_PeriodicFireDamage::StaticClass();
	Executions.Add(ExecDef);

	// Grant State.Burning to the target while this effect is active.
	InheritableOwnedTagsContainer.Added.AddTag(TAG_PD_State_Burning);

	// Block applying burn while target is wet (deprecation warning expected in UE 5.7).
	ApplicationTagRequirements.IgnoreTags.AddTag(TAG_PD_State_Wet);
}
