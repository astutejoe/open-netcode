#pragma once

#include "Engine.h"
#include "GameFramework/HUD.h"
#include "GameHUD.generated.h"

UCLASS()
class FIRSTPERSONCLIENT_API AGameHUD : public AHUD
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly)
	UTexture2D* CrosshairTexture;

public:
	virtual void DrawHUD() override;
};
