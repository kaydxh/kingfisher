#!/usr/bin/env bash

# exit by command return non-zero exit code
set -o errexit
# Indicate an error when it encounters an undefined variable
set -o nounset
# Fail on any error.
set -o pipefail
set -o xtrace

# 项目根目录
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${PROJECT_ROOT}"

# 创建必要的目录
mkdir -p ./scripts
mkdir -p ./pkg/cmake

# 下载 FindProtobuf.cmake
echo "Downloading FindProtobuf.cmake..."
bash -c "curl -s -L -H 'Cache-Control: no-cache' -o ./pkg/cmake/FindProtobuf.cmake https://raw.githubusercontent.com/kaydxh/golang/main/pkg/gocv/cgo/cmake/FindProtobuf.cmake"

# 查找 protoc 可执行文件
PROTOC=""
if [ -f "${PROJECT_ROOT}/third_party/protobuf-v3.6.1/bin/protoc" ]; then
    PROTOC="${PROJECT_ROOT}/third_party/protobuf-v3.6.1/bin/protoc"
elif [ -f "${PROJECT_ROOT}/third_party/protobuf/bin/protoc" ]; then
    PROTOC="${PROJECT_ROOT}/third_party/protobuf/bin/protoc"
elif [ -f "${PROJECT_ROOT}/third_path/protobuf/bin/protoc" ]; then
    PROTOC="${PROJECT_ROOT}/third_path/protobuf/bin/protoc"
else
    echo "Error: protoc not found. Please ensure protobuf is installed."
    exit 1
fi

echo "Using protoc: ${PROTOC}"
"${PROTOC}" --version

# 查找所有 proto 文件
PROTO_FILES=$(find pkg -name "*.proto" -type f | sort)

if [ -z "${PROTO_FILES}" ]; then
    echo "No proto files found in pkg directory"
    exit 0
fi

echo "Found proto files:"
echo "${PROTO_FILES}" | sed 's/^/  /'

# 为每个 proto 文件生成 C++ 代码
for PROTO_FILE in ${PROTO_FILES}; do
    echo ""
    echo "Processing: ${PROTO_FILE}"
    
    # 获取 proto 文件所在目录和文件名
    PROTO_DIR=$(dirname "${PROTO_FILE}")
    PROTO_NAME=$(basename "${PROTO_FILE}" .proto)
    PROTO_FILE_NAME=$(basename "${PROTO_FILE}")
    
    # 切换到 proto 文件所在目录，这样 protoc 就不会创建嵌套路径
    cd "${PROJECT_ROOT}/${PROTO_DIR}"
    
    # 输出目录：当前目录（即 proto 文件所在目录）
    OUTPUT_DIR="."
    
    # 构建 include 路径
    # 1. 项目根目录（用于 import 其他 proto 文件）
    # 2. 当前目录（proto 文件所在目录）
    INCLUDE_PATHS=(
        "-I${PROJECT_ROOT}"
        "-I."
    )
    
    # 如果 proto 文件在深层子目录中，添加父目录到 include 路径以便解析相对 import
    if [[ "${PROTO_DIR}" == pkg/*/* ]]; then
        PARENT_DIR=$(dirname "${PROTO_DIR}")
        INCLUDE_PATHS+=("-I${PROJECT_ROOT}/${PARENT_DIR}")
    fi
    
    # 生成 C++ 代码
    echo "  Output directory: ${PROTO_DIR} (current directory)"
    echo "  Include paths: ${INCLUDE_PATHS[*]}"
    echo "  Proto file: ${PROTO_FILE_NAME}"
    
    # 使用文件名（不带路径）作为 proto 文件参数，输出到当前目录
    "${PROTOC}" \
        --cpp_out="${OUTPUT_DIR}" \
        "${INCLUDE_PATHS[@]}" \
        "${PROTO_FILE_NAME}"
    
    # 切换回项目根目录
    cd "${PROJECT_ROOT}"
    
    if [ $? -eq 0 ]; then
        echo "  ✓ Generated ${OUTPUT_DIR}/${PROTO_NAME}.pb.h and ${OUTPUT_DIR}/${PROTO_NAME}.pb.cc"
    else
        echo "  ✗ Failed to generate code for ${PROTO_FILE}"
        exit 1
    fi
done

echo ""
echo "=== All proto files processed successfully ==="

# 编译go可以使用系统安装的protoc
#bash -c "curl -s -L -H 'Cache-Control: no-cache' -o ./scripts/proto-gen.sh https://raw.githubusercontent.com/kaydxh/golang/main/script/go_proto_gen.sh"
#bash scripts/proto-gen.sh -I .  --proto_file_path ./pkg  --with-cpp
