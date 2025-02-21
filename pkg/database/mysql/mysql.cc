#include "mysql.h"

#include <iostream>

namespace kingfisher {
namespace db {
Mysql::Mysql(const MysqlConfig& conf, const MysqlOptions& opts)
    : conf_(conf), opts_(opts) {}

Mysql::~Mysql() {}

int Mysql::Init() {
  int ret = 0;
  if (!inited_) {
    init_mutex_.lock();
    if (!inited_) {
      ret = mysql_library_init(0, nullptr, nullptr);
      if (ret == 0) {
        inited_ = true;
      }
    }
    init_mutex_.unlock();
  }
  return ret;
}

MYSQL* Mysql::Connect() {
  if (mysql_) {
    return mysql_;
  }
  mysql_ = mysql_init(nullptr);
  if (mysql_ != nullptr) {
    mysql_options(mysql_, MYSQL_OPT_CONNECT_TIMEOUT,
                  (char*)&opts_.dial_timeout);
    mysql_options(mysql_, MYSQL_OPT_READ_TIMEOUT, (char*)&opts_.read_timeout);

    if (mysql_real_connect(mysql_, conf_.host.c_str(), conf_.user_name.c_str(),
                           conf_.password.c_str(), conf_.data_name.c_str(),
                           conf_.port, NULL, 0) == nullptr) {
      mysql_ = nullptr;
    }
  }

  return mysql_;
}

MYSQL* Mysql::GetDatabase() {
  if (mysql_ != nullptr) {
    return mysql_;
  }

  int ret = Init();
  if (ret != 0) {
    return nullptr;
  }
  return Connect();
}

int Mysql::Execute(const std::string& sql) {
  int ret = mysql_query(mysql_, sql.c_str());
  if (ret != 0) {
    std::cout << "exec sql err: " << ret << ", msg: " << mysql_error(mysql_);
  }
  return ret;
}

void Mysql::Close() {
  if (mysql_ != nullptr) {
    mysql_close(mysql_);
    mysql_ = nullptr;
  }
}

}  // namespace db
}  // namespace kingfisher
