# Pandora Linux Dedicated Server 部署指南

## 1. 本地打包（Windows 上交叉编译）

### 1.1 安装 Linux 交叉编译工具链

UE 5.7 推荐版本：**v23 Clang 18.1.0**

下载：https://dev.epicgames.com/documentation/zh-cn/unreal-engine/linux-development-requirements-for-unreal-engine

安装后，**重要**：检查环境变量 `LINUX_MULTIARCH_ROOT` 已经设置，例如：
```
LINUX_MULTIARCH_ROOT=C:\UnrealToolchains\v23_clang-18.1.0-rockylinux8\
```

打开新的命令行 `echo %LINUX_MULTIARCH_ROOT%` 应该有值。

### 1.2 打包

```cmd
F:\work\Pandora\BuildLinuxServer.bat
```

成功后产物位置：
```
F:\work\Pandora\Build\LinuxServer\LinuxServer\
├── PandoraServer.sh            ← 启动脚本
├── PandoraServer               ← ELF 二进制（无后缀）
├── Engine/                      ← 引擎运行时
└── Pandora/                    ← 项目内容
    └── Content/Paks/*.pak
```

## 2. 部署到 Linux 服务器

### 2.1 准备 Linux 系统

推荐：**Ubuntu 22.04 LTS** 或 **Rocky Linux 8/9**

```bash
# 安装必要依赖
sudo apt update
sudo apt install -y libc++1 libc++abi1 libsdl2-2.0-0 libvulkan1

# 创建专用用户
sudo useradd -m -s /bin/bash pandora
sudo mkdir -p /opt/pandora
sudo chown pandora:pandora /opt/pandora
```

### 2.2 上传

```bash
# 在你的开发机执行
scp -r F:/work/Pandora/Build/LinuxServer/LinuxServer/* pandora@<server-ip>:/opt/pandora/
```

### 2.3 启动

```bash
ssh pandora@<server-ip>
cd /opt/pandora
chmod +x PandoraServer.sh PandoraServer
./PandoraServer.sh -log -port=7777
```

## 3. 用 systemd 守护进程（生产环境）

创建 `/etc/systemd/system/pandora.service`：

```ini
[Unit]
Description=Pandora Dedicated Server
After=network.target

[Service]
Type=simple
User=pandora
WorkingDirectory=/opt/pandora
ExecStart=/opt/pandora/PandoraServer.sh -log -port=7777
Restart=on-failure
RestartSec=5s
StandardOutput=append:/var/log/pandora/server.log
StandardError=append:/var/log/pandora/server.err

[Install]
WantedBy=multi-user.target
```

启用：
```bash
sudo mkdir -p /var/log/pandora
sudo chown pandora:pandora /var/log/pandora
sudo systemctl daemon-reload
sudo systemctl enable pandora
sudo systemctl start pandora
sudo systemctl status pandora
```

查看日志：
```bash
sudo journalctl -u pandora -f
tail -f /var/log/pandora/server.log
```

## 4. 防火墙

```bash
# Ubuntu (ufw)
sudo ufw allow 7777/udp

# CentOS / Rocky (firewalld)
sudo firewall-cmd --permanent --add-port=7777/udp
sudo firewall-cmd --reload
```

云服务商（阿里云/腾讯云/AWS）还需要在控制台**安全组**放行 UDP 7777。

## 5. 客户端连接

```cmd
# Windows 客户端连接公网服务器
PandoraClient.exe <server-ip>:7777 -game
```

## 6. 多服节点架构（后期规划）

```
┌──────────────┐
│ Game Client  │
└──────┬───────┘
       │
       ▼
┌─────────────────┐      ┌──────────────────┐
│ Matchmaking     │─────►│ Battle Server #1 │ (7777)
│ Service         │      ├──────────────────┤
│ (你自己写后端)  │─────►│ Battle Server #2 │ (7778)
└─────────────────┘      ├──────────────────┤
                         │ Battle Server #N │
                         └──────────────────┘
```

匹配服分配玩家到具体战斗服，每个战斗服一局游戏跑完销毁。生产环境用 K8s + Agones 调度。