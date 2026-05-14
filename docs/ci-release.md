# CI Release Guide

本文档说明 DeskPal 的跨平台 GitHub Actions 发布流程。

## 目标

CI 支持手动输入发布 tag，并自动完成以下工作：

- 构建 Windows x64 版本。
- 构建 macOS x64 版本。
- 构建 Linux x64 AppImage。
- 缓存 Qt 安装目录，减少重复下载时间。
- 上传构建产物为 GitHub Actions artifacts。
- 自动创建 tag。
- 自动创建或更新 GitHub Release。
- 将平台资产上传到对应 Release。

## 工作流文件

发布工作流位于：

```text
.github/workflows/release.yml
```

触发方式为 GitHub 网页手动触发：

```text
Actions -> Build and Release -> Run workflow
```

需要输入：

- `tag`：发布标签，例如 `v0.1.0`
- `prerelease`：是否标记为预发布版本

## 产物

成功发布后，Release 页面会包含以下资产：

```text
DeskPal-windows-x64.zip
DeskPal-macos-x64.zip
DeskPal-linux-x64.AppImage
```

Windows、macOS 和 Linux 分别使用以下部署方式：

- Windows：`windeployqt`
- macOS：`macdeployqt`
- Linux：`linuxdeploy` + `linuxdeploy-plugin-qt` 生成 AppImage

Linux 使用 `ubuntu-22.04` 构建 AppImage，以获得比更新系统镜像更好的运行兼容性。

## Qt 缓存

工作流使用 `actions/cache` 缓存 Qt 安装目录：

```text
${{ runner.temp }}/Qt
```

缓存 key 由平台、Qt 版本和 Qt 架构组成：

```text
qt-v2-${{ runner.os }}-${{ env.QT_VERSION }}-${{ matrix.qt_arch }}-${{ matrix.qt_dir }}
```

当缓存命中时，CI 会跳过 `aqtinstall` 的 Qt 下载步骤，只重新配置环境变量。

注意：`aqtinstall` 的下载架构名和实际安装目录名不一定相同。例如 Windows 下载参数是 `win64_msvc2022_64`，实际目录通常是 `msvc2022_64`；Linux 下载参数是 `linux_gcc_64`，实际目录通常是 `gcc_64`。

## Qt 版本

CI 当前使用：

```text
Qt 6.10.0
```

项目本地可以继续使用更高版本的 Qt，例如：

```text
E:\Qt\6.11.1\msvc2022_64
```

CMake 配置要求为 Qt 6.10 或更高。

## 推荐发布流程

1. 确认本地代码已经提交并推送到 GitHub。
2. 打开 GitHub 仓库的 Actions 页面。
3. 选择 `Build and Release`。
4. 点击 `Run workflow`。
5. 输入 tag，例如：

```text
v0.1.0
```

6. 等待三个平台构建完成。
7. 在 Releases 页面检查发布资产。
