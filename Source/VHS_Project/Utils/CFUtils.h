#pragma once

#include "CoreMinimal.h"

template< typename UserClass, class Func >
inline bool SetupTimerSwitch(UserClass* InUserObject, FTimerHandle& TH_HandleToStart, float TickRate, Func&& InFunc, FTimerHandle* TH_HandleToStop = nullptr, bool bLoop = false, float Delay = -1.f)
{
	if (!IsValid(InUserObject))
		return false;

	auto* world = InUserObject->GetWorld();
	if (!world)
		return false;

	if (TH_HandleToStop)
		world->GetTimerManager().ClearTimer(*TH_HandleToStop);

	FTimerDelegate TD_Bind;
	TD_Bind.BindWeakLambda(InUserObject, std::bind(InFunc, InUserObject));
	world->GetTimerManager().SetTimer(TH_HandleToStart, TD_Bind, TickRate, bLoop, Delay);
	return true;
}

inline void PlaySFX(UObject* WCO, USoundBase* inSFX)
{
	if (IsValid(inSFX))
		UGameplayStatics::PlaySound2D(WCO, inSFX);
}

inline void PlaySFX(UObject* WCO, USoundBase* inSFX, const FVector& Location)
{
	if (IsValid(inSFX))
		UGameplayStatics::PlaySoundAtLocation(WCO, inSFX, Location);
}