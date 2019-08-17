#include "AICharacter.h"
#include "MainAIController.h"

AAICharacter::AAICharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	exit_location = CreateDefaultSubobject<USceneComponent>(TEXT("ExitLocation"));
	exit_location->bEditableWhenInherited = true;
	exit_location->SetupAttachment(GetRootComponent());
}

void AAICharacter::BeginPlay()
{
	Super::BeginPlay();
	spawn_rotation = GetActorRotation();
}

void AAICharacter::Hit(float damage)
{
	health -= damage;

	if (health <= 0)
		controller->SetMode(EAIMode::Dead);
}
