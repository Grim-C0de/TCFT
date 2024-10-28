#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"

#include "CF_Player.generated.h"

// # Engine Forwards
class UAudioComponent;
class USpringArmComponent;
class UCameraComponent;
class UUserWidget;
class UInputMappingContext;
class UChildActorComponent;
class UMaterialInterface;
class UCF_Widget_VHSOverlay;
class UInputAction;

// # Project Forwards
class AFlashlight;

DECLARE_MULTICAST_DELEGATE(FOnDialoguesReady)

UENUM(BlueprintType)
enum class ELanguage : uint8
{
	es = 0	UMETA(DisplayName = "es"),
	en = 1	UMETA(DisplayName = "en")
};

UENUM(BlueprintType)
enum class EDanielState : uint8
{
	State2 = 0		UMETA(DisplayName = "State 2"),
	State3 = 1		UMETA(DisplayName = "State 3"),
	State4 = 2		UMETA(DisplayName = "State 4"),
	State5 = 3		UMETA(DisplayName = "State 5"),
	State6 = 4		UMETA(DisplayName = "State 6"),
};

USTRUCT(BlueprintType)
struct FST_Dialogue
{
	GENERATED_BODY()

	UPROPERTY() TArray<USoundWave*> es = {};
	UPROPERTY() TArray<USoundWave*> en = {};

	FST_Dialogue() {}

	FST_Dialogue(const TArray<USoundWave*> inES, const TArray<USoundWave*> inEN) : es(inES), en(inEN) {}
};

UCLASS(Blueprintable)
class VHS_PROJECT_API ACF_Player : public ACharacter
{
	GENERATED_BODY()

public:
	
	ACF_Player();

protected:
	
	virtual void BeginPlay() override;

	FOnDialoguesReady OnDialoguesReady;

	const float HalfHeightCrouch = 20.f;
	const float HalfHeightStanding = 90.f;
	FRotator CameraRotation;

	// SFX --->

	UPROPERTY(EditDefaultsOnly, Category = "CustomProperties | SFX") USoundBase* SFX_Flashlight;

	// Components --->

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) UAudioComponent* Audio;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) USpringArmComponent* SpringLeaning;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) USpringArmComponent* SpringCamera;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) UCameraComponent* FirstPersonCamera;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) UChildActorComponent* CA_Flashlight;

	// Properties --->

	AFlashlight* Flashlight = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | Camera")
	TArray<UMaterialInterface*> PostprocessMaterials = {};

	// Leaning --->
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "CustomProperties | Controls | Leaning")
	bool bIsLeaningLeft = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "CustomProperties | Controls | Leaning")
	bool bIsLeaningRight = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "CustomProperties | Controls | Leaning")
	float LeanDirection = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "CustomProperties | Controls | Leaning")
	float LeanStart = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "CustomProperties | Controls | Leaning")
	float LeanDistance = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | Controls | Leaning")
	float LeanDuration = 0.5f;

	// Movement --->
	bool bIsSprinting = false;

	bool bIsCrouching = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | Controls | Movement")
	float WalkSpeed = 230.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | Controls | Movement")
	float SprintSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | Controls | Movement")
	float CrouchSpeed = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | Controls | Movement")
	float FastCrouchSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | Controls | Movement")
	float CrouchDuration = 1.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | Stats | Stamina")
	float Stamina = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | Stats | Stamina")
	float MaxStamina = 100.f;

	/** How much time in seconds can the player sprint */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | Stats | Stamina")
	float MaxSprintTime = 7.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | Stats | Stamina")
	float TimerTickRate = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | Stats | Stamina")
	float DelayStaminaRegen = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | Stats | Stamina")
	float TimeToRegenStamina = 5.f;

	// Dialogues --->
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | Dialogues")
	FString DialoguesPath = "/Game/00_Main/SFX/Sounds/Dialogues/episode_01/";

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | Dialogues")
	TArray<FString> DialogueList = { "Intro", "BackToCar", "CarReached", "NeedMoreRecord", "State_Extra1", "State2", "State3", "State4", "State4B", "State5", "State6", "State6B" };

	TMap<FString, FST_Dialogue> Dialogues;
	TArray<FString> DialoguesStack;
	bool bIsSpeaking = false;
	bool bAreDialoguesReady = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDanielState DanielState = EDanielState::State2;

	// Camera Shakes --->
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CustomProperties | CameraShakes")
	TSubclassOf<UCameraShakeBase> CS_Idle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CustomProperties | CameraShakes")
	TSubclassOf<UCameraShakeBase> CS_Walk;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CustomProperties | CameraShakes")
	TSubclassOf<UCameraShakeBase> CS_Run;

	// -------------------------------------------------------------------------
	
	// Enhanced Input --->
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomProperties | EnhancedInput")
	UInputMappingContext* IMC;

	UPROPERTY(EditDefaultsOnly, Category = "CustomProperties | EnhancedInput") UInputAction* IA_Look;
	UPROPERTY(EditDefaultsOnly, Category = "CustomProperties | EnhancedInput") UInputAction* IA_Move;
	UPROPERTY(EditDefaultsOnly, Category = "CustomProperties | EnhancedInput") UInputAction* IA_Sprint;
	UPROPERTY(EditDefaultsOnly, Category = "CustomProperties | EnhancedInput") UInputAction* IA_Jump;
	UPROPERTY(EditDefaultsOnly, Category = "CustomProperties | EnhancedInput") UInputAction* IA_Crouch;
	UPROPERTY(EditDefaultsOnly, Category = "CustomProperties | EnhancedInput") UInputAction* IA_LeanLeft;
	UPROPERTY(EditDefaultsOnly, Category = "CustomProperties | EnhancedInput") UInputAction* IA_LeanRight;
	UPROPERTY(EditDefaultsOnly, Category = "CustomProperties | EnhancedInput") UInputAction* IA_Zoom;
	UPROPERTY(EditDefaultsOnly, Category = "CustomProperties | EnhancedInput") UInputAction* IA_Flashlight;

	UFUNCTION() void InputLook(const FInputActionValue& Value);
	UFUNCTION() void InputMove(const FInputActionValue& Value);
	UFUNCTION() void InputSprint(const FInputActionValue& Value);
	UFUNCTION() void InputJump();
	UFUNCTION() void InputCrouch(const FInputActionValue& Value);
	UFUNCTION() void InputLeanLeft(const FInputActionValue& Value);
	UFUNCTION() void InputLeanRight(const FInputActionValue& Value);
	UFUNCTION() void InputZoom(const FInputActionValue& Value);
	UFUNCTION() void InputFlashlight(const FInputActionValue& Value);

	UFUNCTION() void StartStaminaConsumption();
	UFUNCTION() void StartStaminaRegen();

	void ChangeSprintState(const bool bInState);

	// -------------------------------------------------------------------------

	// HUD --->
	TSoftClassPtr<UCF_Widget_VHSOverlay> VHSOverlayClass;
	UCF_Widget_VHSOverlay* HUDOverlay;

	TSoftClassPtr<UUserWidget> VHSBlurClass;

	// Timelines --->
	UTimelineComponent* TL_Lean;
	FOnTimelineFloat TimelineLeanFloatFn{};
	UFUNCTION() void HandleTimelineLeanAlpha(float Alpha);

	UTimelineComponent* TL_Crouch;
	FOnTimelineFloat TimelineCrouchFloatFn;
	UFUNCTION() void HandleTimelineCrouchAlpha(float Alpha);
	UFUNCTION() void HandleTimelineCrouchRotYaw(float RotYaw);
	UFUNCTION() void HandleTimelineCrouchUpDown(float UpDown);

	UTimelineComponent* TL_Zoom;
	FOnTimelineFloat TimelineZoomFloatFn{};
	UFUNCTION() void HandleTimelineZoomAlpha(float Alpha);

	// Timer handles --->
	FTimerHandle TH_ConsumeStamina;
	FTimerHandle TH_RegenStamina;

	// -------------------------------------------------------------------------------

	void CalcLeanDirection();

	void ConsumeStamina();

	void RegenStamina();

	void UpdateMovementSpeed();

	bool CanUncrouch() const;

	TArray<USoundWave*> GetWavesInFolder(const FString& FolderName, const ELanguage& Language) const;

	void CheckBreathing();

	UFUNCTION(BlueprintCallable)
	void FlickerFlashlight(const bool bStart);

	void SetupDialogues();

	void SetupCharacterLeaning();

	void SetupHUD();

	void Headbob();

	//

	void ProcessCrouch();

	void StartBreathing();

public:	

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	template <typename T>
	static void Shuffle(TArray<T>& inArray)
	{
		const int32 lastIdx = inArray.Num() - 1;
		for (int32 i = 0; i < lastIdx; ++i)
		{
			const int32 SwapIdx = FMath::RandRange(i, lastIdx);
			if (i != SwapIdx)
				inArray.Swap(i, SwapIdx);
		}
	}

	static FTimerManager* GetTimerManager(UObject* inWCO)
	{
		if (!inWCO)
			return nullptr;

		auto* world = inWCO->GetWorld();
		if (!world)
			return nullptr;

		return &world->GetTimerManager();
	}

	static void PauseTimer(UObject* inWCO, FTimerHandle& TimerHandle)
	{
		if(FTimerManager* tm = GetTimerManager(inWCO))
			tm->PauseTimer(TimerHandle);
	}
	static void UnPauseTimer(UObject* inWCO, FTimerHandle& TimerHandle)
	{
		if (FTimerManager* tm = GetTimerManager(inWCO))
			tm->UnPauseTimer(TimerHandle);
	}
	static void ClearTimer(UObject* inWCO, FTimerHandle& TimerHandle)
	{
		if (FTimerManager* tm = GetTimerManager(inWCO))
			tm->ClearTimer(TimerHandle);
	}

	UFUNCTION(BlueprintCallable)
	void Kevin(float myFloat, const float myConstFloat, UObject* myRef, const UObject* myConstRef) {};
};
