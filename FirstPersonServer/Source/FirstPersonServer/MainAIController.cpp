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