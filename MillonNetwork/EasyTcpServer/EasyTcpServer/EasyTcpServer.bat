
::服务器IP地址
@set ip=any
::服务器端端口
@set port=7654
::消息处理线程数量
@set thread=1
::客户端上限数量
@set client=1

EasyTcpServer %ip% %port% %thread% %client%

@pause
