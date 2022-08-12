#include <signal.h>
#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <sys/types.h> //数据类型定义
#include <sys/stat.h> //文件属性
#include <netinet/in.h> //定义数据结构sockaddr_in
#include <sys/socket.h> //提供socket函数和数据结构
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <arpa/inet.h>
#include <errno.h>       
#include<sys/sem.h>
#include <time.h>
#include <pthread.h>  //线程
#define THREAD_NUMBER 30 //最大链接数
#define MYPORT 8787
#define BUFFER_SIZE 1024
#define WELCOME "Welcome"
struct Users{              //表示用户与线程之间的关联
	char name[50];
	pthread_t thread;
	char buf[BUFFER_SIZE];
	int client_fd;
	char address[20];
	int login;         //是否在线
}users[THREAD_NUMBER];
int sem_id; 
 
//返回系统时间
void get_now_time(char *nt){
	time_t tmpcal_ptr;
	struct tm *tmp_ptr = NULL;
	time(&tmpcal_ptr);
	tmp_ptr = localtime(&tmpcal_ptr);
	sprintf(nt,"%d:%d:%d", tmp_ptr->tm_hour, tmp_ptr->tm_min, tmp_ptr->tm_sec);	
}
								  
//初始化信号量
int init_sem(int sem_id, int init_value){
	union semun{
		int val;
		struct semid_ds *buf;
		unsigned short *array;
	};
	union semun sem_union;
	sem_union.val = init_value;
	if(semctl(sem_id, 0, SETVAL, sem_union) == -1){ //用来执行在信号量集上的控制操作(设置信号量集中一个单独的信号量的值)
		syslog(LOG_ERR, "Initialize semaphore");
		perror("Initialize semaphore");
		return -1;
	}
	return 0;
}
//删除信号量
int del_sem(int sem_id){
	union semun{
		int val;
		struct semid_ds *buf;
		unsigned short *array;
	};
	union semun sem_union;
	if(semctl(sem_id, 1, IPC_RMID, sem_union)==-1){
		syslog(LOG_ERR, "Delete semaphore");
		perror("Delete semaphore");
		return -1;
	}
}
//P 操作函数
int sem_p(int sem_id){
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b, 1)==-1){
		syslog(LOG_ERR, "P operation");
		perror("P operation");
		return -1;
	}
	return 0;
}
//V 操作函数
int sem_v(int sem_id){
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b, 1) == -1){
		syslog(LOG_ERR, "V operation");
		perror("V operation");
		return -1;
	}
	return 0;
}


//发送给在线的所有人

void send_all(char loadsend[BUFFER_SIZE]){
	int j;
	for(j=0;j<THREAD_NUMBER;j++){
		if(users[j].login==1){
			send(users[j].client_fd, loadsend, strlen(loadsend), 0);		
		}
	}
}
//发送给个人
void send_only(char name[50],char loadsend[BUFFER_SIZE]){
	int j;
	for(j=0;j<THREAD_NUMBER;j++){
		if((users[j].login==1)&&(strcmp(users[j].name,name)==0)){ //遍历所有在线用户名，匹配上就发送
			send(users[j].client_fd, loadsend, strlen(loadsend), 0);		
		}
	}
}
//处理接受到的字符并调用相应的函数转发；
void strdeal(char s[],char se_name[]){  
	char sign[8];
	char name[56];
	char buf[BUFFER_SIZE];
	char send_buf[BUFFER_SIZE];
	memset(sign, 0, strlen(sign));
	memset(name, 0, strlen(name));
	memset(buf, 0, strlen(buf));
	memset(send_buf, 0, strlen(send_buf));
	char nt[10];
	int i=0;
	int n=0;
	int j=0;
	for(i;i<strlen(s);i++){
		if(n>1){
			buf[j]=s[i];
			j++;	
		}else if(n==1){
			if(s[i]==':'){
				n++;
				name[j]='\0';
				j=0;
				continue;
			}
			name[j]=s[i];
			j++;
		}else{
			if(s[i]==':'){
				n++;
				sign[j]='\0';
				j=0;
				continue;
			}
			sign[j]=s[i];
			j++;
		}	
	}
	if(strcmp(sign,"All")==0){
		get_now_time(nt);
		sprintf(send_buf,"%s用户< %s >群发消息->\t\t%s:\n\t%s","User:",se_name,nt,buf);
		send_all(send_buf);
	}else{
		get_now_time(nt);
		sprintf(send_buf,"%s用户< %s >------>\t\t%s:\n\t%s","User:",se_name,nt,buf);
		send_only(name,send_buf);
	}
		
	
}

//线程函数
void *thrd_func(void *arg)     
{
	long i = (long)arg;	
	int recvbytes;
	char nt[10];
	while(1){
		memset(users[i].buf , 0, sizeof(users[i].buf));
		if ((recvbytes = recv(users[i].client_fd, users[i].buf, BUFFER_SIZE, 0)) <= 0)//结束当前线程
		{
			char end[100];
			memset(end, 0, 100);
			get_now_time(nt);
			sprintf(end,"%s%s%s\n用户：%s%s\n","Inform:",nt,"-通知：",users[i].name,"退出聊天室");  // 添加时间：
			send_all(end);			
			users[i].login = 0;
			sem_v(sem_id);
		        close(users[i].client_fd);
			int n=0;
			int j=0;
			for(j;j<THREAD_NUMBER;j++){
				if(users[j].login==0)
					n++;		
			}		
			printf("%s用户退出，还可以上线%d个\n",users[i].name, n);
                        memset(users[i].name,0,strlen(users[i].name));
			printf("目前在线用户：\n");
                	int k = 0;
                	for(k;k<THREAD_NUMBER;k++){
                            if(k != i) 
		            { printf("%s\t",users[k].name);}		
			}
                        printf("\n");

			pthread_exit(0);   //结束线程
		}
		strdeal(users[i].buf,users[i].name);
		
		
	}
}


//端口绑定函数。创建套接字，并绑定到指定端口
int bindPort(unsigned short int port)
{
	int sockfd, sendbytes;
	struct sockaddr_in my_addr;
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0))== -1)//创建基于流套接字
	{
		syslog(LOG_ERR, "socket");
		perror("socket");
	}

	/*设置sockaddr_in 结构体中相关参数*/   
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET; //IPV4协议族
	my_addr.sin_port = htons(port); //转换端口为网络字节序
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero),0,8);

	/* 允许重复使用本地地址与套接字进行绑定 */
	int i = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
	if (bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		syslog(LOG_ERR, "fail to bind");
		perror("fail to bind");
	}
 	printf("success!\n");
	return sockfd;
}
/*自定义信号处理函数*/
void my_func(int sign_no)
{	
	char loadsend[100];
	sprintf(loadsend,"%s%s","Inform:","over");   //发送通知 over 客户端收到后自动退出
	send_all(loadsend);
	printf("即将退出服务器\n");
	fflush(stdout);//清除读写缓冲区
	sleep(1);
	exit(0);
}
int main(int argc, char *argv[])
{
	
	int sockfd,recvbytes;
	char loadsend[100];
	long i=0;  //循环来做连接
	int sendbytes, res;
	struct sockaddr_in client_sockaddr;
	int sin_size;
	int repetition,j,n;
	sockfd = bindPort(MYPORT);
	for(i=0;i<THREAD_NUMBER;i++)//初始化使所有用户全部下线
		users[i].login=0;
	i=0;
	if (listen(sockfd, THREAD_NUMBER) == -1)//监听socket能处理的最大并发连接请求数
	{	
		syslog(LOG_ERR, "listen");
		perror("listen");
	}
	printf("Listening....\n");
	sem_id = semget(ftok("/", 1), 1, 0666|IPC_CREAT);//创建一个新的信号量
	init_sem(sem_id, THREAD_NUMBER);
	char nt[10];
	openlog("daemon_syslog", LOG_PID, LOG_DAEMON);   //打开系统日志服务
	//捕捉信号量，并跳转到信号处理函数处 
	signal(SIGINT, my_func);
	signal(SIGQUIT, my_func);
	signal(SIGTSTP,my_func);
	while(1){
		sem_p(sem_id);
		while(i<THREAD_NUMBER){                       //遍历列表
			if(users[i].login==0)                  //找到可用资源  0 表示可用  1 不可用
				break;
			i++;
			if(i==THREAD_NUMBER){                 //循环查找
				i=0;	
				}
			}
		n=0;
		j=0;
		//找到在线的人数n
		for(j;j<THREAD_NUMBER;j++){ 
			if(users[j].login==1)
				n++;		
		}		
		printf("已经上线%d个用户，还可以上线%d个\n",n,THREAD_NUMBER-n);               
                printf("目前在线用户：\n");
                j = 0;
                for(j;j<THREAD_NUMBER;j++){ 
		      printf("%s\t",users[j].name);		
		}		
		if(users[i].login==0){
			printf("等待第下个连接\n");
			/*调用accept()函数，等待客户端的连接*/			
			if ((users[i].client_fd = accept(sockfd,(struct sockaddr*)&client_sockaddr, &sin_size)) == -1){
				syslog(LOG_ERR, "accept");				
				perror("accept");
			
			}
			//获取客户端的IP
			inet_ntop(AF_INET, &client_sockaddr.sin_addr, users[i].address, sizeof(users[i].address));
			if ((recvbytes=recv(users[i].client_fd, users[i].name, BUFFER_SIZE, 0)) <= 0){
				sem_v(sem_id);
				continue;			
			};
			printf("本次连接的是用户：%s\n",users[i].name);
			j=0;	
			repetition=0;//判断是否重名		
			for(j=0;j<THREAD_NUMBER;j++){
				if(users[j].login==1)	//从连接的用户寻找
				if(strcmp(users[i].name,users[j].name)==0){
					repetition=1;	
				}
			}
			if(repetition==1){
				send(users[i].client_fd, "g", strlen("g"), 0);
				sem_v(sem_id);
				continue;		
			}else{
				users[i].login = 1;
				send(users[i].client_fd, WELCOME, strlen(WELCOME), 0);
			}			
			//广播上线消息。
			memset(loadsend, 0, 100);
			get_now_time(nt);
			sprintf(loadsend,"%s%s%s\n用户：%s%s\n","Inform:",nt,"-通知：",users[i].name,"-上线了！大家欢迎！");
			send_all(loadsend);
			res = pthread_create(&users[i].thread, NULL, thrd_func, (void*)i);  //开启线程
			if (res != 0)
			{	
				syslog(LOG_ERR, "Create thread failed");
				perror("Create thread failed");
				users[i].login = 0;
				sem_v(sem_id);
			}
		}
	}
	close(sockfd);
	return 0;
}
