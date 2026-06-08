# Pandora（Pandora）— Codex 上下文

> 这个文件是给 Codex 看的项目上下文摘要。新会话开始时会自动读入。
> 用户视角的完整文档在 `README.md`。

---

## 项目定位

- **名字**：Pandora（Pandora）—— 仙道风英文代号 + 现代军事 FPS 玩法（反差搭配）
- **玩法对标**：腾讯《三角洲行动》手游，战术 FPS、多人对战
- **架构**：Unreal Engine 5.7.4 源码版 + Dedicated Server + Client
- **仓库**：https://github.com/luyuancpp/Pandora（public）
- **状态**：项目骨架完成，可生成 VS 工程，未完成 Editor 编译

**重要：用户希望做一款"上线运营"的游戏**。但他/她也理解类似规模的游戏需要 30+ 人团队、亿级预算、数年时间。这个仓库只是**正确的工程起点**，不是游戏本体。

## 关键路径

| 路径 | 内容 |
|---|---|
| `F:\work\UnrealEngine` | UE 5.7.4 源码版引擎（**绝对不要改任何 tracked 文件**） |
| `F:\work\Pandora` | 项目根目录（本仓库） |
| `F:\work\Pandora\.Codex` | Codex 会话状态（已 gitignored） |
| `F:\work\UnrealEngine\.Codex` | **Junction → Pandora/.Codex**，引擎里物理上不存在真实数据 |

## 最高指令：不动引擎源码

用户的核心要求：**绝对不修改 `F:\work\UnrealEngine` 里任何 git tracked 文件**。

执行这条规则的机制：

1. **`Tools/CheckEngineUntouched.bat`** —— 每次构建前检查 `git status --porcelain` 过滤 `??` 之外的项，发现就 abort
2. **Junction**：`UnrealEngine/.Codex → Pandora/.Codex`，让 Codex 写入透明重定向
3. **pre-commit hook**：装在 `UnrealEngine/.git/hooks/pre-commit`，阻断任何 `.Codex/` 路径的 commit
4. **`Tools/SetupEngineGuards.bat`** 一键复现（幂等）
5. **`global.json` 放在项目目录**：dotnet 从工作目录向上查找，所以引擎仓库**不需要**也**不要**放 global.json

> **绝对禁止**：在 `F:\work\UnrealEngine` 里 `Edit` / `Write` 任何 tracked 文件，包括 `.csproj`、`.cs`、`.cpp`、`.h`、`.ini` 等。引擎自带的工具会找不到 props 文件等问题，**不要**通过编辑引擎文件解决——必须用项目侧的 workaround。

### 这条原则的具体应用：警告/小问题先忍，再想 workaround

引擎源码会持续产生各种"看起来不爽但无害"的输出。**默认决策是忽略**，不要主动去修。常见案例：

| 现象 | 来源 | 正确做法 |
|---|---|---|
| `CS8981` 警告（`类型名称 "simde"/"zlib"/"jemalloc"... 仅包含小写 ASCII`，~26 行） | `UnrealEngine/Engine/Source/ThirdParty/*/*.Build.cs` | **忽略**。Epic 自己的命名风格，公开 API 一部分，他们 8 年没改。修它要动 26 个引擎 .cs 文件，违反最高指令 |
| `LogStreaming: Warning: Failed to read file 'doc_16x.png'` 等图标缺失 | 引擎 Slate 资源 | **忽略**。引擎自带、与项目无关 |
| `LogWindows: Failed to load 'aqProf.dll' / 'VtuneApi.dll' / 'Wintab32.dll'` | 引擎试探可选探查 dll | **忽略**。本机没装就略过，不影响功能 |
| `PixWinPlugin: PIX capture plugin failed to initialize` | 引擎自带 PIX 插件 | **忽略**。除非你真打算用 PIX |
| VS2026 编整个 sln 13 个失败（SlateViewer / Catch2 / AutoRTFMTests） | 引擎自带工具/测试 | **不要编整个 sln**，右键单独编 `Pandora` 项目即可 |

**判定标准**：如果"修这个"的成本是"动引擎 tracked 文件"，那就**不修**。即使警告每次编译都刷屏 26 行，也比破坏"零引擎改动"的红线划算——后者一旦破坏：
- 升级 UE（5.7.5 → 5.8 → ...）每次都得人工 merge 冲突
- 团队/CI 拉代码后 Setup.bat 会覆盖你的改动
- pre-commit hook 直接拒绝
- 没人记得当年为啥改了，后来出 bug 时多一个怀疑对象

**正确的 workaround 方向**（按优先级）：
1. 项目侧脚本（.bat/.py/Build.cs）绕开问题
2. 项目侧加 C++ 工具类（如 `UPandoraSocketTools`，绕开 Python 反射限制）
3. 项目侧 `Directory.Build.props` / `.editorconfig` / `<NoWarn>` 屏蔽噪声
4. 文档化"已知噪声"让团队知道这是预期行为
5. **真要改引擎**：先在这个表里加一行、说清成本、用户书面同意、单独建一个引擎 fork 分支，且每次升级前重新审视

## 已实现的 C++ 模块

```
Source/Pandora/
├── PandoraGameMode      服务器权威规则、登入登出
├── PandoraGameState     全局状态（比赛计时，已 Replicate）
├── PandoraPlayerController  EnhancedInput 上下文注册
├── PandoraPlayerState   K/D/队伍数据（已 Replicate）
├── PandoraCharacter     FPS 角色 + 移动/视角/跳跃/蹲伏/开火 + 血量同步
├── PandoraWeapon        hitscan 武器、Server RPC、Multicast 特效、散布、射速、弹匣
└── PandoraHUD           Canvas 准星、血条、弹药
```

所有网络相关类已配好 Replication，服务器权威设计。

## 4 个 Target

- `Pandora.Target.cs` (Game)
- `PandoraEditor.Target.cs`
- `PandoraServer.Target.cs` (Dedicated Server)
- `PandoraClient.Target.cs`

## 构建工具链

### 必装环境
- **Visual Studio 2026**（C++ 游戏开发工作负载）—— 用户实测可用，UE 5.7.4 release 内部已识别 VS18 toolchain（`MSVC 14.44.x`）
  - 也可用 VS2022，切换方式：`set VS_VERSION=2022 && GenerateProjectFiles.bat`
  - **不要**编整个解决方案，VS2026 下引擎自带的 `SlateViewer` / Catch2 测试会失败（与项目无关），右键单独编 `Pandora` 项目即可
- **.NET 8 SDK**（必须，UE 5.7 的 .Build.cs 用了 C# 12 集合表达式 `[...]`）
  - 安装：`winget install Microsoft.DotNet.SDK.8`
  - .NET 10 / .NET 9 都不行
- **Git LFS**（UE 美术资源必备）
- **UE 源码**：`F:\work\UnrealEngine` 必须跑过 `Setup.bat` 拉完二进制依赖

### 项目级 .NET 锁定

`F:\work\Pandora\global.json`：
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
   - `BP_PandoraCharacter` 继承 `PandoraCharacter`
   - `BP_PandoraWeapon_AK` 继承 `PandoraWeapon`
   - `BP_PandoraPlayerController` 继承 `PandoraPlayerController`
   - `BP_PandoraGameMode` 继承 `PandoraGameMode`

2. 创建 EnhancedInput 资产：
   - `IMC_Default`（InputMappingContext）
   - `IA_Move` / `IA_Look` / `IA_Jump` / `IA_Fire` / `IA_Crouch`（5 个 InputAction）
   - 在 IMC 里绑键：WASD→IA_Move、鼠标→IA_Look、Space→IA_Jump、鼠标左键→IA_Fire、Ctrl→IA_Crouch

3. 配置：
   - `BP_PandoraPlayerController.DefaultMappingContext = IMC_Default`
   - `BP_PandoraCharacter` 指派 IA_* + `DefaultWeaponClass = BP_PandoraWeapon_AK`
   - `BP_PandoraGameMode.DefaultPawnClass = BP_PandoraCharacter` + `PlayerControllerClass = BP_PandoraPlayerController`
   - `Project Settings → Maps & Modes → Default GameMode = BP_PandoraGameMode`

4. 角色 Mesh 上加一个 `WeaponSocket`（武器挂点），否则 `SpawnDefaultWeapon` 时挂载失败。

## 当前进度（2026-05-30）

### 已完成里程碑

**M0 完成** (2026-05-23/24)：DS 联机骨架已通
- ✅ M0.1-M0.3 工业级 Content 目录 + L_Whitebox_01 关卡 + Project Settings 默认地图
- ✅ M0.4 PIE 单机不崩
- ✅ M0.5 真 DS 进程在 0.0.0.0:7777 监听
- ✅ M0.6 2 个 Client 同时连入，GameMode::PostLogin 报 Total: 2

**M1.1 完成** (2026-05-29)：白盒角色已就位
- ✅ Mannequin 资产复制到 `Content/Characters/Mannequins/` (49 个 .uasset, 93MB)
- ✅ ABP_Unarmed 编译通过 (141ms)
- ✅ BP_PandoraCharacter 蓝图：Mesh=SKM_Manny_Simple, AnimClass=ABP_Unarmed
  - Mesh Transform: Location=(0,0,-88), Rotation=(0,0,-90)（FPS 标准朝向）
  - Mesh.bOwnerNoSee=true（第一人称隐藏自己，业界惯例）
- ✅ BP_PandoraGameMode + BP_PandoraPlayerController 蓝图链路完整

**M1.2 完成** (2026-05-29)：EnhancedInput 全部接通
- ✅ 5 个 IA：Move/Look (Axis2D) + Jump/Fire/Crouch (bool)
- ✅ IMC_Default 配 8 条 mapping，WASD 4 方向都对
- ✅ 用 `Tools/ConfigureInput.py` Python 脚本一键配置（避免手动配 Modifier 出错）

**M1.3 完成** (2026-05-30)：BP_Weapon_AK + WeaponSocket 全链路通
- ✅ C++ `PandoraWeapon::WeaponMesh` 改 `UStaticMeshComponent`（接 placeholder Cube）
- ✅ C++ 新增 `UPandoraSocketTools::AddWeaponSocketToMesh`（BlueprintCallable, WITH_EDITOR）
  - 绕开 UE 5.7 Python 对 `BlueprintReadOnly` 字段的写禁止
  - C++ 端裸字段赋值 + `AddSocket(bAddToSkeleton=true)` + `NumSockets` 校验
- ✅ `Tools/AddWeaponSocket.py` 在 SKM_Manny_Simple 上加 WeaponSocket（hand_r, 偏移 (10,4,-2) Yaw=90）
- ✅ `Tools/CreateWeaponBP.py` 建 `Content/Weapons/BP_Weapon_AK`（继承 APandoraWeapon）
  - WeaponMesh = `/Engine/BasicShapes/Cube`, scale=(0.5, 0.1, 0.1)
  - BP_PandoraCharacter.DefaultWeaponClass = BP_Weapon_AK
- ✅ PIE 验证：右手有 Cube placeholder AK，鼠标左键开火出 DrawDebugLine
- ⚠️ 准星和命中反馈缺失（白盒角色没参照物难瞄）→ M1.4 UMG HUD 解决

**M1.4 代码完成** (2026-05-30)：UMG HUD 走 **MVVM 路线**，链路就绪，待用户编 Editor + 跑脚本 + Designer 手搭 + 配 MVVM
- ✅ C++ `APandoraHUD` 降级为占位（DrawHUD 清空，类保留作 GameMode HUDClass 默认指派）
- ✅ `Pandora.uproject` 启用 `ModelViewViewModel` 插件
- ✅ `Source/Pandora/Pandora.Build.cs` 加 `ModelViewViewModel` + `FieldNotification` 模块依赖
- ✅ C++ 新增 `UPandoraPlayerViewModel : UMVVMViewModelBase`：
  - 4 个 `FieldNotify` 字段：`Health` / `MaxHealth` / `CurrentAmmo` / `MagazineSize`
    （Setter 用 `UE_MVVM_SET_PROPERTY_VALUE` 宏，自动比较新旧值 + 不变就不广播）
  - 3 个 `FieldNotify` 派生 UFUNCTION：`GetHealthPercent` / `GetHealthText` / `GetAmmoText`
    （用 `meta=(FieldNotifyDependencies="Health,MaxHealth")` 让 MVVM 编译器跟踪依赖自动重算）
- ✅ C++ `APandoraPlayerController`：
  - 加 `HUDWidgetClass` + `HUDWidget` + `PlayerViewModel` + `PlayerViewModelName="PlayerViewModel"` 字段
  - `TryCreateHUD(From)` 复用 IMC "三时机兜底" 模式 (`BeginPlay` / `AcknowledgePossession` / `OnPossess`)
    + 创建 widget 后立即 `NewObject<UPandoraPlayerViewModel>(this)` + `TryInjectViewModel`
  - `TryInjectViewModel`：通过 `UMVVMSubsystem::GetViewFromUserWidget(HUDWidget)` 拿 `UMVVMView`，
    再 `View->SetViewModel(PlayerViewModelName, PlayerViewModel)`
  - `PlayerTick` 每帧 `PushStateToViewModel`（把 Pawn.Health / Weapon.CurrentAmmo 推到 ViewModel）；
    依赖 `SetPropertyValue` 内部新旧值比较，**未变化时零广播开销**
  - `EndPlay` 清理 `HUDWidget->RemoveFromParent()` + `PlayerViewModel = nullptr`
- ✅ `Tools/CreateHUDWidgets.py` 简化为：
  - 幂等创建 `/Game/UI/WBP_PlayerHUD`（空控件树）
  - 自动配 `BP_PandoraPlayerController.HUDWidgetClass = WBP_PlayerHUD.GeneratedClass`
  - main() 末尾打印详尽 Designer + MVVM 操作指南（替代原 WidgetTree 自动化，已确认 UE 5.7 反射搞不定）
- ⏳ 待用户操作：
  1. `GenerateProjectFiles.bat`（uproject 加了插件，需重生 sln）
  2. VS 右键 `Pandora` 项目 Build
  3. `LaunchEditor.bat` 启动 Editor（首次编 MVVM 插件 1-2 分钟）
  4. `Tools → Execute Python Script → Tools/CreateHUDWidgets.py`
  5. **Designer 拖控件树**（5 分钟）：Crosshair / HealthBar / HealthText / AmmoText 四个控件 + 锚点位置（脚本输出指南有详细坐标）
  6. **MVVM 面板配置**（5 分钟）：`Window → View Bindings`：
     - 加 ViewModel：Class=`PandoraPlayerViewModel`、Name=`PlayerViewModel`、Creation Type=`Manual`
     - 加 3 条 Binding：`HealthBar.Percent ← HealthPercent`、`HealthText.Text ← HealthText`、`AmmoText.Text ← AmmoText`
  7. PIE 验证：准星 + 血条 + 弹药、开火 AMMO 递减、受击血条短缩
- ⚠️ KillFeed 挂账：移到 M1.5 之后单独做（需 GameMode 击杀广播 + Multicast 事件 + 队伍判定）

**为什么走 MVVM 而不是函数 Binding**（决策记录）：
- 函数 Binding 是 UE 官方标记的"性能陷阱"——每帧 Tick 调用，10 个 binding × 60fps = 600 次/秒 蓝图 VM 调用
- MVVM 走 `INotifyFieldValueChanged` 推送，**只在数据变化时广播**，静止时零开销
- Epic Fortnite Chapter 4+ / 米哈游崩铁 / 三角洲手游 UI 都是 MVVM
- 项目当前 0 个 WBP 无历史包袱，最佳切入点；先函数 Binding 后迁 MVVM 要重写，一次到位省事
- UE 5.7 ModelViewViewModel 插件虽 `IsBetaVersion=true` 但 5.1 引入 3 年已稳定，宏 `UE_MVVM_SET_PROPERTY_VALUE` 5.3+ 不变

### M1 剩余子里程碑（FPS PoC 路线图）

```
M1.5 GAS 框架 + Pandora冰咒示例技能           3-5 天
  - 启用 GameplayAbilities 插件（uproject + Build.cs）
  - Character 加 ASC + AttributeSet (HP/Mana/Speed/Damage)
  - GA_PandoraFrostCurse：按 Q 释放，对前方目标施加 GE_Slow（速度 -50% × 3s）
  - 这是仙道 vs 现代 FPS 反差的差异化核心

M1.x KillFeed（M1.5 后插入）                1 天
  - GameMode 加 OnPlayerKilled 委托 + Multicast 广播 (Killer/Victim/Weapon)
  - WBP_KillFeed (VerticalBox 右上, 飘字 + 3s 淡出)
  - PlayerController 创建第二个 widget 并 AddToViewport
```

### M2-M5 后续大节点（M1 完成后规划，不到时不动）

- **M2 可内测**：匹配服 + 账号 + 反��弊 + 数据上报（1-2 月）
- **M3 可外测**：CI/CD + Linux DS + 大厅 + 商城 + 多地图（2-3 月）
- **M4 可上线**：性能调优 + 反外挂 + 风控 + 客服后台（3-6 月）
- **M5 运营**：赛季 + 活动 + 内容更新管线（持续）

### 长期挂账（用户自己做的事）

1. EOS（Epic Online Services）公网匹配集成（需注册 Epic 开发者拿 ProductID/SandboxID）
2. 后端服务（Go/Java 账号 + 匹配服）
3. CI/CD（GitHub Actions 自动编译 DS）
4. 商标查询（"Pandora"在中国商标网）+ 域名注册
5. 美术外包对接（M2 阶段换正式角色资产，按 SK_Mannequin 标准骨架做就零改动替换）
6. **Git LFS**：`.gitattributes` 已配 `*.uasset filter=lfs`，但本机 git-lfs 未装。
   M2 前补装：`winget install GitHub.GitLFS` → `git lfs install` → `git lfs migrate import --include="*.uasset,*.umap"`

## 项目设计原则

1. **服务器权威**：伤害计算、武器射击、移动校验全在服务器；客户端只发请求和做预测
2. **小步快跑**：每完成一个独立改动就 commit + push，不要堆大 PR
3. **不动引擎**：所有 workaround 都通过项目侧脚本解决
4. **代码 > 蓝图**：核心逻辑用 C++，蓝图只用来挂资产和调参数
5. **UI 走 MVVM**：所有 HUD/菜单数据通过 `UMVVMViewModelBase` 子类暴露给 WBP，**禁止函数 Binding**（每帧 Tick 调用，UE 官方标记的性能陷阱）。Setter 用 `UE_MVVM_SET_PROPERTY_VALUE` 宏；派生属性用 `UFUNCTION(BlueprintPure, FieldNotify, meta=(FieldNotifyDependencies="A,B"))`。对齐 Epic Fortnite Chapter 4+ / 米哈游崩铁 / 三角洲手游做法。

## 蓝图 vs C++ 性能（用户问过，结论记牢）

**用户疑问**："蓝图效率比 C++ 差很多吗？上线时能不能转 C++？"

**答案**：

### 蓝图分两种用法，性能差异巨大

| 用途 | 性能损耗 | 项目里怎么用 |
|---|---|---|
| 只挂资产 + 改 Default 值（Mesh / 动画蓝图 / 武器 class / 血量上限） | **0**，运行时和 C++ default 完全相同 | ✅ AGENTS.md 列的 4 个蓝图都是这种 |
| EventGraph 写逻辑（Tick 里连节点算伤害） | VM 解释，比 C++ 慢 5-10 倍 | ❌ 不要这么做，逻辑全在 C++ |
| AnimGraph 动画蓝图 | 高度优化，C++ 写不会更快 | ✅ 必须用 |
| 蓝图调 C++ 函数（UFUNCTION BlueprintCallable） | 单次 ~200-300ns 额外开销 | 可忽略 |

**关键**：性能瓶颈不在"蓝图 vs C++"，在"**Tick 里跑了什么**"。项目里移动/网络/命中判定已经全在 C++，蓝图只挂资产 → 性能等价纯 C++。

### 蓝图转 C++（Nativization）这条路不存在

- UE 4.20~4.27 有 "Nativize Blueprint Assets" 选项
- **UE 5.0 官方废弃**，5.1+ 从 Project Settings 彻底删除
- 项目用的 UE 5.7.4 **没有这条路**，不要承诺用户能转
- Epic 废弃理由：转出来不可读、不可调试、收益 < 5%、Fortnite 自己都不用

### 真正的上线优化（按收益排序，和蓝图无关）

1. 关 Tick：`PrimaryActorTick.bCanEverTick = false`，不需要的一律关
2. Significance Manager：远处玩家降更新频率
3. `NetUpdateFrequency` 分级 + AOI
4. LOD / HLOD：远处低模
5. Lumen / Nanite 分级，DS 关渲染
6. DS Shipping target 关 logging：`bUseLoggingInShipping = false`

每一项收益都比"蓝图转 C++"大 10-100 倍。

### 给用户的标准回复模板

如果用户再问类似问题，直接答：
- 现在建的蓝图是"资产容器"，性能 0 损耗
- UE5 没有蓝图转 C++ 这条路，也不需要
- Fortnite/PUBG/原神都是 C++ 框架 + 蓝图配资产，能跑 120fps 不靠"全 C++"
- 真要优化，做 Tick/网络/LOD/Significance，不是改语言

## 已知坑（避免重蹈覆辙）

| 坑 | 解法 |
|---|---|
| .bat 默认 GBK 编码，UTF-8 中文注释会报"al 不是命令" | 所有 .bat 顶部 `chcp 65001 > nul`，注释用英文 |
| dotnet 残留进程锁住 dll，导致 build 失败 | build 前 `Get-Process dotnet,MSBuild,VBCSCompiler | Stop-Process -Force` |
| Setup.bat 在沙箱环境立刻 exit 0，啥都没做 | 直接调 `Engine/Binaries/DotNET/GitDependencies/win-x64/GitDependencies.exe --force --threads=8` |
| GitHub HTTPS 被国内运营商重置 | 用 SSH remote：`git@github.com:luyuancpp/Pandora.git`（已配好） |
| UE 自带 .NET 8.0.412 在 Windows 缺 host/fxr 目录 | 不用它，用系统的 .NET 8 SDK（项目级 global.json 锁定 8.0.100） |
| `git reset --hard origin/release` 会丢失本地未推送 commit | 先 `git reflog` 找回；这次没造成永久损失 |
| UE 5.7.4 release csproj 引用 props 文件但仓库里没有 | GitDependencies 会拉，但必须等它完整跑完（30 分钟+） |
| VS2026 下编整个解决方案有 13 个失败（Catch2 测试 / SlateViewer / AutoRTFMTests） | 引擎自带工具问题，**与项目无关**；右键只编 `Pandora` 项目，自动按依赖带上 UnrealEditor |
| Target.cs 用 `BuildSettingsVersion.V5` 编 Editor 报 `PandoraEditor modifies the values of properties` | UE 5.7 默认 V6，必须升级所有 4 个 Target.cs 到 V6 |
| V6 升级后 `error C4458: "Owner"的声明隐藏了类成员` | 局部变量名和 `AActor::Owner` 冲突；改为 `OwnerChar` 等明确名字 |
| 启动 Editor 崩在 `FDerivedDataBackendGraph` 行 217 | ZenServer HTTP 服务在 `[::1]` 起不来；用 `-ddc=NoZenLocalFallback` 禁掉 Zen，走文件系统 DDC。已封装在 `LaunchEditor.bat` 里 |
| 真 DS 模式 (LaunchServer.bat) WASD 不响应，PIE 多人正常 | `GlobalDefaultServerGameMode` 必须也指向 BP_GameMode；之前还是 C++ 裸类导致 DS 端 PC/Pawn 都是 C++ 裸类，蓝图层 IA 字段全 nullptr |
| EnhancedInput IMC 注册在 DS+Client 模式失败 | `BeginPlay` 时 ULocalPlayer 未就绪；改用 `AcknowledgePossession`（client）+ `OnPossess`（ListenServer）三时机兜底注册 |
| Python 创建的 UE 对象（modifier 等）保存后丢失 | `unreal.new_object(Class)` 默认 outer = `/Engine/Transient/`，序列化时被丢弃。**必须**传 `outer=父资产`，例如 `unreal.new_object(InputModifierNegate, outer=imc)`，对象才会作为父资产子对象持久化 |
| UE5 Manny 资产命名陷阱：`SK_Mannequin` 是 USkeleton，`SKM_Manny_Simple` 才是 USkeletalMesh | 反直觉但是 Epic 的命名约定。socket / 骨骼相关脚本要先 `isinstance` 守卫，挂错类型会一直走死路 |
| `USkeleton.Sockets` Python 读不到，报 "is protected and cannot be read" | UPROPERTY 没标 BlueprintReadOnly。**不要**直接操作 USkeleton.sockets，改走 `USkeletalMesh.AddSocket(socket, bAddToSkeleton=true)`——这条是 BlueprintCallable，bAddToSkeleton 会自动镜像进 Skeleton |
| `USkeletalMeshSocket` 所有字段 `BlueprintReadOnly`，Python 写不进 | `set_editor_property`、attribute 赋值都被拒，因为 Python 反射走 BP 可写性检查。**唯一解**：C++ 写一�� `UFUNCTION(BlueprintCallable)` 工具函数（如 `UPandoraSocketTools::AddWeaponSocketToMesh`），裸字段赋值不走反射检查。同理适用于其他 BlueprintReadOnly + EditAnywhere 字段 |
| UE 5.7 `USkeletalMesh::AddSocket` 严格检查 socket outer 必须是 SkeletalMesh，否则 log error 但 void 返回不抛异常 | `NewObject<USkeletalMeshSocket>(SkeletalMesh)` outer 设 Mesh，传 `bAddToSkeleton=true` 让引擎内部自己复制一份到 Skeleton。**调用后必须**用 `NumSockets()` before/after 校验是否真加进去——UE 这种"void 返回但只 log error"的 API 必须用副作用校验做闸门 |
| Python `SceneComponent.set_relative_location(loc)` 报 `required argument 'sweep' (pos 2) not found` | 这是运行时 API，sweep/teleport 必填。**编辑 CDO 默认值**走 `set_editor_property("relative_location", ...)` / `set_editor_property("relative_scale3d", ...)`，符合"编辑默认值"语义 |
| UE 5.7 `WidgetBlueprint.WidgetTree` 是 protected 字段，Python `get_editor_property("WidgetTree")` 抛 "is protected and cannot be read" | 同 `USkeleton.Sockets` 那类反射坑。理论上可写 C++ `UPandoraUITools::GetWidgetTree` 工具函数绕开（同 SocketTools 套路），但**UMG 控件树本来就是高频迭代品**（美术每天调样式），Epic 官方 *UMG Best Practices* 也明确推荐 Designer 手搭——脚本只建 WBP + 链 PC.HUDWidgetClass，控件树交给用户 5 分钟手点。这条"不修"是设计决策不是妥协 |
| UMG 函数 Binding 性能陷阱 | `HealthBar.Percent → Bind Function` 每帧 Tick 调用（10 个 binding × 60fps = 600 次/秒蓝图 VM）。**全项目禁用**，改 MVVM (`UMVVMViewModelBase` + `UE_MVVM_SET_PROPERTY_VALUE` ���)，只在数据变化时广播 |

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