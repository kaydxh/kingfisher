#!/usr/bin/env bash

# exit by command return non-zero exit code
set -o errexit
# Indicate an error when it encounters an undefined variable
set -o nounset
# Fail on any error.
set -o pipefail
set -o xtrace

mkdir -p ./scripts
bash -c "curl -s -L -H 'Cache-Control: no-cache' -o ./pkg/cmake/FindProtobuf.cmake https://raw.githubusercontent.com/kaydxh/golang/main/pkg/gocv/cgo/cmake/FindProtobuf.cmake"
bash -c "curl -s -L -H 'Cache-Control: no-cache' -o ./scripts/proto-gen.sh https://raw.githubusercontent.com/kaydxh/golang/main/script/go_proto_gen.sh"
bash scripts/proto-gen.sh -I .  --proto_file_path ./pkg  --with-cpp 
