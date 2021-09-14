#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_EVENTS 10000

typedef struct {
	int fd;
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
	if(argc!=3)
	{
	printf("usage:a.out<IPaddress>\n");
	return -1;
	}
	struct sockaddr_in servaddr;
	
	int epollfd;
	int nfds;
	int n=0;
	int elapsed;
	float sp;
	usr USR[1002];
	int sockfd[1000];
	
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(atoi(argv[2]));
	servaddr.sin_addr.s_addr=inet_addr(argv[1]);
	
	struct epoll_event ev,events[MAX_EVENTS];
	epollfd=epoll_create(MAX_EVENTS);
		
	if(epollfd<=0)
	{
		printf("epoll_creat error\n");
		return 0;
	}
	
	for(int i=0;i<1000;i++)
	{
		if((sockfd[i]=socket(AF_INET,SOCK_STREAM,0))<0)
		{
		printf("socket error\n");
		return -1;
		}

		if(connect(sockfd[i],(struct sockaddr*)&servaddr,sizeof(servaddr))!=0)
		{
		printf("connect error\n");
		return -1;
		}
		printf("connect OK!");
		
		setNonBlocking(sockfd[i]);
		
		ev.events=EPOLLIN|EPOLLET|EPOLLOUT;
		ev.data.ptr=&USR[i];
		USR[i].fd=sockfd[i];
		strcpy(USR[i].qingqiu,"test!");
		if(epoll_ctl(epollfd,EPOLL_CTL_ADD,sockfd[i],&ev)<0)
		{
			printf("epoll_ctl error\n");
			return 0;
		}
	}
	
		struct timeval tik,tok;
		gettimeofday(&tik,NULL);
		
		for(;;)
		{
			
			nfds=epoll_wait(epollfd,events,MAX_EVENTS,-1);
			for(int i=0;i<nfds;i++)
			{
				if(events[i].events&EPOLLOUT)
				{
					usr * qq=(usr*)events[i].data.ptr;
					write(qq->fd,qq->qingqiu,sizeof(qq->qingqiu));
					n++;
					
					ev.data.ptr=events[i].data.ptr;
					ev.events=EPOLLIN|EPOLLET;
					epoll_ctl(epollfd,EPOLL_CTL_MOD,qq->fd,&ev);
				}
				
				else if(events[i].events&EPOLLIN)
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
					n++;
					ev.data.ptr=qq;
					ev.events=EPOLLOUT|EPOLLET;
					epoll_ctl(epollfd,EPOLL_CTL_MOD,qq->fd,&ev);	
				}
				if(n==20000)
				{
					gettimeofday(&tok, NULL);
					elapsed = (tok.tv_usec + tok.tv_sec * 1000000) - (tik.tv_usec + tik.tv_sec * 1000000);
					sp=(((float)n)/((float)elapsed))*1000000;
					printf("处理1000个用户的吞吐量为%f\n",sp);
					sleep(1);
					n=0;
					gettimeofday(&tik,NULL);
				}
			}
		}
	for(int i=0;i<1000;i++)
		close(sockfd[i]);
	return 0;
}
	



