#include "CF_Widget_VHSOverlay.h"

#include "MediaPlayer.h"
#include "MediaSource.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "TimerManager.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCF_Widget_VHSOverlay::NativeConstruct()
{
	Super::NativeConstruct();

	UpdateTime();

	auto batteryBrush = Battery->GetBrush();
	if (auto* mat = Cast<UMaterial>(batteryBrush.GetResourceObject()))
		Mat_Battery = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, mat);

	Battery->SetBrushFromMaterial(Mat_Battery);

	if(auto* world = GetWorld())
	{
		FTimerHandle TH_UpdateTime;
		world->GetTimerManager().SetTimer(TH_UpdateTime, this, &UCF_Widget_VHSOverlay::UpdateTime, 10.f, true);

		FTimerHandle TH_UpdateBattery;
		world->GetTimerManager().SetTimer(TH_UpdateBattery, this, &UCF_Widget_VHSOverlay::UpdateBattery, TimeToDie * 60.f / 4.f, true);
	}

	RandomizeOverlay();
}

FString UCF_Widget_VHSOverlay::FixTimeString(const FString& inTime) const
{
	return FString::Printf(TEXT("%02d"), *inTime);
}

FText UCF_Widget_VHSOverlay::CalculateTime() const
{
	const float realTime = UGameplayStatics::GetRealTimeSeconds(this) / 3600.f;
	const int32 truncTime = FMath::TruncToInt32(realTime);

	int32 totalHours = truncTime + StartHour;

	int32 totalMinutes = StartMinute + (realTime - (totalHours - (totalHours % 24) + truncTime) * 60);

	const int32 hours = totalMinutes / 60 + (totalHours % 24) % 24;
	const int32 mins = totalMinutes % 60;

	const FString period = hours < 12 ? "AM" : "PM";

	return FText::FromString(FString::Printf(TEXT("%s %02d:%02d"), *period, hours, mins));
}

UMediaSource* UCF_Widget_VHSOverlay::GetRandomOverlay()
{
	int32 idx = FMath::RandRange(0, Overlays.Num() - 1);
	const auto& overlay = Overlays[idx];

	if (overlay == LastOverlay)
		return GetRandomOverlay();

	LastOverlay = overlay;
	return overlay;
}

void UCF_Widget_VHSOverlay::RandomizeOverlay()
{
	MP_Overlay->OpenSource(GetRandomOverlay());

	StartRandomOverlayTimer();
}

void UCF_Widget_VHSOverlay::StartRandomOverlayTimer()
{
	if (auto* world = GetWorld())
	{
		FTimerHandle TH_Overlay;
		world->GetTimerManager().SetTimer(TH_Overlay, this, &UCF_Widget_VHSOverlay::RandomizeOverlay, FMath::RandRange(5.f, 20.f), false);
	}
}

void UCF_Widget_VHSOverlay::UpdateTime()
{
	TXT_Time->SetText(CalculateTime());
}

void UCF_Widget_VHSOverlay::UpdateBattery()
{
	MissingBattery += 0.33f;

	const float twoThirds = 1.f / 1.5f;
	Mat_Battery->SetScalarParameterValue(FName("MissingBattery"), FMath::Min(MissingBattery, twoThirds));

	if (MissingBattery < twoThirds)
		return;

	Mat_Battery->SetScalarParameterValue(FName("BatteryLow"), 1.f);
}

void UCF_Widget_VHSOverlay::UpdateZoom(const float inZoom)
{
	float zoom = FMath::Clamp(inZoom, 1.f, 4.f) / 2.f;
	const FString zoomStr = FString::Printf(TEXT("X %f"), zoom);

	TXT_Zoom->SetText(FText::FromString(zoomStr));
}
