"""Diagnostic v2: discover the right API for building FKey and adding mappings in UE 5.7.

在 UE Editor 里执行: Tools -> Execute Python Script -> 选 DumpInputEnums.py
"""

import unreal


def main():
    print("=" * 60)
    print("[Pandora] UE 5.7 EnhancedInput API 探测 v2")
    print("=" * 60)

    # 1. 找所有可能用来构造 FKey 的类型
    print("\n=== unreal.Key* 或 *Key 候选类型 ===")
    for name in sorted(dir(unreal)):
        lname = name.lower()
        if ("key" in lname and not "keyboard" in lname
            and not lname.startswith("keys_")
            and len(name) < 50):
            obj = getattr(unreal, name)
            kind = type(obj).__name__
            print(f"  unreal.{name}   (类型: {kind})")

    # 2. 探查 InputMappingContext 上有什么方法
    print("\n=== unreal.InputMappingContext 上的非私有属性/方法 ===")
    imc_path = "/Game/Input/IMC_Default"
    imc = unreal.EditorAssetLibrary.load_asset(imc_path)
    if imc is None:
        print(f"  (无法加载 {imc_path})")
    else:
        for attr in sorted(dir(imc)):
            if attr.startswith("_"):
                continue
            try:
                v = getattr(imc, attr)
                if callable(v):
                    print(f"  [method] {attr}")
                else:
                    pass  # 不打印数据属性, 太多
            except Exception:
                pass

    # 3. 探查 EnhancedActionKeyMapping (单个 mapping 的结构体)
    print("\n=== unreal.EnhancedActionKeyMapping 是否存在 ===")
    try:
        cls = unreal.EnhancedActionKeyMapping
        print(f"  存在: {cls}")
        # 试着新建一个看其属性
        try:
            sample = unreal.EnhancedActionKeyMapping()
            print(f"  实例化成功, 属性列表:")
            for attr in sorted(dir(sample)):
                if attr.startswith("_") or callable(getattr(sample, attr, None)):
                    continue
                try:
                    v = sample.get_editor_property(attr)
                    print(f"    {attr} = {v!r}")
                except Exception:
                    pass
        except Exception as e:
            print(f"  实例化失败: {e}")
    except AttributeError as e:
        print(f"  不存在: {e}")

    # 4. 试 unreal.Key (这是 5.0+ 标准 FKey 的 Python 暴露)
    print("\n=== unreal.Key 测试 ===")
    try:
        cls = unreal.Key
        print(f"  unreal.Key 存在: {cls}")
        try:
            k = unreal.Key(key_name="W")
            print(f"  unreal.Key(key_name='W') 成功: {k}")
        except Exception as e:
            print(f"  unreal.Key(key_name='W') 失败: {e}")
        try:
            k = unreal.Key("W")
            print(f"  unreal.Key('W') 成功: {k}")
        except Exception as e:
            print(f"  unreal.Key('W') 失败: {e}")
    except AttributeError as e:
        print(f"  unreal.Key 不存在: {e}")

    # 5. 试常见 helper
    print("\n=== unreal.SystemLibrary / GameplayStatics 上和 key 有关的 ===")
    for name in sorted(dir(unreal)):
        lname = name.lower()
        if "inputkey" in lname or "fkey" in lname:
            print(f"  unreal.{name}")

    # 6. 试用 DefaultKeyMappings (新 API 暗示)
    print("\n=== imc.DefaultKeyMappings 检查 ===")
    if imc:
        try:
            dkm = imc.get_editor_property("default_key_mappings")
            print(f"  default_key_mappings = {dkm!r}")
        except Exception as e:
            print(f"  get default_key_mappings 失败: {e}")

    print("\n" + "=" * 60)
    print("跑完贴整段输出给我")
    print("=" * 60)


main()