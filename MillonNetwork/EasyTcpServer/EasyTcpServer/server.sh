#设置当前环境为shell脚本所在目录
#cd `dirname $0`
#服务器IP地址
#ip="any"
#服务器端口
#port=4568
#消息处理线程数量
#thread=6
#客户端连接上限
#client=1
########################
#key-val
#字典
#std::map<k,v>
########################
cd `dirname $0`
cmd='ip=any'
cmd=$cmd' port=5000'
cmd=$cmd' thread=4'
cmd="$cmd client=5"
cmd=$cmd' -p'


#./server $ip $port $thread $client
./server $cmd

read -p "...press any key to continue..." var