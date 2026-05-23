# Xuanming（玄冥）— Claude Code 上下文

> 这个文件是给 Claude Code 看的项目上下文摘要。新会话开始时会自动读入。
> 用户视角的完整文档在 `README.md`。

---

## 项目定位

- **名字**：Xuanming（玄冥）—— 仙道风英文代号 + 现代军事 FPS 玩法（反差搭配）
- **玩法对标**：腾讯《三角洲行动》手游，战术 FPS、多人对战
- **架构**：Unreal Engine 5.7.4 源码版 + Dedicated Server + Client
- **仓库**：https://github.com/luyuancpp/Xuanming（public）
- **状态**：项目骨架完成，可生成 VS 工程，未完成 Editor 编译

**重要：用户希望做一款"上线运营"的游戏**。但他/她也理解类似规模的游戏需要 30+ 人团队、亿级预算、数年时间。这个仓库只是**正确的工程起点**，不是游戏本体。

## 关键路径

| 路径 | 内容 |
|---|---|
| `F:\work\UnrealEngine` | UE 5.7.4 源码版引擎（**绝对不要改任何 tracked 文件**） |
| `F:\work\Xuanming` | 项目根目录（本仓库） |
| `F:\work\Xuanming\.claude` | Claude Code 会话状态（已 gitignored） |
| `F:\work\UnrealEngine\.claude` | **Junction → Xuanming/.claude**，引擎里物理上不存在真实数据 |

## 最高指令：不动引擎源码

用户的核心要求：**绝对不修改 `F:\work\UnrealEngine` 里任何 git tracked 文件**。

执行这条规则的机制：

1. **`Tools/CheckEngineUntouched.bat`** —— 每次构建前检查 `git status --porcelain` 过滤 `??` 之外的项，发现就 abort
2. **Junction**：`UnrealEngine/.claude → Xuanming/.claude`，让 Claude Code 写入透明重定向
3. **pre-commit hook**：装在 `UnrealEngine/.git/hooks/pre-commit`，阻断任何 `.claude/` 路径的 commit
4. **`Tools/SetupEngineGuards.bat`** 一键复现（幂等）
5. **`global.json` 放在项目目录**：dotnet 从工作目录向上查找，所以引擎仓库**不需要**也**不要**放 global.json

> **绝对禁止**：在 `F:\work\UnrealEngine` 里 `Edit` / `Write` 任何 tracked 文件，包括 `.csproj`、`.cs`、`.cpp`、`.h`、`.ini` 等。引擎自带的工具会找不到 props 文件等问题，**不要**通过编辑引擎文件解决——必须用项目侧的 workaround。

## 已实现的 C++ 模块

```
Source/Xuanming/
├── XuanmingGameMode      服务器权威规则、登入登出
├── XuanmingGameState     全局状态（比赛计时，已 Replicate）
├── XuanmingPlayerController  EnhancedInput 上下文注册
├── XuanmingPlayerState   K/D/队伍数据（已 Replicate）
├── XuanmingCharacter     FPS 角色 + 移动/视角/跳跃/蹲伏/开火 + 血量同步
├── XuanmingWeapon        hitscan 武器、Server RPC、Multicast 特效、散布、射速、弹匣
└── XuanmingHUD           Canvas 准星、血条、弹药
```

所有网络相关类已配好 Replication，服务器权威设计。

## 4 个 Target

- `Xuanming.Target.cs` (Game)
- `XuanmingEditor.Target.cs`
- `XuanmingServer.Target.cs` (Dedicated Server)
- `XuanmingClient.Target.cs`

## 构建工具链

### 必装环境
- **Visual Studio 2022**（C++ 游戏开发工作负载）
- **.NET 8 SDK**（必须，UE 5.7 的 .Build.cs 用了 C# 12 集合表达式 `[...]`）
  - 安装：`winget install Microsoft.DotNet.SDK.8`
  - .NET 10 / .NET 9 都不行
- **Git LFS**（UE 美术资源必备）
- **UE 源码**：`F:\work\UnrealEngine` 必须跑过 `Setup.bat` 拉完二进制依赖

### 项目级 .NET 锁定

`F:\work\Xuanming\global.json`：
```json
{ "sdk": { "version": "8.0.100", "rollForward": "latestFeature" } }
```

dotnet 从工作目录向上查找 global.json，所以**只要在项目目录调用 dotnet**，.NET 8 就生效。`GenerateProjectFiles.bat` 内部所有 dotnet 调用都先 `pushd %PROJECT_ROOT%`。

### UBT 重编

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
| `Tools/CheckEngineUntouched.bat` | 引擎纯净度检查 |
| `Tools/SetupEngineGuards.bat` | 安装 junction + pre-commit hook |

## 网络配置（Config/DefaultEngine.ini）

- `OnlineSubsystem`：先用 NULL（局域网测试），后期可换 EOS
- `NetServerMaxTickRate=60`
- `MaxClientRate=15000`、`MaxInternetClientRate=10000`
- `MaxPlayers=16`

## 用户在编辑器里要做的事（C++ 类需要蓝图实例化）

我无法替用户在编辑器里点鼠标。用户需要：

1. 在 `Content/` 下右键创建蓝图：
   - `BP_XuanmingCharacter` 继承 `XuanmingCharacter`
   - `BP_XuanmingWeapon_AK` 继承 `XuanmingWeapon`
   - `BP_XuanmingPlayerController` 继承 `XuanmingPlayerController`
   - `BP_XuanmingGameMode` 继承 `XuanmingGameMode`

2. 创建 EnhancedInput 资产：
   - `IMC_Default`（InputMappingContext）
   - `IA_Move` / `IA_Look` / `IA_Jump` / `IA_Fire` / `IA_Crouch`（5 个 InputAction）
   - 在 IMC 里绑键：WASD→IA_Move、鼠标→IA_Look、Space→IA_Jump、鼠标左键→IA_Fire、Ctrl→IA_Crouch

3. 配置：
   - `BP_XuanmingPlayerController.DefaultMappingContext = IMC_Default`
   - `BP_XuanmingCharacter` 指派 IA_* + `DefaultWeaponClass = BP_XuanmingWeapon_AK`
   - `BP_XuanmingGameMode.DefaultPawnClass = BP_XuanmingCharacter` + `PlayerControllerClass = BP_XuanmingPlayerController`
   - `Project Settings → Maps & Modes → Default GameMode = BP_XuanmingGameMode`

4. 角色 Mesh 上加一个 `WeaponSocket`（武器挂点），否则 `SpawnDefaultWeapon` 时挂载失败。

## 待办（用户要求"先记住"，按指令再做）

1. 角色 Mesh + 武器 Socket 配置（需美术资源）
2. 测试关卡（用 OpenWorld 或简单建一个）
3. EOS（Epic Online Services）公网匹配集成（用户需先注册 Epic 开发者，提供 ProductID/SandboxID）
4. 队伍/阵营 + 重生流程 + 战斗结算
5. GAS（Gameplay Ability System）——仙道风"招式"系统
6. 后端服务（Go/Java 账号 + 匹配服）
7. CI/CD（GitHub Actions 自动编译 DS）
8. 商标查询（"玄冥"在中国商标网）+ 域名注册（用户自己做）

## 项目设计原则

1. **服务器权威**：伤害计算、武器射击、移动校验全在服务器；客户端只发请求和做预测
2. **小步快跑**：每完成一个独立改动就 commit + push，不要堆大 PR
3. **不动引擎**：所有 workaround 都通过项目侧脚本解决
4. **代码 > 蓝图**：核心逻辑用 C++，蓝图只用来挂资产和调参数

## 已知坑（避免重蹈覆辙）

| 坑 | 解法 |
|---|---|
| .bat 默认 GBK 编码，UTF-8 中文注释会报"al 不是命令" | 所有 .bat 顶部 `chcp 65001 > nul`，注释用英文 |
| dotnet 残留进程锁住 dll，导致 build 失败 | build 前 `Get-Process dotnet,MSBuild,VBCSCompiler | Stop-Process -Force` |
| Setup.bat 在沙箱环境立刻 exit 0，啥都没做 | 直接调 `Engine/Binaries/DotNET/GitDependencies/win-x64/GitDependencies.exe --force --threads=8` |
| GitHub HTTPS 被国内运营商重置 | 用 SSH remote：`git@github.com:luyuancpp/Xuanming.git`（已配好） |
| UE 自带 .NET 8.0.412 在 Windows 缺 host/fxr 目录 | 不用它，用系统的 .NET 8 SDK（项目级 global.json 锁定 8.0.100） |
| `git reset --hard origin/release` 会丢失本地未推送 commit | 先 `git reflog` 找回；这次没造成永久损失 |
| UE 5.7.4 release csproj 引用 props 文件但仓库里没有 | GitDependencies 会拉，但必须等它完整跑完（30 分钟+） |

## 用户偏好

- 中文沟通
- 喜欢直接给方案、不绕弯
- 重视"上线运营"的工程严谨性，但理解一个人/小团队做不到大厂规模
- 对破坏性操作很谨慎（之前 git reset 之前我特意确认过）
- 不喜欢长篇大论的修辞，喜欢清单和表格

## 当前未解决问题

- **用户尚未在 VS 里编译 Editor**（首次 10-30 分钟）
- **用户尚未在编辑器里创建蓝图和 EnhancedInput 资产**
- **用户尚未做商标查询和域名抢注**

---

## 给新会话的开场建议

如果用户说"继续做"或类似，先问 ta：
1. 是否已经在 VS 里编译过 Editor？
2. 想先做哪一项待办？

不要直接开搞，因为待办优先级取决于用户当下需求（比如要不要先有玩法 demo 跑起来给老板看）。
