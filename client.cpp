#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#define _CRT_SECURE_NO_WARNINGS 1
#include<winsock2.h>
#include<stdlib.h>
#include<iostream>
#pragma comment(lib, "ws2_32.lib")
using namespace std;
struct FileName {//存储形式："文件名.扩展名"
	char Fname[64];
	int len;
};
class FileSender
{
private:
	FILE * fp;
	SOCKET sock;
	sockaddr_in addr;
	FileName fn;
	char temp[1024];
public:
	FileSender()
	{
		sock = socket(AF_INET, SOCK_STREAM, 0);
	}
	int Connect(const char *ip, int port)
	{
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.S_un.S_addr = inet_addr(ip);
 
		if (connect(sock, (sockaddr*)&addr, sizeof(addr)) != 0)
		{
			cout << "failed.." << endl;
		}
		return 1;
	}
	int openFile(const char *path)
	{
		char name[32], ext[16];
		_splitpath(path, NULL, NULL, name, ext);
		strcat_s(name, 32, ext);
		strcpy_s(fn.Fname, 32, name);
		fn.len = strlen(fn.Fname);
		fp = fopen(path, "rb");
		return 0;
	}
	int SendFile(const char *path)
	{
		openFile(path);
		send(sock, (char*)&fn, sizeof(FileName),0);
		fseek(fp, 0, SEEK_END);
		long long siz = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		send(sock, (CHAR*)&siz, sizeof(siz),0);
		siz = siz / 1024;
		long long index=0;
		int num;
		while (1)
		{
			num=fread(temp, 1, 1024, fp);
			if (num == 0)
				break;
			send(sock, temp, num, 0);
			index++;
			cout << (int)index*100/siz<<"%" << num << endl;
		}
		cout << "Successfully send" << endl;
		return 0;
	}
 
};
int main()
{ 
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	char path[] = "D:\\MyFiles\\Code\\C++\\tcpForImg\\1.jpg";//发送文件的路径
	FileSender fs;
	if(fs.Connect("127.0.0.1", 3000))
	fs.SendFile(path); 
	system("pause");
}