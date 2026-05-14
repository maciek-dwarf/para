#include "PDAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffectTypes.h"
#include "Net/UnrealNetwork.h"

namespace PDAttributeDefaults
{
	// Baseline values for a fresh actor. Typical GAS projects override these via an
	// "init attributes" Gameplay Effect per archetype (player vs enemy etc.).
	static constexpr float StartingHealth    = 100.f;
	static constexpr float StartingMaxHealth = 100.f;
	static constexpr float MinHealth         = 0.f;
}

UPDAttributeSet::UPDAttributeSet()
{
	InitHealth(PDAttributeDefaults::StartingHealth);
	InitMaxHealth(PDAttributeDefaults::StartingMaxHealth);
}

void UPDAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPDAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPDAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UPDAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPDAttributeSet, Health, OldValue);
}

void UPDAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPDAttributeSet, MaxHealth, OldValue);
}

void UPDAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const float Clamped = FMath::Clamp(GetHealth(), PDAttributeDefaults::MinHealth, GetMaxHealth());
		SetHealth(Clamped);
	}
}
