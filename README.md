# 项目说明

**当前进度 支持多个RTMP推流者推流 支持多个HTTP-FLV拉流者拉取对应的流观看**

**使用C++17标准 Windows和Centos平台均使用gcc编译通过正常使用**

**运行后在两个推流者四个观看者情况下 占用内存6Mb CPU使用极低**

**支持IPV4和IPV6**

服务器端代码量 包含网络部分 5000余行

HttpFlv拉流的播放器目前测试通过PotPlayer, VLC, flv.js

RTMP推流播放器仅测试过Obs

网络部分学习自 https://github.com/chenshuo/muduo

跨平台支持学习自 https://github.com/balloonwj/flamingo

HTTP包装 RTMP解析 FLV解析 参考自网络相关文档

# 项目规划

开发轻量级跨Windows和Linux的可用于P2P直播的`服务器端`和`观看客户端`

项目不打算开发UI方面的功能, 视频将使用第三方的支持HttpFlv拉流的播放器来播放(如PotPlayer)

## 开发进度

支持多个RTMP推流者推流(已完成)

支持多个HTTP-FLV拉流者拉取对应的流观看(已完成)

开发`观看客户端`主要用于从`服务器端`接收FLV数据 本地转化成HttpFlv格式 供`HttpFlv拉流软件本地拉取` (未开发)

`观看者客户端`可以从`服务器端`获取数据也可以通过P2P技术与其他`观看者客户端`进行共享 (未开发)

## 初版

主播使用`RTMP推流软件(如Obs)`将RTMP流发送到`服务器端` `服务器端`将Rtmp流解析生成FLV数据包装在HTTP中推送给观看者

观看者直接使用`服务器端`生成的HTTPFLV地址 拉流观看


## P2P版本 (未开发)

# 使用说明

**编译执行文件**
下载源代码后在Windows平台或者Linux平台编译
```shell
git clone https://github.com/HiganFish/LiveBroadcast.git

cd LiveBroadcast/LiveBroadcastServer/

mkdir build && cd build

cmake ..

make -j4
```
生成运行文件`LiveBroadcastServer`


**运行服务器**
```shell
./LiveBroadcastServer 4000 4100 # 4000 Rtmp推流端口 4100 HTTP-FLV拉流端口
```

**Obs推流**
```shell
# 设置->推流->服务器
rtmp://[::1]:4000/test-push  # 注意 rtmp和http链接的对应关系 默认是相同的路径为同一个房间
rtmp://127.0.0.1:4000/test-push # 可以在main.cpp中修改映射关系
```

**HTTP-FLV拉流**
```shell
http://[::1]:4000/test-push # 注意 rtmp和http链接的对应关系 默认是相同的路径为同一个房间
http://127.0.0.1:4000/test-push # 可以在main.cpp中修改映射关系
```

# 项目所需技术

## 服务器端

### FLV
FLV文件由一个FileHeader起头 后面为若干个FileTag 直到文件结束前四个字节
最后四个字节是上一个FileTag的大小减去4

FLV文件格式
```
class FileHeader
{
	char flv[3]; // FLV
	uint8_t version; // 1
	uint8_t type_flags; // 5
	uint32_t header_length; // 9
};

class FileTag
{
	uint32_t previous_tag_size; // 不含previous_tag_size  sizeof 上一个Tag - 4
	uint8_t tag_type; // 音频 8 视频 9 scripts 18
	uint8_t data_size[3]; // AudioTag VideoTag 的数据长度 从stream_id后开始算起
	uint8_t timestamp[3];
	uint8_t timestamp_extend;
	uint8_t stream_id[3]; // 0

	char* data;
};
```

前三个Tag可以认为有特殊作用 所以单独说一下

以后的所有Tag均为音视频数据
```
FileHeader file_header; // 每次开始都要发送一次
FileTag info_tag; // 每次开始都要发送一次
FileTag sps_pps_tag; // 每次开始都要发送一次 第一个视频tag存储编码信息
FileTag audio_tag; // 每次开始都要发送一次 第一个音频tag存储编码信息

FileTag data[N]; // 真正的数据部分
```

每次有`观看者客户端`连接的时候 都要发送一次`file_header`,`info_tag`和`sps_pps_tag` 以及`audio_tag`之后便是数据部分

### RTMP

#### RTMP握手协议
Obs客户端首先会发送C1 服务器端直接将C1作为S1发送回去, Obs会发送C2 服务端继续发送C1作为S2 握手成功


商定窗口大小
```
-->
connect

<-- Window Acknowledgement Size 5000000
[02 00 00 00 00 00 04 05 00 00 00 00 00 4c 4b 40]


<-- Set Peer Bandwidth 5000000,Dynamic|Set Chunk Size 4096|_result('NetConnection.Connect.Success')
[02 00 00 00 00 00 05 06 00 00 00 00 00 4c 4b 40 02]
[02 00 00 00 00 00 04 01 00 00 00 00 00 00 10 00]
[03 00 00 00 00 00 be 14 00 00 00 00 02 00 07 5f 72 65 73 75 6c 74 00 3f f0 00 00 00 00 00 00 03 00 06 66 6d 73 56 65 72 02 00 0d 46 4d 53 2f 33 2c 30 2c 31 2c 31 32 33 00 0c 63 61 70 61 62 69 6c 69 74 69 65 73 00 40 3f 00 00 00 00 00 00 00 00 09 03 00 05 6c 65 76 65 6c 02 00 06 73 74 61 74 75 73 00 04 63 6f 64 65 02 00 1d 4e 65 74 43 6f 6e 6e 65 63 74 69 6f 6e 2e 43 6f 6e 6e 65 63 74 2e 53 75 63 63 65 73 73 00 0b 64 65 73 63 72 69 70 74 69 6f 6e 02 00 15 43 6f 6e 6e 65 63 74 69 6f 6e 20 73 75 63 63 65 65 64 65 64 2e 00 0e 6f 62 6a 65 63 74 45 6e 63 6f 64 69 6e 67 00 00 00 00 00 00 00 00 00 00 00 09]

```

建立流
```
-->
releaseStream
FCPublish
createStream
<-- _result()
[03 00 00 00 00 00 1d 14 00 00 00 00 02 00 07 5f 72 65 73 75 6c 74 00 40 10 00 00 00 00 00 00 05 00 3f f0 00 00 00 00 00 00]




-->
publish
<-- onStatus('NetStream.Publish.Start')
[05 00 00 00 00 00 69 14 01 00 00 00 02 00 08 6f 6e 53 74 61 74 75 73 00 00 00 00 00 00 00 00 00 05 03 00 05 6c 65 76 65 6c 02 00 06 73 74 61 74 75 73 00 04 63 6f 64 65 02 00 17 4e 65 74 53 74 72 65 61 6d 2e 50 75 62 6c 69 73 68 2e 53 74 61 72 74 00 0b 64 65 73 63 72 69 70 74 69 6f 6e 02 00 10 53 74 61 72 74 20 70 75 62 6c 69 73 68 69 6e 67 00 00 09]

```

至此连接建立完毕, Obs开始推流

#### RTMP推流协议部分

RTMP的消息单元称为Message单个Message太大被拆分为多块 包装在Message Chunk中发送

所以服务器端需要从TCP流中获取Message Chunk组装出Message 再从Message中获取到音视频数据


##### Message Chunk
`Message Chunk` = `Message Chunk Header` + `Chunk Data`

`Message Chunk Header` = `Basic Header` + `Message Header` + `extern timestamp`

如以下例子
```
0000   04 00 00 00 00 00 31 09 01 00 00 00 17 00 00 00
0010   00 01 64 00 28 ff e1 00 1d 67 64 00 28 ac d9 40
0020   78 02 27 e5 9a 80 80 80 a0 00 00 03 00 20 00 00
0030   07 91 e3 06 32 c0 01 00 04 68 ef bc b0
```

**Basic Header** 
```
04 = 00  00 0100// fmt = 0 csid = 4
```
1字节 初期只关注csid为4的数据部分, 因为csid为4时对应音视频数据

**Message Header**

fmt = 0 11字节
```
00 00 00 // timestamp
00 00 31 // msglength
09 // typeid 9视频
01 00 00 00 // 小端存储  message stream id
```

fmt = 1 7字节
```
00 00 00 // timestamp 前一个块时间戳与当前块时间戳的差值，即相对时间戳
00 00 31 // msglength
09 // typeid 9视频
// 不含 message stream id 此时块与之前的块取相同的消息流ID
```

fmt = 2 3字节
```
00 00 00 // timestamp 前一个块时间戳与当前块时间戳的差值，即相对时间戳
// 不含 msglength 与前一个相同
// 09 不含typeid 9视频
// 不含 message stream id 此时块与之前的块取相同的消息流ID
```

fmt = 3 0字节
```
不含头 一个消息被分成多个的时候 使用这个类型
```

至于`Chunk Data`就是Flv的data部分

Obs发送`@SetDataFrame()`其中Data为info_tag的data部分
Obs发送`Audio Data`其中Data为audio_tag的data部分  音频数据包
Obs发送`Video Data`其中Data为sps_pps_tag的data部分  视频数据包

之后便是正常画面声音的Flv数据发送.

# 文件介绍
```
├── CMakeLists.txt
├── main.cpp
├── network # 网络部分学习自muduo
│   ├── Acceptor.cpp
│   ├── Acceptor.h
│   ├── Callback.h
│   ├── Channel.cpp
│   ├── Channel.h
│   ├── EventLoop.cpp
│   ├── EventLoop.h
│   ├── InetAddress.cpp
│   ├── InetAddress.h
│   ├── multiplexing
│   │   ├── Epoll.cpp
│   │   ├── Epoll.h
│   │   ├── MultiplexingBase.cpp
│   │   ├── MultiplexingBase.h
│   │   ├── Select.cpp
│   │   └── Select.h
│   ├── PlatformNetwork.cpp # 网络跨平台相关学习自flamingo
│   ├── PlatformNetwork.h # 网络跨平台相关学习自flamingo
│   ├── protocol # 将Rtmp推流者和HTTPFlv拉流者的TCP连接包装
│   │   ├── RtmpClientConnection.cpp
│   │   ├── RtmpClientConnection.h
│   │   ├── RtmpServerConnection.cpp
│   │   └── RtmpServerConnection.h
│   ├── Socket.cpp
│   ├── Socket.h
│   ├── SocketOps.cpp
│   ├── SocketOps.h
│   ├── TcpConnection.cpp
│   ├── TcpConnection.h
│   ├── TcpServer.cpp
│   ├── TcpServer.h
│   └── test
│       └── TcpServerTest.cpp
└── utils
    ├── Buffer.cpp
    ├── Buffer.h
    ├── codec
    │   ├── FlvCodec.cpp # FLV解析
    │   ├── FlvCodec.h
    │   ├── FlvManager.cpp # 管理FLV的解析 保存重要的几个Tag
    │   ├── FlvManager.h
    │   ├── RtmpCodec.cpp # Rtmp解析
    │   ├── RtmpCodec.h
    │   ├── RtmpManager.cpp # 管理Rtmp解析 将Rtmp数据部分转换成FlvTag
    │   ├── RtmpManager.h
    │   └── test
    │       ├── FlvManagerTest.cpp
    │       └── RtmpManagerTest.cpp
    ├── File.cpp # 文件读写工具类
    ├── File.h
    ├── Format.cpp # 字符串格式化
    ├── Format.h
    ├── Logger.cpp # 简易低性能日志
    ├── Logger.h
    ├── PlatformBase.cpp # 基础部分跨平台代码
    ├── PlatformBase.h
    ├── test
    │   └── LoggerTest.cpp
    ├── Timestamp.cpp
    └── Timestamp.h

8 directories, 55 files
```