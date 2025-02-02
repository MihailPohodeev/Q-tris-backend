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

std::vector<std::string> splitJson(const std::string&);

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
	std::string data = dequeue_request();
	if (data == "")
		return false;
	else
	{
		std::cout << data << '\n';
		json responceJSON;
		try
		{
			responceJSON = json::parse(data);
			std::string command = responceJSON["Command"];
			if (command == "Identification")
			{
				_username = responceJSON["Username"];
			}
			json response;
			response["Command"] = "Responce";
			response["Status"] = "Successful";
			
			std::cout << "RESP : " << response.dump() << '\n';
			send_information(response.dump());
		}
		catch (const json::parse_error& e)
		{
			std::cerr << "Identification: Parse error at byte : " << e.byte << " : " \
				<< e.what() << '\n';
		}
		catch (const json::type_error& e)
		{
			std::cerr << "Identification: Type error : " << e.what() << '\n';
		}
		catch (const json::out_of_range& e)
		{
			std::cerr << "Identification: Out of range error : " << e.what() << '\n';
		}
		return true;
	}
}

std::string User::dequeue_request()
{
	char* data = get_information();
	if (data != nullptr)
	{
		std::vector<std::string> vec = splitJson(data);
		for (auto& x : vec)
		{
			std::cout << "ELEMENT : " << x << '\n';
			_requestQueue.push(x);
		}
		delete data;
	}
	if (_requestQueue.size() == 0)
		return "";
	else std::cout << "REQUEST_QUQUE size : " << _requestQueue.size() << '\n';
	std::string result = _requestQueue.front();
	_requestQueue.pop();
	return result;
}

void User::send_information(const std::string& str)
{
	int totalSent = 0;
        while (totalSent < str.size())
        {
                int bytesSent = send(_socket, str.c_str(), str.size() + 1, 0);
                if (bytesSent < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                std::cout << "Send would block, try again later." << std::endl;
                        } else {
                                std::cerr << "Error sending data: " << strerror(errno) << std::endl;
                        }
                } else {
                        totalSent += bytesSent;
                        std::cout << "Sent " << bytesSent<<" bytes: " << str << std::endl;
                }
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
		std::cout << "RECEIVED : " << result << '\n';
		return result;
	}
	return nullptr;
}

std::vector<std::string> splitJson(const std::string& input) {
    std::vector<std::string> jsonObjects;
    int braceCount = 0;
    size_t start = 0;

    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '{') {
            if (braceCount == 0) {
                start = i; // Mark the start of a new JSON object
            }
            braceCount++;
        } else if (input[i] == '}') {
            braceCount--;
            if (braceCount == 0) {
                // We found a complete JSON object
                jsonObjects.push_back(input.substr(start, i - start + 1));
            }
        }
    }

    return jsonObjects;
}







