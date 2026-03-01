#!/bin/bash
# install-hooks.sh - 安装 Git hooks 以实现自动化操作
#
# 使用方式 (在项目根目录执行):
#   bash scripts/install-hooks.sh
#
# 安装后的效果:
#   - git pull / git merge 后自动更新子模块、合并分割文件并解压
#   - git checkout 后自动合并分割文件并解压

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
HOOKS_DIR="$REPO_ROOT/.git/hooks"

echo "======================================"
echo " 安装 Git Hooks"
echo "======================================"

# --- post-merge hook ---
POST_MERGE="$HOOKS_DIR/post-merge"
cat > "$POST_MERGE" << 'EOF'
#!/bin/sh
command -v git-lfs >/dev/null 2>&1 || { echo >&2 "\nThis repository is configured for Git LFS but 'git-lfs' was not found on your path. If you no longer wish to use Git LFS, remove this hook by deleting the 'post-merge' file in the hooks directory (set by 'core.hookspath'; usually '.git/hooks').\n"; exit 2; }
git lfs post-merge "$@"

# 自动更新子模块并合并解压 third_party 中的分割文件
echo "[post-merge] 更新子模块..."
git submodule update --init --recursive

REPO_ROOT="$(git rev-parse --show-toplevel)"
SETUP_SCRIPT="$REPO_ROOT/third_party/setup.sh"
if [ -x "$SETUP_SCRIPT" ]; then
    echo "[post-merge] 执行 third_party/setup.sh ..."
    bash "$SETUP_SCRIPT"
fi
EOF
chmod +x "$POST_MERGE"
echo "[OK] post-merge hook 已安装"

# --- post-checkout hook ---
POST_CHECKOUT="$HOOKS_DIR/post-checkout"
cat > "$POST_CHECKOUT" << 'EOF'
#!/bin/sh
command -v git-lfs >/dev/null 2>&1 || { echo >&2 "\nThis repository is configured for Git LFS but 'git-lfs' was not found on your path. If you no longer wish to use Git LFS, remove this hook by deleting the 'post-checkout' file in the hooks directory (set by 'core.hookspath'; usually '.git/hooks').\n"; exit 2; }
git lfs post-checkout "$@"

# 自动更新子模块并合并解压 third_party 中的分割文件
REPO_ROOT="$(git rev-parse --show-toplevel)"
SETUP_SCRIPT="$REPO_ROOT/third_party/setup.sh"
if [ -x "$SETUP_SCRIPT" ]; then
    echo "[post-checkout] 执行 third_party/setup.sh ..."
    bash "$SETUP_SCRIPT"
fi
EOF
chmod +x "$POST_CHECKOUT"
echo "[OK] post-checkout hook 已安装"

echo ""
echo "======================================"
echo " 安装完成！"
echo " Git pull/merge/checkout 后将自动"
echo " 合并解压 third_party 中的分割文件。"
echo "======================================"
