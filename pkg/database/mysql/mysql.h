#ifndef KINGFISHER_PKG_DATABASE_MYSQL_H_
#define KINGFISHER_PKG_DATABASE_MYSQL_H_

#include <mysql.h>

#include <mutex>
#include <string>

namespace kingfisher {
namespace db {

struct MysqlConfig {
  std::string host;
  unsigned int port;
  std::string data_name;
  std::string user_name;
  std::string password;
};

struct MysqlOptions {
  int max_conns = 100;
  int max_idle_conns = 10;
  int dial_timeout = 0;
  int read_timeout = 0;
};

class Mysql {
 public:
  Mysql(const MysqlConfig& conf, const MysqlOptions& opts);
  virtual ~Mysql();
  int Init();
  MYSQL* Connect();
  void Close();
  MYSQL* GetDatabase();

 private:
  bool inited_ = false;
  std::mutex init_mutex_;
  MysqlConfig conf_;
  MYSQL* mysql_ = nullptr;
  MysqlOptions opts_;
};

}  // namespace db
}  // namespace kingfisher

#endif
