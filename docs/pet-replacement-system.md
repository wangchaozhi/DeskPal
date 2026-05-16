# DeskPal 宠物替换系统

本文档描述 DeskPal 的可替换宠物方案。目标是让桌宠形象从固定 QML 组件升级为可扫描、可配置、可持久化选择的资源系统，同时继续复用统一动作控制。

## 目标

- 2D 宠物可替换为 PNG 序列帧、GIF/APNG、SVG 或 QML 组件。
- 3D 宠物可替换为 `.glb`、`.gltf` 或 Qt Quick 3D 组件。
- 后续可扩展骨骼动画、动作片段映射和资源预览。
- 托盘菜单提供 `Pet` 子菜单，用户可切换 `Default 2D`、`Cat 2D`、`Robot 3D` 等宠物。
- 当前宠物 ID 写入 `SettingsStore`，下次启动自动恢复。

## 模块职责

- `PetCatalog`：扫描和维护宠物列表，内置基础宠物，并读取外部 `assets/pets/*/pet.json`。
- `PetProfile`：描述单个宠物的 ID、名称、类型、渲染器、资源入口和动作资源映射。
- `SettingsStore`：保存当前宠物 ID，以及现有窗口、语言、置顶等设置。
- `TrayController`：生成 `Pet` 菜单，展示可用宠物并发出切换信号。
- `AppController`：协调当前宠物、渲染模式、设置持久化和 QML 属性暴露。
- QML `Loader`：根据当前宠物类型加载 2D 或 3D 渲染组件。
- `ActionController`：继续统一驱动 `idle`、`happy`、`sleepy`、`dragging` 等动作。

## PetProfile

`PetProfile` 是运行时使用的宠物配置结构，核心字段如下：

```cpp
struct Pet2DSpec {
    int fps = 0;       // png-sequence 帧率
};

struct PetView3D {
    qreal cameraDistance, cameraHeight, cameraPitch;
    qreal modelRotationX/Y/Z;
    qreal modelPositionX/Y/Z;
    qreal lightBrightness, lightPitch, lightYaw;
};

struct PetProfile {
    QString id;
    QString name;
    QString type;      // "2d" or "3d"
    QString renderer;  // qml, png-sequence, gif, apng, svg, glb, gltf, quick3d
    QString source;    // 主资源入口
    QString basePath;  // 外部宠物目录
    QHash<QString, QString> actions;
    QHash<QString, QString> animations;  // 3D 动作 → 动画片段名
    QStringList idleActions;             // 随机 idle 动作池
    QStringList speeches;                // 语音气泡文案
    int width = 0, height = 0;
    qreal scale = 1.0;
    Pet2DSpec pet2d;                     // 2D 专属字段（fps）
    PetView3D view3d;                    // 3D 舞台参数（相机/灯光/变换）
};
```

## 资源目录

外部宠物建议放在：

```text
assets/pets/
├─ sample_svg_2d/
│  ├─ pet.json
│  ├─ idle.svg
│  ├─ happy.svg
│  ├─ sleepy.svg
│  └─ dragging.svg
└─ sample_quick3d/
   ├─ pet.json
   └─ SampleQuick3DPet.qml
```

当前程序会尝试扫描以下位置：

```text
<应用目录>/assets/pets
<应用目录>/../assets/pets
<当前工作目录>/assets/pets
```

## pet.json 示例

2D PNG 序列帧（带 `pet2d` 嵌套块）：

```json
{
  "id": "cat_2d",
  "name": "Cat 2D",
  "type": "2d",
  "renderer": "png-sequence",
  "source": "idle",
  "pet2d": { "fps": 12 },
  "actions": {
    "idle": "idle",
    "happy": "happy",
    "sleepy": "sleepy",
    "dragging": "dragging"
  }
}
```

> 旧版 `pet.json` 在顶层写 `"fps": 12`，仍可被读取作为后备路径。新建宠物会写入 `pet2d.fps`。

3D GLB 模型（带 `view3d` 嵌套块和动画片段映射）：

```json
{
  "id": "robot_3d",
  "name": "Robot 3D",
  "type": "3d",
  "renderer": "glb",
  "source": "robot.glb",
  "actions": {
    "idle": "animations/idle",
    "happy": "animations/happy"
  },
  "animations": {
    "idle": "IdleClip",
    "happy": "HappyClip"
  },
  "view3d": {
    "cameraDistance": 360,
    "cameraHeight": 90,
    "cameraPitch": -12,
    "modelRotationY": 15,
    "lightBrightness": 1.4
  }
}
```

GLB/GLTF 运行时通过 `QtQuick3D.AssetUtils.RuntimeLoader` 加载，并在 `animations` 块匹配 Timeline 名后启用对应动画。`view3d` 字段对应配置面板里的"3D 舞台"控件，编辑时桌宠主体会通过 `setLiveView3d` 实时跟随预览。

QML 组件：

```json
{
  "id": "custom_qml_pet",
  "name": "Custom QML Pet",
  "type": "2d",
  "renderer": "qml",
  "source": "CustomPet.qml"
}
```

## 当前实现状态

- 内置 `Default 2D`、`Cat 2D`、`Robot 3D` 三项；外部 `pet.json` 会被自动扫描加入。
- 当前宠物 ID 持久化在 `SettingsStore`。
- QML 渲染层按 `renderer` 分发到 `qml`、`svg`、`gif/apng`、`png-sequence`、`quick3d`、`glb/gltf`。
- 配置面板可视化编辑 `Profile`、`2D Settings`、`3D Stage` 三段；`view3d` 实时驱动预览框和桌宠主体。
- GLB/GLTF 通过 `RuntimeLoader` 加载，Timeline 动画按 `animations` 块匹配启用。
- `validatePet` 输出 `errors`（找不到文件、范围非法等）与 `warnings`（未配置、未知动作引用），UI 分色显示。
- 新建宠物自动从 `sample_svg_2d` / `sample_quick3d` 模板复制资源；面板顶栏 `Generate Sample` 一键生成可编辑副本。
- 资源字段旁的 `…` 按钮调起 `FileDialog`/`FolderDialog`，复制文件到宠物目录并回填路径。
- 多语言支持 `system` / `en` / `zh_CN` / `ja_JP`，托盘菜单可切换。
- `Sleepy at night` 开关启用后，22:00–07:00 的随机 idle 选 `sleepy`。

## 后续方向

1. 时间触发更多动作映射（例如 11–13 点 happy，凌晨 dragging-disabled）。
2. 视觉编辑器：Source 字段旁加 `Browse...` 改成 OpenFile + 资源预览。
3. 3D 骨骼变形和动作过渡（fade-in / fade-out）。
4. 自定义动作种类（不限于 idle/happy/sleepy/dragging）。
