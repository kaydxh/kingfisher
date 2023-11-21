#ifndef KINGFISHER_PKG_FILE_TRANSFER_CONFIG_H_
#define KINGFISHER_PKG_FILE_TRANSFER_CONFIG_H_

#include <google/protobuf/service.h>
#include <yaml-cpp/yaml.h>

#include "ft.pb.h"

namespace kingfisher {
namespace file {

class CompletedConfig;
class FileTransfer;

struct ConfigOptions {
  YAML::Node* node = nullptr;
};

class Config {
 public:
  static Config& NewConfig(const ConfigOptions& opts);

  int LoadYaml();
  CompletedConfig& Complete();

 public:
  go::pkg::filetransfer::FtConfig proto_;
  ConfigOptions options_;

 private:
  int parseViper();
};

class CompletedConfig {
 public:
  CompletedConfig();
  ~CompletedConfig();
  void Init(Config* config, int completed_ret);

  FileTransfer& ApplyOrDie();
  FileTransfer& InstallOrDie();

 private:
  Config* config_ = nullptr;
  int completed_ret_ = 0;
};

}  // namespace file
}  // namespace kingfisher

#endif
