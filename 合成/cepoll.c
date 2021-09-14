#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<fcntl.h>

void login(int connfd)
{
	char s[2];
	char m[20];
	char name1[10];
	char passwd1[10];
	while(1)
	{
	printf("**********请登录**********\n**********请输入用户名：**********\n");
	memset(name1,0,sizeof(name1));
	scanf("%s",name1);
	write(connfd,name1,sizeof(name1));
	printf("**********请输入密码：**********\n");
	scanf("%s",passwd1);
	write(connfd,passwd1,sizeof(passwd1));
	read(connfd,s,sizeof(s));
	if(s[0]=='y')
	{
		printf("**********登录成功**********\n");
		break;
	}
	else if(s[0]=='n')
	{
		printf("**********用户名或密码错误**********\n");
		continue;
	}
	}
}

int main(int argc,char **argv)
{
	if(argc!=3)
	{
	printf("usage:a.out<IPaddress>\n");
	return -1;
	}
	struct sockaddr_in servaddr;
	
	int sockfd;
	char Buf[128];
	
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
	printf("socket error\n");
	return -1;
	}
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(atoi(argv[2]));
	servaddr.sin_addr.s_addr=inet_addr(argv[1]);

	if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))!=0)
	{
	printf("connect error\n");
	return -1;
	}
	printf("connect OK!\n");
	
	char t[2];
	char name[10];
	char passwd[10];
	printf("**********是否进行注册？y or n**********\n");
	memset(t,0,sizeof(t));
	scanf("%s",t);
	write(sockfd,t,sizeof(t));
	if(t[0]=='y')
	{
	printf("**********请设置用户名：**********\n");
	memset(name,0,sizeof(name));
	scanf("%s",name);
	write(sockfd,name,sizeof(name));
	printf("**********请设置密码：**********\n");
	memset(passwd,0,sizeof(passwd));
	scanf("%s",passwd);
	write(sockfd,passwd,sizeof(passwd));
	login(sockfd);
	}
	if(t[0]=='n')
		login(sockfd);

	for(int i=0;i<10000;i++)
	{
		char tip[27];
		memset(tip,0,sizeof(tip));
		printf("Are you going to get a file?file or no_file\n");scanf("%s",tip);
		if(write(sockfd,tip,sizeof(tip))<=0)
		{
		printf("write error\n");
		return -1;
		}
		if(strcmp(tip, "file") == 0)
		{
			int fp;
			int mode=0664;
			int flags=O_RDWR|O_CREAT|O_TRUNC;
			fp=open("i.txt",flags,mode);
			if (fp==-1) 
				fprintf(stderr, "cannot open file %s\n"，"i.txt");
			memset(Buf,0,sizeof(Buf));
			if(read(sockfd,Buf,sizeof(Buf))<=0)
			{
			printf("read error\n");
			return -1;
			}
			write(fp,Buf,strlen(Buf));
			close(fp);
		}
		else
		{
			printf("**********请输入请求：**********\n");
			memset(Buf,0,sizeof(Buf));
			scanf("%s",Buf);
			if(write(sockfd,Buf,sizeof(Buf))<=0)
			{
			printf("write error\n");
			return -1;
			}
			memset(Buf,0,sizeof(Buf));
			if(read(sockfd,Buf,sizeof(Buf))<=0)
			{
			printf("read error\n");
			return -1;
			}
			printf("%s\n",Buf);
		}
	}
	close(sockfd);
}
