# Kingfisher

![image](https://github.com/kaydxh/kingfisher/blob/master/doc/image/kingfisher.jpeg)

[![build](https://img.shields.io/badge/build-standard-brightgreen.svg?style=flat-square)](./docs/build/build.md)
[![license](https://img.shields.io/badge/license-MIT-blue.svg?style=flat-square)](./LICENSE)
[![C++17](https://img.shields.io/badge/c%2B%2B-17-blue.svg?style=flat-square)]()

## 简介

Kingfisher 是一个用于在 Linux 平台上构建 C++ 微服务的基础库。它提供了完整的 WebServer 框架（同时支持 HTTP + gRPC）、中间件链、图像/视频处理、网络通信、配置管理等丰富的基础能力，可以快速构建高性能的服务端应用。

## 核心特性

- **WebServer 框架** — 基于 YAML 配置驱动，同时支持 HTTP（cpp-httplib）和 gRPC，对标 Go 微服务架构
- **中间件链** — HTTP / gRPC 双链路中间件，内置 RequestID、Recovery、Timer、InOutPrinter
- **gRPC Gateway** — HTTP/JSON → gRPC 代理网关，类似 Go 的 grpc-gateway
- **健康检查** — 内置 `/healthz`、`/readyz`、`/livez` 端点，支持自定义检查器
- **生命周期管理** — PostStartHook / PreShutdownHook + 优雅关闭
- **图像处理** — 基于 GraphicsMagick/OpenCV 的图像编解码、变换、滤镜、分析
- **视频处理** — 基于 FFmpeg 的视频编解码、滤镜、硬件加速
- **丰富的基础库** — 网络、线程池、协程、加密、编码、LRU 缓存、JSON 序列化等

## 项目结构

```
kingfisher/
├── base/                    # 基础库
│   ├── config/              #   配置解析（TOML / YAML）
│   ├── container/           #   容器（Buffer）
│   ├── core/                #   核心工具（Singleton, ScopeGuard, Noncopyable）
│   ├── crypto/              #   加密（MD5, SHA256, Hex）
│   ├── dump/                #   崩溃堆栈追踪
│   ├── encoding/            #   编码（Base64, UTF8）
│   ├── file/                #   文件操作
│   ├── lru/                 #   LRU 缓存
│   ├── net/                 #   网络
│   │   ├── event/           #     事件驱动（EventLoop, Channel）
│   │   ├── grpc/            #     gRPC 客户端
│   │   ├── http/            #     HTTP 客户端/拦截器链
│   │   ├── poller/          #     Epoll 轮询器
│   │   ├── socket/          #     Socket 封装
│   │   └── tcp/             #     TCP 服务端/客户端
│   ├── os/                  #   操作系统（信号处理, getwd）
│   ├── path/                #   路径操作
│   ├── pool/                #   对象池 / SDK 连接池
│   ├── random/              #   随机数生成
│   ├── serializer/          #   JSON 序列化（RapidJSON）
│   ├── stream/              #   流操作
│   ├── strings/             #   字符串操作
│   ├── sync/                #   同步原语（Mutex, Condition, WaitGroup）
│   ├── thread/              #   线程池 / 协程 / 阻塞队列
│   ├── time/                #   时间（Timestamp, Timer, TimeZone, Date）
│   └── uuid/                #   UUID 生成
├── pkg/                     # 高级功能包
│   ├── webserver/           #   WebServer 框架（HTTP + gRPC）
│   │   ├── config.*         #     YAML 配置驱动
│   │   ├── webserver.*      #     GenericWebServer 核心
│   │   └── controller/      #     内置控制器（date, healthz）
│   ├── middleware/           #   中间件
│   │   ├── api/             #     gRPC ApiGuard（RAII 中间件链）
│   │   ├── http-middleware/ #     HTTP 中间件（RequestID, Recovery, Timer, InOutPrinter, CleanPath）
│   │   ├── grpc-middleware/ #     gRPC 中间件（RequestID, Recovery, Timer, InOutPrinter）
│   │   └── brpc-middleware/ #     BRPC 中间件
│   ├── grpc-gateway/        #   gRPC Gateway（HTTP → gRPC 代理）
│   ├── cv/                  #   计算机视觉
│   │   ├── image/           #     图像处理（编解码、变换、滤镜、色彩空间、分析）
│   │   ├── video/           #     视频处理（FFmpeg 编解码、滤镜、硬件加速）
│   │   ├── geometry/        #     几何运算
│   │   └── vector/          #     向量运算
│   ├── database/            #   数据库（MySQL）
│   ├── file-transfer/       #   文件传输（支持代理、重试、负载均衡）
│   └── log/                 #   日志配置（glog）
├── test/                    # 单元测试（Google Test）
├── third_party/             # 第三方依赖
├── cmake/                   # CMake 构建配置
├── scripts/                 # 构建脚本
└── docs/                    # 文档
```

## 依赖

| 依赖库 | 版本 | 用途 |
|--------|------|------|
| [cpp-httplib](https://github.com/yhirose/cpp-httplib) | v0.14.0 | HTTP 服务器 |
| [gRPC](https://grpc.io/) | v1.31.1 | RPC 框架（可选） |
| [Protobuf](https://protobuf.dev/) | v3.6.1 | 序列化 |
| [glog](https://github.com/google/glog) | v0.0.6 | 日志 |
| [gflags](https://github.com/gflags/gflags) | v2.2.2 | 命令行参数 |
| [yaml-cpp](https://github.com/jbeder/yaml-cpp) | v0.0.8 | YAML 配置解析 |
| [Google Test](https://github.com/google/googletest) | — | 单元测试 |
| [GraphicsMagick](http://www.graphicsmagick.org/) | v1.3.35 | 图像处理（可选） |
| [OpenCV](https://opencv.org/) | v2/v4 | 计算机视觉（可选） |
| [FFmpeg](https://ffmpeg.org/) | v5.1.4 | 视频处理（可选） |
| [OpenSSL](https://www.openssl.org/) | v1.1.1 | 加密（可选） |
| [LevelDB](https://github.com/google/leveldb) | v1.19 | KV 存储（可选） |
| [MySQL Connector](https://dev.mysql.com/) | v6.1.11 | 数据库（可选） |
| [libcurl](https://curl.se/) | v7.77.0 | HTTP 客户端（可选） |

## 快速开始

### 编译

```bash
# 1. 初始化第三方依赖（自动解压分割文件）
make deps

# 2. 编译项目
make build

# 3. 运行测试
make test

# 产物输出到 output/ 目录:
#   output/bin/   — 可执行文件
#   output/lib/   — 静态/动态库
```

### 最小 WebServer 示例

**配置文件** `webserver.yaml`:

```yaml
web:
  bind_address:
    host: 0.0.0.0
    port: 10000
  grpc:
    timeout: 5s
  http:
    api_formatter: tcloud_api_v30
  shutdown_timeout_duration: 5s
  debug:
    enable_profiling: true
    disable_print_inoutput_methods: []
```

**C++ 代码**:

```cpp
#include "webserver/config.h"
#include "webserver/controller/date/date.h"
#include "webserver/controller/healthz/healthz.h"
#include "config/yaml/yaml.h"

int main() {
    // 1. 加载 YAML 配置
    YAML::Node yaml_data = kingfisher::yaml::GetYaml("webserver.yaml");

    // 2. 准备 WebHandler
    std::vector<std::shared_ptr<kingfisher::web::WebHandler>> handlers;
    handlers.push_back(std::make_shared<kingfisher::web::HealthzWebHandler>());
    handlers.push_back(std::make_shared<kingfisher::web::DateWebHandler>());

    // 3. 创建并启动 WebServer（YAML 配置驱动）
    kingfisher::web::ConfigOptions opts;
    opts.node = &yaml_data;
    opts.web_handlers = handlers;

    auto& ws = kingfisher::web::Config::NewConfig(opts).Complete().ApplyOrDie();

    // 4. 添加自定义 Hook
    ws.AddPostStartHook("my-hook", []() -> int {
        LOG(INFO) << "Server started!";
        return 0;
    });

    // 5. 添加自定义健康检查
    ws.AddReadyzChecker(
        std::make_shared<kingfisher::web::FuncHealthChecker>(
            "db-check", []() -> int { return 0; }));

    // 6. 阻塞运行（信号优雅关闭）
    ws.Run();
    return 0;
}
```

**测试服务**:

```bash
# 启动服务后测试
curl http://localhost:10000/api/date/now
# {"request_id":"952465EA-C181-894C-9F1E-44DDB8E7D42A","date":"2026-03-01 17:35:11"}

curl http://localhost:10000/healthz
# {"status":"ok","checks":[{"name":"ping","status":"ok"}]}
```

内置 HTTP 中间件链会自动为每个请求生成 `X-Request-Id`，并记录请求耗时日志：

```
[952465EA-C181-894C-9F1E-44DDB8E7D42A] GET /api/date/now, status: 200, resp_size: 89, cost: 0.083ms
```

### 自定义 WebHandler

```cpp
class MyHandler : public kingfisher::web::WebHandler {
 public:
  void SetRoutes(httplib::Server& http_server) override {
    http_server.Post("/api/v1/process", [](const httplib::Request& req,
                                           httplib::Response& resp) {
      // 业务逻辑
      resp.set_content(R"({"code":0,"message":"ok"})", "application/json");
    });
  }
};
```

### gRPC 中间件（ApiGuard）

在 gRPC service handler 中使用 `API_GUARD` 宏，一行代码启用完整的中间件链（RequestID → Recovery → Timer → InOutPrinter）：

```cpp
void MyService::DoSomething(::google::protobuf::RpcController* controller,
                            const MyRequest* req, MyResponse* resp,
                            ::google::protobuf::Closure* done) {
    API_GUARD;  // 自动启用 gRPC 中间件链
    // ... 业务逻辑 ...
}
```

## 中间件架构

```
┌─────────────────────────────────────────────────┐
│                   HTTP 请求                      │
├────────────────┬────────────────────────────────┤
│ pre_routing    │ RequestID（生成唯一请求ID）      │
│                │ CleanPath（路径规范化）           │
├────────────────┼────────────────────────────────┤
│ handler        │ 业务路由处理                     │
├────────────────┼────────────────────────────────┤
│ post_routing   │ Timer（计算耗时）                │
│                │ InOutPrinter（记录请求/响应日志） │
├────────────────┼────────────────────────────────┤
│ exception      │ Recovery（异常恢复）             │
└────────────────┴────────────────────────────────┘

┌─────────────────────────────────────────────────┐
│                   gRPC 请求                      │
├─────────────────────────────────────────────────┤
│ API_GUARD（RAII 风格中间件链）                    │
│  ├─ RequestID  — 提取/生成请求ID                 │
│  ├─ Timer      — 计时 + Metric                  │
│  └─ InOutPrinter — 打印请求/响应                 │
│  + try/catch   — Recovery 异常恢复               │
└─────────────────────────────────────────────────┘
```

## 编译选项

通过 CMake 选项控制可选功能的编译：

```bash
cmake .. -DENABLE_GRPC=ON         # 启用 gRPC 支持
cmake .. -DENABLE_FFMPEG=ON       # 启用视频处理
cmake .. -DENABLE_OPENCV=ON       # 启用 OpenCV
cmake .. -DENABLE_GRAPHICS_MAGICK=ON  # 启用 GraphicsMagick
cmake .. -DENABLE_BRPC=ON         # 启用 BRPC
cmake .. -DENABLE_CURL=ON         # 启用 libcurl
cmake .. -DENABLE_MYSQL=ON        # 启用 MySQL
cmake .. -DENABLE_LEVELDB=ON      # 启用 LevelDB
cmake .. -DENABLE_LIBCO=ON        # 启用协程库
```

## 许可证

本项目采用 [MIT License](./LICENSE) 许可证。