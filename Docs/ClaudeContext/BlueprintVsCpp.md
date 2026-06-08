# 蓝图 vs C++ 性能（用户问过，结论记牢）

> 用户疑问："蓝图效率比 C++ 差很多吗？上线时能不能转 C++？"

## 蓝图分两种用法，性能差异巨大

| 用途 | 性能损耗 | 项目里怎么用 |
|---|---|---|
| 只挂资产 + 改 Default 值（Mesh / 动画蓝图 / 武器 class / 血量上限） | **0**，运行时和 C++ default 完全相同 | ✅ 项目里 4 个蓝图都是这种 |
| EventGraph 写逻辑（Tick 里连节点算伤害） | VM 解释，比 C++ 慢 5-10 倍 | ❌ 不要这么做，逻辑全在 C++ |
| AnimGraph 动画蓝图 | 高度优化，C++ 写不会更快 | ✅ 必须用 |
| 蓝图调 C++ 函数（UFUNCTION BlueprintCallable） | 单次 ~200-300ns 额外开销 | 可忽略 |

**关键**：性能瓶颈不在"蓝图 vs C++"，在"**Tick 里跑了什么**"。项目里移动/网络/命中判定已经全在 C++，蓝图只挂资产 → 性能等价纯 C++。

## 蓝图转 C++（Nativization）这条路不存在

- UE 4.20~4.27 有 "Nativize Blueprint Assets" 选项
- **UE 5.0 官方废弃**，5.1+ 从 Project Settings 彻底删除
- 项目用的 UE 5.7.4 **没有这条路**，不要承诺用户能转
- Epic 废弃理由：转出来不可读、不可调试、收益 < 5%、Fortnite 自己都不用

## 真正的上线优化（按收益排序，和蓝图无关）

> **权威版在主 CLAUDE.md "真正的上线优化" 章节**，这里只做引用。改动在主文件改，避免漂移。

1. 关 Tick：`PrimaryActorTick.bCanEverTick = false`，不需要的一律关
2. Significance Manager：远处玩家降更新频率
3. `NetUpdateFrequency` 分级 + AOI
4. LOD / HLOD：远处低模
5. Lumen / Nanite 分级，DS 关渲染
6. DS Shipping target 关 logging：`bUseLoggingInShipping = false`

每一项收益都比"蓝图转 C++"大 10-100 倍。

## 给用户的标准回复模板

如果用户再问类似问题，直接答：

- 现在建的蓝图是"资产容器"，性能 0 损耗
- UE5 没有蓝图转 C++ 这条路，也不需要
- Fortnite/PUBG/原神都是 C++ 框架 + 蓝图配资产，能跑 120fps 不靠"全 C++"
- 真要优化，做 Tick/网络/LOD/Significance，不是改语言