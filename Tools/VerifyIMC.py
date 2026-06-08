"""Verify what's actually inside IMC_Default after ConfigureInput.py ran.

Tools -> Execute Python Script -> 选 VerifyIMC.py
"""

import unreal


def dump_struct(s, indent=2):
    """递归打印 struct 字段."""
    pad = " " * indent
    if s is None:
        print(f"{pad}(None)")
        return
    # 试着列出字段
    for attr in sorted(dir(s)):
        if attr.startswith("_") or callable(getattr(s, attr, None)):
            continue
        try:
            v = s.get_editor_property(attr)
            print(f"{pad}{attr} = {v!r}")
        except Exception:
            pass


def main():
    print("=" * 60)
    print("[Pandora] 验证 IMC_Default 实际内容")
    print("=" * 60)

    imc = unreal.EditorAssetLibrary.load_asset("/Game/Input/IMC_Default")

    print("\n--- 通过 deprecated 'mappings' 属性读 (旧路径) ---")
    try:
        mappings_old = imc.get_editor_property("mappings")
        print(f"mappings 类型: {type(mappings_old).__name__}, 长度: {len(mappings_old)}")
        for i, m in enumerate(mappings_old):
            print(f"\n[{i}] mapping:")
            try:
                action = m.get_editor_property("action")
                key    = m.get_editor_property("key")
                mods   = m.get_editor_property("modifiers")
                print(f"  action     = {action}")
                print(f"  key        = {key}")
                print(f"  key.repr   = {key!r}")
                # 尝试读 key 的 key_name
                try:
                    kn = key.get_editor_property("key_name")
                    print(f"  key.key_name = {kn!r}")
                except Exception as e:
                    print(f"  key.key_name 读失败: {e}")
                # 列出 key 上所有非私有属性
                print(f"  key 字段:")
                for attr in sorted(dir(key)):
                    if attr.startswith("_") or callable(getattr(key, attr, None)):
                        continue
                    try:
                        v = key.get_editor_property(attr)
                        print(f"    {attr} = {v!r}")
                    except Exception:
                        pass
                # modifiers
                print(f"  modifiers ({len(mods)} 个):")
                for j, mod in enumerate(mods):
                    print(f"    [{j}] {type(mod).__name__}")
                    for attr in sorted(dir(mod)):
                        if attr.startswith("_") or callable(getattr(mod, attr, None)):
                            continue
                        try:
                            v = mod.get_editor_property(attr)
                            # 只打印小的值
                            if isinstance(v, (int, float, bool, str)) or v is None:
                                print(f"        {attr} = {v!r}")
                        except Exception:
                            pass
            except Exception as e:
                print(f"  读取失败: {e}")
    except Exception as e:
        print(f"读 mappings 失败: {e}")

    print("\n--- 通过新 'default_key_mappings' 读 ---")
    try:
        dkm = imc.get_editor_property("default_key_mappings")
        print(f"default_key_mappings = {dkm!r}")
    except Exception as e:
        print(f"读 default_key_mappings 失败: {e}")

    print("\n" + "=" * 60)
    print("贴整段输出给我")
    print("=" * 60)


main()