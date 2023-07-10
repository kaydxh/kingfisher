#!/usr/bin/env bash

# exit by command return non-zero exit code
set -o errexit
# Indicate an error when it encounters an undefined variable
set -o nounset
# Fail on any error.
set -o pipefail
set -o xtrace

# if script called by source, $0 is the name of father script, not the name of source run script
ROOT_PATH=$(cd `dirname "${BASH_SOURCE[0]}"`;pwd)
CPUS=$(nproc)
DOWNLOAD_DIR=${ROOT_PATH}/.download
FFMPEG_BUILD=${ROOT_PATH}/ffmpeg_build
FFMPEG_PACK=${ROOT_PATH}/pack/ffmpeg
FFMPEG_PREFIX=${FFMPEG_PACK}/include
FFMPEG_BIN=${FFMPEG_PACK}/bin
FFMPEG_VERSION="5.1.3"
FFMPEG_NAME="ffmpeg-${FFMPEG_VERSION}.tar.bz2"
FFMPEG_TAR_PATH="${DOWNLOAD_DIR}/ffmpeg-${FFMPEG_VERSION}.tar.bz2"
FFMPEG_UNTAR_PATH="${DOWNLOAD_DIR}/ffmpeg-${FFMPEG_VERSION}"
mkdir -p ${FFMPEG_BUILD}
mkdir -p ${FFMPEG_PREFIX}
mkdir -p ${FFMPEG_BIN}

function download_and_tar() {
  echo "downloading..."
  mkdir -p ${DOWNLOAD_DIR}
  if [[ ! -f "${FFMPEG_TAR_PATH}" ]]; then
    curl -s -L -o ${FFMPEG_TAR_PATH} "http://ffmpeg.org/releases/${FFMPEG_NAME}"
  fi
  mkdir -p ${FFMPEG_UNTAR_PATH}
  tar xvf "${FFMPEG_TAR_PATH}" -C ${DOWNLOAD_DIR}
}

function build_yasm() {
  echo "downloading yasm..."
  yasm_version="1.3.0"
  yasm_name="yasm-${yasm_version}"
  yasm_path="${DOWNLOAD_DIR}/${yasm_name}.tar.gz"
  if [[ ! -f "${yasm_path}" ]]; then
    curl -s -L -o ${yasm_path} "http://www.tortall.net/projects/yasm/releases/${yasm_name}.tar.gz"
  fi
  yasm_untar_path=${DOWNLOAD_DIR}/${yasm_name}
  mkdir -p ${yasm_untar_path}
  tar xvf "${yasm_path}" -C ${DOWNLOAD_DIR}

  cd ${yasm_untar_path}
  ./configure \
  --prefix="${FFMPEG_PREFIX}" \
  --bindir="${FFMPEG_BIN}"

  make -j ${CPUS}
  make install
}

function build() {
  cd "${FFMPEG_UNTAR_PATH}"
  PKG_CONFIG_PATH="${FFMPEG_PACK}/lib/pkgconfig" ./configure \
  --prefix="${FFMPEG_PREFIX}" \
  --bindir="${FFMPEG_BIN}" \
  --extra-cflags="-fPIC -m64 -I${FFMPEG_PACK}/include -I/usr/local/cuda/include" \
  --extra-ldflags="-L${FFMPEG_PACK}/lib -L/usr/local/cuda/lib64" \
  --enable-gpl \
  --enable-shared \
  --enable-nonfree \
  --disable-debug \
  --enable-cuda \
  --enable-cuvid \
  --enable-nvenc \
  --enable-cuda-nvcc \
  --enable-pic \
  --yasmexe=${FFMPEG_BIN}/yasm \
  --enable-decoder=h264_mediacodec \
  --enable-decoder=hevc_mediacodec \
  --enable-decoder=mpeg4_mediacodec \
  --enable-decoder=vp9_mediacodec \
  --enable-libx264 \
  --enable-libx265 

  make -j ${CPUS}
  make install
}

build_yasm
download_and_tar
build
