# Claude Code 上下文索引

> CLAUDE.md 是主入口（短），这里是按需引用的参考资料。
> 主入口告诉 Claude "什么时候来这里查"。

| 文件 | 什么时候读 |
|---|---|
| `Modules.md` | 要碰 C++ 类、加新模块、查 Target 时 |
| `Toolchain.md` | 编译报错、改 .bat 脚本、配 .NET / UBT、调网络参数时 |
| `EditorSetup.md` | 用户问"编辑器里怎么配蓝图 / IMC / 资产"时 |
| `Milestones.md` | 用户问"做到哪了 / 下一步是啥 / M1.x 是啥"时 |
| `SprintPlan.md` | 一个月冲刺规划（主城 DS / 战斗服 / Demo 后招人）相关时 |
| `Pitfalls.md` | 遇到 Build/Editor/Python/Input/MVVM/GAS 报错前先查这里 |
| `EngineNoise.md` | 看到引擎警告纠结要不要修时（默认答案是不修） |
| `BlueprintVsCpp.md` | 用户问"蓝图慢吗 / 能转 C++ 吗 / 上线优化怎么做"时 |
| `Backlog.md` | 长期挂账（EOS / 后端 / CI / LFS / 热更）相关时 |

**写新条目的规则**：

- 主 CLAUDE.md 只放"红线 + 设计原则 + 用户偏好 + 索引"。任何"参考资料级"的内容都进这个目录。
- 新踩的坑 → `Pitfalls.md` 末尾追加
- 新里程碑完成 → `Milestones.md` 追加，主 CLAUDE.md 只动一行"当前进度"
- 新建议/方法论 → 优先放对应主题文件，主 CLAUDE.md 不扩
