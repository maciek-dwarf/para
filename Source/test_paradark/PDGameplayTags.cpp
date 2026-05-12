#include "PDGameplayTags.h"

#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_PD_State_Burning, "PD.State.Burning", "Target is burning (GAS state).");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_PD_State_Wet, "PD.State.Wet", "Target is wet; blocks applying burn briefly.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_PD_Data_HealthDelta, "PD.Data.HealthDelta", "SetByCaller: additive health change (negative = damage).");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_PD_Data_BurnDuration, "PD.Data.BurnDuration", "SetByCaller: burn duration in seconds.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_PD_Data_PeriodicFireDamage, "PD.Data.PeriodicFireDamage", "SetByCaller: fire damage applied each burn tick.");
