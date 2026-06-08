// Copyright Pandora. All Rights Reserved.

#include "PandoraSocketTools.h"

#if WITH_EDITOR
#include "Engine/SkeletalMesh.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Animation/Skeleton.h"
#include "ReferenceSkeleton.h"

bool UPandoraSocketTools::AddWeaponSocketToMesh(
	USkeletalMesh* SkeletalMesh,
	FName SocketName,
	FName ParentBoneName,
	FVector RelativeLocation,
	FRotator RelativeRotation,
	FVector RelativeScale)
{
	if (!SkeletalMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("[PandoraSocketTools] SkeletalMesh is null"));
		return false;
	}
	if (SocketName.IsNone() || ParentBoneName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[PandoraSocketTools] SocketName/ParentBoneName cannot be None"));
		return false;
	}

	USkeleton* Skeleton = SkeletalMesh->GetSkeleton();
	if (!Skeleton)
	{
		UE_LOG(LogTemp, Error, TEXT("[PandoraSocketTools] %s has no Skeleton"),
			*SkeletalMesh->GetName());
		return false;
	}

	// 校验骨骼存在
	const FReferenceSkeleton& RefSkel = SkeletalMesh->GetRefSkeleton();
	const int32 BoneIndex = RefSkel.FindBoneIndex(ParentBoneName);
	if (BoneIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Error, TEXT("[PandoraSocketTools] Bone '%s' not found in %s"),
			*ParentBoneName.ToString(), *SkeletalMesh->GetName());
		return false;
	}

	// 幂等：Mesh 或 Skeleton 上已经有同名 socket 就直接返回
	if (USkeletalMeshSocket* Existing = SkeletalMesh->FindSocket(SocketName))
	{
		UE_LOG(LogTemp, Display, TEXT("[PandoraSocketTools] Socket '%s' already exists on '%s' (idempotent)"),
			*SocketName.ToString(), *Existing->GetOuter()->GetName());
		return true;
	}

	// 构造 socket，outer 必须是 SkeletalMesh —— UE 5.7 的 AddSocket 严格检查
	// （早期版本可以用 Skeleton 当 outer，5.7 加了硬约束）
	// 即使 outer 是 Mesh，传 bAddToSkeleton=true 时 AddSocket 内部会复制一份到 Skeleton
	USkeletalMeshSocket* Socket = NewObject<USkeletalMeshSocket>(SkeletalMesh);
	if (!Socket)
	{
		UE_LOG(LogTemp, Error, TEXT("[PandoraSocketTools] NewObject<USkeletalMeshSocket> failed"));
		return false;
	}

	// 直接裸字段赋值——C++ 端没有 BP 可写性检查
	Socket->SocketName = SocketName;
	Socket->BoneName = ParentBoneName;
	Socket->RelativeLocation = RelativeLocation;
	Socket->RelativeRotation = RelativeRotation;
	Socket->RelativeScale = RelativeScale;

	// 写入 Mesh + Skeleton。检查返回值（5.7 AddSocket 会拒绝 outer 错的 socket）
	const int32 NumBefore = SkeletalMesh->NumSockets();
	SkeletalMesh->AddSocket(Socket, /*bAddToSkeleton=*/ true);
	const int32 NumAfter = SkeletalMesh->NumSockets();

	if (NumAfter <= NumBefore)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[PandoraSocketTools] AddSocket 调用没增加 socket 数量（before=%d, after=%d）。看上面 LogSkeletalMesh 错误"),
			NumBefore, NumAfter);
		return false;
	}

	// 标 dirty 让保存机制知道有改动
	SkeletalMesh->MarkPackageDirty();
	Skeleton->MarkPackageDirty();

	UE_LOG(LogTemp, Display, TEXT("[PandoraSocketTools] Added socket '%s' on bone '%s' to %s (mirror in Skeleton %s)"),
		*SocketName.ToString(), *ParentBoneName.ToString(),
		*SkeletalMesh->GetName(), *Skeleton->GetName());
	return true;
}

#endif // WITH_EDITOR