#!/usr/bin/env bash

# exit by command return non-zero exit code
set -o errexit
# Indicate an error when it encounters an undefined variable
set -o nounset
# Fail on any error.
set -o pipefail
# set -o xtrace

# if script called by source, $0 is the name of father script, not the name of source run script
ROOT_PATH=$(cd `dirname "${BASH_SOURCE[0]}"`;pwd)
CPUS=$(nproc)
DOWNLOAD_DIR=${ROOT_PATH}/.download
FFMPEG_PREFIX=${ROOT_PATH}/ffmpeg_build
FFMPEG_VERSION="5.1.3"
FFMPEG_NAME="ffmpeg-${FFMPEG_VERSION}.tar.bz2"
FFMPEG_TAR_PATH="${DOWNLOAD_DIR}/ffmpeg-${FFMPEG_VERSION}.tar.bz2"
FFMPEG_UNTAR_PATH="${DOWNLOAD_DIR}/ffmpeg-${FFMPEG_VERSION}"

function download_and_tar() {
  echo "downloading..."
  mkdir -p ${DOWNLOAD_DIR}
  if [[ ! -f "${FFMPEG_TAR_PATH}" ]]; then
    curl -s -L -o ${FFMPEG_TAR_PATH} "http://ffmpeg.org/releases/${FFMPEG_NAME}"
  fi
  mkdir -p ${FFMPEG_UNTAR_PATH}
  tar xvf "${FFMPEG_TAR_PATH}" -C ${DOWNLOAD_DIR}
}

function build() {
  cd "${FFMPEG_UNTAR_PATH}"
  PKG_CONFIG_PATH="${FFMPEG_PREFIX}/lib/pkgconfig" ./configure \
  --prefix="${FFMPEG_PREFIX}" \
  --bindir="${FFMPEG_PREFIX}/bin" \
  --extra-cflags="-fPIC -m64 -I${FFMPEG_PREFIX}/include -I/usr/local/cuda/include" \
  --extra-ldflags="-L${FFMPEG_PREFIX}/lib -L/usr/local/cuda/lib64" \
  --enable-gpl \
  --enable-shared \
  --enable-nonfree \
  --disable-debug \
  --enable-cuda \
  --enable-cuvid \
  --enable-nvenc \
  --enable-cuda-nvcc \
   --enable-pic
  --enable-decoder=h264_mediacodec \
  --enable-decoder=hevc_mediacodec \
  --enable-decoder=mpeg4_mediacodec \
  --enable-decoder=vp9_mediacodec \
  --enable-libx264 \
  --enable-libx265 

  make -j ${CPUS}
}

download_and_tar
build
