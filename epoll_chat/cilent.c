#include "func.h"

int main(int argc,char** argv)
{
	args_check(argc,3);
    int socketfd=socket(AF_INET,SOCK_STREAM,0);
    if(-1==socketfd)
    {
        perror("socket");
        return -1;
    }
    struct sockaddr_in ser;
    memset(&ser,0,sizeof(struct sockaddr_in));
    ser.sin_family=AF_INET;
    ser.sin_port=htons(atoi(argv[2]));
    ser.sin_addr.s_addr=inet_addr(argv[1]);//struct in_addr 的成员只有一个就是s_addr是旧的接口
    int ret;
    ret=connect(socketfd,(struct sockaddr*)&ser,sizeof(struct sockaddr_in));
    if(ret==-1)
    {
        perror("connect");
        return -1;
    }
    char buf[128]={0};
	printf("I am chat2\n");
	fd_set rdset;//描述符监控的读集合
	while(1)
	{
		FD_ZERO(&rdset);
		FD_SET(0,&rdset);
		FD_SET(socketfd,&rdset);
		ret=select(socketfd+1,&rdset,NULL,NULL,NULL);
		if(ret>0)
		{
			if(FD_ISSET(STDIN_FILENO,&rdset))
			{
				memset(buf,0,sizeof(buf));
				ret=read(STDIN_FILENO,buf,sizeof(buf));//读取标准输入
				if(0==ret)
				{
					printf("this is end\n");
					break;
				}
				send(socketfd,buf,strlen(buf)-1,0);//写端管道内
			}
			if(FD_ISSET(socketfd,&rdset))
			{
				memset(buf,0,sizeof(buf));
				ret=recv(socketfd,buf,sizeof(buf),0);
				if(0==ret)//写端断开，读端read返回，得到0
				{
					printf("byebye\n");
					break;
				}
				printf("%s\n",buf);
			}
		}
    }
	close(socketfd);
	return 0;
}
