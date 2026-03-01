MAKEFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
PROJECT_ROOT_DIR := $(realpath ${MAKEFILE_DIR})
PKG_CONFIG_PATH := ${PROJECT_ROOT_DIR}/pkgconfig
SCRIPT_PATH := ${MAKEFILE_DIR}/script
TARGET := $(shell basename ${MAKEFILE_DIR})

# https://web.mit.edu/gnu/doc/html/make_8.html
_empty:=
# define space with $(_empty)
_space:= $(_empty) $(_empty)
define joinwith
  $(subst $(_space),$1,$(strip $2))
endef

.PHONY: all
all: deps generate
	@echo "${MAKEFILE_DIR}"
	@echo "${PROJECT_ROOT_DIR}"
	@echo "${TARGET}"

.PHONY: setup_third_party
setup_third_party:
	@echo "  >  设置 third_party: 合并分割文件并解压"
	@if [ -x "${PROJECT_ROOT_DIR}/third_party/setup.sh" ]; then \
		bash "${PROJECT_ROOT_DIR}/third_party/setup.sh"; \
	fi

.PHONY: deps
deps: setup_third_party
	@echo "  >  downloading deps library"

.PHONY: version
version:
	@bash -c "bash ${SCRIPT_PATH}/version.sh gitinfos"

.PHONY: build
build:
	@if [ ! -d "${PROJECT_ROOT_DIR}/build" ]; then \
		mkdir -p ${PROJECT_ROOT_DIR}/build && cd ${PROJECT_ROOT_DIR}/build && cmake ..; \
	fi
	@cmake --build ${PROJECT_ROOT_DIR}/build -- -j `nproc`

.PHONY: test
test: build
	@echo "  >  Running C++ tests"
	@cd ${PROJECT_ROOT_DIR}/build && ctest --output-on-failure

.PHONY: generate 
generate:
	@echo "  >  start to generate protocol buffers for target ${TARGET} in golang"
	@bash -c "bash  proto.gen.sh"
	@echo "  >  start to generate protocol buffers by cpp"
	@if [[ -d "${PROJECT_ROOT_DIR}/build" ]]; then rm -rf "${PROJECT_ROOT_DIR}/build"; fi
	@mkdir -p ${PROJECT_ROOT_DIR}/build; cd ${PROJECT_ROOT_DIR}/build; cmake .. 
	@cmake --build ${PROJECT_ROOT_DIR}/build -- -j `nproc`;
