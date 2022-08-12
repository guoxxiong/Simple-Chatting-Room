#include <gtk/gtk.h>	// 头文件
#include <string.h>
#include <stdio.h>
#include <netinet/in.h> //定义数据结构sockaddr_in
#include <sys/socket.h> //定义socket函数以及数据结构
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h> 
#include <uuid/uuid.h>
GtkWidget *window;   //登录窗口
GtkWidget *home;	//主窗口
int clientfd,b_file;
struct sockaddr_in clientaddr; 
char user_name[50];
char fname[]="/var/tmp/";
//char userName[50]
//处理登录
void deal_pressed(GtkWidget *button, gpointer entry){
	int sendbytes;
	char *buff;
	struct hostent *host;//定义一个hostent结构体指针
	char wel[]="Welcome";
	host = gethostbyname("127.0.0.1");   //本地地址
	buff = (char *)malloc(9);
	
	const gchar  *text = gtk_entry_get_text(GTK_ENTRY(entry));
        //userName = text;
	if(strlen(text)==0){
		printf("不能为空\n");         // 提示 不能为空
	}
	else{	
		if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			perror("fail to create socket");
			exit(1);
		}
                //客户程序填充服务器端的资料
		bzero(&clientaddr, sizeof(clientaddr));//初始化，置零
		clientaddr.sin_family = AF_INET;//IPV4
		clientaddr.sin_port = htons((uint16_t)atoi("8787"));//atoi()函数将"8787"转化为整数，htons()函数将主机字符顺序转化为网络字符顺序
		clientaddr.sin_addr = *((struct in_addr *)host->h_addr);//IP地址
	
		if (connect(clientfd, (struct sockaddr *)&clientaddr, sizeof(struct sockaddr)) == -1)
		{
			perror("fail to connect");
			exit(1);
		}
		if ((sendbytes = send(clientfd, text, strlen(text), 0)) == -1)
		    {
			perror("fail to send");
			exit(1);
		    }

		if (recv(clientfd, buff, 7, 0) == -1)
		{
			perror("fail to recv");
			exit(1);
		}
		if(strcmp(buff,wel)==0){  //和从服务器端收到的"welcome"比较，判断是否连接上服务器
			strcpy(user_name,text);
			gtk_widget_destroy(window);//关闭登录窗口		
		}else{
			//  弹窗 提醒 提示 昵称重复
			GtkWidget *dialog;
                        //创建一个对话框
			dialog = gtk_message_dialog_new((gpointer)window,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK,
			"昵称重复，拒绝登录");
			gtk_window_set_title(GTK_WINDOW(dialog), "拒绝");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);	
			close(clientfd);
		}
	}
}
//登录界面
void login(int argc,char *argv[]){
	// 初始化
	gtk_init(&argc, &argv);		
	// 创建顶层窗口
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	// 设置窗口的标题
	gtk_window_set_title(GTK_WINDOW(window), "登录");
	// 设置窗口在显示器中的位置为居中
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	// 设置窗口的最小大小
	gtk_widget_set_size_request(window, 300, 200);
	// 固定窗口的大小
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE); 
	// "destroy" 和 gtk_main_quit 连接
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	//创建一个固定容器
 	GtkWidget *fixed = gtk_fixed_new(); 	
	// 将布局容器放窗口中
	gtk_container_add(GTK_CONTAINER (window), fixed);
	// 创建标签
	GtkWidget *label_one = gtk_label_new("请输入昵称");
	// 将按钮放在布局容器里
	gtk_fixed_put(GTK_FIXED(fixed), label_one,120,30);
	// 行编辑的创建
	GtkWidget *entry = gtk_entry_new();
	//设置最大长度
	gtk_entry_set_max_length(GTK_ENTRY(entry),50);
	// 设置行编辑允许编辑
	gtk_editable_set_editable(GTK_EDITABLE(entry), TRUE);

	gtk_fixed_put(GTK_FIXED(fixed), entry,70,60); 
	// 创建按钮
	GtkWidget *button = gtk_button_new_with_label("  登录  "); 
    
	gtk_fixed_put(GTK_FIXED(fixed), button,130,110);
	//绑定点击事件
	g_signal_connect(button, "pressed", G_CALLBACK(deal_pressed), entry);  
	// 显示窗口全部控件
	gtk_widget_show_all(window);
	//启动主循环
 	gtk_main();	
}
//发送目标用户窗口
GtkWidget *entryname;
//发送信息
void sendtouser(GtkButton  *button, gpointer entry){
	char *buf;
	buf = (char *)malloc(1024);
	memset(buf, 0, 1024);	
	int sendbytes;
	const gchar  *text = gtk_entry_get_text(GTK_ENTRY(entry));
	const char *but = gtk_button_get_label(button);
	if(strlen(text)==0){
		printf("不能为空\n");                               //发送内容不能为空
	}else{
		if(strcmp(but,"--发送--")==0){
			const gchar  *name = gtk_entry_get_text(GTK_ENTRY(entryname));
			if(strlen(name)==0){
					printf("name为空。\n");
					return;                                  //目标用户不能为空
			}
			sprintf(buf,"%s%s%s%s\n","User:",name,":",text);   //发送个单一用户‘User:’为标记
			if ((sendbytes = send(clientfd, buf, strlen(buf), 0)) == -1)
			{
				perror("fail to send");          
			}
			return ;		
		}else{
			sprintf(buf,"%s%s\n","All::",text);				//群发‘All:’为标记
			if ((sendbytes = send(clientfd, buf, strlen(buf), 0)) == -1)
			{
				perror("fail to send");
			}
			return ;
		}
	}
}
void savetxt(GtkButton  *button, gpointer entry){
	struct flock lock;
	lock.l_whence = SEEK_SET;//I_start从文件头开始计算偏移值
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_type = F_WRLCK;    //建立一个供写入用的锁定
	lock.l_pid = -1;    //初始化l_pid
	int src_file, dest_file;
	unsigned char buff[1024];
	int real_read_len;
	char txt_name[60];
	sprintf(txt_name,"%s%s","./msgsave_",user_name);//转换，连接存储到txt-name，
	src_file = open(fname, O_RDONLY);
	dest_file = open(txt_name,O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (src_file< 0 || dest_file< 0)
	{
		return;
	}	
	fcntl(dest_file, F_SETLKW, &lock);//获得/设置记录锁
	while ((real_read_len = read(src_file, buff, sizeof(buff))) > 0)//把src_file的内容读到buff里面
	{
		write(dest_file, buff, real_read_len); //写到目标文件里去
	}
	fcntl(dest_file, F_UNLCK, &lock);//删除之前建立的锁定
	close(dest_file);
	close(src_file);

}
// 文本框缓冲区
GtkTextBuffer *bufferuser;
GtkTextBuffer *buffernotice;
void *strdeal(void *arg){  //处理接受到的字符并调用相应的函数转发；
	char sign[10];
	char buf[1024];
	char s[1024];
	struct flock lock;
	lock.l_whence = SEEK_SET;//以文件开头为锁定的起始位置
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_type = F_WRLCK;    //初始化l_type
	lock.l_pid = -2;    //初始化l_pid
	while(1){
		memset(s, 0, strlen(s));//置空
		memset(sign, 0, strlen(sign));
		memset(buf, 0, strlen(buf));
		if(recv(clientfd, s, 1024, 0) <= 0)
		    {
			perror("fail to recv");
			close(clientfd);
			exit(1);
		    }
		int i=0;
		int n=0;
		int j=0;
		for(i;i<strlen(s);i++)
                {
		   if(n==1)
                   {
		     buf[j]=s[i];
		     j++;
	            }
		   else
		   {
		     if(s[i]==':')
                     {
			n++;
			sign[j]='\0';
			j=0;
			continue;				
		      }
		      sign[j]=s[i];
		      j++;
		    }	
		 }
		if(strcmp(sign,"User")==0){    //判断是用户消息还是系统消息
			b_file = open(fname,O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
			fcntl(b_file, F_SETLKW, &lock);	   //文件锁 临时储存消息记录		
			write(b_file, buf, strlen(buf));	
			fcntl(b_file, F_UNLCK, &lock);
			close(b_file);
			GtkTextIter start,end;//定义两个迭代器
			gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(bufferuser),&start,&end);//得到当前buffer中开始位置，结束位置的ITER
			gtk_text_buffer_insert(GTK_TEXT_BUFFER(bufferuser),&start,buf,strlen(buf));
		}else{	
			GtkTextIter start,end;
			gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffernotice),&start,&end);
			if(strcmp(buf,"over")==0){   // 收到over说明服务停止，所以程序退出  
				strcpy(buf,"服务停止，程序即将退出\n");   
                                gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffernotice),&start,buf,strlen(buf));
				sleep(2);
                                close(clientfd);
                                unlink(fname); 
				exit(0);
			}
	                gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffernotice),&start,buf,strlen(buf));
		}	
	}	
}
//主界面
void homepage(int argc,char *argv[]){
	char *buf;
	buf = (char *)malloc(1024);
	memset(buf, 0, 1024);
	pid_t pid;
	// 初始化
        gtk_init(&argc, &argv);		
	// 创建顶层窗口
	home = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	// 设置窗口的标题
	gtk_window_set_title(GTK_WINDOW(home),user_name);
	// 设置窗口在显示器中的位置为居中
	gtk_window_set_position(GTK_WINDOW(home), GTK_WIN_POS_CENTER);
	// 设置窗口的最小大小
	gtk_widget_set_size_request(home, 500, 400);
	// 固定窗口的大小
	gtk_window_set_resizable(GTK_WINDOW(home), FALSE); 
	// "destroy" 和 gtk_main_quit 连接
	g_signal_connect(home, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	//创建一个固定容器
 	GtkWidget *fixed = gtk_fixed_new(); 	
	gtk_container_add(GTK_CONTAINER(home), fixed);

	GtkWidget *label_two;
	GtkWidget *label_one;
	// 创建标签
	label_one = gtk_label_new("聊天内容：");
	// 将按钮放在布局容器里
	gtk_fixed_put(GTK_FIXED(fixed), label_one,20,10);	

	// 创建标签
	label_two = gtk_label_new("系统通知：");
	gtk_fixed_put(GTK_FIXED(fixed), label_two,320,10);

	label_two = gtk_label_new("发送用户：");
	gtk_fixed_put(GTK_FIXED(fixed), label_two,24,295);
	
	label_two = gtk_label_new("发送内容：");
	gtk_fixed_put(GTK_FIXED(fixed), label_two,24,335);


	// 行编辑的创建
	entryname = gtk_entry_new();
	// 最大长度
	gtk_entry_set_max_length(GTK_ENTRY(entryname),500);
	// 设置行编辑允许编辑
	gtk_editable_set_editable(GTK_EDITABLE(entryname), TRUE);
	gtk_fixed_put(GTK_FIXED(fixed), entryname,90,290); 

	GtkWidget *entry = gtk_entry_new();		
	gtk_entry_set_max_length(GTK_ENTRY(entry),500);
	gtk_editable_set_editable(GTK_EDITABLE(entry), TRUE);
	gtk_fixed_put(GTK_FIXED(fixed), entry,90,330);	
	// 创建按钮
	GtkWidget *bsend = gtk_button_new_with_label("--发送--");
	gtk_fixed_put(GTK_FIXED(fixed), bsend,325,300);

	GtkWidget *send_all = gtk_button_new_with_label("--群发--");
	gtk_fixed_put(GTK_FIXED(fixed), send_all,390,300);
	
	GtkWidget *save = gtk_button_new_with_label("--保存记录--");
	gtk_fixed_put(GTK_FIXED(fixed), save,345,340);	
	// 绑定回调函数
	g_signal_connect(bsend, "pressed", G_CALLBACK(sendtouser), entry);
	g_signal_connect(send_all, "pressed", G_CALLBACK(sendtouser), entry);
	g_signal_connect(save, "pressed", G_CALLBACK(savetxt), entry);

	// 文本框
	GtkWidget *view = gtk_text_view_new(); 
	gtk_widget_set_size_request (view, 280, 250);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);//使鼠标光标消失
	gtk_fixed_put(GTK_FIXED(fixed), view, 20, 30);
	// 获取文本缓冲区
	bufferuser=gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));     	
	

	GtkWidget *name_view = gtk_text_view_new(); 
	gtk_widget_set_size_request (name_view, 150, 230);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(name_view), FALSE);
  	gtk_fixed_put(GTK_FIXED(fixed), name_view, 320, 30);
	buffernotice=gtk_text_view_get_buffer(GTK_TEXT_VIEW(name_view));
	// 显示窗口全部控件
	gtk_widget_show_all(home);	
	
	int sendbytes, res;
	pthread_t thread;
	// 开启线程监听收到的数据
	res = pthread_create(&thread, NULL, strdeal, NULL);
	if (res != 0)
	{          
		exit(res);
	}
	usleep(10);
	gtk_main();
}

//主函数
int main(int argc,char *argv[])
{	
	uuid_t uuid;
	char str[36];
	uuid_generate(uuid);//生成一个不重复的未解析的"8-4-4-4-12"标准uuid文件名
	uuid_unparse(uuid, str);//将uuid内容解析为字符串，保存到str字符数组
	strcat(fname,str);//字符串连接
	
	login(argc,argv);
	homepage(argc,argv);
	
	unlink(fname);   //删除临时储存消息记录文件
	close(clientfd);
    return 0;
}
