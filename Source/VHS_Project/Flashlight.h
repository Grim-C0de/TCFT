#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"

#include "Flashlight.generated.h"

class USpotLightComponent;
class UCurveFloat;

UCLASS()
class VHS_PROJECT_API AFlashlight : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AFlashlight();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USceneComponent* Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USpotLightComponent* SpotLight;

	UFUNCTION() void StartFlickering();
	void StopFlickering();

	void ToggleFlashlight();

protected:
	
	virtual void BeginPlay() override;

	float LightIntensity = 50000.f;

	bool bIsLightOn = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<UCurveFloat*> FlickerCurves = {};

	UTimelineComponent* TL_Flickering;

	FOnTimelineFloat TimelineFlickeringFloatFn;
	FOnTimelineEventStatic TimelineFlickeringFinishedFn;

	// ----------------------------------------------------

	UFUNCTION() void HandleTL(float Value);

	void UpdateIntensity(const float Alpha);

	void SwitchLight(const bool bIsOn);

	void TimedFlickering(const float Duration);

	void CheckEndFlickering();

public:	
	
	virtual void Tick(float DeltaTime) override;

};
