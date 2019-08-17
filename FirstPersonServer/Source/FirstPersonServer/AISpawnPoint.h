#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "AISpawnPoint.generated.h"

UCLASS()
class FIRSTPERSONSERVER_API AAISpawnPoint : public ATargetPoint
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "AI Defaults")
	TSet<FName> enemy_tags;
};
