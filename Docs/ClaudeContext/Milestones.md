# 里程碑进度

## 已完成

### M0 (2026-05-23/24)：DS 联机骨架已通

- ✅ M0.1-M0.3 工业级 Content 目录 + L_Whitebox_01 关卡 + Project Settings 默认地图
- ✅ M0.4 PIE 单机不崩
- ✅ M0.5 真 DS 进程在 0.0.0.0:7777 监听
- ✅ M0.6 2 个 Client 同时连入，GameMode::PostLogin 报 Total: 2

### M1.1 (2026-05-29)：白盒角色已就位

- ✅ Mannequin 资产复制到 `Content/Characters/Mannequins/` (49 个 .uasset, 93MB)
- ✅ ABP_Unarmed 编译通过 (141ms)
- ✅ BP_PandoraCharacter 蓝图：Mesh=SKM_Manny_Simple, AnimClass=ABP_Unarmed
  - Mesh Transform: Location=(0,0,-88), Rotation=(0,0,-90)（FPS 标准朝向）
  - Mesh.bOwnerNoSee=true（第一人称隐藏自己，业界惯例）
- ✅ BP_PandoraGameMode + BP_PandoraPlayerController 蓝图链路完整

### M1.2 (2026-05-29)：EnhancedInput 全部接通

- ✅ 5 个 IA：Move/Look (Axis2D) + Jump/Fire/Crouch (bool)
- ✅ IMC_Default 配 8 条 mapping，WASD 4 方向都对
- ✅ 用 `Tools/ConfigureInput.py` Python 脚本一键配置（避免手动配 Modifier 出错）

### M1.3 (2026-05-30)：BP_Weapon_AK + WeaponSocket 全链路通

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

### M1.4 (2026-06-01)：UMG HUD 走 MVVM 路线，端到端验证通过

自伤 + 打 Dummy 都扣血，HUD 实时刷新。

- ✅ C++ `APandoraHUD` 降级为占位（DrawHUD 清空，类保留作 GameMode HUDClass 默认指派）
- ✅ `Pandora.uproject` 启用 `ModelViewViewModel` 插件
- ✅ `Source/Pandora/Pandora.Build.cs` 加 `ModelViewViewModel` + `FieldNotification` 模块依赖
- ✅ C++ 新增 `UPandoraPlayerViewModel : UMVVMViewModelBase`：
  - 4 个 `FieldNotify` 字段：`Health` / `MaxHealth` / `CurrentAmmo` / `MagazineSize`
    （Setter 用 `UE_MVVM_SET_PROPERTY_VALUE` 宏 + `UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED` 显式广播派生属性）
  - 3 个 `FieldNotify` 派生 UFUNCTION：`GetHealthPercent` / `GetHealthText` / `GetAmmoText`
- ✅ C++ `APandoraPlayerController`：
  - `HUDWidgetClass` + `HUDWidget` + `PlayerViewModel` + `PlayerViewModelName` 字段
  - `TryCreateHUD` / `TryInjectViewModel` 复用 IMC "三时机兜底" 模式
  - `PlayerTick` 每帧 `PushStateToViewModel`（轮询 Pawn.Health/Weapon.CurrentAmmo 推到 ViewModel）
  - `ApplyFPSInputMode`：HUDWidget->SetIsFocusable(false) + InputModeGameOnly + ViewportClient capture 三件套
    + `LookConsumeFramesAfterModeSwitch=5` 兜底吞首次 mouse capture 多帧大 delta
- ✅ C++ `APandoraCharacter::PandoraDamageSelf(Amount)` UFUNCTION(Exec) —— PIE 控制台 `PandoraDamageSelf 50` 验证 MVVM
- ✅ Weapon C++ 修复：
  - `HandleFire` client 端不再预写 LastFireTime（修死锁）
  - `LineTrace` 改 ECC_Pawn（从 ECC_Visibility，修打不中 Character）
- ✅ `Content/UI/WBP_PlayerHUD`：Designer 手搭 4 控件 + MVVM `Window → View Bindings` 配 ViewModel + 3 条 Binding
- ✅ PIE 端到端验证：
  - 控制台 `PandoraDamageSelf 50` → HUD 100/100 → 50/50 ✓
  - 朝 Dummy 开火 → Health 100→75→50→25→0 → `Character died` ✓

**为什么走 MVVM 而不是函数 Binding**（决策记录）：
- 函数 Binding 是 UE 官方标记的"性能陷阱"——每帧 Tick 调用，10 个 binding × 60fps = 600 次/秒蓝图 VM 调用
- MVVM 走 `INotifyFieldValueChanged` 推送，**只在数据变化时广播**，静止时零开销
- Epic Fortnite Chapter 4+ / 米哈游崩铁 / 三角洲手游 UI 都是 MVVM
- 项目当前 0 个 WBP 无历史包袱，最佳切入点
- UE 5.7 ModelViewViewModel 插件虽 `IsBetaVersion=true` 但 5.1 引入 3 年已稳定

### M1.5 (2026-06-02)：GAS 框架 + Pandora冰咒示例技能 ✅ 闭环

C++ 落地 + 编辑器配置 + PIE 单端 / PIE 多人验证全过。DS+多 Client 验证留 M1.x（需先 `BuildServer.bat` 重编 server）。

- ✅ `Pandora.uproject` 启用 `GameplayAbilities` 插件
- ✅ `Source/Pandora/Pandora.Build.cs` 加 `GameplayAbilities` + `GameplayTags` + `GameplayTasks`
- ✅ C++ 新增 `UPandoraAttributeSet`（属性：Health/MaxHealth/Mana/MaxMana/MoveSpeed/Damage(meta)）：
  - `ATTRIBUTE_ACCESSORS` 宏批量生成 4 件套（GetXxxAttribute / GetXxx / SetXxx / InitXxx）
  - `DOREPLIFETIME_CONDITION_NOTIFY(..., REPNOTIFY_Always)` GAS 推荐复制策略
  - `PreAttributeChange` 钳制上下限；`PostGameplayEffectExecute` 把 Damage meta 转成 Health 扣减、MoveSpeed 同步到 `CharacterMovement.MaxWalkSpeed`、Health 镜像到 `Character.Health`（兼容旧 ViewModel/HUD 推送链路）
- ✅ `APandoraPlayerState` 实现 `IAbilitySystemInterface`，构造 `UAbilitySystemComponent`（Mixed 复制模式）+ `UPandoraAttributeSet`，`NetUpdateFrequency=100`
- ✅ `APandoraCharacter` 实现 `IAbilitySystemInterface`（转发 PlayerState 的 ASC）：
  - `PossessedBy`（server）+ `OnRep_PlayerState`（client）双时机调 `InitAbilityActorInfo`
  - server 端 `GiveAbility(StartupAbilities)`，幂等 `bAbilitiesGiven` 防重复
  - 新增 `OnHealthDepleted` 在 AttributeSet Health 归零时触发 `HandleDeath`
  - 新增 `IA_FrostCurse` UPROPERTY + `Input_FrostCurse_Started` 调 `ASC->TryActivateAbilityByClass`
- ✅ C++ 新增三个 GE 子类（C++ 只配 Duration + Modifier，Tag 下沉到 BP 子类）：
  - `UGE_FrostSlow`：Duration 3s，MoveSpeed Multiplicitive `-0.5`（实际 ×0.5），Granted Tag `State.Debuff.Slow` 在 `BP_GE_FrostSlow` 配
  - `UGE_FrostCost`：Instant，Mana -25
  - `UGE_FrostCooldown`：Duration 5s，Granted Tag `Cooldown.FrostCurse` 在 `BP_GE_FrostCooldown` 配
  - **不要**在 C++ 构造函数里调 `FindOrAddComponent<UTargetTagsGameplayEffectComponent>()`，UE 5.7 严格模式触发 fatal（详见 `Pitfalls.md`）
- ✅ C++ 新增 `UPandoraGA_FrostCurse`（NetExecutionPolicy=ServerOnly, InstancedPerActor）：
  - 默认 `CostGameplayEffectClass=GE_FrostCost`、`CooldownGameplayEffectClass=GE_FrostCooldown`、`SlowEffectClass=GE_FrostSlow`
  - `ActivateAbility`：CommitAbility（一次性 check + apply Cost+Cooldown）→ 从 Caster 眼睛 LineTrace（`ECC_Pawn`，与 Weapon 一致）→ 命中 Pawn 时 `MakeOutgoingSpec` + `ApplyGameplayEffectSpecToTarget`，DrawDebugLine 反馈
  - 用 `UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor)` 拿目标 ASC（**不是** `GetAbilitySystemComponentFromActor`，那个 API 不存在）
  - `bAllowSelfTarget` 调试 flag（默认 false，验证期勾上让 LineTrace 没命中也回退到 Caster）
- ✅ `APandoraCharacter::BindAttributeDelegates` 订阅 `OnGameplayAttributeValueChange(MoveSpeed)` → 同步��� `CharacterMovement->MaxWalkSpeed`（**Duration GE 不触发 PostGameplayEffectExecute**，必须走这条委托链路）
- ✅ 编辑器配置完成：
  - `Config/DefaultGameplayTags.ini` 注册 `State.Debuff.Slow` / `Cooldown.FrostCurse`
  - `Content/Input/IA_FrostCurse.uasset` 新建 + `IMC_Default` 加 Q 键映射
  - `Content/Abilities/Frost/` 新建 `BP_GE_FrostSlow` / `BP_GE_FrostCooldown` / `BP_GE_FrostCost` / `BP_GA_FrostCurse`，BP_GE_* 在 Components 配 Granted Tag
  - `BP_PandoraCharacter` 指派 `IA_FrostCurse` + `StartupAbilities[0]=BP_GA_FrostCurse`
  - `Content/Maps/Sandbox/L_Sandbox_01.umap` 新建测试关（含 PlayerStart + Dummy BP_PandoraCharacter）
- ✅ 验证通过路径：PIE 单人 + 自伤模式（`bAllowSelfTarget=true`）+ PIE 朝 Dummy 打别人，MoveSpeed 1.0→0.5、Mana 100→75、Cooldown 5s 阻拦、3s 后 MoveSpeed 自动恢复
- ⏳ DS + 2 Client 验证留 M1.x：需先 `BuildServer.bat` 重编 server（`PandoraServer.exe` 改 UPROPERTY 后必须重编，详见 `Pitfalls.md`）
- ⚠️ HUD 当前还显示 `Character.Health`（旧 ViewModel 链路），AttributeSet.Health 通过 `PostGameplayEffectExecute` 镜像到 `Character.Health` 保持兼容；后续 M1.x 可把 ViewModel 直接对接 AttributeSet（监听 `GetGameplayAttributeValueChangeDelegate`）做"一次到位"的 GAS HUD

## M1 剩余子里程碑（FPS PoC 路线图）

```
M1.x 死亡-重生系统（M1.5 后插入）         1-2 天
  - HandleDeath 禁 capsule collision + 隐藏/销毁 actor
  - GameMode 延迟重生 (3s) + 通知 PlayerState K/D
  - 现状: 已死 Character 还能被打、Health(after)=0 一直触发, 不影响 M1.4 验证但要修

M1.x KillFeed（M1.5 后插入）              1 天
  - GameMode 加 OnPlayerKilled 委托 + Multicast 广播 (Killer/Victim/Weapon)
  - WBP_KillFeed (VerticalBox 右上, 飘字 + 3s 淡出)

M1.x 测试关卡 L_Sandbox_01                0.5 天
  - 单独的 Dummy 训练场, 拖一排 BP_PandoraCharacter 当靶子
  - L_Whitebox_01 保持干净 (DS smoke test 用)
```

## M2-M5 后续大节点（M1 完成后规划，不到时不动）

- **M2 可内测**：匹配服 + 账号 + 反作弊 + 数据上报（1-2 月）
- **M3 可外测**：CI/CD + Linux DS + 大厅 + 商城 + 多地图（2-3 月）
- **M4 可上线**：性能调优 + 反外挂 + 风控 + 客服后台（3-6 月）
- **M5 运营**：赛季 + 活动 + 内容更新管线（持续）

详见 `SprintPlan.md` 一个月冲刺规划。