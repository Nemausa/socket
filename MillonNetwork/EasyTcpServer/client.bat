@echo off
::������IP��ַ
set cmd="ip=127.0.0.1"
::�������˿�
set cmd=%cmd% port=4567
::�����߳�����
set cmd=%cmd% n_thread=5
::ÿ�������̴߳������ٸ��ͻ���
set cmd=%cmd% n_client=2000
::�ͻ���ÿ�η�������Ϣ
::ÿ����Ϣ100�ֽ�(Login)
set cmd=%cmd% n_msg=100
::д����Ϣ���������ļ��ʱ��
set cmd=%cmd% n_sendsleep=1000
::��������ʱ��
set cmd=%cmd% n_worksleep=1
::�ͻ��˷��ͻ������Ĵ�С(�ֽ�)
set cmd=%cmd% n_send_buffer_size=20480
::�ͻ��˽��ջ������Ĵ�С(�ֽ�)
set cmd=%cmd% n_recv_buffer_size=20480
::����յ���������ϢID�Ƿ�����
set cmd=%cmd% -check_msg_id

::��������
client %cmd% 

pause