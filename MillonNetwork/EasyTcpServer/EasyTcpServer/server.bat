@echo off
::服务器IP地址
::set ip=any
::服务器端端口
::set port=7654
::消息处理线程数量
::set thread=1
::客户端上限数量
::set client=1

::服务器IP地址
set cmd="ip=127.0.0.1"
::服务器端端口
set cmd=%cmd% port=5000
::消息处理线程数量
set cmd=%cmd% thread=2
::客户端上限数量
set cmd=%cmd% client=1
::启动程序
server %cmd% -p

@pause
