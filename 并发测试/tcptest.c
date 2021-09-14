#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include <unistd.h>

#define MAX_EVENTS 10240

typedef struct {
	int fd;
	char name[10];
	char mima[10];
	char qingqiu[10];
}usr;

int setNonBlocking(int p_nSock)
{   
    int nOpts;   
    nOpts=fcntl(p_nSock,F_GETFL);   
    if(nOpts<0)   
    {   
        printf("[%s %d] Fcntl Sock GETFL fail!\n",__FUNCTION__,__LINE__);
        return -1;
    }   
    nOpts = nOpts|O_NONBLOCK;   
    if(fcntl(p_nSock,F_SETFL,nOpts)<0)   
    {  
        printf("[%s %d] Fcntl Sock SETFL fail!\n",__FUNCTION__,__LINE__);
        return -1;   
    } 
    return 0;
} 

int main(int argc,char **argv)
{
	if(argc!=2)
	{
	printf("usage:a.out<IPaddress>\n");
	return -1;
	}
	struct sockaddr_in servaddr,clientaddr;				
	socklen_t clilen = sizeof(struct sockaddr_in);
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(atoi(argv[1]));
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);

	int listenfd;
	int epollfd;
	int nfds;
	int connfd;
	int sockfd;
	int j=-1;
	usr USR[10240];

	listenfd=socket(AF_INET,SOCK_STREAM,0);
	
	if(listenfd<0)
	{
	printf("socket error\n");
	return -1;
	}

	if (setNonBlocking(listenfd) < 0)
	{
		return 0;
	}

	if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
	{
	printf("bind error\n");
	return -1;
	}

	if(listen(listenfd,12800)!=0)
	{
	printf("listen error\n");
	return -1;
	}
	
	printf("listensock=%d\n",listenfd);
	
	struct epoll_event ev,events[MAX_EVENTS];
	epollfd=epoll_create(MAX_EVENTS);
	
	if(epollfd<=0)
	{
		printf("epoll_creat error\n");
		return 0;
	}

	ev.events=EPOLLIN;
	ev.data.fd=listenfd;

	if(epoll_ctl(epollfd,EPOLL_CTL_ADD,listenfd,&ev)<0)
	{
		printf("epoll_ctl error\n");
		return 0;
	}

	for(;;)
	{
		nfds=epoll_wait(epollfd,events,MAX_EVENTS,-1);
		for(int i=0;i<nfds;i++)
		{
			if(events[i].data.fd==listenfd)
			{
				j++;
				connfd=accept(listenfd,(struct sockaddr*)&clientaddr,&clilen);
				if(connfd<0)
				{
					printf("accept error");
					continue;
				}
				if (setNonBlocking(connfd) < 0)
				{
					return 0;
				}
				printf("IP:%s,Port:%d\n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
				printf("连接数为%d\n",(connfd-4));
				ev.data.ptr=&USR[j];
				USR[j].fd=connfd;
				ev.events=EPOLLIN|EPOLLET;
				if(epoll_ctl(epollfd,EPOLL_CTL_ADD,connfd,&ev)<0)
				{
					printf("epoll_ctl2 error");
					continue;
				}
				printf("client(socket=%d) connect ok.\n",USR[j].fd);
				continue;
			}
			if(events[i].events & EPOLLIN)
			{
				usr * qq=(usr*)events[i].data.ptr;
				memset(qq->qingqiu,0,sizeof(qq->qingqiu));
				ssize_t count;
				if((count =read(qq->fd,(*qq).qingqiu,sizeof(qq->qingqiu)))<=0)
				{
				printf("read error");
				continue;
				}
				//printf("recv(eventfd=%d,size=%ld):%s\n",qq->fd,count,qq->qingqiu);
				

				ev.data.ptr=qq;
				ev.events=EPOLLOUT|EPOLLET;
				epoll_ctl(epollfd,EPOLL_CTL_MOD,qq->fd,&ev);	
			}
			else if(events[i].events & EPOLLOUT)
			{
				usr * qq=(usr*)events[i].data.ptr;
				write(qq->fd,qq->qingqiu,sizeof(qq->qingqiu));

				ev.data.ptr=events[i].data.ptr;
				ev.events=EPOLLIN|EPOLLET;
				epoll_ctl(epollfd,EPOLL_CTL_MOD,qq->fd,&ev);
			}	
		}
	}
	close(listenfd);
	return 0;
}
