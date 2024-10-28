#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CF_Widget_VHSOverlay.generated.h"

class UMediaPlayer;
class UMediaSource;
class UImage;
class UTextBlock;

UCLASS()
class VHS_PROJECT_API UCF_Widget_VHSOverlay : public UUserWidget
{
	GENERATED_BODY()

public:

	void UpdateZoom(const float inZoom);

protected:

	virtual void NativeConstruct() override;

	//

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget)) UTextBlock* TXT_Time;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget)) UTextBlock* TXT_Zoom;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget)) UImage* BatteryFrame;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget)) UImage* Battery;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget)) UImage* VHS_Overlay_One;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget)) UImage* VHS_Overlay_Two;

	UPROPERTY(EditDefaultsOnly, BLueprintReadOnly)
	int32 StartHour = 2;

	UPROPERTY(EditDefaultsOnly, BLueprintReadOnly)
	int32 StartMinute = 53;

	float MissingBattery = 0.f;

	/** Time in minutes */
	float TimeToDie = 30.f;

	UMediaPlayer* MP_Overlay = nullptr;

	UMediaSource* LastOverlay = nullptr;

	TArray<UMediaSource*> Overlays = {};

private:

	UMaterialInstanceDynamic* Mat_Battery = nullptr;

	//

	FString FixTimeString(const FString& inTime) const;

	FText CalculateTime() const;

	UMediaSource* GetRandomOverlay();

	UFUNCTION() void RandomizeOverlay();

	void StartRandomOverlayTimer();

	UFUNCTION() void UpdateTime();

	UFUNCTION() void UpdateBattery();
};
