#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FirstPersonClientGameModeBase.h"
#include "PlayerPawn.h"
#include "MainPlayerController.generated.h"

const unsigned int SEND_BUFFER_SIZE = (sizeof(OnlinePlayerInput) + UPDATE_HEADER_OFFSET);
const unsigned short MAX_HISTORY = 1200;

const float SPRINT_SPEED = 600.0f; //cm/s
const float SPRINT_ACCELERATION = 1200.0f; //cm/s
const float JOG_SPEED = 400.0f; //cm/s
const float JOG_ACCELERATION = 800.0f; //cm/s
const float CROUCH_SPEED = 200.0f; //cm/s
const float CROUCH_ACCELERATION = 400.0f; //cm/s

const float DEACELERATION_COEFICIENT = 4.0f;

const float BACKWARD_COEFICIENT = 2.0f;

const float DISCREPANCY_THRESHOLD = 0.01f;

const float ADS_FOV_DIFFERENCE = 20.0f;

const float CAMERA_INTERPOLATION_SPEED = 5.0f;

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
	void BeginAim();
	void EndAim();
	void Jump();
	void Crouch();
	void Sprint();
	void Unsprint();
	void Fire();
	void EndFire();
	void Reload();

	FVector2D UpdateVelocity(float move_forward, float move_right, float speed, float acceleration, float DeltaTime, bool grounded, bool jumped);

	float ads_camera_fov;
	float camera_fov;

	FVector camera_target_location;
	bool interpolate_camera;

	bool firing_pressed = false;

private:
	void ApplyGravity(float DeltaTime);
};
