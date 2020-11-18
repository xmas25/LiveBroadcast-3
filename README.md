当前进度 `服务器端`解析Rtmp推流保存并为可以播放的Flv文件 (初版完成)

服务器端代码量 包含网络部分 3000余行 - 2020年11月18日

# 项目规划

开发轻量级的可用于P2P直播的`服务器端`和`观看客户端`

项目不打算开发UI方面的功能, 视频将使用第三方的支持HttpFlv拉流的播放器来播放(如PotPlayer)

开发可以接收Rtmp推流的`服务器端`  (初版完成)



`服务器端`可以`接收HttpFlv的拉流` (未开发)

开发`观看客户端`主要用于从`服务器端`接收二进制的Flv数据 本地转化成HttpFlv格式 供`HttpFlv拉流软件本地拉取` (未开发)

`观看者客户端`可以从`服务器端`获取数据 野通过P2P与其他`观看者客户端`进行共享 (未开发)

## 初版

主播使用`RTMP推流软件(如Obs)`将RTMP流发送到`服务器端` `服务器端`将Rtmp流以FLv格式保存在内存缓冲区 或者写入Flv文件

观看者直接使用`服务器端`生成的HTTPFLV地址 拉流观看



观看者下载`观看者客户端`从`服务器端`获得Socket数据流

`观看者客户端`将Socket数据流包装成HTTP协议 生成本地连接如`http://127.0.0.1:4000/xxxxx.mp4` 使用第三方网络视频播放器进行播放

## P2P版本 (未开发)

由于`观看者客户端`从`服务器端`获取的是Socket数据流 `观看者客户端`可以将其包装成HTTP格式 进行本地播放或者给其他`观看者客户端`提供链接进行播放

# 项目所需技术

## 服务器端

### 缓存FLV数据

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

FileHeader file_header; // 每次开始都要发送一次
FileTag info_tag; // 每次开始都要发送一次
FileTag sps_pps_tag; // 每次开始都要发送一次 第一个视频tag存储编码信息
FileTag audio_tag; // 每次开始都要发送一次 第一个音频tag存储编码信息

FileTag data[N]; // 真正的数据部分
```

每次有`观看者客户端`连接的时候 都要发送一次`file_header`,`info_tag`和`sps_pps_tag` 以及`audio_tag`之后便是数据部分

### 解析RTMP协议 从中得到音视频流

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

Obs发送`@SetDataFrame()`其中Data为info_tag的data部分
Obs发送`Audio Data`其中Data为audio_tag的data部分  音频数据包
Obs发送`Video Data`其中Data为sps_pps_tag的data部分  视频数据包

之后便是数据的发送.

# 文件介绍

```
├── CMakeLists.txt 总配置文件
├── CMakeSettings.json vsstudio2019 cmake项目配置文件
├── main.cpp 接收Obs推流数据 除去握手和建立连接部分 保存到data中 实测会导致 obs发送的publish也被保存
├── network 网络核心模块
│   ├── Acceptor.cpp
│   ├── Acceptor.h
│   ├── Callback.h
│   ├── Channel.cpp
│   ├── Channel.h
│   ├── CMakeLists.txt
│   ├── Epoll.cpp
│   ├── Epoll.h
│   ├── EventLoop.cpp
│   ├── EventLoop.h
│   ├── InetAddress.cpp
│   ├── InetAddress.h
│   ├── Socket.cpp
│   ├── Socket.h
│   ├── SocketOps.cpp
│   ├── SocketOps.h
│   ├── TcpConnection.cpp
│   ├── TcpConnection.h
│   ├── TcpServer.cpp
│   └── TcpServer.h
├── server
│   ├── CMakeLists.txt
│   ├── codec 编解码器 用于解析数据流 Flv和Rtmp的数据包解析位置以及相关数据包格式
│   │   ├── FlvCodec.cpp 供FlvManager使用 不直接使用
│   │   ├── FlvCodec.h
│   │   ├── RtmpCodec.cpp 供RtmpManager使用 不直接使用 包含Rtmp数据包到Flv数据包的转换函数
│   │   └── RtmpCodec.h
│   ├── FlvManager.cpp Flv管理器用于保存解析好的多个FlvTag信息, 管理Flv解析过程
│   ├── FlvManager.h
│   ├── RtmpManager.cpp Rtmp管理器用于将解析好的Rtmp转换成Flv数据包 保存到FlvManager中, 管理Rtmp流解析过程
│   ├── RtmpManager.h
│   └── test
│       ├── FlvManagerTest.cpp 测试FlvManager对正常Flv文件的解析 以及相关数据的存储
│       └── RtmpManagerTest.cpp 测试RTMPManager对Rtmp数据流的解析 转换成Flv数据保存 并将Flv数据保存为Flv文件
└── utils 工具类
    ├── Buffer.cpp 通用缓冲区
    ├── Buffer.h
    ├── CMakeLists.txt
    ├── File.cpp 文件对象 用于简单的读写文件
    └── File.h
```