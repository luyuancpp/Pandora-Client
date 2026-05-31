// Copyright Xuanming. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "XuanmingPlayerViewModel.generated.h"

/**
 * 玩家 HUD 数据 ViewModel (MVVM 模式).
 *
 * 数据流: PlayerController::PlayerTick 把 Pawn/Weapon 状态推到这里
 *         -> Setter (UE_MVVM_SET_PROPERTY_VALUE 宏) 比较新旧值, 不同才广播
 *         -> WBP_PlayerHUD 通过 MVVM Bindings 自动刷 UI
 *
 * 性能优势 vs 函数 Binding: 函数 Binding 每帧 Tick 调用 (10 个 binding × 60fps = 600 次/秒);
 * MVVM 只在数据变化时广播, 静止时零开销.
 *
 * 派生属性 (HealthPercent / HealthText / AmmoText) 用 FieldNotifyDependencies 元数据
 * 让 MVVM 编译器自动追踪依赖, 依赖字段变化时自动重算.
 */
UCLASS(BlueprintType)
class XUANMING_API UXuanmingPlayerViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	// === 基础字段 (来自 Pawn/Weapon, 由 PC 喂数据) ===

	UFUNCTION(BlueprintPure, Category = "Xuanming|HUD")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, Category = "Xuanming|HUD")
	void SetHealth(float NewValue);

	UFUNCTION(BlueprintPure, Category = "Xuanming|HUD")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "Xuanming|HUD")
	void SetMaxHealth(float NewValue);

	UFUNCTION(BlueprintPure, Category = "Xuanming|HUD")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	UFUNCTION(BlueprintCallable, Category = "Xuanming|HUD")
	void SetCurrentAmmo(int32 NewValue);

	UFUNCTION(BlueprintPure, Category = "Xuanming|HUD")
	int32 GetMagazineSize() const { return MagazineSize; }

	UFUNCTION(BlueprintCallable, Category = "Xuanming|HUD")
	void SetMagazineSize(int32 NewValue);

	// === 派生属性 (UI 直接绑定这些, MVVM 跟踪依赖自动重算) ===

	/** 0..1 范围的血量比例, 给 ProgressBar.Percent 绑 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Xuanming|HUD",
		meta = (FieldNotifyDependencies = "Health,MaxHealth"))
	float GetHealthPercent() const;

	/** "HP 100 / 100" 格式, 给 TextBlock.Text 绑 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Xuanming|HUD",
		meta = (FieldNotifyDependencies = "Health,MaxHealth"))
	FText GetHealthText() const;

	/** "AMMO 30 / 30" 格式, 给 TextBlock.Text 绑 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Xuanming|HUD",
		meta = (FieldNotifyDependencies = "CurrentAmmo,MagazineSize"))
	FText GetAmmoText() const;

private:
	// 字段需要 FieldNotify 才能让 MVVM 编译器识别 + 派生属性追踪它的变化
	// Setter/Getter 元数据让 BlueprintReadWrite 走我们的实现 (内部用 UE_MVVM_SET_PROPERTY_VALUE)
	UPROPERTY(BlueprintGetter = GetHealth, BlueprintSetter = SetHealth, FieldNotify,
		Category = "Xuanming|HUD")
	float Health = 100.f;

	UPROPERTY(BlueprintGetter = GetMaxHealth, BlueprintSetter = SetMaxHealth, FieldNotify,
		Category = "Xuanming|HUD")
	float MaxHealth = 100.f;

	UPROPERTY(BlueprintGetter = GetCurrentAmmo, BlueprintSetter = SetCurrentAmmo, FieldNotify,
		Category = "Xuanming|HUD")
	int32 CurrentAmmo = 30;

	UPROPERTY(BlueprintGetter = GetMagazineSize, BlueprintSetter = SetMagazineSize, FieldNotify,
		Category = "Xuanming|HUD")
	int32 MagazineSize = 30;
};
