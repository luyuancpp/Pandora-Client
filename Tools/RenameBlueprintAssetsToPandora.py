import unreal


RENAMES = [
    ("/Game/Characters/BP_XuanmingCharacter", "/Game/Characters", "BP_PandoraCharacter"),
    (
        "/Game/Blueprints/PlayerControllers/BP_XuanmingPlayerController",
        "/Game/Blueprints/PlayerControllers",
        "BP_PandoraPlayerController",
    ),
    (
        "/Game/Blueprints/GameModes/BP_XuanmingGameMode",
        "/Game/Blueprints/GameModes",
        "BP_PandoraGameMode",
    ),
]


def main():
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    rename_data = []

    for old_path, new_package_path, new_name in RENAMES:
        new_path = f"{new_package_path}/{new_name}"
        if unreal.EditorAssetLibrary.does_asset_exist(new_path):
            unreal.log(f"[Pandora] Asset already renamed: {new_path}")
            continue

        asset = unreal.EditorAssetLibrary.load_asset(old_path)
        if not asset:
            unreal.log_warning(f"[Pandora] Asset not found, skipping: {old_path}")
            continue

        rename_data.append(unreal.AssetRenameData(asset, new_package_path, new_name))

    if rename_data:
        succeeded = asset_tools.rename_assets(rename_data)
        if not succeeded:
            raise RuntimeError("Asset rename failed")

    unreal.EditorAssetLibrary.save_directory("/Game", only_if_is_dirty=True, recursive=True)
    unreal.log("[Pandora] Blueprint asset rename pass complete.")


main()
