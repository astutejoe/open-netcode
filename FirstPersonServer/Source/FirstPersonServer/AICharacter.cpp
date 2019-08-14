#include "AICharacter.h"
#include "MainAIController.h"

AAICharacter::AAICharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAICharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAICharacter::Hit(float damage)
{
	health -= damage;
}
