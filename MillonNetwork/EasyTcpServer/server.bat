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
set cmd="ip=any"
::服务器端端口
set cmd=%cmd% port=4567
::消息处理线程数量
set cmd=%cmd% thread=6
::客户端上限数量
set cmd=%cmd% max_client=10000
::客户端发送缓冲区大小(字节)
set cmd=%cmd% send_buffer_size=20480
::客户端接收缓冲区大小(字节)
set cmd=%cmd% recv_buffer_size=20480
::收到消息后将返回应答消息
set cmd=%cmd% -sendback
::提示发送缓冲区一写满
::当出现sendfull提示时，表示当次消息被丢弃
set cmd=%cmd% -sendfull
::检查接收到的客户端消息ID是否连续
set cmd=%cmd% -checkid
::自定义标志 未使用
set cmd=%cmd% -p

::启动程序
server %cmd% 

pause
