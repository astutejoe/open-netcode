#include "MainAIController.h"

void AMainAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	pawn = Cast<AAICharacter>(InPawn);
	pawn->controller = this;
}

void AMainAIController::SetTarget(APawn* new_target)
{
	GetBlackboardComponent()->SetValueAsObject(target_key, new_target);
}

void AMainAIController::ShootTarget()
{
	Cast<AFirstPersonServerGameModeBase>(GetWorld()->GetAuthGameMode())->ReplicateShot(pawn->id);
	float rand = FMath::FRand();

	//30% chance
	if (rand > 0.3)
	{
		Cast<APlayerPawn>(GetBlackboardComponent()->GetValueAsObject(target_key))->Hit(50.0f);
	}
}