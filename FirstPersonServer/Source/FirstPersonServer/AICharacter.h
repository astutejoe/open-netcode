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

	float health;
	int32 id;
	bool ads;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Character")
	bool crouching;

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Hit(float damage);

	class AMainAIController* controller = nullptr;
};
