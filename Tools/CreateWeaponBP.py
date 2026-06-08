"""
Pandora - 创建 BP_Weapon_AK + 链到 BP_PandoraCharacter（M1.3）

在 UE 编辑器里执行:
  Tools -> Execute Python Script -> 选 Tools/CreateWeaponBP.py

做的事:
  1. 创建 /Game/Weapons/BP_Weapon_AK（继承自 APandoraWeapon C++ 类），幂等
  2. 给 WeaponMesh 组件赋值引擎自带 /Engine/BasicShapes/Cube
  3. 把 Cube 缩放成枪状（细长）
  4. 给 BP_PandoraCharacter 的 CDO 设 DefaultWeaponClass = BP_Weapon_AK
  5. 保存

为什么不用 SubobjectDataSubsystem 改 BP 子组件:
  - UE 5.7 那套 API 复杂、跨版本不稳
  - 简化路径: 直接修改 WeaponMesh 这个 C++ 创建的 default subobject 的 default 值
    （在 BP CDO 上 set_editor_property 即可）

依赖:
  - APandoraWeapon C++ 类已编译进 Editor（M1.3 改了 WeaponMesh 类型，必须先编 Editor）
  - BP_PandoraCharacter 已存在（M1.1 完成）
"""

import unreal


# ---------- 资产路径 ----------
WEAPON_BP_PATH    = "/Game/Weapons/BP_Weapon_AK"
WEAPON_BP_FOLDER  = "/Game/Weapons"
WEAPON_BP_NAME    = "BP_Weapon_AK"

CHARACTER_BP_PATH = "/Game/Characters/BP_PandoraCharacter"

CUBE_MESH_PATH    = "/Engine/BasicShapes/Cube"

# Cube 缩放：让它看起来像把枪（细长，朝 +X 是枪管方向）
WEAPON_MESH_SCALE = unreal.Vector(0.5, 0.1, 0.1)
# 局部偏移：稍微往前推点，让枪握把对准 socket
WEAPON_MESH_LOCATION = unreal.Vector(15.0, 0.0, 0.0)


def load_or_die(path):
    a = unreal.EditorAssetLibrary.load_asset(path)
    if a is None:
        raise RuntimeError(f"找不到资产: {path}")
    return a


def get_pandora_weapon_class():
    """加载 C++ APandoraWeapon 类作为父类"""
    cls = unreal.load_class(None, "/Script/Pandora.PandoraWeapon")
    if cls is None:
        raise RuntimeError(
            "找不到 APandoraWeapon 类。Editor 是不是还没编 M1.3 的 C++ 改动？"
        )
    return cls


def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)
        print(f"  [Mkdir] {path}")


def create_weapon_bp():
    """幂等创建 BP_Weapon_AK"""
    ensure_folder(WEAPON_BP_FOLDER)

    if unreal.EditorAssetLibrary.does_asset_exist(WEAPON_BP_PATH):
        print(f"  [Skip] {WEAPON_BP_PATH} 已存在，复用")
        return load_or_die(WEAPON_BP_PATH)

    parent_class = get_pandora_weapon_class()

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    bp = asset_tools.create_asset(
        asset_name=WEAPON_BP_NAME,
        package_path=WEAPON_BP_FOLDER,
        asset_class=unreal.Blueprint.static_class(),
        factory=factory,
    )
    if bp is None:
        raise RuntimeError("create_asset 返回 None")
    print(f"  [Create] {WEAPON_BP_PATH}")
    return bp


def configure_weapon_bp(bp):
    """给 BP_Weapon_AK 的 WeaponMesh 默认子对象赋值 Cube"""
    cdo = unreal.get_default_object(bp.generated_class())
    if cdo is None:
        raise RuntimeError("拿不到 BP_Weapon_AK 的 CDO")

    weapon_mesh_comp = cdo.get_editor_property("WeaponMesh")
    if weapon_mesh_comp is None:
        raise RuntimeError(
            "CDO 上没 WeaponMesh 组件。APandoraWeapon C++ 是不是没编进去？"
        )

    # 赋值 Cube
    # 注意：set_relative_location / set_relative_scale3d 是运行时 API（要 sweep/teleport 参数）
    # 对 CDO 直接走 set_editor_property 写字段，更可靠且符合"编辑默认值"语义
    cube = load_or_die(CUBE_MESH_PATH)
    weapon_mesh_comp.set_editor_property("static_mesh", cube)
    weapon_mesh_comp.set_editor_property("relative_location", WEAPON_MESH_LOCATION)
    weapon_mesh_comp.set_editor_property("relative_scale3d", WEAPON_MESH_SCALE)
    print(f"  [Mesh] WeaponMesh.StaticMesh = {CUBE_MESH_PATH}")
    print(f"         scale    = {WEAPON_MESH_SCALE}")
    print(f"         location = {WEAPON_MESH_LOCATION}")


def link_to_character(weapon_bp):
    """改 BP_PandoraCharacter.DefaultWeaponClass = BP_Weapon_AK"""
    char_bp = load_or_die(CHARACTER_BP_PATH)
    char_cdo = unreal.get_default_object(char_bp.generated_class())
    if char_cdo is None:
        raise RuntimeError("拿不到 BP_PandoraCharacter CDO")

    char_cdo.set_editor_property("DefaultWeaponClass", weapon_bp.generated_class())
    print(f"  [Link] BP_PandoraCharacter.DefaultWeaponClass = {WEAPON_BP_NAME}")
    return char_bp


def main():
    print("=" * 60)
    print("[Pandora] M1.3 创建 BP_Weapon_AK")
    print("=" * 60)

    print("\n[1/4] 创建 BP_Weapon_AK")
    weapon_bp = create_weapon_bp()

    print("\n[2/4] 配置 WeaponMesh 默认值（挂 Cube）")
    configure_weapon_bp(weapon_bp)

    print("\n[3/4] 链到 BP_PandoraCharacter.DefaultWeaponClass")
    char_bp = link_to_character(weapon_bp)

    print("\n[4/4] 保存")
    for asset in [weapon_bp, char_bp]:
        unreal.EditorAssetLibrary.save_loaded_asset(asset)
        print(f"  [Save] {asset.get_path_name()}")

    print("\n" + "=" * 60)
    print("[Pandora] M1.3 蓝图链路完成")
    print("验证:")
    print("  1. PIE 跑起来，看角色右手是不是有个 Cube 形状的 'AK'")
    print("  2. 鼠标左键开火，控制台 / 屏幕看 DrawDebugLine 是否绘制")
    print("  3. 对着另一个角色（或 PlayerStart 旁边的 BP_PandoraCharacter）开火")
    print("     检查 Health 是不是从 100 减到 75 / 50 / 25 / 0")
    print("=" * 60)


main()