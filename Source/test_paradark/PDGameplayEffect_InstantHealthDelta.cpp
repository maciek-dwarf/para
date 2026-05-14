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
	FSetByCallerFloat SBC;
	SBC.DataTag = TAG_PD_Data_HealthDelta;
	Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);
	Modifiers.Add(Mod);
}
