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
set cmd="ip=any"
::�������˶˿�
set cmd=%cmd% port=4567
::��Ϣ�����߳�����
set cmd=%cmd% thread=6
::�ͻ�����������
set cmd=%cmd% max_client=10000
::�ͻ��˷��ͻ�������С(�ֽ�)
set cmd=%cmd% send_buffer_size=20480
::�ͻ��˽��ջ�������С(�ֽ�)
set cmd=%cmd% recv_buffer_size=20480
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

pause
