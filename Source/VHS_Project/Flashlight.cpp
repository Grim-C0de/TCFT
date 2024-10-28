#include "Flashlight.h"

#include "Components/SpotLightComponent.h"
#include "Curves/CurveFloat.h"

AFlashlight::AFlashlight()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>("Scene");
	SetRootComponent(Root);
	SpotLight = CreateDefaultSubobject<USpotLightComponent>("SpotLight");
	SpotLight->SetupAttachment(RootComponent);

	TL_Flickering = CreateDefaultSubobject<UTimelineComponent>(TEXT("TL_Flickering"));
}

void AFlashlight::StartFlickering()
{
	if (!TL_Flickering)
		return;

	int32 idx = FMath::RandRange(0, FlickerCurves.Num() - 1);
	TL_Flickering->SetFloatCurve(FlickerCurves[idx], TEXT("Alpha"));
	TL_Flickering->SetPlayRate(FMath::RandRange(0.6f, 1.f));
	TL_Flickering->PlayFromStart();
}

void AFlashlight::StopFlickering()
{
	if (TL_Flickering)
		TL_Flickering->Stop();

	CheckEndFlickering();
}

void AFlashlight::ToggleFlashlight()
{
	bIsLightOn = !bIsLightOn;
	SwitchLight(bIsLightOn);
}

void AFlashlight::BeginPlay()
{
	Super::BeginPlay();
	
	SwitchLight(bIsLightOn);
	TimelineFlickeringFloatFn.BindUFunction(this, FName("HandleTL"));
	TimelineFlickeringFinishedFn.BindUFunction(this, FName("StartFlickering"));

	TL_Flickering->SetTimelineFinishedFunc(TimelineFlickeringFinishedFn);

	UCurveFloat* Curve = NewObject<UCurveFloat>(this, UCurveFloat::StaticClass(), TEXT("FloatCurve"));
	Curve->FloatCurve.UpdateOrAddKey(0.f, 0.f);
	Curve->FloatCurve.UpdateOrAddKey(.5f, 1.f);
	Curve->FloatCurve.UpdateOrAddKey(1.f, 0.f);
	
	TL_Flickering->AddInterpFloat(Curve, TimelineFlickeringFloatFn, FName("Alpha"));
	TL_Flickering->SetLooping(false);
	TL_Flickering->SetIgnoreTimeDilation(true);
}

void AFlashlight::HandleTL(float Value)
{
	UpdateIntensity(Value);
}

void AFlashlight::UpdateIntensity(const float Alpha)
{
	SpotLight->SetIntensity(Alpha * LightIntensity);
}

void AFlashlight::SwitchLight(const bool bIsOn)
{
	SpotLight->SetIntensity(bIsOn * LightIntensity);
}

void AFlashlight::TimedFlickering(const float Duration)
{
	FTimerHandle TH_Flicker;
	StartFlickering();
	GetWorld()->GetTimerManager().SetTimer(TH_Flicker, FTimerDelegate::CreateLambda([this]() { StopFlickering(); }), Duration, false);
}

void AFlashlight::CheckEndFlickering()
{
	SwitchLight(bIsLightOn);
}

void AFlashlight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

