@echo off
::������IP��ַ
::set ip=any
::�������˶˿�
::set port=7654
::��Ϣ�����߳�����
::set thread=1
::�ͻ�����������
::set client=1

::������IP��ַ
set cmd="ip=127.0.0.1"
::�������˶˿�
set cmd=%cmd% port=5000
::��Ϣ�����߳�����
set cmd=%cmd% thread=2
::�ͻ�����������
set cmd=%cmd% max_client=1
::�ͻ��˷��ͻ�������С(�ֽ�)
set cmd=%cmd% send_buffer_size=81920
::�ͻ��˽��ջ�������С(�ֽ�)
set cmd=%cmd% recv_buffer_size=81920
::�յ���Ϣ�󽫷���Ӧ����Ϣ
set cmd=%cmd% -sendback
::��ʾ���ͻ�����һд��
::������sendfull��ʾʱ����ʾ������Ϣ������
set cmd=%cmd% -sendfull
::�����յ��Ŀͻ�����ϢID�Ƿ�����
set cmd=%cmd% -checkid
::�Զ����־ δʹ��
set cmd=%cmd% -p

::��������
server %cmd% 

@pause
