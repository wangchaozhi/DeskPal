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
struct PetProfile {
    QString id;
    QString name;
    QString type;      // "2d" or "3d"
    QString renderer;  // qml, png-sequence, gif, apng, svg, glb, gltf, quick3d
    QString source;    // 主资源入口
    QString basePath;  // 外部宠物目录
    QHash<QString, QString> actions;
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

2D PNG 序列帧：

```json
{
  "id": "cat_2d",
  "name": "Cat 2D",
  "type": "2d",
  "renderer": "png-sequence",
  "source": "idle",
  "actions": {
    "idle": "idle",
    "happy": "happy",
    "sleepy": "sleepy",
    "dragging": "dragging"
  }
}
```

3D GLB 模型：

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
  }
}
```

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

- 已内置 `Default 2D`、`Cat 2D`、`Robot 3D` 三个菜单项。
- 已支持扫描外部 `pet.json` 并加入 `PetCatalog`。
- 已保存和恢复当前宠物 ID。
- 已向 QML 暴露当前宠物的 `id`、`name`、`type`、`renderer`、`source` 和 `basePath`。
- QML 渲染层已按 `renderer` 分发到 `qml`、`svg`、`gif/apng`、`png-sequence`、`quick3d`、`glb/gltf`。
- `qml` 和 `quick3d` 支持加载外部 QML 组件，组件可选暴露 `pressed` 和 `action` 属性。
- `png-sequence` 会读取动作目录中的图片帧并循环播放。
- `glb/gltf` 已有基础模型入口，骨骼动画片段会在后续阶段接入。
- 已提供 `sample_svg_2d` 和 `sample_quick3d` 示例宠物包。

## 后续开发顺序

1. 让 `ActionController` 根据 `PetProfile::actions` 选择对应动作资源和动画片段。
2. 增加宠物资源校验、错误占位图和设置页预览。
3. 支持 3D 骨骼动画片段和动作过渡。
