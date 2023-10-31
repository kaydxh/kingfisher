#ifndef KINGFISHER_BASE_OS_SIGNAL_SIGNAL_H_
#define KINGFISHER_BASE_OS_SIGNAL_SIGNAL_H_

#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace kingfisher {
namespace os {

class SignalHandler {
 public:
  SignalHandler();
  ~SignalHandler();

  static int SetSignal(int signum, const std::function<void(int)>& cb);
  static int SetCoreDump(bool effect, int coreFileSize);
  static std::vector<int> StopSignals();

  static int InstallStopHandler(const std::function<void(int)>& handler);

 private:
  static void exitHandler(int signum);
  static void registerHandler(int signum,
                              const std::function<void(int)>& handler);

 private:
  static std::timed_mutex sigMutex_;
  static std::unordered_map<int, std::function<void(int)>> sigHandlers_;
};

}  // namespace os
}  // namespace kingfisher

#endif
