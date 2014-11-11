========================================================================
    CONSOLE AND MFC mixed APPLICATION : AMSServer Project Overview
========================================================================
目录结构：cert，ssl证书；include，头文件；src，源文件，包括资源文件
	lib，依赖的库，如果是动态库，需要把对应的dll文件放在EXE所在的目录内

	相关目录需要在项目属性->VC++目录中添加；
	为了在CONSOLE程序中使用MFC，需要对MFC模块进行初始化，在main函数中进行
	并在项目属性中设置"Use MFC in a Shared DLL"；


计划内还未实现或者已知问题的模块：
1、多线程环境下完成端口内存回收
2、多线程环境下mysql开发
3、完成端口下OpenSSL开发
4、log文件根据日期分类



开发顺序：后台CONSOLE程序->添加界面，这种方式比传统的界面开发方式要好，可以剥离界面，单独调试后台功能
		对于后台CONSOLE程序，可以进行层次调试，先调试基类和底层的类，再联调上层功能类

/////////////////////////////////////////////////////////////////////////////
源文件:
AMSServer.h
AMSServer.cpp
    程序主函数，包含main函数和MFC模块初始化，这里可以添加CONSOLE程序或者新的MFC界面
	实现后台CMD窗口和界面同时启动，这样可以在后台CMD看到打印，还能进行别的异步操作
	可以剥离界面单独调试console程序


CONSOLE程序：

FileLog.h
FileLog.cpp
	实现程序打印输出到文件的功能，使用单件模式，防止打印混乱
	还未实现功能：每天打印到不同的文件内，主要是获取系统时间

HeapProtectMutex.h
HeapProtectMutex.cpp
	多线程对class内存空间new和delete重叠的时候可能会造成堆栈错误，可能与windows系统有关
	采用mutex并使用单件模式，对全局的new和delete进行保护

PerIOData.h
PerIOData.cpp
	客户端、服务端每次交互的IO数据，每个连接的客户端socket可以有多个IO数据

SocketHandle.h
SocketHandle.cpp
	管理连接的客户端socket的相关数据，包括IO数据
	本来可以采用list对IO数据进行管理，但是在多线程环境下需要用局部mutex保护，影响性能

IOCPServer.h
IOCPServer.cpp
	完成端口服务端，应用协议服务端应该继承此类
	遗留问题：多线程环境下，对完成端口操作完成的SocketHandle进行快速移除时，可能引起segment fault
		即使使用全局mutex进行保护也不行，和完成端口有关系，因为完成端口返回socket的顺序是乱序的
		此问题未出现在浏览器上，出现在一个压力测试客户端上，每秒50+连接时会出现此错误
	设想方案：采用垃圾回收线程，进行统一回收

SSLserver.h
SSLserver.cpp
	是IOCPServer的派生类
	集成OpenSSL，未完成；主要是AcceptEx函数的处理有待测试，至于完成端口的Recv和Send
	可以采用以下网页的信息：
	http://stackoverflow.com/questions/4403816/io-completion-ports-and-openssl

HttpServer.h
HttpServer.cpp
	http应用协议服务端
	如果使用https方式，应该继承SSLserver；
	如果不使用https方式，应该继承IOCPServer；

HttpData.h
HttpData.cpp
	http应用协议数据处理辅助类

MySqlHandle.h
MySqlHandle.cpp
	mysql的接口类，目前还未移植过来，这里是为了多处使用mysql的接口
	在多线程环境下应该使用单件模式，并参考mysql官方文档进行：
	http://dev.mysql.com/doc/refman/5.6/en/c-api-threaded-clients.html

MySqlHttpData.h
MySqlHttpData.cpp
	是HttpData的派生类，主要用于从mysql中取得数据和处理数据



MFC界面：

MFCAPP.h
MFCAPP.cpp
	和MFC wizard生成的标准程序一致

MFCLoginGUI.h
MFCLoginGUI.cpp
	登录界面

MFCMainGUI.h
MFCMainGUI.cpp
	程序主界面

resource.h
GUI.rc
	界面控件和其他宏

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    预编译头文件和其创建文件

/////////////////////////////////////////////////////////////////////////////
项目文件

AMSServer.vcxproj
    主项目文件，包含VC版本、平台信息、配置和项目特性

AMSServer.vcxproj.filters
    项目filters，有三种filters：头文件、源文件和资源文件