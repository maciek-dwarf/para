#include "PDGameplayEffect_InstantHealthDelta.h"

#include "PDAttributeSet.h"
#include "PDGameplayTags.h"
#include "GameplayEffectTypes.h"

UPDGameplayEffect_InstantHealthDelta::UPDGameplayEffect_InstantHealthDelta()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo Mod;
	Mod.Attribute = UPDAttributeSet::GetHealthAttribute();
	Mod.ModifierOp = EGameplayModOp::Additive;
	Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FGameplayEffectMagnitudeSetByCaller(TAG_PD_Data_HealthDelta));
	Modifiers.Add(Mod);
}
