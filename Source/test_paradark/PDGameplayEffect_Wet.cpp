#include "PDGameplayEffect_Wet.h"

#include "PDGameplayTags.h"
#include "GameplayEffect.h"

namespace PDWetDefaults
{
	// How long the "wet" window lasts after a water hit. Burn application is blocked while this tag is present.
	static constexpr float DurationSeconds = 2.f;
}

UPDGameplayEffect_Wet::UPDGameplayEffect_Wet()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(PDWetDefaults::DurationSeconds));

	InheritableOwnedTagsContainer.Added.AddTag(TAG_PD_State_Wet);
}
