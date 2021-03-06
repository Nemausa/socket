@echo off
::服务器IP地址
set cmd="ip=127.0.0.1"
::服务器端口
set cmd=%cmd% port=4567
::工作线程数量
set cmd=%cmd% n_thread=5
::每个工作线程创建多少个客户单
set cmd=%cmd% n_client=2000
::客户端每次发几个消息
::每条消息100字节(Login)
set cmd=%cmd% n_msg=100
::写入消息到缓冲区的间隔时间
set cmd=%cmd% n_sendsleep=1000
::工作休眠时间
set cmd=%cmd% n_worksleep=1
::客户端发送缓冲区的大小(字节)
set cmd=%cmd% n_send_buffer_size=20480
::客户端接收缓冲区的大小(字节)
set cmd=%cmd% n_recv_buffer_size=20480
::检测收到服务器消息ID是否连续
set cmd=%cmd% -check_msg_id

::启动程序
client %cmd% 

pause