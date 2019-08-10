#include "MainPlayerController.h"
#include "NavigationSystem.h"
#include "Engine/Engine.h"
#include <cmath>

const float GRAVITY = -980.f;

APlayerPawn* pawn;

OnlinePlayerInput player_input;

ObjectHistory my_histories[MAX_HISTORY];
int current_history = 0;

uint32 latency = 0;
uint32 last_sequence_update = 0xffffffff;
uint32 input_sequence = 0;

FVector velocity;

FVector target_location;

bool grounded = false;
bool jumping = false;
bool jumped = false;

AMainPlayerController::AMainPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	pawn = Cast<APlayerPawn>(GetPawn());

	velocity.X = 0.0f;
	velocity.Y = 0.0f;
	velocity.Z = 0.0f;

	UNavigationSystemV1* nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	FNavLocation tmp;
	bool inside_navmesh = nav->ProjectPointToNavigation(pawn->GetActorLocation(), tmp);

	if (!inside_navmesh)
	{
		PRINT_DEBUG("Invalid spawn location!");
	}

	camera_fov = pawn->camera->FieldOfView;
	ads_camera_fov = camera_fov - ADS_FOV_DIFFERENCE;
}

void AMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward");
	InputComponent->BindAxis("MoveRight");
	InputComponent->BindAxis("Turn");
	InputComponent->BindAxis("TurnUp");
	InputComponent->BindAction("Aim", IE_Pressed, this, &AMainPlayerController::BeginAim);
	InputComponent->BindAction("Aim", IE_Released, this, &AMainPlayerController::EndAim);
	InputComponent->BindAction("Jump", IE_Pressed, this, &AMainPlayerController::Jump);
	InputComponent->BindAction("Crouch", IE_Pressed, this, &AMainPlayerController::Crouch);
	InputComponent->BindAction("Fire", IE_Pressed, this, &AMainPlayerController::Fire);
	InputComponent->BindAction("Fire", IE_Released, this, &AMainPlayerController::EndFire);
	InputComponent->BindAction("Reload", IE_Pressed, this, &AMainPlayerController::Reload);
	InputComponent->BindAction("Sprint", IE_Pressed, this, &AMainPlayerController::Sprint);
	InputComponent->BindAction("Sprint", IE_Released, this, &AMainPlayerController::Unsprint);
}

void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (interpolate_camera)
	{
		FVector camera_relative_location = pawn->spine_reference->RelativeLocation;
		pawn->spine_reference->SetRelativeLocation(FMath::VInterpConstantTo(camera_relative_location, camera_target_location, DeltaTime, CAMERA_INTERPOLATION_SPEED*80.0f));

		if (camera_relative_location.Equals(camera_target_location))
		{
			interpolate_camera = false;
		}
	}

	if (firing_pressed)
	{
		if (pawn->weapon != nullptr && pawn->weapon->Fire())
		{
			//ClientPlayCameraShake(pawn->weapon->cam_shake);

			TurnUp(-pawn->weapon->recoil);

#if ONLINE
			StaticNetworking::SendRPC((uint8)PacketType::RPC, (uint8)RPCAction::Fire, 0, nullptr);
#endif
		}
	}

	player_input.move_forward = GetInputAxisValue("MoveForward");
	player_input.move_right = GetInputAxisValue("MoveRight");
	player_input.turn = GetInputAxisValue("Turn");
	player_input.turn_up = GetInputAxisValue("TurnUp");
	player_input.jumped = jumped;
	jumped = false;

	//Makes delta time independent
	player_input.move_forward = player_input.move_forward * DeltaTime;
	player_input.move_right = player_input.move_right * DeltaTime;

	float speed = JOG_SPEED;
	float acceleration = JOG_ACCELERATION;

	if (pawn->crouching)
	{
		speed = CROUCH_SPEED;
		acceleration = CROUCH_ACCELERATION;
	}
	else if (pawn->sprinting)
	{
		speed = SPRINT_SPEED;
		acceleration = SPRINT_ACCELERATION;
	}

	player_input.crouching = pawn->crouching;
	player_input.sprinting = pawn->sprinting;

	//Normalize and update the unormalized velocity
	FVector2D velocity_normal = UpdateVelocity(player_input.move_forward, player_input.move_right, speed, acceleration, DeltaTime, grounded, player_input.jumped);

	MoveForward(velocity_normal.X * DeltaTime);
	MoveRight(velocity_normal.Y * DeltaTime);
	MoveUp(velocity.Z * DeltaTime);

	pawn->velocity = velocity;

	Turn(player_input.turn);
	TurnUp(player_input.turn_up);

#if ONLINE
	ObjectHistory my_object_history = AFirstPersonClientGameModeBase::GetMyObject();

	if (my_object_history.object.id >= 0)
	{
		if (my_object_history.sequence != last_sequence_update)
		{
			FVector server_position = FVector(my_object_history.object.position[0], my_object_history.object.position[1], my_object_history.object.position[2]);
			FRotator server_rotation = FRotator(my_object_history.object.rotation[1], my_object_history.object.rotation[2], my_object_history.object.rotation[0]);
			FVector my_position;
			FRotator my_rotation;

			int history_position;
			for (history_position = 0; history_position < MAX_HISTORY; history_position++)
			{
				if (my_histories[history_position].sequence == my_object_history.sequence)
				{
					my_position = FVector(my_histories[history_position].object.position[0], my_histories[history_position].object.position[1], my_histories[history_position].object.position[2]);
					my_rotation = FRotator(my_histories[history_position].object.rotation[1], my_histories[history_position].object.rotation[2], my_histories[history_position].object.rotation[0]);
					latency = (uint32)((FPlatformTime::Seconds() - my_histories[history_position].timestamp) * 1000);
					break;
				}
			}

			float location_discrepancy = (my_position - server_position).Size();
			float rotation_discrepancy = (my_rotation.Euler() - server_rotation.Euler()).Size();

			if (location_discrepancy > DISCREPANCY_THRESHOLD)
			{
				PRINT_DEBUG("Position corrected");
				pawn->SetActorLocation(server_position);

				FRotator rotation = GetControlRotation();
				rotation.Yaw = server_rotation.Yaw;

				SetControlRotation(rotation);
				pawn->SetActorRotation(rotation);

				pawn->spine_reference->SetRelativeRotation(FRotator(server_rotation.Roll, 0.0f, 0.0f));

				velocity.X = my_object_history.object.velocity[0];
				velocity.Y = my_object_history.object.velocity[1];
				velocity.Z = my_object_history.object.velocity[2];

				grounded = my_object_history.object.grounded;

				jumped = my_object_history.player_input.jumped;

				history_position++;
				while (my_histories[history_position].sequence > my_object_history.sequence)
				{
					float sim_speed = JOG_SPEED;
					float sim_acceleration = JOG_ACCELERATION;

					if (my_histories[history_position].player_input.crouching)
					{
						sim_speed = CROUCH_SPEED;
						sim_acceleration = CROUCH_ACCELERATION;
					}
					else if (my_histories[history_position].player_input.sprinting)
					{
						sim_speed = SPRINT_SPEED;
						sim_acceleration = SPRINT_ACCELERATION;
					}

					FVector2D sim_velocity_normal = UpdateVelocity(my_histories[history_position].player_input.move_forward,
						my_histories[history_position].player_input.move_right,
						sim_speed,
						sim_acceleration,
						my_histories[history_position].player_input.delta_time,
						my_histories[history_position].object.grounded,
						my_histories[history_position].player_input.jumped);

					MoveForward(sim_velocity_normal.X * my_histories[history_position].player_input.delta_time);
					MoveRight(sim_velocity_normal.Y * my_histories[history_position].player_input.delta_time);

					MoveUp(velocity.Z * my_histories[history_position].player_input.delta_time);

					Turn(my_histories[history_position].player_input.turn);
					TurnUp(my_histories[history_position].player_input.turn_up);

					my_histories[history_position].object.position[0] = pawn->GetActorLocation().X;
					my_histories[history_position].object.position[1] = pawn->GetActorLocation().Y;
					my_histories[history_position].object.position[2] = pawn->GetActorLocation().Z;

					my_histories[history_position].object.rotation[0] = pawn->spine_reference->GetComponentRotation().Pitch;
					my_histories[history_position].object.rotation[1] = pawn->GetActorRotation().Pitch;
					my_histories[history_position].object.rotation[2] = pawn->GetActorRotation().Yaw;

					history_position = (history_position + 1) % MAX_HISTORY;
				}

				velocity_normal = UpdateVelocity(player_input.move_forward, player_input.move_right, speed, acceleration, DeltaTime, grounded, player_input.jumped);

				MoveForward(velocity_normal.X * DeltaTime);
				MoveRight(velocity_normal.Y * DeltaTime);
				MoveUp(velocity.Z * DeltaTime);

				Turn(player_input.turn);
				TurnUp(player_input.turn_up);
			}
			else if (rotation_discrepancy > DISCREPANCY_THRESHOLD)
			{
				PRINT_DEBUG("Rotation corrected");

				FRotator rotation = pawn->GetActorRotation();
				rotation.Yaw = server_rotation.Yaw;

				SetControlRotation(rotation);
				pawn->SetActorRotation(rotation);

				pawn->spine_reference->SetWorldRotation(FRotator(server_rotation.Roll, 0.0f, 0.0f));

				history_position++;
				while (my_histories[history_position].sequence > my_object_history.sequence)
				{
					Turn(my_histories[history_position].player_input.turn);
					TurnUp(my_histories[history_position].player_input.turn_up);

					my_histories[history_position].object.rotation[0] = pawn->spine_reference->GetComponentRotation().Pitch;
					my_histories[history_position].object.rotation[1] = pawn->GetActorRotation().Pitch;
					my_histories[history_position].object.rotation[2] = pawn->GetActorRotation().Yaw;

					history_position = (history_position + 1) % MAX_HISTORY;
				}

				Turn(player_input.turn);
				TurnUp(player_input.turn_up);
			}

			last_sequence_update = my_object_history.sequence;
		}

		if (my_object_history.object.health <= 0.0f && !GetWorld()->GetGameViewport()->bDisableWorldRendering)
		{
			GetWorld()->GetGameViewport()->bDisableWorldRendering = true;
		}
		else if (my_object_history.object.health > 0.0f && GetWorld()->GetGameViewport()->bDisableWorldRendering)
		{
			GetWorld()->GetGameViewport()->bDisableWorldRendering = false;
		}

		pawn->health = my_object_history.object.health;
	}

	if (StaticNetworking::Joined() && my_object_history.object.id >= 0)
	{
		uint8 buffer[SEND_BUFFER_SIZE];
		memset(buffer, 0, SEND_BUFFER_SIZE);
		buffer[PACKET_TYPE] = (uint8)PacketType::Update;
		memcpy(buffer + 1, &latency, sizeof(uint32));

		player_input.sequence = input_sequence;
		player_input.ads = pawn->aiming_downsights;
		player_input.delta_time = DeltaTime;

		memcpy(buffer + UPDATE_HEADER_OFFSET, &player_input, sizeof(OnlinePlayerInput));

		StaticNetworking::SendData(buffer, SEND_BUFFER_SIZE);

		ObjectHistory my_history;
		my_history.sequence = input_sequence;

		my_history.object.velocity[0] = velocity.X;
		my_history.object.velocity[1] = velocity.Y;
		my_history.object.velocity[2] = velocity.Z;

		my_history.object.position[0] = pawn->GetActorLocation().X;
		my_history.object.position[1] = pawn->GetActorLocation().Y;
		my_history.object.position[2] = pawn->GetActorLocation().Z;

		my_history.object.rotation[0] = pawn->spine_reference->GetComponentRotation().Pitch;
		my_history.object.rotation[1] = pawn->GetActorRotation().Pitch;
		my_history.object.rotation[2] = pawn->GetActorRotation().Yaw;

		my_history.object.grounded = grounded;

		my_history.player_input = player_input;

		my_history.timestamp = FPlatformTime::Seconds();

		my_histories[current_history] = my_history;

		current_history = (current_history + 1) % MAX_HISTORY;

		input_sequence++;
	}

#endif
	GEngine->AddOnScreenDebugMessage(0, 3.f, FColor::Blue, FString::Printf(TEXT("My position: %f %f %f, velocity %f %f %f"), pawn->GetActorLocation().X, pawn->GetActorLocation().Y, pawn->GetActorLocation().Z, velocity.X, velocity.Y, velocity.Z), false);

#if ONLINE
	GEngine->AddOnScreenDebugMessage(1, 3.f, StaticNetworking::Joined() ? FColor::Green : FColor::Red, FString::Printf(TEXT("Connected: %s"), StaticNetworking::Joined() ? TEXT("true") : TEXT("false")), false);
	GEngine->AddOnScreenDebugMessage(2, 3.f, my_object_history.object.id >= 0 ? FColor::Green : FColor::Red, FString::Printf(TEXT("Syncing: %s"), my_object_history.object.id >= 0 ? TEXT("true") : TEXT("false")), false);
	GEngine->AddOnScreenDebugMessage(3, 3.f, FColor::Blue, FString::Printf(TEXT("Latency: %d"), latency), false);
#endif
}

void AMainPlayerController::BeginAim()
{
	if (pawn->weapon != nullptr && !pawn->weapon->reloading)
	{
		pawn->camera_target_fov = ads_camera_fov;

		FVector weapon_relative_location = pawn->weapon_component->RelativeLocation;
		FVector front_sight_location = pawn->weapon_mesh->GetSocketLocation("front_sight");
		FTransform fps_camera_world_transform = pawn->camera->GetComponentTransform();
		FVector front_sight_relative_location = fps_camera_world_transform.InverseTransformPosition(front_sight_location);

		front_sight_relative_location.X = 0.0f;

		FVector final_location = weapon_relative_location - front_sight_relative_location;

		pawn->weapon_target_location = final_location;

		pawn->interpolate_weapon_location = true;
		pawn->aiming_downsights = true;
		pawn->sprinting = false;
	}
}

void AMainPlayerController::EndAim()
{
	pawn->camera_target_fov = camera_fov;
	if (pawn->weapon != nullptr)
	{
		pawn->aiming_downsights = false;
		pawn->interpolate_weapon_location = false;
	}
}

void AMainPlayerController::Jump()
{
	if (grounded)
	{
		jumping = true;
		jumped = true;
	}
}

void AMainPlayerController::Sprint()
{
	pawn->sprinting = true;
}

void AMainPlayerController::Unsprint()
{
	pawn->sprinting = false;
}

void AMainPlayerController::Crouch()
{
	pawn->crouching = !pawn->crouching;
	if (pawn->crouching)
		camera_target_location = pawn->crouch_camera_location;
	else
		camera_target_location = pawn->camera_location;

	interpolate_camera = true;
}

void AMainPlayerController::Fire()
{
	if (pawn->weapon != nullptr && pawn->weapon->Fire())
	{
		//pawn->PlayerFired();

		//ClientPlayCameraShake(pawn->weapon->cam_shake);

		TurnUp(-pawn->weapon->recoil);

		firing_pressed = true;

#if ONLINE
		StaticNetworking::SendRPC((uint8)PacketType::RPC, (uint8)RPCAction::Fire, 0, nullptr);
#endif
	}
}

void AMainPlayerController::EndFire()
{
	firing_pressed = false;
}

void AMainPlayerController::Reload()
{
	if (pawn->weapon != nullptr && pawn->weapon->Reload())
	{
#if ONLINE
		StaticNetworking::SendRPC((uint8)PacketType::RPC, (uint8)RPCAction::Reload, 0, nullptr);
#endif
	}
}

void AMainPlayerController::MoveForward(float value)
{
	if (value != 0)
	{
		pawn->AddActorLocalOffset(FVector::ForwardVector * value, false, (FHitResult*) nullptr, ETeleportType::None);

		UNavigationSystemV1* nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

		FNavLocation tmp;
		bool inside_navmesh = nav->ProjectPointToNavigation(pawn->GetActorLocation(), tmp);

		//rollback to previous position
		if (!inside_navmesh)
		{
			pawn->AddActorLocalOffset(-(FVector::ForwardVector * value), false, (FHitResult*) nullptr, ETeleportType::None);
			pawn->velocity.X = 0.0f;
		}
	}
}

void AMainPlayerController::MoveRight(float value)
{
	if (value != 0)
	{
		pawn->AddActorLocalOffset(FVector::RightVector * value, false, (FHitResult*) nullptr, ETeleportType::None);

		UNavigationSystemV1* nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

		FNavLocation tmp;
		bool inside_navmesh = nav->ProjectPointToNavigation(pawn->GetActorLocation(), tmp);

		if (!inside_navmesh)
		{
			pawn->AddActorLocalOffset(-(FVector::RightVector * value), false, (FHitResult*) nullptr, ETeleportType::None);
			pawn->velocity.Y = 0.0f;
		}
	}
}

void AMainPlayerController::ApplyGravity(float DeltaTime)
{
	velocity.Z += GRAVITY * DeltaTime * 2.0f; //multiplier that makes more arcadeful, less real
}

void AMainPlayerController::MoveUp(float value)
{
	const FCollisionShape& collisionShape = pawn->capsule->GetCollisionShape();

	FNavLocation navmesh_location;

	UNavigationSystemV1* nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	nav->ProjectPointToNavigation(pawn->GetActorLocation(), navmesh_location);

	FVector playerPositionOnNavMesh = navmesh_location.Location + FVector::UpVector * collisionShape.Capsule.HalfHeight;
	const FVector movement = FVector::UpVector * value;

	const bool is_falling = value < 0.f;
	const FVector playerPosition = (pawn->GetActorLocation() + movement);
	const float distanceFromGround = playerPosition.Z - playerPositionOnNavMesh.Z;

	const bool attemptSnapToGround = (is_falling || !jumping);

	const float maxDownStep = 2.f;
	grounded = attemptSnapToGround && distanceFromGround <= maxDownStep;

	pawn->grounded = grounded;
	pawn->is_falling = is_falling;
	pawn->jumping = jumping;

	if (grounded)
	{
		pawn->SetActorLocation(playerPositionOnNavMesh);

		velocity.Z = 0.f;
		jumping = false;
	}
	else
	{
		pawn->AddActorLocalOffset(movement);
	}
}

void AMainPlayerController::Turn(float value)
{
	FRotator rotation = pawn->GetActorRotation();
	rotation.Yaw += value;

	SetControlRotation(rotation);
	pawn->SetActorRotation(rotation);
}

void AMainPlayerController::TurnUp(float value)
{
	FRotator spine_reference_rotation = pawn->spine_reference->GetComponentRotation();

	spine_reference_rotation.Pitch = FMath::ClampAngle(spine_reference_rotation.Pitch - value, -MAX_ROTATION, MAX_ROTATION);
	spine_reference_rotation.Roll = 0.0f;
	spine_reference_rotation.Yaw = 0.0f;

	pawn->spine_reference->SetRelativeRotation(spine_reference_rotation);
}

FVector2D AMainPlayerController::UpdateVelocity(float move_forward, float move_right, float speed, float acceleration, float DeltaTime, bool grounded, bool jumped)
{
	if (move_forward != 0.0f)
	{
		velocity.X += acceleration * move_forward;
	}
	else
	{
		if (velocity.X > 0.0f)
		{
			velocity.X -= acceleration * DeltaTime;

			if (velocity.X < 0.0f)
				velocity.X = 0.0f;
		}
		else
		{
			velocity.X += acceleration * DeltaTime;

			if (velocity.X > 0.0f)
				velocity.X = 0.0f;
		}
	}

	velocity.X = velocity.X > speed ? speed : velocity.X;
	velocity.X = velocity.X < -speed ? -speed : velocity.X;

	if (move_right != 0.0f)
	{
		velocity.Y += acceleration * move_right;
	}
	else
	{
		if (velocity.Y > 0.0f)
		{
			velocity.Y -= acceleration * DeltaTime;

			if (velocity.Y < 0.0f)
				velocity.Y = 0.0f;
		}
		else
		{
			velocity.Y += acceleration * DeltaTime;

			if (velocity.Y > 0.0f)
				velocity.Y = 0.0f;
		}
	}

	velocity.Y = velocity.Y > speed ? speed : velocity.Y;
	velocity.Y = velocity.Y < -speed ? -speed : velocity.Y;

	ApplyGravity(DeltaTime);

	if (grounded && jumped)
		velocity.Z = 700.0f;

	//Normalize the velocity so it's always length 1
	FVector2D velocity_normal = FVector2D(velocity.X, velocity.Y);

	if (velocity_normal.Size() > speed)
	{
		velocity_normal.Normalize();
		velocity_normal *= speed;
	}

	if (velocity_normal.X < 0.0f)
	{
		velocity_normal.X = velocity_normal.X / BACKWARD_COEFICIENT;
		velocity_normal.Y = velocity_normal.Y / BACKWARD_COEFICIENT;
	}

	return velocity_normal;
}