#include "PDGameplayEffect_Wet.h"

#include "PDGameplayTags.h"

UPDGameplayEffect_Wet::UPDGameplayEffect_Wet()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(2.f);

	InheritableGameplayEffectTags.Added.TargetTags.AddTag(TAG_PD_State_Wet);
}
