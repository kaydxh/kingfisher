#!/usr/bin/env bash

# exit by command return non-zero exit code
set -o errexit
# Indicate an error when it encounters an undefined variable
set -o nounset
# Fail on any error.
set -o pipefail
set -o xtrace

mkdir -p ./scripts
# 编译cpp proto使用third party中的protoc 版本
bash -c "curl -s -L -H 'Cache-Control: no-cache' -o ./pkg/cmake/FindProtobuf.cmake https://raw.githubusercontent.com/kaydxh/golang/main/pkg/gocv/cgo/cmake/FindProtobuf.cmake"

# 编译go可以使用系统安装的protoc
#bash -c "curl -s -L -H 'Cache-Control: no-cache' -o ./scripts/proto-gen.sh https://raw.githubusercontent.com/kaydxh/golang/main/script/go_proto_gen.sh"
#bash scripts/proto-gen.sh -I .  --proto_file_path ./pkg  --with-cpp 
