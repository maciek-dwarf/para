#include "PDCombatCharacter.h"

#include "PDAbilitySystemComponent.h"
#include "PDAttributeSet.h"
#include "PDGameplayEffect_Burning.h"
#include "PDGameplayEffect_InstantHealthDelta.h"
#include "PDGameplayEffect_Wet.h"
#include "PDGameplayTags.h"
#include "GameplayEffect.h"

namespace PDCombatConstants
{
	// We don't use ability levels in this project; all GE specs are built at level 1.
	static constexpr float DefaultEffectLevel = 1.f;

	// We refresh-by-remove on burn re-application; 1 stack == the single active burn we just placed.
	static constexpr int32 BurnStacksToRemove = 1;

	// Resistance is a multiplier on incoming damage; valid range is [0, 1] (0 = no resist, 1 = immune).
	static constexpr float MinResistance = 0.f;
	static constexpr float MaxResistance = 1.f;

	// Heal channel bypasses resistances entirely.
	static constexpr float NoResistance   = 0.f;
}

UAbilitySystemComponent* APDCombatCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

float APDCombatCharacter::GetHealthCurrent() const
{
	return AttributeSet ? AttributeSet->GetHealth() : 0.f;
}

float APDCombatCharacter::GetHealthMax() const
{
	return AttributeSet ? AttributeSet->GetMaxHealth() : 0.f;
}

float APDCombatCharacter::GetHealthNormalized() const
{
	const float Max = GetHealthMax();
	return Max > 0.f ? GetHealthCurrent() / Max : 0.f;
}

APDCombatCharacter::APDCombatCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UPDAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UPDAttributeSet>(TEXT("AttributeSet"));

	InstantHealthDeltaEffect = UPDGameplayEffect_InstantHealthDelta::StaticClass();
	BurnEffect = UPDGameplayEffect_Burning::StaticClass();
	WetEffect = UPDGameplayEffect_Wet::StaticClass();
}

void APDCombatCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityActorInfoIfNeeded();
}

void APDCombatCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilityActorInfoIfNeeded();
}

void APDCombatCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfoIfNeeded();
}

void APDCombatCharacter::InitAbilityActorInfoIfNeeded()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	if (!AbilitySystemComponent->AbilityActorInfo.IsValid())
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

float APDCombatCharacter::GetResistanceForDamageType(const EPDDamageType Type) const
{
	switch (Type)
	{
	case EPDDamageType::Physical: return ResistancePhysical;
	case EPDDamageType::Fire: return ResistanceFire;
	case EPDDamageType::Water: return ResistanceWater;
	case EPDDamageType::Heal: return PDCombatConstants::NoResistance;
	default: return PDCombatConstants::NoResistance;
	}
}

void APDCombatCharacter::ApplyIncomingDamage(const FPDIncomingDamage& Incoming)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	if (Incoming.BaseAmount <= 0.f)
	{
		return;
	}

	if (Incoming.DamageType == EPDDamageType::Heal)
	{
		ApplyInstantHealthDelta(Incoming.BaseAmount);
		UE_LOG(LogTemp, Log, TEXT("[PDCombat] Heal (bypass resist) amount=%.2f (%s)"), Incoming.BaseAmount, *GetName());
		return;
	}

	const float Resist = FMath::Clamp(
		GetResistanceForDamageType(Incoming.DamageType),
		PDCombatConstants::MinResistance,
		PDCombatConstants::MaxResistance);
	const float FinalDamage = Incoming.BaseAmount * (PDCombatConstants::MaxResistance - Resist);

	UE_LOG(LogTemp, Log, TEXT("[PDCombat] ApplyIncomingDamage type=%d base=%.2f resist=%.2f final=%.2f (%s)"),
		static_cast<int32>(Incoming.DamageType),
		Incoming.BaseAmount,
		Resist,
		FinalDamage,
		*GetName());

	if (FinalDamage <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	ApplyInstantHealthDelta(-FinalDamage);
}

void APDCombatCharacter::ApplyInstantHealthDelta(const float Delta)
{
	if (!AbilitySystemComponent || !InstantHealthDeltaEffect)
	{
		return;
	}

	FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
	Ctx.AddSourceObject(this);

	const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
		InstantHealthDeltaEffect, PDCombatConstants::DefaultEffectLevel, Ctx);
	if (!Spec.IsValid() || !Spec.Data.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PDCombat] Failed to build instant health delta spec (%s)"), *GetName());
		return;
	}

	Spec.Data->SetSetByCallerMagnitude(TAG_PD_Data_HealthDelta, Delta);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	UE_LOG(LogTemp, Log, TEXT("[PDCombat] Instant health delta %.2f applied (%s)"), Delta, *GetName());
}

FActiveGameplayEffectHandle APDCombatCharacter::TryApplyBurn()
{
	if (!AbilitySystemComponent || !BurnEffect)
	{
		return FActiveGameplayEffectHandle();
	}

	if (AbilitySystemComponent->HasMatchingGameplayTag(TAG_PD_State_Wet))
	{
		UE_LOG(LogTemp, Log, TEXT("[PDCombat] Burn skipped: wet (%s)"), *GetName());
		return FActiveGameplayEffectHandle();
	}

	if (ActiveBurnHandle.IsValid())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveBurnHandle, PDCombatConstants::BurnStacksToRemove);
		ActiveBurnHandle.Invalidate();
		UE_LOG(LogTemp, Log, TEXT("[PDCombat] Burn refreshed: removed previous effect (%s)"), *GetName());
	}

	FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
	Ctx.AddSourceObject(this);

	const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
		BurnEffect, PDCombatConstants::DefaultEffectLevel, Ctx);
	if (!Spec.IsValid() || !Spec.Data.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PDCombat] Failed to build burn spec (%s)"), *GetName());
		return FActiveGameplayEffectHandle();
	}

	Spec.Data->SetSetByCallerMagnitude(TAG_PD_Data_BurnDuration, BurnDurationSeconds);
	Spec.Data->SetSetByCallerMagnitude(TAG_PD_Data_PeriodicFireDamage, BurnDamagePerTick);

	const FActiveGameplayEffectHandle Handle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	ActiveBurnHandle = Handle;

	UE_LOG(LogTemp, Log, TEXT("[PDCombat] Burn applied: duration=%.2f tickBase=%.2f (%s)"),
		BurnDurationSeconds,
		BurnDamagePerTick,
		*GetName());

	return Handle;
}

void APDCombatCharacter::ExtinguishBurnAndApplyWet()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	if (ActiveBurnHandle.IsValid())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveBurnHandle, PDCombatConstants::BurnStacksToRemove);
		ActiveBurnHandle.Invalidate();
	}

	AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(TAG_PD_State_Burning));
	UE_LOG(LogTemp, Log, TEXT("[PDCombat] Burn cleared via water (%s)"), *GetName());

	if (!WetEffect)
	{
		return;
	}

	const FGameplayEffectSpecHandle WetSpec = AbilitySystemComponent->MakeOutgoingSpec(
		WetEffect,
		PDCombatConstants::DefaultEffectLevel,
		AbilitySystemComponent->MakeEffectContext());

	if (!WetSpec.IsValid() || !WetSpec.Data.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PDCombat] Failed to build wet spec (%s)"), *GetName());
		return;
	}

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*WetSpec.Data.Get());
	UE_LOG(LogTemp, Log, TEXT("[PDCombat] Wet window applied (%s)"), *GetName());
}

void APDCombatCharacter::HandleProjectileImpact_Implementation(const FPDProjectileImpactData& Data)
{
	InitAbilityActorInfoIfNeeded();

	switch (static_cast<int32>(Data.AmmoType))
	{
	case 0:
		ApplyIncomingDamage(Data.IncomingDamage);
		UE_LOG(LogTemp, Log, TEXT("[PDCombat] Regular projectile resolved (%s)"), *GetName());
		break;
	case 1:
		ApplyIncomingDamage(Data.IncomingDamage);
		TryApplyBurn();
		UE_LOG(LogTemp, Log, TEXT("[PDCombat] Fire projectile resolved (%s)"), *GetName());
		break;
	case 2:
		if (Data.IncomingDamage.BaseAmount > 0.f)
		{
			ApplyIncomingDamage(Data.IncomingDamage);
		}
		ExtinguishBurnAndApplyWet();
		UE_LOG(LogTemp, Log, TEXT("[PDCombat] Water projectile resolved (%s)"), *GetName());
		break;
	case 3:
		ApplyIncomingDamage(Data.IncomingDamage);
		UE_LOG(LogTemp, Log, TEXT("[PDCombat] Healing projectile resolved (%s)"), *GetName());
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("[PDCombat] Unknown ammo=%d (%s)"), static_cast<int32>(Data.AmmoType), *GetName());
		break;
	}
}
