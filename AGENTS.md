# DeskPal AI 维护说明

这份说明给后续 Codex、Claude Code 或其他 AI 编程助手使用。请先阅读本文，再修改项目。

## 项目概览

DeskPal 是一个 Qt 6 / Qt Quick / C++20 桌面宠物应用。

核心能力：

- 无边框透明桌宠窗口
- 系统托盘菜单
- 多语言切换，当前包含英文和简体中文
- 2D / 3D 渲染模式
- 动作系统：`idle`、`happy`、`sleepy`、`dragging`
- 可替换宠物系统：内置宠物 + 外部 `assets/pets/*/pet.json`
- 宠物配置可视化面板：托盘菜单 `Pet Settings`
- 示例宠物包：`assets/pets/sample_svg_2d` 和 `assets/pets/sample_quick3d`
- GitHub Actions 手动发布跨平台资产

## 常用命令

Windows 本地开发优先使用 Visual Studio Debug 脚本：

```bat
run-vs-debug.bat
```

Release 构建脚本存在，但不要在没有明确需求时运行：

```bat
run-vs-release.bat
```

部署 Qt 运行时：

```bat
deploy-vs-debug.bat
deploy-vs-release.bat
```

如果本机找不到 CMake，可设置：

```bat
set "CMAKE_EXE=E:\Qt\Tools\CMake_64\bin\cmake.exe"
```

如果 Qt 不在默认目录，可设置：

```bat
set "QT_PREFIX=E:\Qt\6.10.0\msvc2022_64"
```

## 目录结构

```text
.
├─ main.cpp
├─ Main.qml
├─ CMakeLists.txt
├─ src/
│  ├─ AppController.*
│  ├─ ActionController.*
│  ├─ PetCatalog.*
│  ├─ SettingsStore.*
│  ├─ TrayController.*
│  └─ TranslationManager.*
├─ qml/
│  ├─ PetWindow.qml
│  ├─ PetBody.qml
│  ├─ PetModel3D.qml
│  ├─ PetAsset2D.qml
│  ├─ PetAsset3D.qml
│  └─ PetSequence2D.qml
│  └─ PetSettingsWindow.qml
│  ├─ PetPreviewPanel.qml
│  ├─ PetProfileEditor.qml
│  ├─ Pet2DEditor.qml
│  └─ Pet3DEditor.qml
├─ translations/
├─ assets/
└─ docs/
```

## 模块职责

- `AppController`：应用中枢，连接设置、托盘、翻译、动作、宠物目录和 QML。
- `ActionController`：统一驱动当前动作，负责随机 idle 行为和临时动作。
- `PetCatalog`：维护 `PetProfile` 列表，内置宠物并扫描外部宠物配置。
- `SettingsStore`：封装 `QSettings`，保存置顶、语言、渲染模式、当前宠物 ID 和窗口位置。
- `TrayController`：系统托盘菜单，包括显示/隐藏、宠物、渲染模式、语言、置顶、重置位置、退出。
- `TranslationManager`：运行时语言切换。
- `PetWindow.qml`：透明桌宠窗口、拖拽、右键菜单、点击动作。
- `PetAsset2D.qml`：2D 资源渲染分发。
- `PetAsset3D.qml`：3D 资源渲染分发。
- `PetSettingsWindow.qml`：宠物配置可视化面板外壳，负责列表、导入导出和保存流程。
- `PetPreviewPanel.qml`：宠物预览和动作预览按钮。
- `PetProfileEditor.qml`：通用资料编辑，如名称、渲染器、入口、窗口尺寸和缩放。
- `Pet2DEditor.qml`：2D 专属编辑，如帧率和动作资源。
- `Pet3DEditor.qml`：3D 专属编辑，如动作资源和动画片段映射，后续继续扩展相机/灯光/模型变换。

## 宠物系统

当前 `PetProfile` 字段：

```cpp
struct PetProfile {
    QString id;
    QString name;
    QString type;      // "2d" or "3d"
    QString renderer;  // qml, png-sequence, gif, apng, svg, glb, gltf, quick3d
    QString source;
    QString basePath;
    QHash<QString, QString> actions;
    QHash<QString, QString> animations;  // 3D 动作 -> 动画片段名
    QStringList idleActions;             // 随机 idle 动作池
    QStringList speeches;                // 语音气泡文案
    int width = 0;                       // 窗口宽度，0 表示用默认
    int height = 0;                      // 窗口高度，0 表示用默认
    int fps = 0;                         // png-sequence 帧率，0 表示用默认
    qreal scale = 1.0;                   // 渲染缩放
};
```

外部宠物目录：

```text
assets/pets/<pet_id>/pet.json
```

示例：

```json
{
  "id": "cat_2d",
  "name": "Cat 2D",
  "type": "2d",
  "renderer": "png-sequence",
  "source": "idle",
  "width": 220,
  "height": 240,
  "fps": 12,
  "scale": 1.0,
  "actions": {
    "idle": "idle",
    "happy": "happy",
    "sleepy": "sleepy",
    "dragging": "dragging"
  },
  "animations": {
    "idle": "Idle",
    "happy": "Happy"
  },
  "idleActions": ["happy", "sleepy"],
  "speeches": ["Hi there!", "Need a break?"]
}
```

`width`/`height`/`fps`/`scale`/`animations`/`idleActions`/`speeches` 均为可选字段。
配置面板可直接编辑外部宠物并写回 `pet.json`，内置宠物只读。

更多设计见：

- `docs/pet-replacement-system.md`
- `docs/desktop-pet-tech-plan.md`

## QML 渲染规则

`PetWindow.qml` 根据当前宠物类型选择：

- 2D：`PetAsset2D.qml`
- 3D：`PetAsset3D.qml`

`PetAsset2D.qml` 支持：

- `qml`
- `svg`
- `gif`
- `apng`
- `png-sequence`

`PetAsset3D.qml` 支持：

- `quick3d`
- `glb`
- `gltf`

外部 QML 组件如果希望接收桌宠状态，可以暴露：

```qml
property bool pressed
property string action
```

## 多语言

翻译文件在：

```text
translations/DeskPal_zh_CN.ts
```

新增用户可见字符串时，需要同步翻译。CMake 使用 `qt_add_translations`。

## 配置面板

配置面板从托盘菜单 `Pet Settings` 打开。

第一阶段已实现：

- 显示 `PetCatalog` 扫描到的宠物列表。
- 展示当前选中宠物的基础配置。
- 提供 2D/3D 预览区域。
- 支持切换当前宠物。
- 支持基础资源校验和动作预览。

开发清单见：

```text
docs/pet-settings-panel-checklist.md
```

## CI 与发布

工作流在：

```text
.github/workflows/release.yml
```

发布方式：

- 手动触发 `workflow_dispatch`
- 输入 tag
- 各平台构建尽量独立
- 某个平台失败不应阻止其他已构建资产上传 Release

当前发布目标包括：

- Windows MSVC 2022 x64
- Linux AppImage
- macOS x64 / arm64
- macOS DMG，并进行 ad-hoc 自签名

更多见：

```text
docs/ci-release.md
```

## 开发约定

- 使用 C++20。
- 优先 Debug 验证，不要无故跑 Release。
- 不要删除用户未提交改动。
- 不要重置 git 历史。
- 手动改文件优先使用补丁式修改。
- 新增源文件后必须同步 `CMakeLists.txt`。
- 新增 QML 文件后必须加入 `qt_add_qml_module` 的 `QML_FILES`。
- 新增托盘菜单文字时必须更新翻译文件。
- 修改 CI 前先读 `docs/ci-release.md` 和 `.github/workflows/release.yml`。

## 推荐验证流程

普通功能改动：

```bat
run-vs-debug.bat
```

如果程序启动后需要结束测试进程：

```powershell
Get-Process DeskPal -ErrorAction SilentlyContinue | Stop-Process
```

提交前检查：

```powershell
git status --short
git diff --stat
```

## 常见坑

- Windows 上需要 Qt `msvc2022_64`，MinGW 与 MSVC 不要混用。
- GitHub Actions Windows runner 不一定有指定 VS 版本，CI 里固定使用 `windows-2022` 和 VS2022 更稳。
- macOS 的 aqt 安装目录可能是 `Qt/<version>/macos`，不是旧的 `clang_64`。
- Linux 目标名不能和源码目录同名输出到根目录；当前 CMake 已将运行时输出到 `build/bin`。
- 3D 需要 `Qt6::Quick3D`，新增 3D 相关文件时不要漏掉模块依赖。
- 外部宠物资源路径通过 `AppController::resolvePetResource` 解析，不要在 QML 里手写绝对路径。

## 当前后续方向

已完成（截至 2026-05）：

- 外部宠物资源校验、缺失提示、错误与未配置警告分级显示。
- 配置面板：2D / 3D 编辑、`view3d` 嵌套块、`pet2d.fps` 嵌套块、保存到 `pet.json`。
- 动作到动画片段映射；GLB/GLTF 经 `QtQuick3D.AssetUtils.RuntimeLoader` 加载并按片段名启用 Timeline。
- 配置面板 3D 编辑实时驱动预览框与桌宠主体（`setLiveView3d` 通道）。
- 新建宠物从 `sample_svg_2d` / `sample_quick3d` 模板复制资源。
- 顶栏 `Generate Sample` 一键生成可编辑示例宠物副本。
- 资源字段 `…` 浏览按钮，`appController.importPetAsset` 复制并回填路径。
- 配置面板窗口位置 / 尺寸记忆。
- 桌宠：右键托盘 `Action` 子菜单直接触发动作；双击 / 中键 → sleepy；拖到屏幕边缘自动贴边或短距走过去。
- `SettingsStore` 增加 `nightSleepyEnabled`：22:00–07:00 把随机 idle 切到 sleepy。
- 多语言：英文 / 简体中文 / 日语；`TranslationManager` 系统语言自动落点支持 `zh` / `ja`。
- QML 卡片抽 `AccentCard` 组件，左侧彩色细条 + 统一内边距。
- `validatePet` 拆 `validateMeta` / `validateAssets` / `validateReferences`。
- `PetAsset3D` `RuntimeLoader` 动画 Timeline 缓存，避免 `applyClip` 重复遍历整棵场景。

建议优先级（新一轮）：

1. 自定义动作类型：解除 idle/happy/sleepy/dragging 四种固定名字的硬编码，让用户自由命名动作。
2. 桌宠定时提醒：番茄钟 / 喝水 / 久坐，结合语音气泡或托盘通知。
3. 多桌宠同屏 / 桌宠之间互动。
4. 状态过渡动画（动作切换 fade-in / fade-out）。
5. 自动化测试：用 `QTest` 锁住 `PetCatalog::readProfile` 兼容性和 `validatePet` 规则输出。
6. 裁小 Release 包体积（windeployqt 收紧 Quick3D 模块白名单）。
7. 日语翻译补齐（当前仅覆盖约 1/3 字符串）。
8. CI 自动收集 commits 生成 release notes（已部分完成）。
