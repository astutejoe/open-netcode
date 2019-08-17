#include "MainAIController.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

void AMainAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	pawn = Cast<AAICharacter>(InPawn);
	pawn->controller = this;
}

void AMainAIController::SetTarget(APawn* new_target)
{
	target = new_target;

	if (target != nullptr)
	{
		if (target->IsA(APlayerPawn::StaticClass()) && Cast<APlayerPawn>(target)->health <= 0)
		{
			return;
		}

		GetBlackboardComponent()->SetValueAsObject(target_key, target);
		SetFocus(target, EAIFocusPriority::Default);
		pawn->ads = true;
	}
	else
	{
		GetBlackboardComponent()->ClearValue(target_key);
		ClearFocus(EAIFocusPriority::Default);
		pawn->ads = false;
		pawn->crouching = false;
		pawn->SetActorRotation(pawn->spawn_rotation);
	}
}

void AMainAIController::AddTarget(APawn* new_target)
{
	targets.Add(new_target);

	if (target == nullptr)
	{
		SetTarget(new_target);
	}
	else
	{
		float closest_target = FVector::Distance(pawn->GetActorLocation(), targets[0]->GetActorLocation());
		SetTarget(targets[0]);

		for (int i = 1; i < targets.Num(); i++)
		{
			float distance = FVector::Distance(pawn->GetActorLocation(), targets[i]->GetActorLocation());
			if (distance < closest_target)
			{
				closest_target = distance;
				SetTarget(targets[i]);
			}
		}
	}
}

void AMainAIController::RemoveTarget(APawn* removing_target)
{
	targets.Remove(removing_target);

	if (removing_target == target)
	{
		if (targets.Num() == 0)
		{
			SetTarget(nullptr);
		}
		else
		{
			float closest_target = FVector::Distance(pawn->GetActorLocation(), targets[0]->GetActorLocation());
			SetTarget(targets[0]);

			for (int i = 1; i < targets.Num(); i++)
			{
				float distance = FVector::Distance(pawn->GetActorLocation(), targets[i]->GetActorLocation());
				if (distance < closest_target)
				{
					closest_target = distance;
					SetTarget(targets[i]);
				}
			}
		}
	}
}

void AMainAIController::Reload()
{
	Cast<AFirstPersonServerGameModeBase>(GetWorld()->GetAuthGameMode())->ReplicateReload(pawn->id);
}

void AMainAIController::ShootTarget()
{
	if (target->IsA(APlayerPawn::StaticClass()) && Cast<APlayerPawn>(target)->health <= 0)
	{
		SetTarget(nullptr);
	}

	USceneComponent* exit_location = pawn->exit_location;

	FVector exit_direction = (exit_location->GetComponentRotation() + FRotator(FMath::FRandRange(-AI_SPREAD, AI_SPREAD), FMath::FRandRange(-AI_SPREAD, AI_SPREAD), 0.0f)).Vector();

	FVector trace_start = exit_location->GetComponentLocation();
	FVector trace_end = trace_start + (exit_direction * MAX_SHOT_RANGE);

	FHitResult hit_out;

	FCollisionObjectQueryParams object_trace_params(
		ECC_TO_BITFIELD(ECC_WorldDynamic) |
		ECC_TO_BITFIELD(ECC_WorldStatic) |
		ECC_TO_BITFIELD(ECC_Pawn) |
		ECC_TO_BITFIELD(ECC_PhysicsBody) |
		ECC_TO_BITFIELD(ECC_Destructible)
	);

	FCollisionQueryParams trace_params(
		FName(TEXT("FireTrace")),
		true
	);

	APlayerPawn* try_cast_player = nullptr;
	AAICharacter* try_cast_character = nullptr;

	bool hit_something = GetWorld()->LineTraceSingleByObjectType(hit_out, trace_start, trace_end, object_trace_params, trace_params);

	if (hit_something && hit_out.Actor != nullptr && hit_out.Actor->IsValidLowLevel())
	{
		try_cast_player = Cast<APlayerPawn>(hit_out.Actor.Get());
		try_cast_character = Cast<AAICharacter>(hit_out.Actor.Get());

		if (try_cast_player != nullptr)
		{
			if (try_cast_player->health > 0.0f)
			{
				try_cast_player->Hit(5.0f); //pending complex damage system
			}

			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Emerald, hit_out.BoneName.ToString(), true);
		}
		else if (try_cast_character != nullptr)
		{
			if (try_cast_character->health > 0.0f)
			{
				try_cast_character->Hit(5.0f);
				Cast<AMainAIController>(try_cast_character->GetController())->SetTarget(pawn);
			}

			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Emerald, hit_out.BoneName.ToString(), true);
		}
	}

	DrawDebugLine(GetWorld(), trace_start, trace_end, FColor::Red, false, 30.0f, ESceneDepthPriorityGroup::SDPG_World, 1);

	Cast<AFirstPersonServerGameModeBase>(GetWorld()->GetAuthGameMode())->ReplicateShot(pawn->id);

	if (try_cast_player != nullptr || try_cast_character != nullptr)
	{
		int32 object_id = try_cast_player != nullptr ? try_cast_player->object_id : try_cast_character->id;
		FRotator hit_backward = (pawn->exit_location->GetForwardVector() * -1).Rotation(); //shot direction inverted

		Cast<AFirstPersonServerGameModeBase>(GetWorld()->GetAuthGameMode())->ReplicateHit(object_id, hit_out, hit_backward);
	}
}