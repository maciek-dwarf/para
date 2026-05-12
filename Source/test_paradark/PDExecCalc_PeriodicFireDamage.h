#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "PDExecCalc_PeriodicFireDamage.generated.h"

UCLASS()
class TEST_PARADARK_API UPDExecCalc_PeriodicFireDamage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UPDExecCalc_PeriodicFireDamage();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
