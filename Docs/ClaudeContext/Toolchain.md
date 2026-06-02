# 构建工具链

> 路径用占位符：`${PROJECT_ROOT}` = 项目根，`${ENGINE_ROOT}` = UE 5.7.4 源码引擎根。
> 本机当前布局：两者位于同一父目录、平级。Claude 每次会话用 `pwd` / `git rev-parse --show-toplevel` 自行确认。

## 必装环境

- **Visual Studio 2026**（C++ 游戏开发工作负载） —— 用户实测可用，UE 5.7.4 release 内部已识别 VS18 toolchain（`MSVC 14.44.x`）
  - 也可用 VS2022，切换方式：`set VS_VERSION=2022 && GenerateProjectFiles.bat`
  - **不要**编整个解决方案，VS2026 下引擎自带的 `SlateViewer` / Catch2 测试会失败（与项目无关），右键单独编 `Xuanming` 项目即可
- **.NET 8 SDK**（必须，UE 5.7 的 .Build.cs 用了 C# 12 集合表达式 `[...]`）
  - 安装：`winget install Microsoft.DotNet.SDK.8`
  - .NET 10 / .NET 9 都不行
- **Git LFS**（UE 美术资源必备，本机未装见 `Backlog.md`）
- **UE 源码**：`${ENGINE_ROOT}` 必须跑过 `Setup.bat` 拉完二进制依赖

## 项目级 .NET 锁定

`${PROJECT_ROOT}/global.json`：

```json
{ "sdk": { "version": "8.0.100", "rollForward": "latestFeature" } }
```

dotnet 从工作目录向上查找 global.json，所以**只要在项目目录调用 dotnet**，.NET 8 就生效。`GenerateProjectFiles.bat` 内部所有 dotnet 调用都先 `pushd %PROJECT_ROOT%`。

引擎仓库**不要**也**不需要**放 global.json（避免破坏"不动引擎"红线）。

## UBT 重编

UE 5.7.4 release 自带的 `Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.dll` 是 **net6.0 编译的旧版**，会报 `CS1525` 错误。`GenerateProjectFiles.bat` 的处理：

- 如果 `Engine/Source/Programs/UnrealBuildTool/bin/Development/UnrealBuildTool.dll` 不存在，自动用项目 .NET 8 编译它
- UBT 编译产物在 `bin/` 目录（被引擎自身 .gitignore 屏蔽，不算修改 tracked 文件）
- 之后用新编的 UBT 生成 sln，**不**用 `Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.dll`

## 一键脚本（项目根）

| 脚本 | 用途 |
|---|---|
| `GenerateProjectFiles.bat` | 总入口：护栏 → .NET 8 检查 → UBT 编译 → 生成 sln |
| `BuildEditor.bat` | 编译 Editor（首次 10-30 分钟） |
| `BuildServer.bat` | 编译 DS（Win64 Development） |
| `BuildClient.bat` | 编译 Client |
| `BuildLinuxServer.bat` | 打包 Linux DS（需交叉编译工具链） |
| `BuildWin64ServerShipping.bat` | 打包 Win64 DS Shipping |
| `LaunchServer.bat` | 启动本地 DS（端口 7777） |
| `LaunchClient.bat` | 启动客户端连 127.0.0.1:7777 |
| `LaunchEditor.bat` | 启动 Editor（带 `-ddc=NoZenLocalFallback` 兜底 Zen 崩溃） |
| `Tools/CheckEngineUntouched.bat` | 引擎纯净度检查 |
| `Tools/SetupEngineGuards.bat` | 安装 junction + pre-commit hook |

## 网络配置（Config/DefaultEngine.ini）

- `OnlineSubsystem`：先用 NULL（局域网测试），后期可换 EOS（见 `Backlog.md`）
- `NetServerMaxTickRate=60`
- `MaxClientRate=15000`、`MaxInternetClientRate=10000`
- `MaxPlayers=16`（**M2 主城/战斗服改造时升到 50/100**，见 `SprintPlan.md`）
