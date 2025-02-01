#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <list>

#include "json.hpp"
#include "Room.hxx"
#include "User.hxx"

#define PORT 18881
#define BUFFER_SIZE 2048

void listening_new_connections();
void handle_new_users();
void handle_game_processes();
int  set_non_blocking(int sock);

int serverSocket;
struct sockaddr_in address;
int addrlen;
std::list<User> unidentifiedUsers;
std::list<User> users;
std::list<Room> rooms;
char buffer[BUFFER_SIZE] = {0};

using json = nlohmann::json;

int main()
{
	int opt = 1;
	addrlen = sizeof(address);

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

	set_non_blocking(serverSocket);

	std::cout << "Server is listening on port " << PORT << std::endl;

	std::thread listeningThread(listening_new_connections);
	std::thread usersHandlingThread(handle_new_users);
	std::thread roomsHandlingThread(handle_game_processes);

	listeningThread.join();
	usersHandlingThread.join();
	roomsHandlingThread.join();

	close(serverSocket);

	return 0;
}

void listening_new_connections()
{
	while(1)
	{
		int newSocket;
		if ((newSocket = accept(serverSocket, nullptr, nullptr)) < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				continue;
			else
				std::cerr << "Accept failed!";
		}
		else
		{
			std::cout << "successful connection!\n";
			unidentifiedUsers.push_back(User(newSocket, ""));
		}
	}
}

void handle_new_users()
{
	while(1)
	{
		for(std::list<User>::iterator it = unidentifiedUsers.begin(); \
				it != unidentifiedUsers.end();)
		{
			if (it->try_identify())
			{
				users.push_back(*it);
				it = unidentifiedUsers.erase(it);
			}
			else
				++it;
		}
		for(std::list<User>::iterator it = users.begin(); it != users.end(); ++it)
		{
			char* data = it->get_information();
			if (data == nullptr)
				continue;
			json responceJSON;
			try
			{
				responceJSON = json::parse(data);
				std::string command = responceJSON["Command"];
				if (command == "CreateRoom")
				{

				}
				else if (command == "ConnectToRoom")
				{

				}
			}
			catch(const json::parse_error& e)
			{
				std::cerr << "Parse error at byte : " << e.byte << " : " \
					<< e.what() << '\n';
			}
			catch (const json::type_error& e)
			{
				std::cerr << "Type error : " << e.what() << '\n';
			}
			catch (const json::out_of_range& e)
			{
				std::cerr << "Out of range error : " << e.what() << '\n';
			}
			delete data;
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

int set_non_blocking(int sock)
{
	int flags = fcntl(sock, F_GETFL, 0);
	return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}
