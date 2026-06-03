# 已知坑（避免重蹈覆辙）

> 遇到 Build/Editor/Python/Input/MVVM/GAS 报错时，**先在这里查**再开搞。
> 新踩的坑往末尾追加，附"现象"+"修法"。

## Build / 工具链

| 坑 | 解法 |
|---|---|
| .bat 默认 GBK 编码，UTF-8 中文注释会报"al 不是命令" | 所有 .bat 顶部 `chcp 65001 > nul`，注释用英文 |
| dotnet 残留进程锁住 dll，导致 build 失败 | build 前 `Get-Process dotnet,MSBuild,VBCSCompiler \| Stop-Process -Force` |
| Setup.bat 在沙箱环境立刻 exit 0，啥都没做 | 直接调 `Engine/Binaries/DotNET/GitDependencies/win-x64/GitDependencies.exe --force --threads=8` |
| GitHub HTTPS 被国内运营商重置 | 用 SSH remote：`git@github.com:luyuancpp/Xuanming.git`（已配好） |
| UE 自带 .NET 8.0.412 在 Windows 缺 host/fxr 目录 | 不用它，用系统的 .NET 8 SDK（项目级 global.json 锁定 8.0.100） |
| `git reset --hard origin/release` 会丢失本地未推送 commit | 先 `git reflog` 找回；操作前先确认 |
| UE 5.7.4 release csproj 引用 props 文件但仓库里没有 | GitDependencies 会拉，但必须等它完整跑完（30 分钟+） |
| VS2026 下编整个解决方案有 13 个失败（Catch2 测试 / SlateViewer / AutoRTFMTests） | 引擎自带工具问题，**与项目无关**；右键只编 `Xuanming` 项目，自动按依赖带上 UnrealEditor |
| Target.cs 用 `BuildSettingsVersion.V5` 编 Editor 报 `XuanmingEditor modifies the values of properties` | UE 5.7 默认 V6，必须升级所有 4 个 Target.cs 到 V6 |
| V6 升级后 `error C4458: "Owner"的声明隐藏了类成员` | 局部变量名和 `AActor::Owner` 冲突；改为 `OwnerChar` 等明确名字 |

## Editor / 运行时

| 坑 | 解法 |
|---|---|
| 启动 Editor 崩在 `FDerivedDataBackendGraph` 行 217 | ZenServer HTTP 服务在 `[::1]` 起不来；用 `-ddc=NoZenLocalFallback` 禁掉 Zen，走文件系统 DDC。已封装在 `LaunchEditor.bat` 里 |
| 真 DS 模式 (LaunchServer.bat) WASD 不响应，PIE 多人正常 | `GlobalDefaultServerGameMode` 必须也指向 BP_GameMode；之前还是 C++ 裸类导致 DS 端 PC/Pawn 都是 C++ 裸类，蓝图层 IA 字段全 nullptr |
| EnhancedInput IMC 注册在 DS+Client 模式失败 | `BeginPlay` 时 ULocalPlayer 未就绪；改用 `AcknowledgePossession`（client）+ `OnPossess`（ListenServer）三时机兜底注册 |
| UE Editor 进程占用 .uasset/.umap 文件，git checkout 报 "Invalid argument" | 编辑器开着的时候 git 不能替换它锁着的文件。**解决**：完全关闭 Editor（不仅 PIE，整个 UnrealEditor.exe 进程都要关），再跑 git checkout / reset --hard。任务管理器确认无 UnrealEditor.exe 残留 |

## Python 编辑器脚本

| 坑 | 解法 |
|---|---|
| Python 创建的 UE 对象（modifier 等）保存后丢失 | `unreal.new_object(Class)` 默认 outer = `/Engine/Transient/`，序列化时被丢弃。**必须**传 `outer=父资产`，例如 `unreal.new_object(InputModifierNegate, outer=imc)`，对象才会作为父资产子对象持久化 |
| UE5 Manny 资产命名陷阱：`SK_Mannequin` 是 USkeleton，`SKM_Manny_Simple` 才是 USkeletalMesh | 反直觉但是 Epic 的命名约定。socket / 骨骼相关脚本要先 `isinstance` 守卫，挂错类型会一直走死路 |
| `USkeleton.Sockets` Python 读不到，报 "is protected and cannot be read" | UPROPERTY 没标 BlueprintReadOnly。**不要**直接操作 USkeleton.sockets，改走 `USkeletalMesh.AddSocket(socket, bAddToSkeleton=true)`——这条是 BlueprintCallable，bAddToSkeleton 会自动镜像进 Skeleton |
| `USkeletalMeshSocket` 所有字段 `BlueprintReadOnly`，Python 写不进 | `set_editor_property`、attribute 赋值都被拒，因为 Python 反射走 BP 可写性检查。**唯一解**：C++ 写一个 `UFUNCTION(BlueprintCallable)` 工具函数（如 `UXuanmingSocketTools::AddWeaponSocketToMesh`），裸字段赋值不走反射检查。同理适用于其他 BlueprintReadOnly + EditAnywhere 字段 |
| UE 5.7 `USkeletalMesh::AddSocket` 严格检查 socket outer 必须是 SkeletalMesh，否则 log error 但 void 返回不抛异常 | `NewObject<USkeletalMeshSocket>(SkeletalMesh)` outer 设 Mesh，传 `bAddToSkeleton=true` 让引擎内部自己复制一份到 Skeleton。**调用后必须**用 `NumSockets()` before/after 校验是否真加进去——UE 这种"void 返回但只 log error"的 API 必须用副作用校验做闸门 |
| Python `SceneComponent.set_relative_location(loc)` 报 `required argument 'sweep' (pos 2) not found` | 这是运行时 API，sweep/teleport 必填。**编辑 CDO 默认值**走 `set_editor_property("relative_location", ...)` / `set_editor_property("relative_scale3d", ...)`，符合"编辑默认值"语义 |
| UE 5.7 `WidgetBlueprint.WidgetTree` 是 protected 字段，Python `get_editor_property("WidgetTree")` 抛 "is protected and cannot be read" | 同 `USkeleton.Sockets` 那类反射坑。理论上可写 C++ `UXuanmingUITools::GetWidgetTree` 工具函数绕开（同 SocketTools 套路），但**UMG 控件树本来就是高频迭代品**（美术每天调样式），Epic 官方 *UMG Best Practices* 也明确推荐 Designer 手搭——脚本只建 WBP + 链 PC.HUDWidgetClass，控件树交给用户 5 分钟手点。这条"不修"是设计决策不是妥协 |

## FPS 输入 / 命中

| 坑 | 解法 |
|---|---|
| FPS 子弹用 ECC_Visibility 通道打不中 Pawn | UE5 Pawn 默认 collision profile 对 `ECC_Visibility` = **Ignore**（这样玩家不会挡住光照视线），导致 LineTrace ECC_Visibility 直接穿过 Character capsule。**FPS 子弹必须用 `ECC_Pawn`**（profile 默认 Block），不要用 ECC_Visibility。表现：视线点积 0.99 几乎完美对准，但 bHit=0 |
| Weapon::HandleFire client 端预写 LastFireTime 自卡死锁 | `HandleFire` 在 client 路径写 `LastFireTime=Now` 后同帧调 `Server_Fire` RPC，进入 `Server_Fire_Implementation` 的 `CanFire()` 检查 `Now-LastFireTime=0 < Interval` → 永远 reject。**修法**：client 端只做 UI 预测（CurrentAmmo--），CD 检查只在 server 上做，LastFireTime 只能由 server 在真实 fire 时写 |
| FPS 首次点击 viewport 镜头瞬间朝下脚下 | UE 5.7 PIE 模式下，HUDWidget AddToViewport 默认 IsFocusable=true 抢焦点 + 首次点击触发 mouse capture 重置，期间累积的 mouse delta（≈30）灌进 `Input_Look`，`AddControllerPitchInput` 被 clamp 到 -89° 即俯视脚下。**修法（4 层防御）**：(1) `HUDWidget->SetIsFocusable(false)` 阻止 UMG 抢焦点；(2) `SetInputModeGameOnly`（不要带 `SetConsumeCaptureMouseDown(true)`，那反而触发 capture 重置）；(3) ViewportClient 三件套：`CapturePermanently_IncludingInitialMouseDown` + `LockAlways` + `HideCursorDuringCapture`；(4) `FSlateApplication::SetAllUserFocusToGameViewport()` 提前给焦点。即使 4 层都做了，UE 5.7.4 仍可能分多帧灌 delta，所以再加 `LookConsumeFramesAfterModeSwitch=5` 兜底吞前 5 帧 Look 输入 |
| UE5 Character `FDamageEvent` 前置声明，cpp 用要 include | `Actor.h` 里 `TakeDamage` 用 `struct FDamageEvent const&` 前置声明，cpp 里要构造 `FDamageEvent DummyEvent` 必须 `#include "Engine/DamageEvents.h"`。错过会报 `error C2079: "DummyEvent" 使用未定义的 struct "FDamageEvent"` |

## MVVM / UMG

| 坑 | 解法 |
|---|---|
| UMG 函数 Binding 性能陷阱 | `HealthBar.Percent → Bind Function` 每帧 Tick 调用（10 个 binding × 60fps = 600 次/秒蓝图 VM）。**全项目禁用**，改 MVVM (`UMVVMViewModelBase` + `UE_MVVM_SET_PROPERTY_VALUE` 宏)，只在数据变化时广播 |
| MVVM `UFUNCTION(Exec)` 是 PIE 控制台调试 BlueprintCallable 的最干净方式 | 想在 PIE 控制台触发 BlueprintCallable，**不要用 `ke * FuncName Args`**——`ke` 只能调 BlueprintCallable，且对未注册的 actor 名 `0 instances succeeded`。**改用 `UFUNCTION(Exec)`**：函数自动注册为控制台命令，PIE 里直接输入 `XmDamageSelf 50` 调用，最稳。注意：Exec 只对 PlayerController/Pawn/Character 等 Outer Auto 类有效 |

## GAS

| 坑 | 解法 |
|---|---|
| GAS `InitAbilityActorInfo` 必须 server + client 双时机调 | ASC 挂在 PlayerState 上时：server 端 `PossessedBy` 后 PS 已就绪可初始化；client 端 PS 是网络复制过来的，必须 `OnRep_PlayerState` 触发再初始化一次。**只调一处**会导致一端 ASC.AvatarActor 为空，TryActivateAbility 静默失败 |
| GAS `GiveAbility` 只能在 server 调 | client 端调返回 0 但不报错，技能列表不会同步。`HasAuthority()` 守卫 + 一次性 `bAbilitiesGiven` 防 OnRep_PlayerState 反复 give |
| AttributeSet 的 `DOREPLIFETIME` 要用 `REPNOTIFY_Always` | GAS 推荐 `DOREPLIFETIME_CONDITION_NOTIFY(..., COND_None, REPNOTIFY_Always)`。普通 RepNotify 在新值==旧值时不广播，但 GAS prediction 回滚时旧值可能短暂相等导致 client 错过通知，HUD 表现为"扣血了但血条没刷" |
| GAS Damage 走 meta attribute 不直接复制 | 别让 client 看到 server 端的伤害数值（防作弊 + 节省带宽）。`Damage` 字段不进 GetLifetimeReplicatedProps，只在 server 的 `PostGameplayEffectExecute` 里读出来转成 `Health -= Damage`，再让 Health 复制下去 |
| AttributeSet 改 MoveSpeed 不会自动同步到 CharacterMovement | `FGameplayAttributeData` 只是数字，引擎不知道你想干啥。在 `PostGameplayEffectExecute` 里手动 `Movement->MaxWalkSpeed = BaseSpeed * GetMoveSpeed()`，否则冰咒 GE 应用了但角色照常跑。**而且** `PostGameplayEffectExecute` 只对 Instant GE 触发，Duration/Periodic GE **不走**这条路 → 见下一条 |
| GAS Duration / Periodic GE 改 attribute **不触发** `PostGameplayEffectExecute` | 只有 Instant GE 触发，Duration/Periodic 走 Aggregator 内部算，回调链路不一样。**唯一正确做法**：在 Character 端订阅 `ASC->GetGameplayAttributeValueChangeDelegate(Attr).AddUObject(this, &OnXxxChanged)`。这条委托对 server 命中 + client `OnRep_*`（`GAMEPLAYATTRIBUTE_REPNOTIFY` 宏内部触发）两端都会调到。表现：Slow GE 让 MoveSpeed 数字 1.0→0.5 但角色实际跑速没变。修法见 `XuanmingCharacter::BindAttributeDelegates` |
| UE 5.4+ GE 配标签用 `UTargetTagsGameplayEffectComponent`，不要用 `InheritableOwnedTagsContainer` | 5.3 起旧 API 标 deprecated，5.5+ 可能直接删除。新写法：`FindOrAddComponent<UTargetTagsGameplayEffectComponent>()` + `FInheritedTagContainer.Added.AddTag(...)` + `SetAndApplyTargetTagChanges(...)`。**但 5.7 在 GE 子类构造函数里调 `FindOrAddComponent` 会触发 fatal** → 见下一条 |
| UE 5.7 GE 构造函数里 `FindOrAddComponent<...>()` 触发 fatal `NewObject with empty name can't be used to create default subobjects` | `UGameplayEffect::AddComponent` 内部 `NewObject<T>(this, NAME_None, ...)`，5.7 严格模式禁止构造函数里裸 NewObject。**修法**：C++ 只配 Duration + Modifier，Granted/Asset/Target Tags 全部下沉到 BP 子类 `Details → Components → Target Tags Gameplay Effect Component → Inherited Tags Added` 配。这也是 Epic 5.4+ 推荐做法（策划/新人改标签不必动 C++） |
| GAS Cooldown GE 必须 Grant 至少一个 tag | 否则 asset validator 报 `grants no tags. ... must grant tags to be used as cooldown`。GA 通过 Cooldown GE 上的 GrantedTag 推断 CooldownTags 做查询。C++ 端 Tag 下沉到 BP 后，必须在 `BP_GE_*Cooldown` 的 Components 里配 Granted Tag |
| GAS `Multiplicitive` Modifier 是 `(1+Σ)` 不是 `Σ` | 想让 MoveSpeed × 0.5 必须传 -0.5（`1 + (-0.5) = 0.5`），传 0.5 实际是 ×1.5 加速。Epic 拼写错也没修。多个 Multiplicitive 同时挂是**相加再乘**，反直觉（两个 -0.5 = 1+(-1.0) = 0，速度归零；两个 0.5 = 1+1 = ×2.0 加速到 200%）。需要叠乘改用 `Final` 或多层 GE |
| GAS GE 子类构造函数里 `RequestGameplayTag` 时 tag 未注册会 ensure | 推荐 `Config/DefaultGameplayTags.ini` 里预注册（走 git diff 评审 + 拉代码自动生效），或 Tag Manager UI 加（Source=DefaultGameplayTags.ini，等价）。注意：ini 只是注册"tag 字典"，不影响"哪个 GE 授予哪个 tag"——后者必须在 GE 上配 Components |
| `GetAbilitySystemComponentFromActor` API 不存在 | 这是常见的记忆混淆。从任意 Actor 拿 ASC 用 `UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor)`（在 `AbilitySystemBlueprintLibrary.h`），内部先走 IAbilitySystemInterface 再 fallback 到 FindComponentByClass，最鲁棒 |
| 改了 .h 里的 UPROPERTY 后启动 DS 崩在 `FPackageFileSummary::Serialize` / `FBufferReaderBase` 越界 | `XuanmingServer.exe` 是独立 target，`BuildServer.bat` 编出来跟 Editor 不共享二进制。改 UPROPERTY/UFUNCTION 后必须重跑 `BuildServer.bat`，否则 server 加载新 .uasset 时反序列化对不上字段 |
