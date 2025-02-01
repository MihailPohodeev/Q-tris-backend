#include "User.hxx"
#include "json.hpp"
#include <iostream>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>

using json = nlohmann::json;

User::User(int socket, const std::string& username) : _socket(socket), _username(username) {}

std::string User::get_username() const
{
	return _username;
}

int User::get_socket() const
{
	return _socket;
}

bool User::try_identify()
{
	char* responce = get_information();
	if (responce == nullptr)
		return false;
	else
	{
		json responceJSON;
		try
		{
			responceJSON = json::parse(responce);
			std::string command = responceJSON["Command"];
			if (command == "Identification")
			{
				_username = responceJSON["Username"];
			}
		}
		catch (const json::parse_error& e)
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
		delete responce;
		return true;
	}
}

char* User::get_information()
{
	std::vector<char> receivedData;
	receivedData.assign(1024, 0);
	int currentPosition = 0;
	int bytesRead = 0;
	do
	{
		bytesRead = recv(_socket, &receivedData[0] + currentPosition, \
				receivedData.size() - currentPosition, 0);
		//if ((float)receivedData.size() / receivedData.capacity() > 0.6)
		currentPosition += bytesRead;
	} while(bytesRead > 0);
	if (bytesRead < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			std::cerr << "recv failed";
		}
	}
	if (currentPosition > 0)
	{
		receivedData[currentPosition] = 0;
		char* result = new char[currentPosition + 1];
		strncpy(result, &receivedData[0], currentPosition + 1);
		return result;
	}
	return nullptr;
}









