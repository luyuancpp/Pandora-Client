"""
Pandora - 给 Mannequin 加 WeaponSocket（M1.3）

在 UE 编辑器里执行:
  Tools -> Execute Python Script -> 选 Tools/AddWeaponSocket.py

走的路径:
  Python -> UPandoraSocketTools::AddWeaponSocketToMesh(C++ BlueprintCallable)
  -> 直接裸字段赋值 USkeletalMeshSocket（C++ 端没 BP 可写性检查）

为什么不直接在 Python 里构造 USkeletalMeshSocket:
  UE 5.7 的 USkeletalMeshSocket 所有字段（SocketName/BoneName/RelativeLocation/...）
  都是 BlueprintReadOnly + EditAnywhere。
  - set_editor_property → "is read-only and cannot be set"
  - 直接 attribute 赋值 → 同样被拒
  C++ 端裸字段赋值不走这套反射检查，是唯一可行路径。

依赖:
  Source/Pandora/Public/PandoraSocketTools.h（M1.3 引入）
  必须先编 Editor 才能在 Python 里调到 UPandoraSocketTools。

UE5 Manny 资产命名约定:
  SK_Mannequin       = USkeleton（骨架）
  SKM_Manny_Simple   = USkeletalMesh（蒙皮网格）
"""

import unreal


# ---------- 资产路径 ----------
SKELETAL_MESH_PATH = "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"
SOCKET_NAME = "WeaponSocket"
PARENT_BONE = "hand_r"

# Socket 在父骨骼坐标系下的相对 Transform
SOCKET_LOCATION = unreal.Vector(10.0, 4.0, -2.0)
SOCKET_ROTATION = unreal.Rotator(0.0, 0.0, 90.0)
SOCKET_SCALE    = unreal.Vector(1.0, 1.0, 1.0)


def main():
    print("=" * 60)
    print(f"[Pandora] 给 Mannequin 加 {SOCKET_NAME}")
    print("=" * 60)

    skm = unreal.EditorAssetLibrary.load_asset(SKELETAL_MESH_PATH)
    if skm is None:
        raise RuntimeError(f"找不到 {SKELETAL_MESH_PATH}")
    if not isinstance(skm, unreal.SkeletalMesh):
        raise RuntimeError(
            f"{SKELETAL_MESH_PATH} 不是 USkeletalMesh（type={type(skm).__name__}）"
        )
    print(f"[1/3] 加载 SkeletalMesh: {skm.get_path_name()}")

    skeleton = skm.skeleton
    if skeleton is None:
        raise RuntimeError("SKM_Manny_Simple 的 skeleton 字段为空？")
    print(f"      关联 Skeleton: {skeleton.get_path_name()}")

    # 走 C++ BlueprintCallable，绕�� Python 对 BlueprintReadOnly 字段的写禁止
    # 函数全名: UPandoraSocketTools::AddWeaponSocketToMesh
    # Python 暴露名: unreal.PandoraSocketTools.add_weapon_socket_to_mesh
    print(f"[2/3] 调用 C++ AddWeaponSocketToMesh")
    ok = unreal.PandoraSocketTools.add_weapon_socket_to_mesh(
        skeletal_mesh=skm,
        socket_name=SOCKET_NAME,
        parent_bone_name=PARENT_BONE,
        relative_location=SOCKET_LOCATION,
        relative_rotation=SOCKET_ROTATION,
        relative_scale=SOCKET_SCALE,
    )
    if not ok:
        raise RuntimeError("AddWeaponSocketToMesh 返回 false，看 LogTemp 错误")

    # 验证
    verify = skm.find_socket(SOCKET_NAME)
    if verify is None:
        raise RuntimeError("C++ 调用成功但 find_socket 仍找不到，可能 API 行为变化")
    outer_name = verify.get_outer().get_name()
    print(f"      [Verify] find_socket({SOCKET_NAME}) -> outer={outer_name}")

    # 保存
    print(f"[3/3] 保存 Mesh 和 Skeleton")
    unreal.EditorAssetLibrary.save_loaded_asset(skm)
    unreal.EditorAssetLibrary.save_loaded_asset(skeleton)

    print("\n" + "=" * 60)
    print(f"[Pandora] {SOCKET_NAME} 已加到 hand_r")
    print(f"  Location = {SOCKET_LOCATION}")
    print(f"  Rotation = {SOCKET_ROTATION}")
    print("接下来:")
    print("  1. 想可视化微调: 双击 SKM_Manny_Simple，左侧 Sockets 面板找 WeaponSocket")
    print("  2. 跑 CreateWeaponBP.py 建 BP_Weapon_AK")
    print("=" * 60)


main()