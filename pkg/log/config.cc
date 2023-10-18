#include "config.h"

#include <memory>

#include "butil/logging.h"
#include "config/yaml/yaml.h"
#include "core/singleton.hpp"

namespace kingfisher {
namespace log {

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

CompletedConfig& Config::Complete() {
  int completed_ret = LoadYaml();
  auto& completed_config =
      kingfisher::core::Singleton<CompletedConfig>::Instance();
  completed_config.Init(this, completed_ret);
  return completed_config;
}

CompletedConfig::CompletedConfig() {}

CompletedConfig::~CompletedConfig() {
  if (completed_ret_ == 0) {
#ifdef ENABLE_BRPC
#else
    google::ShutdownGoogleLogging();
#endif
  }
}

void CompletedConfig::Init(const Config* config, int completed_ret) {
  config_ = config;
  completed_ret_ = completed_ret;
}

int CompletedConfig::Apply() {
  if (completed_ret_ != 0) {
    return completed_ret_;
  }

  return Install();
}

int CompletedConfig::Install() {
#ifdef ENABLE_BRPC
#else
  return InstallGLog();
#endif

  return 0;
}

int CompletedConfig::installGLog() {
#ifdef ENABLE_BRPC
  google::InitGoogleLogging(config_->options_.app.c_str());

  auto& proto = config_->proto_.log();
  using namespace api::v1::viper::logs;

  FLAGS_log_dir = proto.filepath();
  FLAGS_max_log_size = proto.rotate_size();
  if (FLAGS_max_log_size <= 0) {
    FLAGS_max_log_size = 100;  // default 100MB
  }

  auto max_day = proto.max_age();
  if (max_day > 0) {
    google::EnableLogCleaner(max_day);
  }

  switch (proto.level()) {
    case Log_Level::Log_Level_trace:
    case Log_Level::Log_Level_debug:
    case Log_Level::Log_Level_info: {
      FLAGS_minloglevel = google::INFO;
      break;
    }
    case Log_Level::Log_Level_warn: {
      FLAGS_minloglevel = google::WARNING;
      break;
    }
    case Log_Level::Log_Level_error: {
      FLAGS_minloglevel = google::ERROR;
      break;
    }
    case Log_Level::Log_Level_fatal:
    case Log_Level::Log_Level_panic: {
      FLAGS_minloglevel = google::FATAL;
      break;
    }

    default:
      FLAGS_minloglevel = google::INFO;
      break;
  }

  return 0;
#else
  return -1;
#endif
}

#if 0
int CompletedConfig::installBLog() {
#ifdef ENABLE_BRPC
  auto& proto = config_->proto_.log();
  logging::LoggingSettings log_setting;
  log_setting.log_file = proto.filepath().c_str();
  log_setting.logging_dest = logging::LOG_TO_FILE;
  logging::InitLogging(log_setting);

  using namespace api::v1::viper::logs;

  switch (proto.level()) {
    case Log_Level::Log_Level_trace:
    case Log_Level::Log_Level_debug:
    case Log_Level::Log_Level_info: {
      logging::SetMinLogLevel(logging::BLOG_INFO);
      break;
    }
    case Log_Level::Log_Level_warn: {
      logging::SetMinLogLevel(logging::BLOG_WARNING);
      break;
    }
    case Log_Level::Log_Level_error: {
      logging::SetMinLogLevel(logging::BLOG_ERROR);
      break;
    }
    case Log_Level::Log_Level_fatal:
    case Log_Level::Log_Level_panic: {
      logging::SetMinLogLevel(logging::BLOG_FATAL);
      break;
    }

    default:
      logging::SetMinLogLevel(logging::BLOG_INFO);
      break;
  }

  return 0;
#else
  return -1;
#endif
}
#endif

}  // namespace log
}  // namespace kingfisher
