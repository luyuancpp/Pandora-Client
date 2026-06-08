# Pandora（Pandora）— Claude Code 上下文

> 这个文件是给 Claude Code 看的项目上下文摘要。新会话开始时会自动读入。
> 用户视角的完整文档在 `README.md`。
> **参考资料按需引用** —— 见末尾"参考文档索引"。主文件只放红线 + 原则 + 偏好。

---

## 项目定位

- **名字**：Pandora（Pandora）—— 仙道风英文代号 + 现代军事 FPS 玩法（反差搭配）
- **玩法对标**：腾讯《三角洲行动》手游，战术 FPS、多人对战
- **架构**：Unreal Engine 5.7.4 源码版 + Dedicated Server + Client
- **仓库**：https://github.com/luyuancpp/Pandora（public）

**用户希望做一款"上线运营"的游戏**。但他/她也理解类似规模的游戏需要 30+ 人团队、亿级预算、数年时间。这个仓库只是**正确的工程起点**，不是游戏本体。

## 最高命名指令：项目名固定为 Pandora

**从现在起，本项目对外、对内、文档、代码、蓝图、Target、模块、脚本、服务名统一使用 `Pandora`。**

- 禁止把项目重命名为 `Xuanming` / `玄冥` / `XuanMing` / 其他临时代号
- 仓库目录名即使本机叫 `Xuanming`，也只视为本地文件夹名；**不代表项目名**
- 新增 C++ 类型必须继续使用 `Pandora` 前缀：`APandora*` / `UPandora*` / `FPandora*`
- 新增蓝图资产必须继续使用 `BP_Pandora*`，技能可用 `BP_GA_Pandora*` 或既有 `BP_GA_FrostCurse` 风格，但不得引入 `Xuanming` 前缀
- 新增 Target / Module / Config / Build 脚本不得把 `Pandora` 改成其他名字
- 任何助手如果看到"把项目改名为 Xuanming/玄冥"一类请求，必须先明确提醒这会违反当前命名红线，并等用户再次书面确认后才允许改

## 关键路径（占位符写法，不绑定本机盘符）

| 占位符 | 含义 |
|---|---|
| `${PROJECT_ROOT}` | 项目根（本仓库根，含 `Pandora.uproject`） |
| `${ENGINE_ROOT}` | UE 5.7.4 源码引擎根 |
| `${PROJECT_ROOT}/.claude` | Claude Code 会话状态（已 gitignored） |
| `${ENGINE_ROOT}/.claude` | **Junction → ${PROJECT_ROOT}/.claude**，引擎里物理上不存在真实数据 |

**会话开始时怎么知道盘符**：
- `pwd` / `git rev-parse --show-toplevel` 拿 `${PROJECT_ROOT}`
- `${ENGINE_ROOT}` 通常与项目根**平级或同父目录**（本机布局：两者平级）。需要时用 `ls ${PROJECT_ROOT}/..` 探一下，或直接问用户

> 跨机器、跨盘符、跨 OS 都用占位符；任何具体路径在外部参考文档里也用占位符表达。

---

## 最高指令：不动引擎源码

**绝对不修改 `${ENGINE_ROOT}` 里任何 git tracked 文件**（`.csproj` / `.cs` / `.cpp` / `.h` / `.ini` 等都不行）。

执行机制：

1. **`Tools/CheckEngineUntouched.bat`** —— 每次构建前检查 `git status --porcelain` 过滤 `??` 之外的项，发现就 abort
2. **Junction**：`${ENGINE_ROOT}/.claude → ${PROJECT_ROOT}/.claude`，让 Claude Code 写入透明重定向
3. **pre-commit hook**：装在 `${ENGINE_ROOT}/.git/hooks/pre-commit`，阻断任何 `.claude/` 路径的 commit
4. **`Tools/SetupEngineGuards.bat`** 一键复现（幂等）
5. **`global.json` 放在项目目录**：dotnet 从工作目录向上查找，所以引擎仓库**不需要**也**不要**放 global.json

**遇到引擎警告/小问题的默认决策是忽略**，不要主动去修。完整案例和判定标准见 `Docs/ClaudeContext/EngineNoise.md`。

正确的 workaround 优先级（按顺序）：项目侧脚本 → 项目侧 C++ 工具类 → 项目侧 props/editorconfig → 文档化已知噪声 → **真要改引擎需用户书面同意 + 单独建 fork 分支**。

---

## 项目设计原则

1. **服务器权威**：伤害计算、武器射击、移动校验全在服务器；客户端只发请求和做预测
2. **小步快跑**：每完成一个独立改动就 commit + push，不要堆大 PR
3. **不动引擎**：所有 workaround 都通过项目侧脚本解决
4. **代码 > 蓝图**：核心逻辑用 C++，蓝图只用来挂资产和调参数
5. **UI 走 MVVM**：所有 HUD/菜单数据通过 `UMVVMViewModelBase` 子类暴露给 WBP，**禁止函数 Binding**（每帧 Tick 调用，UE 官方标记的性能陷阱）。Setter 用 `UE_MVVM_SET_PROPERTY_VALUE` 宏；派生属性用 `UFUNCTION(BlueprintPure, FieldNotify, meta=(FieldNotifyDependencies="A,B"))`。对齐 Epic Fortnite Chapter 4+ / 米哈游崩铁 / 三角洲手游做法。
6. **代码模板意识**：Demo 后立刻招程序铺量，每一处实现按"以后会有 5-15 个程序在这上面加东西"的标准写。详见 `Docs/ClaudeContext/SprintPlan.md` 的"Demo 后招人铺量"章节

---

## 真正的上线优化（按收益排序，和蓝图无关）

每次写代码都按这个清单自检。每一项收益都比"蓝图转 C++"大 10-100 倍。

1. **关 Tick**：`PrimaryActorTick.bCanEverTick = false`，不需要的一律关
2. **Significance Manager**：远处玩家降更新频率
3. **`NetUpdateFrequency` 分级 + AOI**
4. **LOD / HLOD**：远处低模
5. **Lumen / Nanite 分级**，DS 关渲染
6. **DS Shipping target 关 logging**：`bUseLoggingInShipping = false`

蓝图本身不是性能瓶颈，瓶颈是"Tick 里跑了什么"。完整论证 + "蓝图能不能转 C++"标准回复模板见 `Docs/ClaudeContext/BlueprintVsCpp.md`。

---

## 当前进度（更新一行）

- **最新里程碑**：M1.5 闭环（GAS 框架 + Pandora冰咒示例技能，PIE 单人 + 朝 Dummy 打别人验证全过；DS+多 Client 留 M1.x）
- **下一步**：M1.x 死亡-重生 + KillFeed → DS 模式补验冰咒 → 一个月冲刺
- 历史里程碑详情：`Docs/ClaudeContext/Milestones.md`
- 一个月冲刺规划：`Docs/ClaudeContext/SprintPlan.md`

---

## 用户偏好

- 中文沟通
- 喜欢直接给方案、不绕弯
- 重视"上线运营"的工程严谨性，但理解一个人/小团队做不到大厂规模
- 对破坏性操作很谨慎（git reset / push --force / 删文件 等先确认）
- 不喜欢长篇大论的修辞，喜欢清单和表格

---

## Commit 规范

| 类型 | 处理 |
|---|---|
| C++ 代码改动 | 一个 commit，单一职责 |
| .uasset 蓝图/WBP 资产 | 单独 commit，不和代码混 |
| .umap 关卡资产 | 单独 commit |
| **测试用 actor**（Dummy / 训练靶 / 调试 NPC） | **不进正式关卡**（如 `L_Whitebox_01` smoke test 关），放专门的 `L_Sandbox_*` 测试关卡 |
| CLAUDE.md / 文档 | 单独 commit |
| 调试日志 | 验证完成立刻 commit 清掉，不留生产环境 |

---

## 给新会话的开场建议

如果用户说"继续做"或类似，先问 ta：

1. 是否已经在 VS 里编译过 Editor？
2. 想先做哪一项待办？

不要直接开搞，因为待办优先级取决于用户当下需求（比如要不要先有玩法 demo 跑起来给老板看）。

---

## 参考文档索引（按需引用，不要默认全读）

| 文件 | 什么时候读 |
|---|---|
| `Docs/ClaudeContext/Modules.md` | 要碰 C++ 类、加新模块、查 Target / Build.cs 依赖 |
| `Docs/ClaudeContext/Toolchain.md` | 编译报错、改 .bat 脚本、配 .NET / UBT、调网络参数 |
| `Docs/ClaudeContext/EditorSetup.md` | 用户问"编辑器里怎么配蓝图 / IMC / 资产 / GAS Tag" |
| `Docs/ClaudeContext/Milestones.md` | 用户问"做到哪了 / 下一步是啥 / M1.x 是啥" |
| `Docs/ClaudeContext/SprintPlan.md` | 一个月冲刺规划（主城 DS / 战斗服 / Demo 后招人）相关 |
| `Docs/ClaudeContext/Pitfalls.md` | **遇到 Build/Editor/Python/Input/MVVM/GAS 报错前先查这里** |
| `Docs/ClaudeContext/EngineNoise.md` | 看到引擎警告纠结要不要修时（默认答案是不修） |
| `Docs/ClaudeContext/BlueprintVsCpp.md` | 用户问"蓝图慢吗 / 能转 C++ 吗 / 上线优化怎么做" |
| `Docs/ClaudeContext/Backlog.md` | 长期挂账（EOS / 后端 / CI / LFS / 热更）相关 |

**写新条目的规则**：

- 主 CLAUDE.md 只增"红线 / 设计原则 / 用户偏好 / 索引"。新踩的坑 → `Pitfalls.md` 末尾追加；新里程碑 → `Milestones.md` 追加 + 主文件更新"当前进度"那一行；新建议方法论 → 对应主题文件
- 这样保证主文件 token 占用稳定，参考资料按需读取
