#include <stdio.h> // basic I/O
#include <stdlib.h>
#include <sys/types.h> // standard system types
#include <netinet/in.h> // Internet address structures
#include <sys/socket.h> // socket API
#include <arpa/inet.h>
#include <netdb.h> // host to IP resolution
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

#define HOSTNAMELEN 40 // maximal host name length; can make it variable if you want
#define BUFLEN 1024 // maximum response size; can make it variable if you want
#define BUFFER_SIZE 256
// prototype
void parametercheck(int argc, char *argv[]);
int openSocket(void);
void connectwithserver(int socket_id, struct sockaddr_in srv_ad);
struct hostent* getHostName(char* nameOhost);
void rcvMsg(int socket_id, char buff[], int buff_size);
void sendMsg(int socket_id, char buff[], int buff_size);
void clearBuff(char buff[]);
void showDirClient(void);
string getFName(char buff[]);
bool isExists(string f_name);
bool createFile(string f_name);

int main(int argc, char *argv[])
{
  	// variable definitions
	int c_sock_fd, port;
	struct sockaddr_in srv_ad;
	struct hostent *serv;
	char buff[BUFFER_SIZE];

  	// check that there are enough parameters
 	parametercheck(argc, argv);

    port = atoi(argv[2]);	// getting port no.

	c_sock_fd = openSocket();	// open socket
	
	char *nameOhost = argv[1];	
	serv = getHostName(nameOhost);	// get hostname

	// server details
	srv_ad.sin_family = AF_INET;
	srv_ad.sin_port = htons(port);
	int serv_l = serv->h_length;
	bcopy((char *)serv->h_addr, (char *) &srv_ad.sin_addr.s_addr, serv_l);
	
	connectwithserver(c_sock_fd, srv_ad);	// Connect to server

	clearBuff(buff);
	
	rcvMsg(c_sock_fd, buff, BUFFER_SIZE);	// Welcome Message
	cout << endl << buff << endl << endl;

	bool flag = true;
	string cmd;
	do
	{
		clearBuff(buff);
		cout << "\nc : ";
		fgets(buff, BUFFER_SIZE, stdin);

		if (strncmp(buff, "List client", 11) == 0)
		{
			clearBuff(buff);
			cmd = "_noA";
			strcpy(buff, cmd.c_str());

			sendMsg(c_sock_fd, buff, strlen(buff));
			
			

			showDirClient();
			
		}
		else if (strncmp(buff, "List server", 11) == 0)
		{	
			clearBuff(buff);
			cmd = "list";
			strcpy(buff, cmd.c_str());

			sendMsg(c_sock_fd, buff, strlen(buff));

			clearBuff(buff);
			rcvMsg(c_sock_fd, buff, BUFFER_SIZE);
			cout << buff << endl;

		}
		else if (strncmp(buff, "Create client", 13) == 0)
		{
			string f_name = getFName(buff);

			clearBuff(buff);
			cmd = "_noA";
			strcpy(buff, cmd.c_str());

			sendMsg(c_sock_fd, buff, strlen(buff));


			if (isExists(f_name))
			{
				cout << "Client: '" << f_name << "' already exists.";
			}
			else if (createFile(f_name))
			{
				cout << "Client: '" << f_name << "' has been created.";
			}

		}
		else if (strncmp(buff, "Create server", 13) == 0)
		{
			string f_name = getFName(buff);

			clearBuff(buff);
			cmd = "_cre";
			strcpy(buff, cmd.c_str());
			
			sendMsg(c_sock_fd, buff, strlen(buff));
			
			clearBuff(buff);
			strcpy(buff, f_name.c_str());
			sendMsg(c_sock_fd, buff, strlen(buff));
			
			clearBuff(buff);
			rcvMsg(c_sock_fd, buff, BUFFER_SIZE);
			cout << buff << endl;			

		}
		else if (strncmp(buff, "exit", 4) == 0)
		{
			flag = false;

			clearBuff(buff);
			cmd = "exit";
			strcpy(buff, cmd.c_str());
			
			sendMsg(c_sock_fd, buff, strlen(buff));	
		}
		else
		{
			clearBuff(buff);
			cmd = "help";
			strcpy(buff, cmd.c_str());

			sendMsg(c_sock_fd, buff, strlen(buff));	

			clearBuff(buff);
			rcvMsg(c_sock_fd, buff, BUFFER_SIZE);
			cout << buff << endl;
		}

	} while (flag);

	close(c_sock_fd);

  return 0;
}

void parametercheck(int argc, char* argv[])
{
	if (argc != 3)
    {
      fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
      exit(-1);
    }
}

int openSocket(void)
{
	int socket_id = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_id < 0)
	{
		cout << "Socket Opening Failed\n";
		exit(-1);
	}
	return socket_id;
}

void connectwithserver(int socket_id, struct sockaddr_in srv_ad)
{
	int srv_size = sizeof(srv_ad);
	if (connect(socket_id, (struct sockaddr *) &srv_ad, srv_size) < 0)
	{
		cout << "Connection Failed\n";
		exit(-1);
	}
}

struct hostent* getHostName(char* nameOhost)
{
	struct hostent *serv;
	serv = getHostName(nameOhost);
	if (serv == NULL)
	{
		cout << "Error: Host Unavailable\n";
		exit(-1);
	}
	return serv;
}

void rcvMsg(int socket_id, char buff[], int buff_size)
{
	int x = read(socket_id, buff, buff_size);
	if (x < 0)
	{
		cout << "Failure of reading from socket\n";
		exit(-1);
	}
}

void sendMsg(int socket_id, char buff[], int buff_size)
{
	int x = write(socket_id, buff, buff_size);
	if (x < 0)
	{
		cout << "Failure of writing to socket\n";
		exit(-1);
	}
}

void clearBuff(char buff[])
{
	bzero(buff, BUFFER_SIZE);	
}

void showDirClient(void)
{
	cout << "\n> List of client's files\n";
	DIR *_dir;
	struct dirent *_files;

	_dir = opendir("./");
	if (_dir != NULL)
	{
		while (_files = readdir(_dir))
		{
			cout << "> " << _files->d_name << endl;
		}
	}
}

string getFName(char buff[])
{
	vector<string> cmd;
	string temp = "";
	for (int i = 0; i < strlen(buff) - 1; i++)
	{
		if (buff[i] == ' ')
		{
			cmd.push_back(temp);
			temp = "";
		}
		else
		{
			temp += buff[i];
		}
	}
	cmd.push_back(temp);

	return cmd[cmd.size() - 1];
}

bool isExists(string f_name)
{

	ifstream in_file;
	in_file.open(f_name.c_str());
	if (in_file.fail())
	{
		return false;
	}
	
	in_file.close();
	return true;
}

bool createFile(string f_name)
{
	ofstream of_file;
	of_file.open(f_name.c_str());
	if (of_file.fail())
	{
		return false;
	}
	
	of_file.close();
	return true;
}