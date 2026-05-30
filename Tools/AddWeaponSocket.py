"""
Xuanming - 给 Mannequin 加 WeaponSocket（M1.3）

在 UE 编辑器里执行:
  Tools -> Execute Python Script -> 选 Tools/AddWeaponSocket.py

做的事:
  1. 加载 SKM_Manny_Simple（USkeletalMesh）
  2. 检查 Mesh / Skeleton 上是否已经存在 WeaponSocket（幂等：有就跳过）
  3. 构造 USkeletalMeshSocket（outer=Skeleton），父骨骼=hand_r
  4. 调 SkeletalMesh.add_socket(socket, b_add_to_skeleton=True)
       → socket 同时进入 Mesh 和 Skeleton 的 sockets 数组
       → 之后所有共享 SK_Mannequin 骨架的 Mesh 都能用这个 socket
  5. 保存 Mesh 和 Skeleton

为什么走 USkeletalMesh.add_socket 而不是 USkeleton.sockets:
  - USkeleton.Sockets 是 UPROPERTY() 但没 BlueprintReadOnly/Write，Python 反射读不到
  - USkeletalMesh.AddSocket() 是 BlueprintCallable，Python 调得到
  - bAddToSkeleton=True 让 socket 同时进 Skeleton.sockets[]，效果等价

C++ 端 SpawnDefaultWeapon() 写死了 AttachToComponent(GetMesh(), ..., "WeaponSocket")，
所以名字必须就是 "WeaponSocket"。

UE5 Manny 资产命名约定:
  SK_Mannequin       = USkeleton（骨架）
  SKM_Manny_Simple   = USkeletalMesh（蒙皮网格）

经验偏移（之后可在 Editor 里 Skeleton/Mesh 编辑器微调）:
  Location  = (X=10, Y=4, Z=-2)   往手心前方一点、稍微往拇指侧偏
  Rotation  = (Roll=0, Pitch=0, Yaw=90)  让枪管朝前（取决于武器 mesh 的 +X 方向）
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
    print(f"[Xuanming] 给 Mannequin 加 {SOCKET_NAME}")
    print("=" * 60)

    skm = unreal.EditorAssetLibrary.load_asset(SKELETAL_MESH_PATH)
    if skm is None:
        raise RuntimeError(f"找不到 {SKELETAL_MESH_PATH}")
    if not isinstance(skm, unreal.SkeletalMesh):
        raise RuntimeError(
            f"{SKELETAL_MESH_PATH} 不是 USkeletalMesh（type={type(skm).__name__}）"
        )
    print(f"[1/5] 加载 SkeletalMesh: {skm.get_path_name()}")

    skeleton = skm.skeleton
    if skeleton is None:
        raise RuntimeError("SKM_Manny_Simple 的 skeleton 字段为空？")
    print(f"      关联 Skeleton: {skeleton.get_path_name()}")

    # 幂等检查：USkeletalMesh.find_socket 会同时查 Mesh 和 Skeleton 的 sockets
    print(f"[2/5] 幂等��查")
    existing = skm.find_socket(SOCKET_NAME)
    if existing is not None:
        print(f"      已存在 {SOCKET_NAME}（来源={existing.get_outer().get_name()}），跳过")
        return
    print(f"      未找到 {SOCKET_NAME}，继续创建")

    # 校验 hand_r 骨骼存在（不存在则不要污染数据）
    print(f"[3/5] 校验骨骼 {PARENT_BONE} 存在")
    # USkeleton.get_reference_pose 或 bone names；走 SkeletalMesh.find_bone_index 更直接
    bone_index = skm.find_bone_index(PARENT_BONE)
    if bone_index < 0:
        raise RuntimeError(
            f"SKM_Manny_Simple 找不到骨骼 '{PARENT_BONE}'，"
            f"可能 Manny 骨骼命名变了，需要打开 Skeleton 编辑器人工核对"
        )
    print(f"      bone_index = {bone_index}")

    # 构造 socket，outer 必须是 Skeleton（不然 add_socket(bAddToSkeleton=True) 会复制一份新的）
    print(f"[4/5] 创建 {SOCKET_NAME} 挂到 {PARENT_BONE}")
    socket = unreal.SkeletalMeshSocket(skeleton)
    socket.set_editor_property("socket_name", SOCKET_NAME)
    socket.set_editor_property("bone_name", PARENT_BONE)
    socket.set_editor_property("relative_location", SOCKET_LOCATION)
    socket.set_editor_property("relative_rotation", SOCKET_ROTATION)
    socket.set_editor_property("relative_scale", SOCKET_SCALE)

    # 加进 Mesh，b_add_to_skeleton=True 同时写到 Skeleton
    # 这样所有共享 SK_Mannequin 的 mesh（SKM_Quinn_Simple 等）也能用
    skm.add_socket(socket, b_add_to_skeleton=True)

    # 保存
    print(f"[5/5] 保存 Mesh 和 Skeleton")
    unreal.EditorAssetLibrary.save_loaded_asset(skm)
    unreal.EditorAssetLibrary.save_loaded_asset(skeleton)

    # 验证
    verify = skm.find_socket(SOCKET_NAME)
    if verify is None:
        raise RuntimeError("add_socket 调用后仍找不到 WeaponSocket，可能 API 行为变化")
    print(f"      [Verify] find_socket({SOCKET_NAME}) -> 来自 {verify.get_outer().get_name()}")

    print("\n" + "=" * 60)
    print(f"[Xuanming] {SOCKET_NAME} 已加到 hand_r")
    print(f"  Location = {SOCKET_LOCATION}")
    print(f"  Rotation = {SOCKET_ROTATION}")
    print("接下来:")
    print("  1. 想可视化微调: 双击 SKM_Manny_Simple，左侧 Sockets 面板找 WeaponSocket")
    print("  2. 跑 CreateWeaponBP.py 建 BP_Weapon_AK")
    print("=" * 60)


main()
