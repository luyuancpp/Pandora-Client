// Copyright Pandora. All Rights Reserved.

#include "PandoraPlayerViewModel.h"

void UPandoraPlayerViewModel::SetHealth(float NewValue)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(Health, NewValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthText);
	}
}

void UPandoraPlayerViewModel::SetMaxHealth(float NewValue)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxHealth, NewValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthText);
	}
}

void UPandoraPlayerViewModel::SetCurrentAmmo(int32 NewValue)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(CurrentAmmo, NewValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetAmmoText);
	}
}

void UPandoraPlayerViewModel::SetMagazineSize(int32 NewValue)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MagazineSize, NewValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetAmmoText);
	}
}

float UPandoraPlayerViewModel::GetHealthPercent() const
{
	return MaxHealth > 0.f ? FMath::Clamp(Health / MaxHealth, 0.f, 1.f) : 0.f;
}

FText UPandoraPlayerViewModel::GetHealthText() const
{
	return FText::FromString(FString::Printf(TEXT("HP %d / %d"),
		FMath::RoundToInt(Health), FMath::RoundToInt(MaxHealth)));
}

FText UPandoraPlayerViewModel::GetAmmoText() const
{
	return FText::FromString(FString::Printf(TEXT("AMMO %d / %d"),
		CurrentAmmo, MagazineSize));
}