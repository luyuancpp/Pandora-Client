"""
Pandora - 修复 BP_PandoraPlayerController 的 MVVM ViewModel 名称.

用途:
  WBP_PlayerHUD 的 MVVM 面板里 ViewModel Name 应为 "PlayerViewModel"。
  如果 PlayerController 默认值仍是旧的 "PandoraPlayerViewModel"，运行时
  SetViewModel 会失败，HUD 会显示默认值或不刷新。
"""

import unreal


PC_BP_PATH = "/Game/Blueprints/PlayerControllers/BP_PandoraPlayerController"
VIEW_MODEL_NAME = "PlayerViewModel"


def main():
    pc_bp = unreal.EditorAssetLibrary.load_asset(PC_BP_PATH)
    if not pc_bp:
        raise RuntimeError(f"找不到资产: {PC_BP_PATH}")

    cdo = unreal.get_default_object(pc_bp.generated_class())
    if not cdo:
        raise RuntimeError("拿不到 BP_PandoraPlayerController CDO")
    current_name = str(cdo.get_editor_property("PlayerViewModelName"))

    if current_name != VIEW_MODEL_NAME:
        cdo.set_editor_property("PlayerViewModelName", VIEW_MODEL_NAME)
        unreal.EditorAssetLibrary.save_asset(PC_BP_PATH, only_if_is_dirty=False)
        print(f"[Pandora] PlayerViewModelName: {current_name} -> {VIEW_MODEL_NAME}")
    else:
        print(f"[Pandora] PlayerViewModelName already OK: {VIEW_MODEL_NAME}")


if __name__ == "__main__":
    main()
