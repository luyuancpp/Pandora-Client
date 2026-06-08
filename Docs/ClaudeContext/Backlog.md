# 长期挂账（用户自己做的事）

1. **EOS（Epic Online Services）公网匹配集成**：需注册 Epic 开发者拿 ProductID/SandboxID
2. **后端服务**：Go/Java 账号 + 匹配服
3. **CI/CD**：GitHub Actions 自动编译 DS
4. **商标查询**："Pandora"在中国商标网 + 域名注册
5. **美术外包对接**：M2 阶段换正式角色资产，按 SK_Mannequin 标准骨架做就零改动替换
6. **Git LFS**：`.gitattributes` 已配 `*.uasset filter=lfs`，但本机 git-lfs 未装。
   M2 前补装：`winget install GitHub.GitLFS` → `git lfs install` → `git lfs migrate import --include="*.uasset,*.umap"`
7. **热更管线**（M3 外测前做，**M1-M2 阶段不要碰**）：
   - 第 1 步 DataTable 远程下发（1 周）：武器属性/技能数值放 DataTable，启动从后端拉 JSON 覆盖
   - 第 2 步 Pak 热更资源（2-3 周）：UnrealPak 打 .pak + CDN（OSS/七牛）+ `FCoreDelegates::OnMountPak` 挂载，先热更贴图/音效，再热更 WBP
   - 第 3 步 WBP/蓝图热更（M3-M4）：活动界面、商城界面打可热更 Pak
   - **不做 Lua/脚本层**：对齐三角洲/PUBGM/和平精英的纯 Pak 方案。腾讯系 UE 项目全行业惯例，不要为"灵活性"叠 VM 拖累性能 + 反作弊
   - MVVM 架构天然支持 View 层 Pak 热更（WBP 在 Pak 里随便换），ViewModel 层（C++）跟版本走，不冲突

## 当前未解决问题

- 用户尚未在 VS 里编译 Editor（首次 10-30 分钟）
- 用户尚未在编辑器里完成 M1.5 GAS 配置（GameplayTags 注册 + IA_FrostCurse + StartupAbilities），见 `EditorSetup.md`
- 用户尚未做商标查询和域名抢注