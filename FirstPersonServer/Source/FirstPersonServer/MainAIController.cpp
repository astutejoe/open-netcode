#include "MainAIController.h"
#include "DrawDebugHelpers.h"
#include "BrainComponent.h"
#include "Engine/Engine.h"

void AMainAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	pawn = Cast<AAICharacter>(InPawn);
	pawn->controller = this;
}

bool AMainAIController::IsValidPawn(APawn* pawn_to_test)
{
	if (pawn_to_test != nullptr)
	{
		if (pawn_to_test->IsA(APlayerPawn::StaticClass()) && Cast<APlayerPawn>(pawn_to_test)->health <= 0)
		{
			return false;
		}
		else if (pawn_to_test->IsA(AAICharacter::StaticClass()) && Cast<AAICharacter>(pawn_to_test)->health <= 0)
		{
			return false;
		}
	}

	return true;
}

bool AMainAIController::IsEnemy(AActor* actor)
{
	for (FName enemy_tag : enemy_tags)
	{
		if (actor->ActorHasTag(enemy_tag))
			return true;
	}

	return false;
}

void AMainAIController::SetTarget(APawn* new_target)
{
	target = new_target;

	if (!IsValidPawn(new_target) || mode == EAIMode::Dead)
		return;

	if (target != nullptr)
	{
		if (GetBlackboardComponent() != nullptr)
			GetBlackboardComponent()->SetValueAsObject(target_key, target);

		SetFocus(target, EAIFocusPriority::Default);
		pawn->ads = true;
		SetMode(EAIMode::Engaged);
	}
	else
	{
		if (GetBlackboardComponent() != nullptr)
			GetBlackboardComponent()->ClearValue(target_key);
		ClearFocus(EAIFocusPriority::Default);
		pawn->ads = false;
		pawn->crouching = false;
		pawn->SetActorRotation(pawn->spawn_rotation);
		SetMode(EAIMode::Idle);
	}
}

void AMainAIController::SetMode(EAIMode new_mode)
{
	if (GetBlackboardComponent() != nullptr)
		GetBlackboardComponent()->SetValueAsEnum(mode_key, (uint8)new_mode);
	mode = new_mode;

	if (new_mode == EAIMode::Dead)
	{
		if (GetBlackboardComponent() != nullptr)
			GetBlackboardComponent()->ClearValue(target_key);
		ClearFocus(EAIFocusPriority::Default);
		pawn->ads = false;
		pawn->crouching = false;
		targets.Empty();
		PawnDied();
	}
}

void AMainAIController::AddTarget(APawn* new_target)
{
	if (!IsValidPawn(new_target) || mode == EAIMode::Dead)
		return;

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
	if (mode == EAIMode::Dead)
		return;

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
	if (!IsValidPawn(target))
	{
		RemoveTarget(target);
		return;
	}

	USceneComponent* exit_location = pawn->exit_location;

	FVector exit_direction = (exit_location->GetComponentRotation() + FRotator(FMath::FRandRange(-AI_SPREAD, AI_SPREAD), FMath::FRandRange(-AI_SPREAD, AI_SPREAD), 0.0f)).Vector();

	FVector trace_start = exit_location->GetComponentLocation();
	FVector trace_end = trace_start + (exit_direction * MAX_SHOT_RANGE);

	FHitResult hit_out;

	FCollisionQueryParams trace_params(
		FName(TEXT("FireTrace")),
		true
	);

	APlayerPawn* try_cast_player = nullptr;
	AAICharacter* try_cast_character = nullptr;

	bool hit_something = GetWorld()->LineTraceSingleByChannel(hit_out, trace_start, trace_end, ECollisionChannel::ECC_Visibility, trace_params);

	if (hit_something && hit_out.Actor != nullptr && hit_out.Actor->IsValidLowLevel())
	{
		try_cast_player = Cast<APlayerPawn>(hit_out.Actor.Get());
		try_cast_character = Cast<AAICharacter>(hit_out.Actor.Get());

		if (try_cast_player != nullptr)
		{
			if (try_cast_player->health > 0.0f)
			{
				try_cast_player->Hit(20.0f); //pending complex damage system
			}

			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Emerald, hit_out.BoneName.ToString(), true);
		}
		else if (try_cast_character != nullptr)
		{
			if (try_cast_character->health > 0.0f)
			{
				try_cast_character->Hit(20.0f);
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