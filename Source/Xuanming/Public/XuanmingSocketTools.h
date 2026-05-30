// Copyright Xuanming. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XuanmingSocketTools.generated.h"

class USkeletalMesh;

/**
 * 玄冥 Editor 专用：骨架/Socket 写入工具
 *
 * 为什么需要这个类:
 *   UE 5.7 Python API 对 USkeletalMeshSocket 所有字段（SocketName/BoneName/RelativeLocation...）
 *   都是 BlueprintReadOnly + EditAnywhere。
 *   - set_editor_property 走 BP 可写性检查 → "is read-only and cannot be set"
 *   - 直接 attribute 赋值（socket.socket_name = ...）也走同一检查 → 同样被拒
 *
 *   C++ 端没有这层反射拦截，可以直接读写裸字段。把这种"基础设施级"操作
 *   封一个 BlueprintCallable static 给 Python 调即可。
 *
 * 仅在编辑器存在；Server/Client 编译时此类完全不参与（WITH_EDITOR 包裹）。
 */
UCLASS()
class XUANMING_API UXuanmingSocketTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	/**
	 * 在 SkeletalMesh 上加一个 Socket，并同步写到关联的 Skeleton。
	 *
	 * @param SkeletalMesh   目标蒙皮网格（不能为空）
	 * @param SocketName     Socket 名（项目里固定 "WeaponSocket"）
	 * @param ParentBoneName 父骨骼名（Manny 持枪手 = "hand_r"）
	 * @param RelativeLocation/Rotation/Scale  Socket 相对父骨骼的 transform
	 * @return 成功创建或已存在 → true；输入非法 → false
	 *
	 * 幂等：如果 SocketName 已经在 Mesh 或 Skeleton 上存在，不重复添加，直接返回 true。
	 * 调用方需要自己保存 Mesh 和 Skeleton（调 EditorAssetLibrary.save_loaded_asset）。
	 */
	UFUNCTION(BlueprintCallable, Category = "Xuanming|Editor",
		meta = (DisplayName = "Add Weapon Socket To Skeletal Mesh"))
	static bool AddWeaponSocketToMesh(
		USkeletalMesh* SkeletalMesh,
		FName SocketName,
		FName ParentBoneName,
		FVector RelativeLocation,
		FRotator RelativeRotation,
		FVector RelativeScale);
#endif // WITH_EDITOR
};
