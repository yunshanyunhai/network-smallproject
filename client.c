#include "wrap.h"
#include "font.h"
#include "project1.h"
#define API_IP "47.107.155.132"
#define API_HTONS 80
int i=0;
int fd;
// 接收OOB数据
void get_weath(int i);
void display(char *p,int i);
int press(int n)         //获取按键的坐标
{
	int input_fd=open("/dev/input/event0",O_RDONLY);
	if(input_fd==-1)
	{
		perror("open input_fd failed");
		return -1;
	}
	
	struct input_event s;
	
	int x,y;
	
	while(1)
	{
		read(input_fd,&s,sizeof(s));
		
		if(s.type==3 && s.code==0)
			x=s.value;
		if(s.type==3 && s.code==1)
			y=s.value;
		if(s.type == 1 && s.code == 330 && s.value > 0)
				break;
	} 
	
	int rea_x = x*800/1024;
	int rea_y = y*480/600;
	
	if((700<rea_x && rea_x<800) && (0<rea_y && rea_y<120))
		n=0;
	else if((700<rea_x && rea_x<800) && (120<rea_y && rea_y<240))
		n=1;
	else if((700<rea_x && rea_x<800) && (240<rea_y && rea_y<360))
		n=2;
	else if((700<rea_x && rea_x<800) && (360<rea_y && rea_y<480))
		n=3;
	else
		n=-1;
	
	close(input_fd);
	return n;
}
void recvOOB(int sig)
{
	printf("%d\n",__LINE__);
    char oob;
    recv(fd, &oob, 1, MSG_OOB);

    if(oob == 'a')
    {
        printf("你私聊的ID有误，对方不存在或已下线\n");
    }

}
// 专门用户读取键盘输入，发给服务器
void *routine(void *arg)
{
    int fd = *(int *)arg;

    char buf[100];
    while(1)
    {
        bzero(buf, 100);
        fgets(buf, 100, stdin);

        write(fd, buf, strlen(buf));
    }

    pthread_exit(NULL);
}
void *information(void *arg)
{
	int fd = *(int *)arg;
	char buf[100];
	while(1)
	{
		bzero(buf,100);
		
		int retre=recv(fd,buf,100,0);
		if(retre==0)
		{
			break;
		}
		if(strcmp(buf,"list")==0)
		{
			
			while(1)
			{
				bzero(buf,100);
				recv(fd,buf,100,0);
				if(strcmp(buf,"end")==0)
					break;
				
				display(buf,i);
				i++;
				if(i==4)i=0;
				printf("%s\n",buf);
			}
		}
		else
		{
			display(buf,i);
				i++;
				if(i==4)i=0;
			printf("收到：%s\n",buf);	
		}
	}
	
	pthread_exit(NULL);
}
//arm-linux-gcc   test.c  -o  main   -L./ -lfont    -lm 

//初始化Lcd
struct LcdDevice *init_lcd(const char *device)
{
	//申请空间
	struct LcdDevice* lcd = malloc(sizeof(struct LcdDevice));
	if(lcd == NULL)
	{
		return NULL;
	} 

	//1打开设备
	lcd->fd = open(device, O_RDWR);
	if(lcd->fd < 0)
	{
		perror("open lcd fail");
		free(lcd);
		return NULL;
	}
	
	//映射
	lcd->mp = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcd->fd,0);

	return lcd;
}

void display(char *p,int i)
{	
    //初始化Lcd
	struct LcdDevice* lcd = init_lcd("/dev/fb0");			
	//打开字体	
	font *f = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");	  
	//字体大小的设置
	fontSetSize(f,32);	
		//创建一个画板（点阵图）
		bitmap *bm = createBitmapWithInit(700,120,4,getColor(0,255,255,255)); //也可使用createBitmapWithInit函数，改变画板颜色
		char buf[1024]={0};
		strcpy(buf,p);
		//将字体写到点阵图上
		fontPrint(f,bm,10,0,buf,getColor(0,255,0,0),608);			
		//把字体框输出到LCD屏幕上
		show_font_to_lcd(lcd->mp,0,0+i*120,bm);
		//关闭字体，关闭画板
		fontUnload(f);  //字库不需要每次都关闭 
		destroyBitmap(bm);//画板需要每次都销毁 
}
void displayx(char *p,int i)
{	
    //初始化Lcd
	struct LcdDevice* lcd = init_lcd("/dev/fb0");			
	//打开字体	
	font *f = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");	  
	//字体大小的设置
		fontSetSize(f,64);	
		//创建一个画板（点阵图）
		bitmap *bm = createBitmapWithInit(100,120,4,getColor(0,255,255,255)); //也可使用createBitmapWithInit函数，改变画板颜色
		char buf[1024]={0};
		strcpy(buf,p);
		//将字体写到点阵图上
		fontPrint(f,bm,0,30,buf,getColor(0,255,255,0),0);			
		//把字体框输出到LCD屏幕上
		show_font_to_lcd(lcd->mp,700,0+i*120,bm);
		//关闭字体，关闭画板
		fontUnload(f);  //字库不需要每次都关闭 
		destroyBitmap(bm);//画板需要每次都销毁 
}

void get_weath(int i)
{
	int tcp_socket = socket(AF_INET,SOCK_STREAM,0);
	//2.链接服务器  
	//设置服务器的IP地址信息  
	struct sockaddr_in  addr;  
	addr.sin_family   = AF_INET; //IPV4 协议  
	addr.sin_port     = htons(API_HTONS); //端口 80  ,所有的HTTP 服务器端口都是  80  
	addr.sin_addr.s_addr = inet_addr(API_IP); //服务器的IP 地址信息
	int ret=connect(tcp_socket,(struct sockaddr *)&addr,sizeof(addr));
	if(ret < 0)
	{
		perror("");
		exit(0); 
	}
	else
	{
		printf("链接网络服务器成功\n");
	}	
		//重点！！定制HTTP 请求协议  
		//https://    cloud.qqshabi.cn    /api/hitokoto/hitokoto.php 
		char *http = "GET /api.php?key=free&appid=0&msg=广州天气 HTTP/1.1\r\nHost:api.qingyunke.com\r\n\r\n";
	
	//发送请求
	write(tcp_socket,http,strlen(http));
	
	//读取返回数据的头数据
	char head[1024] ={0};
	int size=read(tcp_socket,head,1024);

	
	//指向头数据的末尾 
	char buf[1024]={0};
	char *began =  strstr(head,"广州天气");
	char *end = strstr(head,"风力")+4;
	int lenth = end-began;
	strncpy(buf,began,lenth+6);
	printf("%s\n",buf);
	display(buf,i);
	i++;
	if(i==4)i=0;
	
}
void get_joke(int i)
{
	int tcp_socket = socket(AF_INET,SOCK_STREAM,0);
	//2.链接服务器  
	//设置服务器的IP地址信息  
	struct sockaddr_in  addr;  
	addr.sin_family   = AF_INET; //IPV4 协议  
	addr.sin_port     = htons(API_HTONS); //端口 80  ,所有的HTTP 服务器端口都是  80  
	addr.sin_addr.s_addr = inet_addr(API_IP); //服务器的IP 地址信息
	int ret=connect(tcp_socket,(struct sockaddr *)&addr,sizeof(addr));
	if(ret < 0)
	{
		perror("");
		exit(0); 
	}
	else
	{
		printf("链接网络服务器成功\n");
	}	
		//重点！！定制HTTP 请求协议  
		//https://    cloud.qqshabi.cn    /api/hitokoto/hitokoto.php 
		char *http = "GET /api.php?key=free&appid=0&msg=笑话 HTTP/1.1\r\nHost:api.qingyunke.com\r\n\r\n";
	
	//发送请求
	write(tcp_socket,http,strlen(http));
	
	//读取返回数据的头数据
	char head[1024] ={0};
	int size=read(tcp_socket,head,1024);

	
	//指向头数据的末尾 
	char buf[1024]={0};
	char *began =  strstr(head,"★");
	char *end = strstr(head,"提示")-4;
	int lenth = end-began;
	strncpy(buf,began,lenth+6);
	printf("%s\n",buf);
	display(buf,i);
	i++;
	if(i==4)i=0;
}
//129.204.114.106
int main(int argc, char **argv)
{
    // 1，创建TCP套接字
    fd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // 2，准备服务器的IP+PORT
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]); // 填充服务器的IP
    //addr.sin_port   = htons(6000); // 约定好的PORT
    addr.sin_port   = htons(2333); // 约定好的PORT

    // 3，对服务器发起连接请求
    int ID;
    if(Connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0)
    {
        read(fd, &ID, sizeof(ID));
        ID = ntohl(ID);
        printf("连接成功，我分配到的ID是:%d\n", ID);
    }

    // 4，创建一条线程
	pthread_t readinformation;
	pthread_create(&readinformation,NULL,information,(void *)&fd);
	 pthread_t tid;
    pthread_create(&tid, NULL, routine, (void *)&fd);
    // 准备好接收OOB
    signal(SIGURG, recvOOB);

    // 将来在fd上产生的信号，请系统发给进程getpid()
    fcntl(fd, F_SETOWN, getpid());

    // 5，主线程专门读取服务器发来的普通数据
	displayx("天气",0);
	displayx("好友",1);
	displayx("笑话",2);
	displayx("日志",3);
    char buf[100];
	while(1)
	{
		int n=-1;
		n=press(n);
		if(n==1)
		{
			send(fd,"list",4,0);
		}
		if(n==0)
		{
			get_weath(i);
		}
		if(n==2)
		{
			get_joke(i);
		}	
		if(n==3)
		{
			
		}
	}
	
	pthread_join(readinformation,NULL);
	
	return 0;
}