#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<fcntl.h>

#define MAX_EVENTS 1024 

typedef struct {
	int fd;
	char name[10];
	char passwd[10];
	char state[2];
	char qingqiu[30];
	char txt[512];
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

void login(usr * test)
{	
	FILE*p;
	p=fopen("usr.txt","r");
	if(!p)
	fprintf(stderr,"cannot open file\n");
	
	char usr[20];	
	char s[2];
	char check[20];
	
	while(1)
	{
		fseek(p,0,SEEK_SET);
		s[0]='n';
		int numok,numusr;
		memset(usr,0,sizeof(usr));
			
		memset(test->name,0,sizeof(test->name));
		read(test->fd,test->name,sizeof(test->name));
		memset(test->passwd,0,sizeof(test->passwd));
		read(test->fd,test->passwd,sizeof(test->passwd));
		strcat(usr,test->name);
		strcat(usr," ");
		strcat(usr,test->passwd);
		strcat(usr,"\n");
		
		for(;;)
		{
			memset(check,0,sizeof(check));
			char* t=fgets(check,sizeof(check),p);
			if(t==NULL)
				break;
 			if(strcmp(check,usr)==0)
		 	{
		 		s[0]='y';
		 		write(test->fd,s,sizeof(s));
		 		printf("用户%s登录成功\n",test->name);
		 		break;
		 	}
		}
		if(s[0]!='y')
		{
			write(test->fd,s,sizeof(s));
		}
		else
		 	break;
	}
	fclose(p);
} 

int main(int argc,char **argv)
{
	if(argc!=3)
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
	usr USR[1024];
	
	char name[10];
	char passwd[10];
	char temp[50];

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
	if(listen(listenfd,128)!=0)
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

		int fp;
		fp=open(argv[2],O_RDONLY);
		if (fp==-1) 
		fprintf(stderr, "cannot open file %s\n", argv[2]);

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
		
				printf("IP:%s,Port:%d\n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
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
				ssize_t count;
				
				memset(temp,0,sizeof(temp));
				if((count =read(qq->fd,temp,sizeof(temp)))<=0)
				{
				printf("read error");
				continue;
				}
				if(strlen(temp)==1)
				{
					memset(qq->state,0,sizeof(qq->state));
					strcpy(qq->state,temp);
					if(qq->state[0]=='y')
					{
						FILE*usr_fp;
						usr_fp=fopen("usr.txt","a");
						if(!fp)
							fprintf(stderr,"cannot open file\n");
							
						memset(name,0,sizeof(name));
						read(qq->fd,name,sizeof(name));
						fputs(name,usr_fp);
						fputs(" ",usr_fp);
						memset(passwd,0,sizeof(passwd));
						read(qq->fd,passwd,sizeof(passwd));
						fputs(passwd,usr_fp);fputs("\n",usr_fp);
						fclose(usr_fp);
						login(qq);
					}	
					if(qq->state[0]=='n')
						login(qq);
				}
				else if(strlen(temp)>1){	
				memset(qq->qingqiu,0,sizeof(qq->qingqiu));
				strcpy(qq->qingqiu,temp);
				if(strcmp(qq->qingqiu, "file") != 0 && strcmp(qq->qingqiu,"no_file")!=0)
					write(qq->fd,qq->qingqiu,strlen(qq->qingqiu));
				printf("登录名为%s用户的请求是:%s\n",qq->name,qq->qingqiu);
				}

				ev.data.ptr=qq;
				ev.events=EPOLLOUT|EPOLLIN|EPOLLET;
				epoll_ctl(epollfd,EPOLL_CTL_MOD,qq->fd,&ev);	
			}
			else if((events[i].events & EPOLLIN)||(events[i].events & EPOLLOUT))
			{
				usr * qq=(usr*)events[i].data.ptr;

				if(strcmp(qq->qingqiu, "file") == 0)
				{
				memset(qq->txt,0,sizeof(qq->txt));
				read(fp,qq->txt,sizeof(qq->txt));
				write(qq->fd,qq->txt,sizeof(qq->txt));
				}

				ev.data.ptr=events[i].data.ptr;
				ev.events=EPOLLIN|EPOLLET;
				epoll_ctl(epollfd,EPOLL_CTL_MOD,qq->fd,&ev);
			}
			close(fp);	
		}
	}
	close(listenfd);
	return 0;
}
