/*********************************************************************/
                        //powered by kevinballll
                            //仅供个人学习用
/*********************************************************************/

#include "kernel_list.h"
#include "head.h"

#define HTONS 2333

char msg_buf[1024]={0};

// 用户链表节点
struct node
{
    int ID;
    int connfd;
    struct sockaddr_in addr;

    struct list_head list; // 使用内核链表组织所有的用户节点
};

// 用户管理
struct clients
{
    pthread_mutex_t m;
    int active_users;

    struct node *cli; // 用户链表

}*users;


struct clients *init_user_list()
{
    struct clients *users = calloc(1, sizeof(struct clients));
    if(users != NULL)
    {
        pthread_mutex_init(&users->m, NULL);
        users->active_users = 0;

        users->cli = calloc(1, sizeof(struct node));
        if(users->cli != NULL)
        {
            INIT_LIST_HEAD(&users->cli->list);
        }
        else
        {
            free(users);
            perror("分配内存失败");
            return NULL;
        }
    }

    return users;
}

// 创建新的用户节点
struct node * new_client(struct sockaddr_in peerAddr, int connfd)
{
    struct node *newcli = calloc(1, sizeof(struct node));
    if(newcli != NULL)
    {
        newcli->addr   = peerAddr;
        newcli->connfd = connfd;
        newcli->ID     = rand()%10000;

        INIT_LIST_HEAD(&newcli->list);
    }

    return newcli;
}


void show(struct node *user)
{
    struct list_head *pos;
    struct node *p;

    // 遍历内核链表
    list_for_each(pos, &users->cli->list)
    {
        p = list_entry(pos, struct node, list);

		printf("[%d]%d\n",users->active_users,p->ID);
    }
}


//服务端公告功能
void ser_msg(const char *msg)
{
    struct list_head *pos;
    struct node *p;

    // 遍历内核链表
    list_for_each(pos, &users->cli->list)
    {
        p = list_entry(pos, struct node, list);

        Write(p->connfd, msg, strlen(msg));

    }
}


//客户端私聊功能
void private_talk(struct node *sender,const char *msg, int receiverID)
{
    struct list_head *pos;
    struct node *p;

    // 遍历链表，寻找接受者
    list_for_each(pos, &users->cli->list)
    {
        p = list_entry(pos, struct node, list);

        if(p->ID == receiverID)
        {
            Write(p->connfd, msg, strlen(msg));
            return;
        }
    }
}


void *func()
{
    while(1)
    {
        printf("1.发布公告  2.关闭服务器\n");
        int  a=0;
        scanf("%d",&a);

        if(a==1)
        {
            printf("请输入需要发送内容：");
            bzero(msg_buf,sizeof(msg_buf));

            scanf("%s",msg_buf);
            char msg_tit[1024] = {0};
            sprintf(msg_tit,"公告：%s",msg_buf);
            ser_msg(msg_tit);
            printf("已发送公告\n");
        }
        if(a==2)
        {
            printf("服务器已关闭\n");
            kill(getpid(), SIGINT);
        }
    }
}


void *routine(void *arg)
{
    // 获取用户节点指针
    struct node *client = (struct node *)arg;

    // 将自己分离出去，防止变僵尸
    pthread_detach(pthread_self());

    // 不断地读取用户发来的数据，根据实际情况做出操作
    char buf[1024];
	int socknew = client->connfd;
    while(1)
    {
        bzero(buf, 1024);
        int n = read(client->connfd, buf, 1024);

        // 读取套接字出错了
        if(n == -1)
        {
            printf("读取【%s:%hu】数据失败:%s\n",
                    inet_ntoa(client->addr.sin_addr),
                    ntohs(client->addr.sin_port), strerror(errno));
            break;
        }

        // 客户端断开了连接
        if(n == 0)
        {
            printf("客户端【%s:%hu】下线了，再见！\n",
                    inet_ntoa(client->addr.sin_addr),
                    ntohs(client->addr.sin_port));
            break;
        }
		if(strcmp(buf,"list")==0)
		{
			struct list_head *pos;
			struct node *p;
			char number[100];
			
			send(socknew,"list",4,0);
			int i=0;
			list_for_each(pos, &users->cli->list)
			{
				p = list_entry(pos, struct node, list);
				bzero(number,100);
				i++;
				sprintf(number,"Good_friend %d:socket=%d;ID=%d",i,users->active_users,p->ID);
				send(socknew,number,sizeof(number),0);
			}
			send(socknew,"end",3,0);
		}
        char *p = strchr(buf, ':'); // 从左到右查找':'
		
        // 客户端发来私聊消息:  1234:你好啊！
       if(p != NULL)
        {
            private_talk(client, p+1, atoi(buf));
        }
    }

    // 处理后续工作
    close(client->connfd);

    // 加锁，保护链表
    pthread_mutex_lock(&users->m);

    // 删除本用户节点
    struct list_head *del = &client->list;
    list_del(del);
    users->active_users--;
	show(client);
    // 解锁
    pthread_mutex_unlock(&users->m);

    free(client);
    pthread_exit(NULL);
}


int main()
{
    // 1，创建TCP套接字
    int fd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // 2，准备好自身的IP+PORT，便于客户端连接
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 自动获取本机IP
    addr.sin_port   = htons(HTONS); // PORT

    // 3，绑定地址
    Bind(fd, (struct sockaddr *)&addr, sizeof(addr));

    // 4，设置套接字的监听状态
    Listen(fd, 3);

    // 创建一条空的用户链表
    users = init_user_list();

    // 5，准备好客户端地址的结构体
    struct sockaddr_in peerAddr;
    socklen_t len = sizeof(peerAddr);

    printf("服务器已就绪\n");


    // 循环等待客户端的连接
    while(1)
    {
        pthread_t tid1;
        pthread_create(&tid1, NULL, func,NULL);


        bzero(&peerAddr, len);
        int connfd = Accept(fd, (struct sockaddr *)&peerAddr, &len);

        printf("欢迎【%s:%hu】上线\n", inet_ntoa(peerAddr.sin_addr),
                            ntohs(peerAddr.sin_port));
		
        // 创建新的用户节点
        struct node *user = new_client(peerAddr, connfd);

        // 将用户的ID告知客户端
        printf("发送ID:%d\n", user->ID);
        int ID2 = htonl(user->ID);
        Write(connfd, &ID2, sizeof(ID2));

        // 更新用户链表
        pthread_mutex_lock(&users->m);

        list_add_tail(&user->list, &users->cli->list);
        users->active_users++;
        show(user);
        pthread_mutex_unlock(&users->m);

        // 创建一条专门服务这个用户的线程
        pthread_t tid;
        pthread_create(&tid, NULL, routine, (void *)user);
    }

    return 0;
}
