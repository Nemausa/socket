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
set cmd=%cmd% client=1
::��������
server %cmd% -p

@pause
