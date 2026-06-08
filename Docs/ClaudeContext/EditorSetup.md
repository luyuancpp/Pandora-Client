# 用户在编辑器里要做的事（C++ 类需要蓝图实例化）

Claude 无法替用户在编辑器里点鼠标。下面这些只能用户手动做。

## M1 基础链路（M1.1-M1.3 已完成）

1. **在 `Content/` 下右键创建蓝图**：
   - `BP_PandoraCharacter` 继承 `PandoraCharacter`
   - `BP_PandoraWeapon_AK` 继承 `PandoraWeapon`（脚本 `Tools/CreateWeaponBP.py` 已建）
   - `BP_PandoraPlayerController` 继承 `PandoraPlayerController`
   - `BP_PandoraGameMode` 继承 `PandoraGameMode`

2. **创建 EnhancedInput 资产**：
   - `IMC_Default`（InputMappingContext）
   - `IA_Move` / `IA_Look` / `IA_Jump` / `IA_Fire` / `IA_Crouch`（5 个 InputAction）
   - 在 IMC 里绑键：WASD→IA_Move、鼠标→IA_Look、Space→IA_Jump、鼠标左键→IA_Fire、Ctrl→IA_Crouch
   - 用 `Tools/ConfigureInput.py` 一键配置（避免手动配 Modifier 出错）

3. **配置默认值**：
   - `BP_PandoraPlayerController.DefaultMappingContext = IMC_Default`
   - `BP_PandoraCharacter` 指派 IA_* + `DefaultWeaponClass = BP_PandoraWeapon_AK`
   - `BP_PandoraGameMode.DefaultPawnClass = BP_PandoraCharacter` + `PlayerControllerClass = BP_PandoraPlayerController`
   - `Project Settings → Maps & Modes → Default GameMode = BP_PandoraGameMode`
   - `Project Settings → Maps & Modes → GlobalDefaultServerGameMode = BP_PandoraGameMode`（**别忘**，否则真 DS 模式 IA 字段全 nullptr）

4. **角色 Mesh 上加一个 `WeaponSocket`**（武器挂点），否则 `SpawnDefaultWeapon` 时挂载失败。
   - 用 `Tools/AddWeaponSocket.py` 一键加（已加在 SKM_Manny_Simple，hand_r 偏移 (10,4,-2) Yaw=90）

## M1.4 MVVM HUD

1. 启用 `ModelViewViewModel` 插件（`Pandora.uproject` 已开）
2. 用 Designer 在 `Content/UI/` 手搭 `WBP_PlayerHUD`：
   - 4 控件：HealthBar (ProgressBar) / HealthText / AmmoText / Crosshair (Image)
   - `Window → View Bindings`：
     - 创建 ViewModel，类型 `UPandoraPlayerViewModel`，名称 `PlayerViewModel`
     - HealthBar.Percent ↔ PlayerViewModel.HealthPercent
     - HealthText.Text ↔ PlayerViewModel.HealthText
     - AmmoText.Text ↔ PlayerViewModel.AmmoText
3. `BP_PandoraPlayerController.HUDWidgetClass = WBP_PlayerHUD`

> UMG 控件树为什么不脚本化：见 `Pitfalls.md` 关于 `WidgetTree` protected 字段那条 + 设计决策。

## M1.5 GAS（用户当前要做）

1. 在 VS / Editor 里编译 C++（首次新增 GAS 模块会重新生成 Project，5-10 分钟）
2. **GameplayTags 已由项目配置预注册**：
   - `State.Debuff.Slow`
   - `Cooldown.FrostCurse`
   - 配置文件：`Config/DefaultGameplayTags.ini`
   - 如果 Editor 已打开，重启 Editor 或在 Gameplay Tag Manager 里刷新后确认
   - 不要开 `bAllowGameplayTagsCreationByDefault=true`，否则 CI/team 拉代码后 tag 表会漂
3. `Tools/ConfigureInput.py` 已负责把 `IA_FrostCurse`（bool）映射到 Q 键；如果手动配，确认 `IMC_Default` 里有 `IA_FrostCurse -> Q`
4. BP_PandoraCharacter 指派：
   - `IA_FrostCurse = IA_FrostCurse`
   - `StartupAbilities[0] = UPandoraGA_FrostCurse::StaticClass()`（或 BP 子类）
5. 可选：建 BP 子类 `BP_GA_FrostCurse` 调参（射程、Cost 数值、SlowEffect 强度）
