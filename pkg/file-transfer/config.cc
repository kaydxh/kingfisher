#include "config.h"

#include <memory>
#include <sstream>

#include "config/yaml/yaml.h"
#include "core/singleton.hpp"
#include "ft.transfer.h"
#include "log/config.h"

namespace kingfisher {
namespace file {

int Config::LoadYaml() {
  if (options_.node != nullptr) {
    return kingfisher::yaml::UnmarshalProtoMessage(*options_.node, proto_);
  }

  return 0;
}

Config& Config::NewConfig(const ConfigOptions& opts) {
  auto& config = kingfisher::core::Singleton<Config>::Instance();
  config.options_ = opts;

  if (config.LoadYaml() != 0) {
    return config;
  }

  return config;
}

int Config::parseViper() {
  // auto& ft_proto = proto_.ft();
  return 0;
}

CompletedConfig& Config::Complete() {
  int completed_ret = LoadYaml();
  auto& completed_config =
      kingfisher::core::Singleton<CompletedConfig>::Instance();
  completed_config.Init(this, completed_ret);

  parseViper();
  return completed_config;
}

CompletedConfig::CompletedConfig() {}

CompletedConfig::~CompletedConfig() {}

void CompletedConfig::Init(Config* config, int completed_ret) {
  config_ = config;
  completed_ret_ = completed_ret;
}

FileTransfer& CompletedConfig::ApplyOrDie() {
  if (completed_ret_ != 0) {
    // return completed_ret_;
    LOG(FATAL) << "failed to init FileTransfer, err:" << completed_ret_;
  }

  if (!config_->proto_.filetransfer().enabled()) {
    LOG(FATAL) << "not enable FileTransfer";
  }

  return InstallOrDie();
}

FileTransfer& CompletedConfig::InstallOrDie() {
  auto& ft = kingfisher::core::Singleton<FileTransfer>::Instance();

  auto proto = config_->proto_.filetransfer();

  FileTransferOptions opts;
  opts.download_timeout_ms = proto.download_timeout();
  opts.upload_timeout_ms = proto.upload_timeout();
  opts.retry_times = proto.retry_times();
  opts.retry_interval = proto.retry_interval();

  int ret = ft.Init(opts);
  if (ret != 0) {
    LOG(FATAL) << "failed to init FileTransfer, err:" << ret;
  }

  return ft;
}

}  // namespace file
}  // namespace kingfisher
