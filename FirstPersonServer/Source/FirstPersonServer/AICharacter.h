#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "AICharacter.generated.h"

UCLASS()
class FIRSTPERSONSERVER_API AAICharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* exit_location;

	AAICharacter();

	float health = 100.0f;
	int32 id;
	bool ads;

	FRotator spawn_rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Character")
	bool crouching;

	void Hit(float damage);

	class AMainAIController* controller = nullptr;
};
