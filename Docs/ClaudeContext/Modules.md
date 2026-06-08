# 已实现的 C++ 模块 + Target

## C++ 类清单

```
Source/Pandora/
├── PandoraGameMode             服务器权威规则、登入登出
├── PandoraGameState            全局状态（比赛计时，已 Replicate）
├── PandoraPlayerController     EnhancedInput 上下文注册 + HUD 创建 + ViewModel 注入
├── PandoraPlayerState          K/D/队伍数据（已 Replicate）+ ASC + AttributeSet (GAS 宿主)
├── PandoraCharacter            FPS 角色 + 移动/视角/跳跃/蹲伏/开火 + 血量同步 + GAS 接入
├── PandoraWeapon               hitscan 武器、Server RPC、Multicast 特效、散布、射速、弹匣
├── PandoraHUD                  Canvas 占位（M1.4 后已降级为 GameMode HUDClass 默认指派）
├── PandoraPlayerViewModel      MVVM ViewModel：Health/MaxHealth/CurrentAmmo/MagazineSize 字段
├── PandoraAttributeSet         GAS：Health/MaxHealth/Mana/MaxMana/MoveSpeed/Damage(meta)
├── PandoraFrostEffects         3 个 GE 子类：GE_FrostSlow / GE_FrostCost / GE_FrostCooldown
├── PandoraGA_FrostCurse        冰咒技能 GA（ServerOnly + InstancedPerActor）
└── PandoraSocketTools          编辑器工具：UFUNCTION(BlueprintCallable, WITH_EDITOR) 加 socket
```

所有网络相关类已配好 Replication，**服务器权威**设计。

## 4 个 Target

- `Pandora.Target.cs` (Game)
- `PandoraEditor.Target.cs`
- `PandoraServer.Target.cs` (Dedicated Server)
- `PandoraClient.Target.cs`

## Build.cs 模块依赖

`Source/Pandora/Pandora.Build.cs` PublicDependencyModuleNames：

```
Core / CoreUObject / Engine / InputCore / EnhancedInput
UMG / Slate / SlateCore
FieldNotification / ModelViewViewModel    （MVVM）
GameplayAbilities / GameplayTags / GameplayTasks   （GAS）
```

## Sprint 期模块边界规划（参考）

招人铺量阶段会拆成多个 Game Module，**M1.5 当前还没拆**。计划：

- `PandoraCore`（GameMode/PlayerState/Character 基类）
- `PandoraAbility`（GAS 派生）
- `PandoraUI`（MVVM ViewModels）
- `PandoraNet`（匹配服客户端、ClientTravel 封装）

详见 `SprintPlan.md` 的"Demo 后招人铺量"章节。