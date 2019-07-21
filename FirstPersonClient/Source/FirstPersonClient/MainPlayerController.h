#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FirstPersonClientGameModeBase.h"
#include "PlayerPawn.h"
#include "MainPlayerController.generated.h"

const unsigned int SEND_BUFFER_SIZE = (sizeof(OnlinePlayerInput) + UPDATE_HEADER_OFFSET);
const unsigned short MAX_HISTORY = 1200;
const float SPRINT_SPEED = 1000.0f;
const float SPRINT_ACCELERATION = 2000.0f;
const float DEACELERATION_COEFICIENT = 2.0f;
const float BACKWARD_COEFICIENT = 2.0f;
const float DISCREPANCY_THRESHOLD = 0.01f;

UCLASS()
class FIRSTPERSONCLIENT_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AMainPlayerController();

	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	void MoveForward(float value);
	void MoveRight(float value);
	void MoveUp(float value);
	void Turn(float value);
	void TurnUp(float value);
	void Jump();
	void Fire();
	void EndFire();
	void Reload();

	FVector2D UpdateVelocity(float move_forward, float move_right, float speed, float acceleration, float DeltaTime, bool grounded, bool jumped);

	bool firing_pressed = false;

private:
	void ApplyGravity(float DeltaTime);
};
