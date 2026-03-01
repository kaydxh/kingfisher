# Kingfisher 编译指南

## 目录

- [1. 项目概述](#1-项目概述)
- [2. 环境要求](#2-环境要求)
- [3. 克隆仓库](#3-克隆仓库)
- [4. Third Party 子模块管理](#4-third-party-子模块管理)
- [5. 分割文件自动合并解压机制](#5-分割文件自动合并解压机制)
- [6. 编译构建](#6-编译构建)
- [7. Git Hooks 自动化](#7-git-hooks-自动化)
- [8. 常用 Makefile 目标](#8-常用-makefile-目标)
- [9. 常见问题与排查](#9-常见问题与排查)

---

## 1. 项目概述

Kingfisher 是一个基于 C++17 的项目，使用 CMake 作为构建系统，Makefile 作为构建入口。

项目核心目录结构如下：

```
kingfisher/
├── base/                   # 基础库
├── pkg/                    # 功能模块
├── test/                   # 测试代码
├── third_party/            # 第三方依赖库（Git 子模块）
│   ├── brpc_v0.9.6a/       #   brpc 库
│   ├── ffmpeg_v5.1.4/      #   ffmpeg 库
│   ├── yaml-cpp-v0.0.8/    #   yaml-cpp 库
│   ├── setup.sh            #   自动合并解压脚本
│   └── .gitignore          #   忽略解压产物
├── scripts/
│   └── install-hooks.sh    # Git hooks 安装脚本
├── cmake/                  # CMake 模块
├── CMakeLists.txt          # CMake 配置
├── Makefile                # 构建入口
└── .gitmodules             # 子模块配置
```

## 2. 环境要求

| 依赖项 | 最低版本 | 说明 |
|--------|---------|------|
| GCC/G++ | 支持 C++17 | 编译器 |
| CMake | 3.18.1+ | 构建系统 |
| Git | 2.x+ | 版本控制，需支持子模块 |
| Git LFS | 最新版 | 大文件支持 |
| Make | GNU Make | 构建入口 |
| bash | 4.x+ | 脚本执行 |

## 3. 克隆仓库

### 3.1 首次克隆（推荐）

使用 `--recurse-submodules` 选项，自动拉取 `third_party` 子模块：

```bash
git clone --recurse-submodules git@github.com:kaydxh/kingfisher.git
cd kingfisher
```

### 3.2 已克隆但未拉取子模块

如果克隆时未使用 `--recurse-submodules`，需手动初始化：

```bash
cd kingfisher
git submodule update --init --recursive
```

### 3.3 安装 Git Hooks

克隆完成后，执行以下命令安装 Git Hooks，实现 `git pull` 后的自动化操作：

```bash
bash scripts/install-hooks.sh
```

安装后效果：
- `git pull` / `git merge` 后自动更新子模块、合并分割文件并解压
- `git checkout` 后自动合并分割文件并解压

## 4. Third Party 子模块管理

### 4.1 子模块配置

`third_party` 目录作为 Git 子模块引入，指向独立的第三方依赖仓库：

- **远程仓库**：`git@github.com:kaydxh/third_party_cpp.git`
- **本地路径**：`third_party/`
- **配置文件**：`.gitmodules`

```ini
[submodule "third_party"]
    path = third_party
    url = git@github.com:kaydxh/third_party_cpp.git
```

### 4.2 使用子模块的优势

| 优势 | 说明 |
|------|------|
| 版本锁定 | 主仓库精确记录依赖的 commit，确保团队使用相同版本 |
| 仓库瘦身 | 主仓库不存储第三方库文件 blob，只保留 commit 引用 |
| 独立迭代 | `third_party_cpp` 可独立更新，kingfisher 按需升级 |
| 团队协作 | 所有人通过子模块拉取，确保依赖一致性 |

### 4.3 更新子模块到最新版本

```bash
cd third_party
git pull origin master
cd ..
git add third_party
git commit -m "chore: 更新 third_party 子模块到最新版本"
```

### 4.4 查看当前子模块状态

```bash
git submodule status
```

## 5. 分割文件自动合并解压机制

### 5.1 背景

`third_party` 中部分库的预编译二进制文件（`lib/` 目录）体积较大，无法直接上传到 Git 仓库。因此采用分割压缩上传的策略：

1. 将 `lib/` 目录打包为 `lib.tar.gz`
2. 使用 `split` 命令将 `lib.tar.gz` 分割为多个 10MB 的小文件（`lib.tar.gz.part_aa`, `lib.tar.gz.part_ab`, ...）
3. 分割文件上传到 Git 仓库

```bash
# 打包 & 分割（上传时使用）
tar -czf lib.tar.gz lib/
split -b 10M lib.tar.gz lib.tar.gz.part_

# 合并 & 解压（拉取后使用，由 setup.sh 自动执行）
cat lib.tar.gz.part_* > lib.tar.gz
tar -xzf lib.tar.gz
```

### 5.2 自动化脚本 `third_party/setup.sh`

`setup.sh` 脚本实现了自动扫描、合并和解压的功能：

**工作流程：**

```
扫描 third_party 下所有含 .part_* 文件的目录
        │
        ▼
  ┌─ 目标 lib/ 目录已存在且非空？
  │     是 → 跳过（幂等性保证）
  │     否 ↓
  │  cat *.part_* > lib.tar.gz  （合并分割文件）
  │     ↓
  │  tar -xzf lib.tar.gz        （解压）
  │     ↓
  │  rm lib.tar.gz               （清理临时文件）
  │     ↓
  └─ 完成
```

**关键特性：**
- **自动发现**：自动扫描所有包含 `.part_*` 分割文件的目录，无需手动指定
- **幂等性**：如果 `lib/` 目录已存在且非空，自动跳过，不会重复解压
- **错误处理**：合并或解压失败时记录错误并继续处理其他目录

**手动执行：**

```bash
cd third_party
bash setup.sh
```

**输出示例：**

```
======================================
 third_party 分割文件自动合并解压工具
======================================

--------------------------------------
[检查] ./brpc_v0.9.6a (分割文件: lib.tar.gz.part_*)
[合并] cat ./brpc_v0.9.6a/lib.tar.gz.part_* > ./brpc_v0.9.6a/lib.tar.gz
[解压] tar -xzf ./brpc_v0.9.6a/lib.tar.gz -C ./brpc_v0.9.6a
[完成] ./brpc_v0.9.6a/lib 解压成功。

--------------------------------------
[检查] ./ffmpeg_v5.1.4 (分割文件: lib.tar.gz.part_*)
[跳过] ./ffmpeg_v5.1.4/lib 已存在且非空，无需重新解压。

======================================
 处理完成: 成功=1, 跳过=1, 失败=0
======================================
```

### 5.3 `.gitignore` 配置

`third_party/.gitignore` 确保解压产物不被提交到 Git 仓库：

```gitignore
# 忽略合并解压后生成的 lib 目录
*/lib/

# 忽略合并后的临时 tar.gz 文件（仅保留 .part_* 分割文件）
lib.tar.gz
```

### 5.4 如何添加新的分割库

如果需要在 `third_party` 中添加新的预编译库，按以下步骤操作：

```bash
cd third_party

# 1. 创建库目录，将头文件和 lib 放入
mkdir -p new_lib_v1.0/include
cp -r /path/to/headers/* new_lib_v1.0/include/
cp -r /path/to/lib new_lib_v1.0/lib

# 2. 打包 lib 目录
cd new_lib_v1.0
tar -czf lib.tar.gz lib/

# 3. 分割为 10MB 的小文件
split -b 10M lib.tar.gz lib.tar.gz.part_

# 4. 删除原始文件（保留分割文件和头文件）
rm -rf lib/ lib.tar.gz

# 5. 提交
cd ..
git add new_lib_v1.0/
git commit -m "feat: 添加 new_lib_v1.0 预编译库"
```

## 6. 编译构建

### 6.1 完整编译流程

```bash
# 1. 准备依赖（自动执行 setup.sh 合并解压）
make deps

# 2. 完整编译（deps + generate）
make all
```

### 6.2 构建流程详解

```
make all
  │
  ├── make deps
  │     ├── make setup_third_party    ← 自动执行 third_party/setup.sh
  │     └── 下载其他依赖
  │
  └── make generate
        ├── 生成 protobuf Go 代码
        └── CMake 构建 C++ 代码
              ├── cmake ..              ← 配置阶段
              └── cmake --build . -j N  ← 编译阶段（N = CPU 核心数）
```

### 6.3 CMake 构建配置

项目使用 CMake 3.18.1+，关键配置：

- **C++ 标准**：C++17
- **构建类型**：Debug（默认）/ Release
- **输出目录**：`output/`
  - 头文件：`output/include/`
  - 可执行文件：`output/bin/`
  - 库文件：`output/lib/`
- **第三方库**：通过 `include_directories(third_party)` 引入
- **单元测试**：依赖 `third_party/gtest` 或 `third_party/googletest`

### 6.4 仅执行 CMake 构建

```bash
mkdir -p build && cd build
cmake ..
cmake --build . -- -j $(nproc)
```

## 7. Git Hooks 自动化

### 7.1 Hooks 概览

| Hook | 触发时机 | 功能 |
|------|---------|------|
| `post-merge` | `git pull` / `git merge` 后 | 更新子模块 → 执行 `setup.sh` |
| `post-checkout` | `git checkout` 后 | 执行 `setup.sh` |

### 7.2 post-merge hook 流程

```
git pull 完成
    │
    ▼
[post-merge hook]
    │
    ├── git lfs post-merge          ← Git LFS 处理
    │
    ├── git submodule update        ← 更新子模块到锁定版本
    │   --init --recursive
    │
    └── bash third_party/setup.sh   ← 合并分割文件并解压
```

### 7.3 安装 / 重新安装 Hooks

```bash
bash scripts/install-hooks.sh
```

> ⚠️ **注意**：`.git/hooks/` 目录不会被 Git 跟踪，每个开发者需要在首次克隆后执行一次安装脚本。

## 8. 常用 Makefile 目标

| 目标 | 命令 | 说明 |
|------|------|------|
| `all` | `make all` | 完整编译（deps + generate） |
| `deps` | `make deps` | 准备依赖（含 setup_third_party） |
| `setup_third_party` | `make setup_third_party` | 仅执行分割文件合并解压 |
| `generate` | `make generate` | 生成 protobuf 代码 + CMake 构建 |
| `version` | `make version` | 显示版本信息 |
| `test` | `make test` | 运行测试 |

## 9. 常见问题与排查

### Q1: `git clone` 后 `third_party` 目录为空

**原因**：克隆时未使用 `--recurse-submodules`。

**解决方案**：

```bash
git submodule update --init --recursive
```

### Q2: 编译时找不到 `lib/` 下的库文件

**原因**：分割文件未合并解压。

**解决方案**：

```bash
# 方法一：手动执行
bash third_party/setup.sh

# 方法二：通过 Makefile
make setup_third_party

# 方法三：确保已安装 hooks，执行 git pull 会自动触发
bash scripts/install-hooks.sh
```

### Q3: `setup.sh` 报错 "合并失败" 或 "解压失败"

**可能原因**：
- 分割文件不完整（下载中断或传输损坏）
- 磁盘空间不足

**排查步骤**：

```bash
# 检查分割文件完整性
ls -la third_party/brpc_v0.9.6a/lib.tar.gz.part_*

# 检查磁盘空间
df -h .

# 手动合并后验证
cd third_party/brpc_v0.9.6a
cat lib.tar.gz.part_* > lib.tar.gz
file lib.tar.gz          # 应显示 gzip compressed data
tar -tzf lib.tar.gz      # 列出压缩包内容
```

### Q4: 需要重新解压某个库

```bash
# 删除已有的 lib 目录后重新执行
rm -rf third_party/brpc_v0.9.6a/lib
bash third_party/setup.sh
```

### Q5: 如何向 `third_party` 添加新的依赖库

参考 [5.4 如何添加新的分割库](#54-如何添加新的分割库) 章节。

### Q6: `git pull` 后没有自动执行解压

**原因**：未安装 Git Hooks。

**解决方案**：

```bash
bash scripts/install-hooks.sh
```

---

> 📅 文档最后更新：2026-03-01
