#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <list>
#include <unordered_map>
#include <mutex>

#include "json.hpp"
#include "Room.hxx"
#include "User.hxx"
#include "functions.hxx"

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
std::unordered_map<int, Room*> waitingRooms;
std::unordered_map<int, Room*> processedRooms;
char buffer[BUFFER_SIZE] = {0};

int roomIDGenerator = 1;

std::mutex unidentifiedUsersGuard;
std::mutex waitingRoomsGuard;

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
	std::cout << "listen new connections.\n";
	while(1)
	{
		sockaddr_in client_addr{};
		socklen_t client_addr_len = sizeof(client_addr);
		int clientSocket = accept(serverSocket, \
				(struct sockaddr*)&client_addr, &client_addr_len);
		if (clientSocket < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				continue;
			else
				std::cerr << "Accept failed!";
		}
		else
		{
			int flags = fcntl(clientSocket, F_GETFL, 0);
			if (flags == -1) {
				std::cerr << "Failed to get client socket flags: " << strerror(errno) << std::endl;
				close(clientSocket);
				continue;
			}
			if (fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
				std::cerr << "Failed to set client socket to non-blocking: " << strerror(errno) << std::endl;
				close(clientSocket);
				continue;
			}

			int flag = 1;
			if (setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0) {
				std::cerr << "Error disabling Nagle's algorithm: " << \
					strerror(errno) << std::endl;
				close(clientSocket);
				continue;
			}

			std::cout << "successful connection!\n";
			std::lock_guard<std::mutex> lock(unidentifiedUsersGuard);
			unidentifiedUsers.push_back(User(clientSocket, ""));
		}
	}
}

void handle_new_users()
{
	std::cout << "begin handle new users.\n";
	while(1)
	{
		{
			std::lock_guard<std::mutex> lock(unidentifiedUsersGuard);
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
		}
		for(std::list<User>::iterator it = users.begin(); it != users.end(); )
		{
			std::string data = it->dequeue_request();
			if (data == "")
			{
				++it;
				continue;
			}
			std::cout << "DATA : " << data << '\n';
			json responseJSON;
			try
			{
				responseJSON = json::parse(data);
				std::string command = responseJSON["Command"];
				if (command == "CreateRoom")
				{
					GameParameter gp = {responseJSON["Parameters"]["PlayersCount"], responseJSON["Parameters"]["StartLevel"], (std::string)responseJSON["Parameters"]["QueueType"] == std::string("Same") ? true : false};
					{
						std::lock_guard<std::mutex> lock(waitingRoomsGuard);
						int roomID = create_room(gp);
						Room* room = waitingRooms[roomID];
						bool insertUser = add_user_to_room(room, *it);
					}
					it = users.erase(it);
					std::cout << "Successful created room!\n";
					continue;
				}
				else if (command == "ConnectToRoom")
				{
					// TODO.
				}
			}
			catch(const json::parse_error& e)
			{
				std::cerr << "User : "<< it->get_username() << " exception ; Parse error at byte : " << e.byte << " : " \
					<< e.what() << '\n';
			}
			catch (const json::type_error& e)
			{
				std::cerr << "User : "<< it->get_username() << " exception ; Type error : " << e.what() << '\n';
			}
			catch (const json::out_of_range& e)
			{
				std::cerr << "User : "<< it->get_username() << " exception ; Out of range error : " << e.what() << '\n';
			}
			++it;
		}
	}
}

void handle_game_processes()
{
	std::cout << "Handle rooms!\n";
	while(1)
	{
		{
			std::lock_guard<std::mutex> lock(waitingRoomsGuard);
			for (const auto& pair : waitingRooms)
			{
				pair.second->handle_waiting_room();
			}
		}

		for (const auto& pair : processedRooms)
		{
			// TODO.
		}
	}
}

int set_non_blocking(int sock)
{
	int flags = fcntl(sock, F_GETFL, 0);
	return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}
