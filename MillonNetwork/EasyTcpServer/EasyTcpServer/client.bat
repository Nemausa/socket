@echo off
::������IP��ַ
set cmd="ip=127.0.0.1"
::�������˿�
set cmd=%cmd% port=4567
::�����߳�����
set cmd=%cmd% n_thread=1
::ÿ�������̴߳������ٸ��ͻ���
set cmd=%cmd% n_client=10000
::�ͻ���ÿ�η�������Ϣ
set cmd=%cmd% n_msg=1
::д����Ϣ���������ļ��ʱ��
set cmd=%cmd% n_sendsleep=1
::��������ʱ��
set cmd=%cmd% n_worksleep=1
::�ͻ��˷��ͻ������Ĵ�С
set cmd=%cmd% n_send_buffer_size=81920
::�ͻ��˽��ջ������Ĵ�С
set cmd=%cmd% n_recv_buffer_size=81920
::����յ���������ϢID�Ƿ�����
set cmd=%cmd% -check_msg_id

::��������
client %cmd% 

@pause