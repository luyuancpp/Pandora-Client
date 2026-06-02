# 引擎警告/小问题先忍，再想 workaround

> 这是"不动引擎源码"红线的具体应用。**默认决策是忽略**，不要主动去修。

## 常见可忽略噪声

| 现象 | 来源 | 正确做法 |
|---|---|---|
| `CS8981` 警告（`类型名称 "simde"/"zlib"/"jemalloc"... 仅包含小写 ASCII`，~26 行） | `UnrealEngine/Engine/Source/ThirdParty/*/*.Build.cs` | **忽略**。Epic 自己的命名风格，公开 API 一部分，他们 8 年没改。修它要动 26 个引擎 .cs 文件，违反最高指令 |
| `LogStreaming: Warning: Failed to read file 'doc_16x.png'` 等图标缺失 | 引擎 Slate 资源 | **忽略**。引擎自带、与项目无关 |
| `LogWindows: Failed to load 'aqProf.dll' / 'VtuneApi.dll' / 'Wintab32.dll'` | 引擎试探可选探查 dll | **忽略**。本机没装就略过，不影响功能 |
| `PixWinPlugin: PIX capture plugin failed to initialize` | 引擎自带 PIX 插件 | **忽略**。除非你真打算用 PIX |
| VS2026 编整个 sln 13 个失败（SlateViewer / Catch2 / AutoRTFMTests） | 引擎自带工具/测试 | **不要编整个 sln**，右键单独编 `Xuanming` 项目即可 |

## 判定标准

如果"修这个"的成本是"动引擎 tracked 文件"，那就**不修**。即使警告每次编译都刷屏 26 行，也比破坏"零引擎改动"的红线划算——后者一旦破坏：

- 升级 UE（5.7.5 → 5.8 → ...）每次都得人工 merge 冲突
- 团队/CI 拉代码后 Setup.bat 会覆盖你的改动
- pre-commit hook 直接拒绝
- 没人记得当年为啥改了，后来出 bug 时多一个怀疑对象

## 正确的 workaround 方向（按优先级）

1. 项目侧脚本（`.bat` / `.py` / `Build.cs`）绕开问题
2. 项目侧加 C++ 工具类（如 `UXuanmingSocketTools`，绕开 Python 反射限制）
3. 项目侧 `Directory.Build.props` / `.editorconfig` / `<NoWarn>` 屏蔽噪声
4. 文档化"已知噪声"让团队知道这是预期行为
5. **真要改引擎**：先在这个表里加一行、说清成本、用户书面同意、单独建一个引擎 fork 分支，且每次升级前重新审视
