# DeskPal Claude Code 入口

请先阅读 `AGENTS.md`。该文件是本项目给 Codex、Claude Code 和其他 AI 编程助手的统一维护说明，包含项目结构、构建命令、宠物系统、CI 发布和开发约定。

关键提醒：

- 本项目是 Qt 6 / Qt Quick / C++20 桌面宠物应用。
- Windows 本地优先用 `run-vs-debug.bat` 验证。
- 不要无故运行 Release。
- 新增 C++ 或 QML 文件必须同步 `CMakeLists.txt`。
- 不要重置或覆盖用户未提交改动。
