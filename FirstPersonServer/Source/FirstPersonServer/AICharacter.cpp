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
