#include "CF_Player.h"


// # Engine Includes
#include "Components/AudioComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ChildActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Engine/LocalPlayer.h"
#include "Materials/MaterialInterface.h"

// # Project Includes
#include "Flashlight.h"
#include "UI/CF_Widget_VHSOverlay.h"
#include "Utils/CFUtils.h"


ACF_Player::ACF_Player()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create Components
	Audio = CreateDefaultSubobject<UAudioComponent>("Audio");
	SpringLeaning = CreateDefaultSubobject<USpringArmComponent>("SpringLeaning");
	SpringCamera = CreateDefaultSubobject<USpringArmComponent>("SpringCamera");
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>("FPCamera");

	CA_Flashlight = CreateDefaultSubobject<UChildActorComponent>("CAFlashlight");
	CA_Flashlight->SetChildActorClass(AFlashlight::StaticClass());

	// Set Root Component
	SetRootComponent(GetCapsuleComponent());

	// Set Attachments
	Audio->SetupAttachment(GetRootComponent());
	SpringLeaning->SetupAttachment(GetRootComponent());
	SpringCamera->SetupAttachment(SpringLeaning);
	FirstPersonCamera->SetupAttachment(SpringCamera);
	CA_Flashlight->SetupAttachment(SpringCamera);
	GetMesh()->SetupAttachment(GetRootComponent());
	GetArrowComponent()->SetupAttachment(GetRootComponent());

	// Set default values for Comnponents
	GetCapsuleComponent()->SetCapsuleRadius(25.f);
	GetCapsuleComponent()->SetCapsuleHalfHeight(HalfHeightStanding);

	Audio->SetRelativeLocation(FVector(0, 0, 60));

	SpringLeaning->SetRelativeLocation(FVector(0, 0, 30));
	SpringLeaning->SetRelativeRotation(FRotator(-90, 0, 0));
	SpringLeaning->TargetArmLength = 30.f;

	SpringCamera->TargetArmLength = 0.f;
	SpringCamera->bUsePawnControlRotation = true;
	SpringCamera->bEnableCameraLag = true;
	SpringCamera->bEnableCameraRotationLag = true;
	SpringCamera->CameraLagSpeed = 8.f;
	SpringCamera->CameraRotationLagSpeed = 12.f;

	FirstPersonCamera->SetFieldOfView(65.f);
	FirstPersonCamera->SetAspectRatioAxisConstraint(EAspectRatioAxisConstraint::AspectRatio_MAX);
	FirstPersonCamera->bOverrideAspectRatioAxisConstraint = true;
	FirstPersonCamera->bUseFieldOfViewForLOD = false;

	// Initialize Timelines
	TL_Crouch = CreateDefaultSubobject<UTimelineComponent>("TL_Crouch");
	TL_Lean = CreateDefaultSubobject<UTimelineComponent>("TL_Lean");
	TL_Zoom = CreateDefaultSubobject<UTimelineComponent>("TL_Zoom");
}

void ACF_Player::BeginPlay()
{
	Super::BeginPlay();

	// Camera Postprocess materials
	TArray<FWeightedBlendable> blendables;
	for (auto* material : PostprocessMaterials)
	{
		FWeightedBlendable blendable;
		blendable.Object = material;
		blendable.Weight = 1.f;	
	}
	FirstPersonCamera->PostProcessSettings.WeightedBlendables = blendables;

	// Timelines setup
	// - Timeline CROUCH
	TimelineCrouchFloatFn.BindUFunction(this, FName("HandleTimelineCrouchAlpha"));
	TimelineCrouchFloatFn.BindUFunction(this, FName("HandleTimelineCrouchRotYaw"));
	TimelineCrouchFloatFn.BindUFunction(this, FName("HandleTimelineCrouchUpDown"));

	UCurveFloat* CrouchCurveAlpha = NewObject<UCurveFloat>(this, UCurveFloat::StaticClass(), TEXT("FloatCurve1"));
	FKeyHandle KH_CrouchAlphaFirst;
	CrouchCurveAlpha->FloatCurve.SetKeyInterpMode(KH_CrouchAlphaFirst, ERichCurveInterpMode::RCIM_Cubic);

	CrouchCurveAlpha->FloatCurve.AddKey(0.f, 0.f, false, KH_CrouchAlphaFirst);
	CrouchCurveAlpha->FloatCurve.AddKey(1.f, 1.f);
	TL_Crouch->AddInterpFloat(CrouchCurveAlpha, TimelineCrouchFloatFn, FName("Alpha"));

	UCurveFloat* CrouchCurveRotYaw = NewObject<UCurveFloat>(this, UCurveFloat::StaticClass(), TEXT("FloatCurve2"));
	FKeyHandle KH_CrouchRotYawFirst;
	FKeyHandle KH_CrouchRotYawSecond;
	FKeyHandle KH_CrouchRotYawThird;
	CrouchCurveRotYaw->FloatCurve.SetKeyInterpMode(KH_CrouchRotYawFirst, ERichCurveInterpMode::RCIM_Cubic);
	CrouchCurveRotYaw->FloatCurve.SetKeyInterpMode(KH_CrouchRotYawSecond, ERichCurveInterpMode::RCIM_Cubic);
	CrouchCurveRotYaw->FloatCurve.SetKeyInterpMode(KH_CrouchRotYawThird, ERichCurveInterpMode::RCIM_Cubic);

	CrouchCurveRotYaw->FloatCurve.AddKey(0.f, 0.f, false, KH_CrouchRotYawFirst);
	CrouchCurveRotYaw->FloatCurve.AddKey(.25f, -1.f, false, KH_CrouchRotYawFirst);
	CrouchCurveRotYaw->FloatCurve.AddKey(.75f, 1.f, false, KH_CrouchRotYawFirst);
	CrouchCurveRotYaw->FloatCurve.AddKey(1.f, 0.f);
	
	UCurveFloat* CrouchCurveUpDown = NewObject<UCurveFloat>(this, UCurveFloat::StaticClass(), TEXT("FloatCurve3"));
	FKeyHandle KH_CrouchUpDownFirst;
	FKeyHandle KH_CrouchUpDownSecond;
	CrouchCurveUpDown->FloatCurve.SetKeyInterpMode(KH_CrouchUpDownFirst, ERichCurveInterpMode::RCIM_Cubic);
	CrouchCurveUpDown->FloatCurve.SetKeyInterpMode(KH_CrouchUpDownSecond, ERichCurveInterpMode::RCIM_Cubic);

	CrouchCurveUpDown->FloatCurve.AddKey(0.f, 0.f, false, KH_CrouchUpDownFirst);
	CrouchCurveUpDown->FloatCurve.AddKey(.5f, -7.54f, false, KH_CrouchUpDownSecond);
	CrouchCurveUpDown->FloatCurve.AddKey(1.f, 0.f);

	TL_Crouch->AddInterpFloat(CrouchCurveAlpha, TimelineCrouchFloatFn, FName("Alpha"));
	TL_Crouch->AddInterpFloat(CrouchCurveRotYaw, TimelineCrouchFloatFn, FName("RotYaw"));
	TL_Crouch->AddInterpFloat(CrouchCurveUpDown, TimelineCrouchFloatFn, FName("UpDown"));

	// - Timeline LEAN
	TimelineLeanFloatFn.BindUFunction(this, FName("HandleTimelineLeanAlpha"));

	UCurveFloat* LeanCurve = NewObject<UCurveFloat>(this, UCurveFloat::StaticClass(), TEXT("FloatCurve4"));
	LeanCurve->FloatCurve.AddKey(0.f, 0.f);
	LeanCurve->FloatCurve.AddKey(1.f, 1.f);

	TL_Lean->AddInterpFloat(LeanCurve, TimelineLeanFloatFn, FName("Alpha"));

	// - Timeline ZOOM
	TimelineZoomFloatFn.BindUFunction(this, FName("HandleTimelineZoomAlpha"));

	UCurveFloat* ZoomCurve = NewObject<UCurveFloat>(this, UCurveFloat::StaticClass(), TEXT("FloatCurve5"));
	ZoomCurve->FloatCurve.AddKey(0.f, 0.f);
	ZoomCurve->FloatCurve.AddKey(1.f, 1.f);

	TL_Zoom->AddInterpFloat(ZoomCurve, TimelineZoomFloatFn, FName("Alpha"));

	// Enhanced Inputs setup
	if (auto* pc = Cast<APlayerController>(GetController()))
	{
		auto* localPlayer = pc->GetLocalPlayer();
		if (UEnhancedInputLocalPlayerSubsystem* InputSybsystem = localPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			InputSybsystem->AddMappingContext(IMC, 0);
		}
	}

	// Setups
	SetupHUD();
	SetupCharacterLeaning();
	SetupDialogues();

	TL_Crouch->SetPlayRate(1.f / CrouchDuration);
	CameraRotation = FirstPersonCamera->GetRelativeRotation();
	StartBreathing();

	// Flashlight Child Actor
	if (AActor* ChildActor = CA_Flashlight->GetChildActor())
		Flashlight = Cast<AFlashlight>(ChildActor);
}

void ACF_Player::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Headbob();
	CheckBreathing();
}

void ACF_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ACF_Player::InputLook);
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ACF_Player::InputMove);

		EnhancedInputComponent->BindAction(IA_Sprint, ETriggerEvent::Triggered, this, &ACF_Player::InputSprint);

		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &ACF_Player::InputJump);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ACF_Player::StopJumping);

		EnhancedInputComponent->BindAction(IA_Crouch, ETriggerEvent::Triggered, this, &ACF_Player::InputCrouch);

		EnhancedInputComponent->BindAction(IA_LeanLeft, ETriggerEvent::Triggered, this, &ACF_Player::InputLeanLeft);
		EnhancedInputComponent->BindAction(IA_LeanRight, ETriggerEvent::Triggered, this, &ACF_Player::InputLeanRight);
		
		EnhancedInputComponent->BindAction(IA_Zoom, ETriggerEvent::Triggered, this, &ACF_Player::InputZoom);
		EnhancedInputComponent->BindAction(IA_Flashlight, ETriggerEvent::Triggered, this, &ACF_Player::InputFlashlight);
	}
}

void ACF_Player::Headbob()
{
	auto* pc = GetController<APlayerController>();
	if (!pc)
		return;

	const float Speed = GetVelocity().Length();
	TSubclassOf<UCameraShakeBase> camShake = !(Speed > 0 && CanJump()) ? CS_Idle : (Speed < SprintSpeed) ? CS_Walk : CS_Run;

	if(camShake)
		pc->ClientStartCameraShake(camShake, 1.f, ECameraShakePlaySpace::CameraLocal);
}

void ACF_Player::ProcessCrouch()
{
	if (bIsCrouching)
		if (!CanUncrouch())
			return;

	bIsCrouching = !bIsCrouching;

	UpdateMovementSpeed();
}

void ACF_Player::StartBreathing()
{
	Audio->SetIntParameter(FName("IsSpeaking"), 0);
	Audio->SetIntParameter(FName("Breathing"), 0);
	Audio->Play();
}

// -----------------------------------------------------------------------------

void ACF_Player::CalcLeanDirection()
{
	if (!TL_Lean)
		return;

	LeanStart = LeanDirection * TL_Lean->GetPlaybackPosition();
	LeanDirection = -static_cast<int8>(bIsLeaningLeft) + static_cast<int8>(bIsLeaningRight);

	TL_Lean->PlayFromStart();
}

void ACF_Player::ConsumeStamina()
{
	Stamina -= MaxStamina / (MaxSprintTime * TimerTickRate * 100);
	Stamina = FMath::Clamp(Stamina, 0, MaxStamina);

	if (Stamina == 0.f)
	{
		PauseTimer(this, TH_ConsumeStamina);

		bIsSprinting = false;

		UpdateMovementSpeed();
	}
}

void ACF_Player::RegenStamina()
{
	Stamina += MaxStamina / (TimeToRegenStamina * TimerTickRate * 100.f);
	Stamina = FMath::Clamp(Stamina, 0, MaxStamina);

	if (Stamina == MaxStamina)
		ClearTimer(this, TH_RegenStamina);
}

void ACF_Player::UpdateMovementSpeed()
{
	auto* CMC = GetCharacterMovement();

	const float newMaxSpeed = (bIsSprinting && bIsCrouching) ? FastCrouchSpeed : bIsSprinting ? SprintSpeed : bIsCrouching ? CrouchSpeed : WalkSpeed;
	CMC->MaxWalkSpeed = newMaxSpeed;

	if (!TL_Crouch)
		return;

	if (bIsCrouching)
		TL_Crouch->Play();
	else
		TL_Crouch->Reverse();
}

void ACF_Player::HandleTimelineZoomAlpha(float Alpha)
{
	const float FOV = FMath::Lerp(65.f, 30.f, Alpha);
	FirstPersonCamera->SetFieldOfView(FOV);

	if(HUDOverlay)
		HUDOverlay->UpdateZoom(FMath::RoundToInt(Alpha * 2 + 2));
}

void ACF_Player::InputLook(const FInputActionValue& Value)
{
	FVector2D Axis = Value.Get<FVector2D>();

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ACF_Player::InputMove(const FInputActionValue& Value)
{
	FVector2D Axis = Value.Get<FVector2D>();

	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
}

void ACF_Player::InputSprint(const FInputActionValue& Value)
{
	bool active = Value.Get<bool>();

	if (!active)
	{
		ChangeSprintState(false);
	}
	else if (Stamina > 0)
	{
		ChangeSprintState(true);
	}
}

void ACF_Player::InputCrouch(const FInputActionValue& Value)
{
	if(Value.Get<bool>())
		ProcessCrouch();
}

void ACF_Player::InputLeanLeft(const FInputActionValue& Value)
{
	bIsLeaningLeft = Value.Get<bool>();

	CalcLeanDirection();
}
void ACF_Player::InputLeanRight(const FInputActionValue& Value)
{
	bIsLeaningRight = Value.Get<bool>();

	CalcLeanDirection();
}

void ACF_Player::InputZoom(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
		TL_Zoom->Play();
	else
		TL_Zoom->Reverse();
}

void ACF_Player::InputFlashlight(const FInputActionValue& Value)
{
	Flashlight->ToggleFlashlight();
	PlaySFX(this, SFX_Flashlight, Flashlight->GetActorLocation());
}

void ACF_Player::StartStaminaConsumption()
{
	SetupTimerSwitch(this, TH_ConsumeStamina, TimerTickRate, &ACF_Player::ConsumeStamina, &TH_RegenStamina);
}

void ACF_Player::StartStaminaRegen()
{
	SetupTimerSwitch(this, TH_RegenStamina, TimerTickRate, &ACF_Player::RegenStamina, &TH_ConsumeStamina);
}

void ACF_Player::ChangeSprintState(const bool bInState)
{
	if (bInState == bIsSprinting)
		return;

	bIsSprinting = bInState;
	if (bIsSprinting)
		StartStaminaConsumption();
	else
		StartStaminaRegen();

	UpdateMovementSpeed();
}

void ACF_Player::InputJump()
{
	if (bIsCrouching)
		ProcessCrouch();
	else
		Jump();
}

void ACF_Player::HandleTimelineLeanAlpha(float Alpha)
{
	float angle = FMath::Lerp(LeanStart, LeanDirection, Alpha);
	float angleLength = angle * LeanDistance;

	SpringLeaning->SocketOffset = FVector(0, angleLength, 0);
	SpringLeaning->TargetArmLength = 30.f - (angle * 10);

	CameraRotation.Roll = angleLength * .1f;
	FirstPersonCamera->SetRelativeRotation(CameraRotation);
}

void ACF_Player::HandleTimelineCrouchAlpha(float Alpha)
{
	GetCapsuleComponent()->SetCapsuleHalfHeight(FMath::Lerp(HalfHeightStanding, HalfHeightCrouch, Alpha));
}
void ACF_Player::HandleTimelineCrouchRotYaw(float RotYaw)
{
	CameraRotation.Yaw = RotYaw;
	FirstPersonCamera->SetRelativeRotation(CameraRotation);
}
void ACF_Player::HandleTimelineCrouchUpDown(float UpDown)
{
	CameraRotation.Pitch = UpDown;
	FirstPersonCamera->SetRelativeRotation(CameraRotation);
}

bool ACF_Player::CanUncrouch() const
{
	const FVector location = GetActorLocation() + FVector(0, 0, HalfHeightStanding - GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	const float radius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	FHitResult hit;

	UKismetSystemLibrary::CapsuleTraceSingle(this, location, location, radius, HalfHeightStanding, ETraceTypeQuery::TraceTypeQuery1, false, {}, EDrawDebugTrace::None, hit, true);

	return !hit.bBlockingHit;
}


TArray<USoundWave*> ACF_Player::GetWavesInFolder(const FString& FolderName, const ELanguage& Language) const
{
	TArray<USoundWave*> waves;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	const FString fullPath = FString::Printf(TEXT("%s%s/%s"), *DialoguesPath, *UEnum::GetValueAsString<ELanguage>(Language), *FolderName);
	TArray<FAssetData> data;
	AssetRegistry.GetAssetsByPath(FName(*fullPath), data, false, false);

	for (const auto asset : data)
	{
		if (auto* wave = Cast<USoundWave>(asset.GetAsset()))
			waves.Add(wave);
	}

	Shuffle(waves);

	return waves;
}

void ACF_Player::CheckBreathing()
{
	const FVector velocity = GetVelocity();
	const float speed = FVector2D(velocity.X, velocity.Y).Length();

	const float volumeMultiplier = bIsSprinting ? 1.f : FMath::Clamp((speed / 2) / SprintSpeed, 0, 1);

	Audio->SetVolumeMultiplier(volumeMultiplier);
}

void ACF_Player::FlickerFlashlight(const bool bStart)
{
	if (bStart)
		Flashlight->StartFlickering();
	else
		Flashlight->StopFlickering();
}

void ACF_Player::SetupDialogues()
{
	for (auto& folder : DialogueList)
	{
		Dialogues.Add(folder, FST_Dialogue(GetWavesInFolder(folder, ELanguage::es), GetWavesInFolder(folder, ELanguage::en)));
	}

	bAreDialoguesReady = true;
	OnDialoguesReady.Broadcast();
}

void ACF_Player::SetupCharacterLeaning()
{
	if(TL_Lean)
		TL_Lean->SetPlayRate(1.f / LeanDuration);

	UpdateMovementSpeed();
}

void ACF_Player::SetupHUD()
{
	auto* pc = GetController<APlayerController>();
	if (!pc)
		return;

	if (VHSOverlayClass)
	{
		HUDOverlay = CreateWidget<UCF_Widget_VHSOverlay>(pc, VHSOverlayClass->StaticClass());
		if(HUDOverlay)
			HUDOverlay->AddToViewport();
	}

	if (VHSBlurClass)
	{
		UUserWidget* blur = CreateWidget<UUserWidget>(pc, VHSBlurClass->StaticClass());
		if(blur)
			blur->AddToViewport();
	}
}
