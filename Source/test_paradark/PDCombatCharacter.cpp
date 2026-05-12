#include "PDCombatCharacter.h"

#include "PDAbilitySystemComponent.h"
#include "PDAttributeSet.h"
#include "PDGameplayEffect_Burning.h"
#include "PDGameplayEffect_InstantHealthDelta.h"
#include "PDGameplayEffect_Wet.h"
#include "PDGameplayTags.h"
#include "GameplayEffect.h"

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

void APDCombatCharacter::ApplyInstantHealthDelta(const float Delta)
{
	if (!AbilitySystemComponent || !InstantHealthDeltaEffect)
	{
		return;
	}

	FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
	Ctx.AddSourceObject(this);

	const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(InstantHealthDeltaEffect, 1.f, Ctx);
	if (!Spec.IsValid() || !Spec.Data.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PDCombat] Failed to build instant health delta spec (%s)"), *GetName());
		return;
	}

	Spec.Data->SetSetByCallerMagnitude(TAG_PD_Data_HealthDelta, Delta, false);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	UE_LOG(LogTemp, Log, TEXT("[PDCombat] Instant health delta %.2f applied (%s)"), Delta, *GetName());
}

FActiveGameplayEffectHandle APDCombatCharacter::TryApplyBurn(const float FireImpactDamage)
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
		AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveBurnHandle, 1);
		ActiveBurnHandle.Invalidate();
		UE_LOG(LogTemp, Log, TEXT("[PDCombat] Burn refreshed: removed previous effect (%s)"), *GetName());
	}

	FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
	Ctx.AddSourceObject(this);

	const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(BurnEffect, 1.f, Ctx);
	if (!Spec.IsValid() || !Spec.Data.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PDCombat] Failed to build burn spec (%s)"), *GetName());
		return FActiveGameplayEffectHandle();
	}

	Spec.Data->SetSetByCallerMagnitude(TAG_PD_Data_BurnDuration, BurnDurationSeconds, false);
	Spec.Data->SetSetByCallerMagnitude(TAG_PD_Data_PeriodicFireDamage, BurnDamagePerTick, false);

	const FActiveGameplayEffectHandle Handle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	ActiveBurnHandle = Handle;

	if (FireImpactDamage > 0.f)
	{
		ApplyInstantHealthDelta(-FireImpactDamage);
	}

	UE_LOG(LogTemp, Log, TEXT("[PDCombat] Burn applied: duration=%.2f tickDmg=%.2f (%s)"),
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
		AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveBurnHandle, 1);
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
		1.f,
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
		ApplyInstantHealthDelta(-Data.Magnitude);
		UE_LOG(LogTemp, Log, TEXT("[PDCombat] Regular damage=%.2f (%s)"), Data.Magnitude, *GetName());
		break;
	case 1:
		TryApplyBurn(Data.Magnitude);
		UE_LOG(LogTemp, Log, TEXT("[PDCombat] Fire impact=%.2f (%s)"), Data.Magnitude, *GetName());
		break;
	case 2:
		ExtinguishBurnAndApplyWet();
		UE_LOG(LogTemp, Log, TEXT("[PDCombat] Water hit (%s)"), *GetName());
		break;
	case 3:
		ApplyInstantHealthDelta(Data.Magnitude);
		UE_LOG(LogTemp, Log, TEXT("[PDCombat] Heal=%.2f (%s)"), Data.Magnitude, *GetName());
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("[PDCombat] Unknown ammo=%d (%s)"), static_cast<int32>(Data.AmmoType), *GetName());
		break;
	}
}
