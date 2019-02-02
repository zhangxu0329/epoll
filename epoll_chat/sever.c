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
    ret=bind(socketfd,(struct sockaddr*)&ser,sizeof(struct sockaddr_in));
    if(-1==ret)
    {
        perror("bind");
        return -1;
    }
    listen(socketfd,10);
    int new_fd;
    struct sockaddr_in client;
    socklen_t addrlen;
    char buf[128]={0};
    printf("I am chat1\n");
    int epfd=epoll_create(1);//填入非0常数
    struct epoll_event event,eve[3];
    event.events=EPOLLIN;//事件赋予读属性
    event.data.fd=STDIN_FILENO;
    ret=epoll_ctl(epfd,EPOLL_CTL_ADD,STDIN_FILENO,&event);//注册事件
    if(ret==-1)
    {
        perror("epoll_ctl");
        return -1;
    }
    event.data.fd=socketfd;
    ret=epoll_ctl(epfd,EPOLL_CTL_ADD,socketfd,&event);
    if(ret==-1)
    {
        perror("epoll_ctl11");
        return -1;
    }
    int i,fdreadynum;
    while(1)
    {
        memset(eve,0,sizeof(eve));
        fdreadynum=epoll_wait(epfd,eve,3,-1);
        for(i=0;i<fdreadynum;i++)
        {
            if(socketfd==eve[i].data.fd)
            {
                memset(&client,0,sizeof(client));
                new_fd=accept(socketfd,(struct sockaddr*)&client,&addrlen);
                if(-1==new_fd)
                {
                    perror("accept");
                    return -1;
                }
                event.data.fd=new_fd;
                ret=epoll_ctl(epfd,EPOLL_CTL_ADD,new_fd,&event);
                if(ret==-1)
                {
                    perror("epoll_ctl12");
                    return -1;
                }

            }
            if(STDIN_FILENO==eve[i].data.fd)
            {
                memset(buf,0,sizeof(buf));
                ret=read(STDIN_FILENO,buf,sizeof(buf));//读取标准输入
                if(0==ret)
                {
                    printf("this is end\n");
                    break;
                }
                send(new_fd,buf,strlen(buf)-1,0);//写端管道内
            }
            if(new_fd==eve[i].data.fd)
            {
                memset(buf,0,sizeof(buf));
                ret=recv(new_fd,buf,sizeof(buf),0);
                if(0==ret)//写端断开，读端read返回，得到0
                {
                    printf("byebye");
                    fflush(stdin);
                    event.data.fd=new_fd;
                    ret=epoll_ctl(epfd,EPOLL_CTL_DEL,new_fd,&event);
                    if(ret==-1)
                    {
                        perror("epoll_ctl13");
                        return -1;
                    }
                    close(new_fd);
                }
                printf("%s\n",buf);
            }

        }
    }
    close(new_fd);
    close(socketfd);
    return 0;
}
