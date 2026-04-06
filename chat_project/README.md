# 简化版多人聊天系统

这是一个基于 C++ + Qt Widgets + SQLite + TCP 的课程作业演示级聊天系统，包含服务端和客户端两个程序。

## 主要功能

- 用户注册
- 用户登录
- 在线用户列表刷新
- 双击在线用户打开聊天窗口
- 多窗口私聊
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

### 数据库

- 使用 SQLite
- 表：`users`、`messages`

### Qt 图形界面

- 登录窗口
- 注册窗口
- 主窗口
- 多聊天窗口

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

## 运行顺序

1. 启动服务端 `chat_server`
2. 启动客户端 `chat_client`
3. 注册用户，例如 `alice`、`bob`
4. 分别登录
5. 双击在线用户开始聊天

## 测试建议

1. 检查 `logs/server.log` 是否写入
2. 检查 `database/chat.db` 是否生成
3. 检查 `history/` 是否生成本地聊天记录
4. 同时打开两个客户端互发消息
