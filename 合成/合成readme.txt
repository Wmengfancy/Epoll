编译运行说明：
1.对服务器进行编译：
gcc -o tcp tcpepoll.c
2.服务器运行：（参数有端口号、要传输的文件名）
tcp 5005(端口号) data.txt(指定的文件名)
3.对客户端进行编译：
gcc -o client cepoll.c
4.服务器运行：（参数有要连接的服务器的IP地址和端口号）
client IP地址 5005(端口号)