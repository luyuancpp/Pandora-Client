# 一个月冲刺规划（2026-06 立项）

> 目标：**假登录 → 主城（大厅，可放技能）→ 组队匹配 → 战斗服**

## 关键架构决策：主城 / 大厅就是另一个 DS

用户明确强调：主城 DS 和战斗服 DS 是**两个独立进程、独立可执行文件、独立部署**，共用 C++ 代码但 GameMode 不同。**不要**把主城做成"客户端单机场景 + 列表面板"那种纯 UI 大厅。

| 服务器 | 进程 | 端口 | GameMode | 玩家上限 | 同步频率 | 技能行为 |
|---|---|---|---|---|---|---|
| 主城（Hub） | 独立 DS 进程 | 7778 | `PandoraHubGameMode` | **50 人**（甜区） | 10Hz + AOI 半径 50m | 表演型：放特效但**不**结算伤害（State.InHub tag 拦截 ApplyDamage） |
| 战斗服（Match） | 独立 DS 进程 | 7777 | `PandoraMatchGameMode` | **100 人/局**（同局，非同屏） | 60Hz + AOI + Significance | 完整 GAS 命中结算 |

**主城是真 DS 不是 UI 大厅** —— 用户视角进游戏立刻就在主城里看到其他 50 个真人玩家走动放技能聊天，组队从主城 P2P 邀请，匹配从主城排队，战斗结束 ServerTravel 回主城。这个体感对齐三角洲手游 / 原神蒙德城 / FF14 主城。

### 主城上限 50 的理由（不要轻易改）

- UE DS 在 16 人战斗 验证基础上跑 50 人静态/低频很轻松
- 太少（≤20）显得冷清；太多（≥100）AOI 复杂度暴涨且玩家停留久（带宽 × 时长 = 钱）
- 对标：三角洲手游主城同屏 ≈ 30-40，原神蒙德城 ≈ 20，FF14 主城 ≈ 100（端游另当别论）
- "主城能放技能"采用 **D1 表演型**（不打人），原神/三角洲都是这做法，性价比最高

### 战斗服 100 人警告

- 100 人**同局**（PUBGM 模式，4km 地图分散）可行；100 人**同屏**几乎无商业项目（Apex/三角洲都是 60）
- 单帧 Replication O(N²)：16 人=256，100 人=10000，**40 倍**
- 必须做 AOI + Significance Manager + NetUpdateFrequency 分级，否则带宽爆炸（5MB/s 出口）
- 若 M2 压测扛不住，**先砍到 60 对齐三角洲**，不要为了"100"硬撑

## 4 周排期

```
Week 1：假登录 + 主城骨架
  - 假登录：本地 JSON 模拟账号，UMG 登录界面 → 进 L_Hub_01
  - 不接真后端（M2 再做），但 PlayerState 字段按真账号设计（AccountId/Nickname/Avatar）
  - L_Hub_01 主城白盒（100m × 100m，含出生点、传送门、训练场区域）
  - PandoraHubGameMode：继承 GameMode，禁伤害结算（重写 ApplyDamage 或 GAS tag 拦截）
  - 主城 DS 跑通（沿用 LaunchServer.bat 框架，端口 7778 区分战斗服 7777）

Week 2：主城多人 + 表演技能
  - 主城承载 50 人压测（NetUpdateFrequency=10, AOI 50m）
  - 主城技能：复用 GAS，AbilityTags 加 State.InHub，GA::ActivateAbility 检测 tag 时跳过 ApplyDamage
  - 表演型 GE：只播 Cue（特效+音效）+ Multicast，不改 Health/Mana（或 Mana 仍扣，做"消耗感"）
  - AOI 调参 + 50 人压测复盘（带宽、tick 耗时、Replication 包大小）
  - ※ World Chat 不做：聊天系统涉及频道/屏蔽词/举报，一个月做不完，挪到 M2

Week 3：组队 + 匹配
  - 4 人组队：队长发起 → Invite RPC → 队员同意 → PartyComponent 挂 PlayerState
  - 匹配服骨架：单独进程（不是 UE DS，用 Go/C++ 写最简单 FIFO 队列即可）
  - 客户端轮询匹配状态 → 匹配成功 → 服务器返回战斗 DS 地址 → ClientTravel
  - 不做 MMR（M3 再做），先 FIFO 凑够人就开局

Week 4：战斗服 + 联调
  - PandoraMatchGameMode（继承现有 GameMode，开启伤害结算）
  - 战斗服压测：先 60 人，能稳过再加到 100（不行就停在 60）
  - 主城 ↔ 战斗服 ↔ 主城 闭环（战斗结束 ServerTravel 回主城）
  - Bug 修复 + Demo ��制 + commit
```

## 这个月**不**做的事（明确划线）

- ❌ 真账号系统（Week 1 用假登录占位，M2 做）
- ❌ World Chat / 聊天系统（涉及频道/屏蔽词/举报，挪到 M2）
- ❌ 反作弊（M2-M3 做）
- ❌ 热更（M3 前不碰，详见 `Backlog.md`）
- ❌ 排位/MMR（M3 做，先 FIFO）
- ❌ 商城/付费（M3 做）
- ❌ 美术正式资产（白盒贯穿整月，M2 换皮）

## Demo 后招人铺量（Sprint 完成后启动）

用户明确：**一个月 Demo 跑通后立刻招程序铺量**。这意味着 Sprint 不只是"给老板看的 demo"，更是"给未来同事看的代码模板"。Demo 阶段每一处实现都要按"以后会有 5-15 个程序在这上面加东西"的标准写，不是"先糊一个能跑就行"。

### 给招人铺量做的准备（Sprint 期间必须顺手做的事）

1. **代码注释**：所有 C++ 类（特别是 `PandoraHubGameMode` / `PandoraMatchGameMode` / `UPandoraAttributeSet` / `UPandoraGA_*`）必须有中文 class-level 注释，说清职责 + 为什么这么写
2. **模块边界**：尽早拆 Game Module，不要全部塞 `Source/Pandora/`。Sprint 至少拆出：
   - `PandoraCore`（GameMode/PlayerState/Character 基类）
   - `PandoraAbility`（GAS 派生）
   - `PandoraUI`（MVVM ViewModels）
   - `PandoraNet`（匹配服客户端、ClientTravel 封装）
   - 拆早不拆晚，后期拆要动 include 全网
3. **命名规范文档**：写 `Docs/CodingStyle.md`（500 字内）说清楚 `A/U/F/I/E` 前缀、子类用 `Pandora` 前缀、UFUNCTION 命名、UPROPERTY 分组。新人入职第一天看这个
4. **Sprint Week 4 末尾交付物**除了 demo 本身，还要：
   - `Docs/Onboarding.md`：新程序入职 day1 看的（环境搭建、首次编译、跑通 PIE / 真 DS）
   - `Docs/Architecture.md`：架构总览图（主城 DS / 战斗服 DS / 匹配服 / 客户端 四方关系）
   - 至少 3 个示例 PR（功能开发的标准模���）

### 招人后第一波（5-10 人）建议分工（不要现在动手）

| 方向 | 人数 | 主攻 |
|---|---|---|
| 玩法 / GAS | 2-3 | 武器扩展（栓狙/霰弹）、技能扩展、角色差异化 |
| 网络 / DS 优化 | 1-2 | AOI / Significance / 100 人压测 |
| 后端 / 匹配 / 账号 | 1-2 | M2 的真账号系统、MMR 匹配、数据上报 |
| UI / MVVM | 1 | 商城、好友、战绩 |
| 反作弊 | 0 → M3 招 | M2 阶段先用 EAC SDK 占位 |
| TA / 美术 | 0 → 美术外包对接 | 用户自己 + 外包 |

### Demo 期写代码的硬要求（落实到每个文件）

1. 所有 GameMode / GameState / PlayerController 类必须**只放骨架职责**，玩法逻辑下沉到 Component 或 Subsystem。理由：新人接手时改一个功能不应该改 GameMode（改 GameMode 影响面太大）
2. 数值参数全部 `UPROPERTY(EditDefaultsOnly, Category="...")`，不要硬编码。理由：策划/新人调参不应该改 C++
3. 所有 Subsystem 优先用 `UGameInstanceSubsystem` / `UWorldSubsystem`，不要塞 `AGameMode`。理由：Subsystem 是 UE5 推荐的"全局单例"，新人查文档就懂
4. 任何"临时方案"必须在代码里写 `// TODO(M2): xxx` 注释 + 同步加到 `Backlog.md`。理由：避免"临时变永久"