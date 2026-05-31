"""
Xuanming - 创建 WBP_PlayerHUD 并链到 BP_XuanmingPlayerController（M1.4 MVVM 路线）

在 UE 编辑器里执行:
  Tools -> Execute Python Script -> 选 Tools/CreateHUDWidgets.py

做的事（幂等, 100% 自动）:
  1. 确保 /Game/UI 目录存在
  2. 创建 /Game/UI/WBP_PlayerHUD（继承 UUserWidget, 空控件树）
  3. 把 BP_XuanmingPlayerController.HUDWidgetClass 设成 WBP_PlayerHUD.GeneratedClass
  4. 保存两个资产

脚本不做（用户在 Designer + MVVM 面板手搭, 行业惯例）:
  - WBP 控件树 (Crosshair / HealthBar / HealthText / AmmoText)
  - MVVM ViewModel 配置
  - MVVM Bindings 配置

为什么不自动化控件树:
  UE 5.7 已知坑: WidgetBlueprint.WidgetTree 是 protected 字段, Python 反射读不到.
  Epic 官方 *UMG Best Practices* 也明确推荐 Designer 手搭——
  WBP 是高频迭代品, 美术每天调样式, 写脚本不划算.

依赖:
  - C++ Editor 已编译 M1.4 改动 (PlayerController + PlayerViewModel + MVVM 插件)
  - BP_XuanmingPlayerController 已存在 (M1.1)
"""

import unreal


# ---------- 资产路径 ----------
HUD_FOLDER       = "/Game/UI"
HUD_BP_NAME      = "WBP_PlayerHUD"
HUD_BP_PATH      = f"{HUD_FOLDER}/{HUD_BP_NAME}"

PC_BP_PATH       = "/Game/Blueprints/PlayerControllers/BP_XuanmingPlayerController"


# ---------- 工具 ----------
def load_or_die(path):
    a = unreal.EditorAssetLibrary.load_asset(path)
    if a is None:
        raise RuntimeError(f"找不到资产: {path}")
    return a


def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)
        print(f"  [Mkdir] {path}")


# ---------- 主流程 ----------
def create_hud_wbp():
    """幂等创建 WBP_PlayerHUD (空控件树, 用户在 Designer 里手搭)"""
    ensure_folder(HUD_FOLDER)

    if unreal.EditorAssetLibrary.does_asset_exist(HUD_BP_PATH):
        print(f"  [Skip] {HUD_BP_PATH} 已存在, 复用")
        return load_or_die(HUD_BP_PATH)

    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", unreal.UserWidget)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    bp = asset_tools.create_asset(
        asset_name=HUD_BP_NAME,
        package_path=HUD_FOLDER,
        asset_class=unreal.WidgetBlueprint.static_class(),
        factory=factory,
    )
    if bp is None:
        raise RuntimeError("create_asset(WidgetBlueprint) 返回 None")
    print(f"  [Create] {HUD_BP_PATH}")
    return bp


def link_to_player_controller(wbp):
    """改 BP_XuanmingPlayerController.HUDWidgetClass = WBP_PlayerHUD.GeneratedClass"""
    pc_bp = load_or_die(PC_BP_PATH)
    pc_cdo = unreal.get_default_object(pc_bp.generated_class())
    if pc_cdo is None:
        raise RuntimeError("拿不到 BP_XuanmingPlayerController CDO")

    pc_cdo.set_editor_property("HUDWidgetClass", wbp.generated_class())
    print(f"  [Link] BP_XuanmingPlayerController.HUDWidgetClass = {HUD_BP_NAME}")
    return pc_bp


def save(*assets):
    for a in assets:
        unreal.EditorAssetLibrary.save_loaded_asset(a)
        print(f"  [Save] {a.get_path_name()}")


# ---------- main ----------
def main():
    print("=" * 60)
    print("[Xuanming] M1.4 MVVM - 创建 WBP_PlayerHUD + 链 PC")
    print("=" * 60)

    print("\n[1/3] 创建 WBP_PlayerHUD")
    wbp = create_hud_wbp()

    print("\n[2/3] 链到 BP_XuanmingPlayerController.HUDWidgetClass")
    pc_bp = link_to_player_controller(wbp)

    print("\n[3/3] 保存")
    save(wbp, pc_bp)

    print("\n" + "=" * 60)
    print("[Xuanming] M1.4 资产骨架就绪")
    print("=" * 60)
    print()
    print("【接下来需要你在 Editor 里手搭, 约 10 分钟】")
    print()
    print("=" * 60)
    print("STEP A: Designer 拖控件树 (5 分钟)")
    print("=" * 60)
    print("双击 /Game/UI/WBP_PlayerHUD -> Designer 标签")
    print("Hierarchy 下 root (Canvas Panel) 拖入 4 个控件:")
    print()
    print("  1. Image, 命名 'Crosshair':")
    print("       Anchor    = Center (Alignment 预设右下角的居中那个)")
    print("       Alignment = (0.5, 0.5)")
    print("       Position  = (0, 0)")
    print("       Size      = (4, 4)")
    print("       Tint      = White (1, 1, 1, 1)")
    print()
    print("  2. ProgressBar, 命名 'HealthBar':")
    print("       Anchor    = Bottom-Left")
    print("       Alignment = (0, 1)")
    print("       Position  = (40, -60)")
    print("       Size      = (240, 18)")
    print("       Fill Color = 绿 (0.2, 0.9, 0.2, 1)")
    print()
    print("  3. TextBlock, 命名 'HealthText':")
    print("       Anchor    = Bottom-Left")
    print("       Alignment = (0, 1)")
    print("       Position  = (44, -82)")
    print("       Size      = Auto")
    print("       Color     = White, 字号 14")
    print("       默认 Text = 'HP 100 / 100' (运行时会被 ViewModel 覆盖)")
    print()
    print("  4. TextBlock, 命名 'AmmoText':")
    print("       Anchor    = Bottom-Right")
    print("       Alignment = (1, 1)")
    print("       Position  = (-40, -50)")
    print("       Size      = Auto")
    print("       Color     = 黄 (1, 0.85, 0, 1), 字号 24")
    print("       默认 Text = 'AMMO 30 / 30'")
    print()
    print("=" * 60)
    print("STEP B: 配 MVVM (5 分钟)")
    print("=" * 60)
    print("WBP 编辑器顶部菜单 Window -> View Bindings (MVVM 插件提供的窗口)")
    print()
    print("【B1. 添加 ViewModel】左下角 Viewmodels 面板 -> 加号:")
    print("       View Model Class : XuanmingPlayerViewModel")
    print("       View Model Name  : PlayerViewModel  (必须和 PC.PlayerViewModelName 完全一致)")
    print("       Creation Type    : Manual           (PC 手动注入, 不让 MVVM 自动 spawn)")
    print()
    print("【B2. 添加 3 条 Bindings】右侧 Bindings 面板 -> 加号:")
    print("  - Widget: HealthBar.Percent  <-  ViewModel: PlayerViewModel.HealthPercent")
    print("        Mode: One Way to Destination (默认)")
    print("  - Widget: HealthText.Text    <-  ViewModel: PlayerViewModel.HealthText")
    print("  - Widget: AmmoText.Text      <-  ViewModel: PlayerViewModel.AmmoText")
    print()
    print("【B3. Compile + Save】顶部工具栏的 Compile 按钮 + Ctrl+S")
    print()
    print("=" * 60)
    print("STEP C: PIE 验证")
    print("=" * 60)
    print("- 屏幕中心 4x4 白色准星")
    print("- 左下绿色血条 + 'HP 100 / 100'")
    print("- 右下黄色 'AMMO 30 / 30'")
    print("- 鼠标左键开火: AMMO 数字递减 (HealthBar 不变, MVVM 性能优势)")
    print("- 受击: 血条短缩 + HP 数字变")
    print()
    print("如果 PIE 看到准星 / 血条 / 弹药都正确刷新, M1.4 完成 ✓")
    print("=" * 60)


main()
