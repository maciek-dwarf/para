#include "PDGameplayEffect_Wet.h"

#include "PDGameplayTags.h"
#include "GameplayEffect.h"

UPDGameplayEffect_Wet::UPDGameplayEffect_Wet()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(2.f));

	InheritableOwnedTagsContainer.Added.AddTag(TAG_PD_State_Wet);
}
