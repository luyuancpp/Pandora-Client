// Copyright Xuanming. All Rights Reserved.

#include "XuanmingSocketTools.h"

#if WITH_EDITOR
#include "Engine/SkeletalMesh.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Animation/Skeleton.h"
#include "ReferenceSkeleton.h"

bool UXuanmingSocketTools::AddWeaponSocketToMesh(
	USkeletalMesh* SkeletalMesh,
	FName SocketName,
	FName ParentBoneName,
	FVector RelativeLocation,
	FRotator RelativeRotation,
	FVector RelativeScale)
{
	if (!SkeletalMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("[XuanmingSocketTools] SkeletalMesh is null"));
		return false;
	}
	if (SocketName.IsNone() || ParentBoneName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[XuanmingSocketTools] SocketName/ParentBoneName cannot be None"));
		return false;
	}

	USkeleton* Skeleton = SkeletalMesh->GetSkeleton();
	if (!Skeleton)
	{
		UE_LOG(LogTemp, Error, TEXT("[XuanmingSocketTools] %s has no Skeleton"),
			*SkeletalMesh->GetName());
		return false;
	}

	// 校验骨骼存在
	const FReferenceSkeleton& RefSkel = SkeletalMesh->GetRefSkeleton();
	const int32 BoneIndex = RefSkel.FindBoneIndex(ParentBoneName);
	if (BoneIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Error, TEXT("[XuanmingSocketTools] Bone '%s' not found in %s"),
			*ParentBoneName.ToString(), *SkeletalMesh->GetName());
		return false;
	}

	// 幂等：Mesh 或 Skeleton 上已经有同名 socket 就直接返回
	if (USkeletalMeshSocket* Existing = SkeletalMesh->FindSocket(SocketName))
	{
		UE_LOG(LogTemp, Display, TEXT("[XuanmingSocketTools] Socket '%s' already exists on '%s' (idempotent)"),
			*SocketName.ToString(), *Existing->GetOuter()->GetName());
		return true;
	}

	// 构造 socket，outer = Skeleton（这样 AddSocket(bAddToSkeleton=true) 写到 Skeleton 不会复制）
	USkeletalMeshSocket* Socket = NewObject<USkeletalMeshSocket>(Skeleton);
	if (!Socket)
	{
		UE_LOG(LogTemp, Error, TEXT("[XuanmingSocketTools] NewObject<USkeletalMeshSocket> failed"));
		return false;
	}

	// 直接裸字段赋值——C++ 端没有 BP 可写性检查
	Socket->SocketName = SocketName;
	Socket->BoneName = ParentBoneName;
	Socket->RelativeLocation = RelativeLocation;
	Socket->RelativeRotation = RelativeRotation;
	Socket->RelativeScale = RelativeScale;

	// 写入 Mesh + Skeleton
	SkeletalMesh->AddSocket(Socket, /*bAddToSkeleton=*/ true);

	// 标 dirty 让保存机制知道有改动
	SkeletalMesh->MarkPackageDirty();
	Skeleton->MarkPackageDirty();

	UE_LOG(LogTemp, Display, TEXT("[XuanmingSocketTools] Added socket '%s' on bone '%s' to %s (mirror in Skeleton %s)"),
		*SocketName.ToString(), *ParentBoneName.ToString(),
		*SkeletalMesh->GetName(), *Skeleton->GetName());
	return true;
}

#endif // WITH_EDITOR
