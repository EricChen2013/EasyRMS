# EasyRMS #

EasyRMS是EasyDarwin开源流媒体平台的录像与回放服务，能够直接对接安防摄像机、EasyDarwin流媒体服务器、其他标准RTSP服务，基于HLS协议，录像存储为ts流，支持本地存储与阿里云对象存储(OSS云存储)，接口调用非常简单，架构灵活，而且非常易于扩展，开发者可以用EasyRMS进行扩展，开发成自己需要的云存储服务，EasyDarwin团队也将会持续迭代EasyRMS项目。

## 编译 ##

Windows版本编译，可以直接用Visual Studio 2008打开源码文件中的：EasyRMS/WinNTSupport/EasyRMS.sln解决方案文件，编译出exe可执行文件EasyRMS.exe；


## 配置 ##
EasyRMS主要的几个配置项：

	monitor_lan_port：EasyRMS内网服务端口
	
	monitor_wan_port：EasyRMS公网服务端口
	
	record_duration：单个m3u8的录像时长(单位分钟)，也就是单个录像段落的时间，体现在存储中的形式就是单个录像文件夹最大录像时长；
	
	local_record_path：本地存储的目录（record_to_where配置为本地存储时有效）
	
	record_to_where：存储方式，0-阿里云OSS云存储，1-本地存储
	
	oss_endpoint：阿里云OSS服务地址（record_to_where配置为OSS存储时有效）
	
	oss_port：阿里云OSS服务端口，默认为80（record_to_where配置为OSS存储时有效）
	
	oss_bucket_name：阿里云OSS Bucket名称（record_to_where配置为OSS存储时有效）
	
	oss_access_key_id：阿里云提供的OSS登陆id（record_to_where配置为OSS存储时有效）
	
	oss_access_key_secret：阿里云提供的OSS登陆密钥（record_to_where配置为OSS存储时有效）
	
## 运行 ##
Windows版本运行(控制台调试运行)：
EasyRMS.exe -c ./easyrms.xml -d

## 调用方法 ##
### 
- 启动录像

	http://[ip]:[service_port]/api/easyrecordmodule?name=[recordName]&url=[RTSP_URL]

recordName一般为媒体流的ID(或设备ID),例如EasyRMS服务器IP地址是：8.8.8.8，EasyRMS 服务端口：8080，摄像机的RTSP地址是：rtsp://admin:admin@192.168.66.189/22，摄像机的ID为:1234567890，调用方式如下：

*http://8.8.8.8:8080/api/easyrecordmodule?name=1234567890&url=rtsp://admin:admin@192.168.66.189/*
  
### 
- 停止录像

	http://[ip]:[service_port]/api/easyrecordmodule?name=[recordName]&cmd=stop

例如停止上面启动的录像：

*http://8.8.8.8:8080/api/easyrecordmodule?name=1234567890&cmd=stop*

### 
- 检索录像

	http://[ip]:[service_port]/api/easyrecordmodule?name=[recordName]&cmd=list&begin=[beginTime]&end=[endTime]
	
其中begin为查询开始时间格式为YYYYMMDDhhmmss，end为结束时间，格式同开始时间，例如查询2015年11月1日至2015年11月3日15时30分45秒的录像：

*http://8.8.8.8:8080/api/easyrecordmodule?name=1234567890&cmd=list&begin=20151101000000&end=20151103153045*

## 正在开发 ##

- Linux版本支持
- 录像检索与回放接口


## 获取更多信息 ##

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[www.EasyDarwin.org](http://www.easydarwin.org)

QQ交流群：288214068

Copyright &copy; EasyDarwin.org 2012-2015

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)
