#include <ros/ros.h>
#include <std_msgs/String.h>
#include <vector>

#include <sys/stat.h>
#include <termios.h>
//#include <term.h>
#include <unistd.h>
  
//socket
#include <sys/types.h>  
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>

#include <sstream>
#include <stdlib.h>
#include <string>

#define SERVER_PORT    30000
#define LENGTH_OF_LISTEN_QUEUE 20
using namespace std;

int main(int argc, char** argv)
{
	ros::init(argc, argv, "server2topic");
	ros::NodeHandle nh_;
	ros::Publisher voice_pub_;
	voice_pub_ = nh_.advertise<std_msgs::String>("recognizer_output", 1000);
  	
  	//设置一个socket地址结构server_addr,代表服务器internet地址, 端口
	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(server_addr)); //把一段内存区的内容全部设置为0
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);
	 
	//创建用于internet的流协议(TCP)socket,用server_socket代表服务器socket
	int server_socket = socket(PF_INET,SOCK_STREAM,0);
	if( server_socket < 0)
	{
		printf("Create Socket Failed!");
		exit(1);
	}
	int opt =1;
	setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	     
	//把socket和socket地址结构联系起来
	if( bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)))
	{
		printf("Server Bind Port : %d Failed!", SERVER_PORT);
		exit(1);
	}
	 
	//server_socket用于监听
	if ( listen(server_socket, LENGTH_OF_LISTEN_QUEUE) )
	{
		printf("Server Listen Failed!");
		exit(1);
	}
	cout<<"server is listening\n";
	while(1) //服务器端要一直运行
	{
		//定义客户端的socket地址结构client_addr
		struct sockaddr_in client_addr;
		socklen_t length = sizeof(client_addr);
	 
		int new_server_socket = accept(server_socket,(struct sockaddr*)&client_addr,&length);
		if ( new_server_socket < 0)
		{
		    printf("Server Accept Failed!\n");
		    break;
		}
			 
		char t[200];
		cout<<"-- strlen(t) = "<<strlen(t)<<"\n";
		length = recv(new_server_socket, t, 200, 0);
		if (length < 0)
		{
			printf("Server Recieve Data Failed!\n");
			return false;
		}
		else if (t != "")
		{
			cout<<"I am in"<<endl;
//			cout<<t<<"\n";
		    cout<<t<<endl;
			cout<<"123"<<endl;
			std_msgs::String msg;
			msg.data = t;
			voice_pub_.publish(msg);
			//sleep(1);
			
			
		}
		
		//关闭与客户端的连接
		close(new_server_socket);
	}
	//关闭监听用的socket
	close(server_socket);
	return 0;
  		
}
