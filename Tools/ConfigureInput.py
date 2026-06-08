"""
Pandora - 一键配置 EnhancedInput 资产 (UE 5.7 兼容版)

在 UE 编辑器里执行: Tools -> Execute Python Script -> 选 Tools/ConfigureInput.py

UE 5.7 关键发现:
  - imc.map_key(ia, key) 只写到旧 deprecated mappings 数组, Modifier 不会同步到新结构
  - 必须直接操作 imc.default_key_mappings.mappings (新 InputMappingContextMappingData 结构)

C++ 端约定 (PandoraCharacter.cpp:103-104):
    AddMovementInput(Forward, Axis.Y);  -> Y = 前后
    AddMovementInput(Right,   Axis.X);  -> X = 左���
期望键盘输出:
    W -> ( 0, +1)   S -> ( 0, -1)
    D -> (+1,  0)   A -> (-1,  0)
"""

import unreal


# ---------- 资产路径 ----------
IA_MOVE_PATH   = "/Game/Input/IA_Move"
IA_LOOK_PATH   = "/Game/Input/IA_Look"
IA_JUMP_PATH   = "/Game/Input/IA_Jump"
IA_FIRE_PATH   = "/Game/Input/IA_Fire"
IA_CROUCH_PATH = "/Game/Input/IA_Crouch"
IA_FROST_CURSE_PATH = "/Game/Input/IA_FrostCurse"
IMC_DEFAULT_PATH = "/Game/Input/IMC_Default"


# ---------- 工具函数 ----------
def load_asset_or_die(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError(f"[Pandora] 找不到资产: {path}")
    return asset


def set_ia_value_type(ia, value_type_name):
    """设置 InputAction 的 Value Type."""
    enum_value = getattr(unreal.InputActionValueType, value_type_name)
    ia.set_editor_property("value_type", enum_value)
    print(f"  [IA] {ia.get_name()}.value_type = {value_type_name}")


def make_key(key_name):
    """构造 FKey struct."""
    k = unreal.Key()
    k.set_editor_property("key_name", key_name)
    return k


def make_mapping(ia, key_name, modifiers=None):
    """
    构造一个 EnhancedActionKeyMapping 完整结构.
    modifiers: list of 已初始化好的 modifier 实例
    """
    m = unreal.EnhancedActionKeyMapping()
    m.set_editor_property("action", ia)
    m.set_editor_property("key", make_key(key_name))
    if modifiers:
        m.set_editor_property("modifiers", modifiers)
    return m


def make_negate(outer):
    """构造 Negate modifier (默认 X/Y/Z 全勾, 等价于全维度取反).

    outer 必须是 IMC 资产, 否则 modifier 落到 /Engine/Transient/
    导致 Editor 重启后 modifier 丢失."""
    return unreal.new_object(unreal.InputModifierNegate, outer=outer)


def make_swizzle_yxz(outer):
    """构造 Swizzle YXZ modifier."""
    s = unreal.new_object(unreal.InputModifierSwizzleAxis, outer=outer)
    s.set_editor_property("order", unreal.InputAxisSwizzle.YXZ)
    return s


# ---------- 主流程 ----------
def main():
    print("=" * 60)
    print("[Pandora] 配置 EnhancedInput 资产 (UE 5.7 新结构)")
    print("=" * 60)

    # 1. 加载所有 IA
    ia_move   = load_asset_or_die(IA_MOVE_PATH)
    ia_look   = load_asset_or_die(IA_LOOK_PATH)
    ia_jump   = load_asset_or_die(IA_JUMP_PATH)
    ia_fire   = load_asset_or_die(IA_FIRE_PATH)
    ia_crouch = load_asset_or_die(IA_CROUCH_PATH)
    ia_frost_curse = load_asset_or_die(IA_FROST_CURSE_PATH)
    imc       = load_asset_or_die(IMC_DEFAULT_PATH)

    # 2. 校正 IA 的 Value Type
    print("\n[1/3] 校正 IA Value Type...")
    set_ia_value_type(ia_move,   "AXIS2D")
    set_ia_value_type(ia_look,   "AXIS2D")
    set_ia_value_type(ia_jump,   "BOOLEAN")
    set_ia_value_type(ia_fire,   "BOOLEAN")
    set_ia_value_type(ia_crouch, "BOOLEAN")
    set_ia_value_type(ia_frost_curse, "BOOLEAN")

    # 3. 构造完整 Mappings 数组
    print("\n[2/3] 构造 Mappings 数组...")

    mappings = []

    # --- IA_Move: WASD ---
    # UE 关键事实: 键盘 1D 输入升 2D 时, 值放在 X 轴而非 Y 轴.
    # 即按 W 输入 = (1, 0), 不是 (0, 1). 所以:
    #   W -> ( 0,+1) 前进: Swizzle YXZ (把 X 搬到 Y)
    #   S -> ( 0,-1) 后退: Swizzle YXZ + Negate
    #   D -> (+1, 0) 右移: 无 modifier (X 已经是 +1)
    #   A -> (-1, 0) 左移: Negate
    # Modifier 必须以 imc 为 outer 才能持久化到 .uasset, 否则进 /Engine/Transient/
    mappings.append(make_mapping(ia_move, "W", [make_swizzle_yxz(imc)]))
    mappings.append(make_mapping(ia_move, "S", [make_swizzle_yxz(imc), make_negate(imc)]))
    mappings.append(make_mapping(ia_move, "D"))
    mappings.append(make_mapping(ia_move, "A", [make_negate(imc)]))
    print("  [IMC] IA_Move: W=Swizzle, S=Swizzle+Negate, D=naked, A=Negate")

    # --- IA_Look ---
    mappings.append(make_mapping(ia_look, "Mouse2D"))
    print("  [IMC] IA_Look: Mouse2D")

    # --- IA_Jump ---
    mappings.append(make_mapping(ia_jump, "SpaceBar"))
    print("  [IMC] IA_Jump: SpaceBar")

    # --- IA_Fire ---
    mappings.append(make_mapping(ia_fire, "LeftMouseButton"))
    print("  [IMC] IA_Fire: LeftMouseButton")

    # --- IA_Crouch ---
    mappings.append(make_mapping(ia_crouch, "LeftControl"))
    print("  [IMC] IA_Crouch: LeftControl")

    # --- IA_FrostCurse ---
    mappings.append(make_mapping(ia_frost_curse, "Q"))
    print("  [IMC] IA_FrostCurse: Q")

    # 4. 写入新结构 default_key_mappings
    # 用 unreal.InputMappingContextMappingData 结构体
    dkm = unreal.InputMappingContextMappingData()
    dkm.set_editor_property("mappings", mappings)
    imc.set_editor_property("default_key_mappings", dkm)
    print(f"\n  写入 default_key_mappings (共 {len(mappings)} 个 mapping)")

    # 5. 保存
    print("\n[3/3] 保存资产...")
    for asset in [ia_move, ia_look, ia_jump, ia_fire, ia_crouch, ia_frost_curse, imc]:
        unreal.EditorAssetLibrary.save_loaded_asset(asset)
        print(f"  [Save] {asset.get_path_name()}")

    print("\n" + "=" * 60)
    print("[Pandora] EnhancedInput 配置完成!")
    print("接下来:")
    print("  1. 跑 VerifyIMC.py 确认每个 mapping 的 modifiers 都在")
    print("  2. PIE 测试 WASD 方向 / Space / Ctrl / 左键 / Q 冰咒")
    print("=" * 60)


main()