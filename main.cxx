#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <list>

#include "Room.hxx"
#include "User.hxx"

#define PORT 18881
#define BUFFER_SIZE 2048

void listening_new_connections();
void handle_new_users();
void handle_game_processes();

int serverSocket;
struct sockaddr_in address;
int addrlen;
std::list<User> users;
std::list<Room> rooms;

int main()
{
	int opt = 1;
	addrlen = sizeof(address);
	char buffer[BUFFER_SIZE] = {0};

	if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
	        perror("socket failed");
        	exit(EXIT_FAILURE);
	}

	// Attach socket to the port
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
	    perror("setsockopt");
	    exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Bind the socket to the address
	if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	// Start listening for connections
	if (listen(serverSocket, 20000) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	std::cout << "Server is listening on port " << PORT << std::endl;

	std::thread listeningThread(listening_new_connections);
	std::thread usersHandlingThread(handle_new_users);
	std::thread roomsHandlingThread(handle_game_processes);

	listeningThread.join();
	usersHandlingThread.join();
	roomsHandlingThread.join();

	return 0;
}

void listening_new_connections()
{
	while(1)
	{
		int newSocket;
		if ((newSocket = accept(serverSocket, \
			(struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}
		else
		{
			std::cout << "successful connection!\n";
			users.push_back(User(newSocket, "barbaris"));
		}
	}
}

void handle_new_users()
{
	while(1)
	{
		for(std::list<User>::iterator it = users.begin(); it != users.end(); ++it)
		{
			std::cout << it->get_username() << '\n';
		}
	}
}

void handle_game_processes()
{
	while(1)
	{
		for(std::list<Room>::iterator it = rooms.begin(); it != rooms.end(); ++it)
		{
			std::cout << "It's room!";
		}
	}
}
