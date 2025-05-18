# 🌱 植物大战僵尸（仿制版）

> 🎮 使用 C 开发的一款经典塔防游戏 —— 仿《植物大战僵尸》。玩家通过种植植物防御，阻止不断入侵的僵尸。

---

## 📌 项目简介

本项目是对《植物大战僵尸》的简化复刻，使用 C 实现基本的游戏逻辑，包括植物种植、阳光收集、僵尸移动与攻击、胜负判定等。适合用于学习 C 游戏开发、事件驱动编程、图像渲染等内容。
由于最终版未及时备份而导致丢失，只留下了这份半成品。

---

## 🧰 技术栈

- 编程语言：C
- 图形库：EasyX、 graphics.h
- 资源管理：Sprites（精灵）、音效、贴图等静态资源

---

## 🕹️ 游戏玩法

- 鼠标点击种植植物，消耗阳光点数
- 每种植物具备不同的攻击或功能属性
- 僵尸分批次进攻，若进入家园即游戏失败
- 持续击败僵尸直至关卡胜利

---

## 📁 项目结构

```
pvz-clone/
├── res/              # 图片、音效等资源
│   ├── audio/
│   ├── bullets/
│   ├── Cards/
│   ├── Map/
│   ├── ...
├── vector/           # 阳光运动函数
│   ├── vector2.cpp
│   └── vector2.h
├── tools.cpp         # 去除JPG、PNG背景黑框
├── tools.h
├── PlantsVSZombies.exe    # 游戏入口
└── README.md         # 项目说明
```
---

## 🚀 快速开始

1. **克隆仓库**
   ```bash
   git clone https://github.com/your_username/pvz-clone.git
   cd pvz-clone

2. **运行游戏**
    ```bash
    .\PlantsVSZombies.exe
