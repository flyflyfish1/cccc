# 简化版多人聊天系统

这是一个基于 C++ + Qt Widgets + SQLite + TCP 的课程作业演示级聊天系统，包含服务端和客户端两个程序。
现在额外提供了一个浏览器演示入口：保留原有 Qt TCP 服务端，再通过 WebSocket 网关把聊天界面展示到网页中。

## 主要功能

- 用户注册
- 用户登录
- 在线用户列表刷新
- 双击在线用户打开聊天窗口
- 多窗口私聊
- 浏览器网页端登录、查看在线用户、聊天
- 服务器保存用户与消息到 SQLite
- 服务器写日志到 `logs/server.log`
- 客户端保存本地聊天记录到 `history/*.txt`
- 程序自动创建 `logs`、`history`、`config`、`database` 目录

## 技术点对应课程要求

### 文件 IO

- 服务端日志文件：`logs/server.log`
- 客户端本地聊天记录：`history/userA_userB.txt`
- 配置文件读取：`config/server.conf`

### 目录 IO

- 程序启动时自动创建 `logs`、`history`、`config`、`database`

### Makefile

- 根目录提供总 `Makefile`
- `client/` 和 `server/` 下各自提供 `Makefile`
- 实际构建使用 `qmake` 生成 `Makefile.generated`，再调用 `mingw32-make`

### 多线程

- 服务端每个客户端连接使用一个 `QThread` 子线程处理
- 客户端使用 Qt Socket 异步信号槽接收消息，避免界面阻塞

### 网络编程

- 使用 `QTcpServer` + `QTcpSocket`
- 默认端口 `8888`
- JSON 文本协议，每条消息一行
- 网页演示额外增加 `WebSocket` 网关，默认端口 `9999`

### 数据库

- 使用 SQLite
- 表：`users`、`messages`

### Qt 图形界面

- 登录窗口
- 注册窗口
- 主窗口
- 多聊天窗口

### 浏览器演示

- `web_gateway/`：Qt WebSockets 网关，把浏览器消息转发给原 TCP 服务端
- `web_client/`：静态网页客户端，可在浏览器里演示登录、在线列表和聊天

## Windows + Qt 编译运行

### 方法一：Qt Creator

1. 分别打开 `server/server.pro` 和 `client/client.pro`
2. 选择 Qt MinGW Kit
3. 先编译运行服务端，再运行多个客户端

### 方法二：命令行

在 Qt MinGW 命令行中进入 `chat_project`：

```bash
make
```

或分别编译：

```bash
cd server
make
cd ../client
make
```

网页网关单独编译：

```bash
cd web_gateway
make
```

## 运行顺序

1. 启动服务端 `chat_server`
2. 启动客户端 `chat_client`
3. 注册用户，例如 `alice`、`bob`
4. 分别登录
5. 双击在线用户开始聊天

## 浏览器演示运行方式

1. 启动 `server/release/chat_server.exe`
2. 启动 `web_gateway/release/chat_web_gateway.exe`
3. 在 `web_client/` 目录执行：

```bash
py -3 -m http.server 8080 --bind 127.0.0.1
```

4. 浏览器打开 `http://127.0.0.1:8080`
5. 页面里默认连接 `ws://127.0.0.1:9999`
6. 注册并登录多个账号后，即可在网页里演示聊天

### 一键启动

根目录已经提供：

- `start_web_demo.bat`
- `stop_web_demo.bat`

直接双击 `start_web_demo.bat` 即可自动完成：

1. 启动 TCP 服务端
2. 启动 WebSocket 网关
3. 启动本地网页静态服务
4. 自动打开浏览器到 `http://127.0.0.1:8080`

演示结束后，双击 `stop_web_demo.bat` 可关闭相关进程。

## 测试建议

1. 检查 `logs/server.log` 是否写入
2. 检查 `database/chat.db` 是否生成
3. 检查 `history/` 是否生成本地聊天记录
4. 同时打开两个客户端互发消息
5. 或者使用两个浏览器标签页/两个浏览器窗口测试网页端聊天
