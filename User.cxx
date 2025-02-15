#include "User.hxx"
#include "json.hpp"
#include <iostream>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>

using json = nlohmann::json;

std::vector<std::string> splitJson(const std::string&);

User::User(int socket, const std::string& username) : _socket(socket), _username(username), _sizeOfBuffer(32000)
{
	_buffer = new char[_sizeOfBuffer];
}

User::~User()
{
	delete [] _buffer;
}

// copy constructor.
User::User(const User& usr) : _socket(usr._socket), _username(usr._username), _sizeOfBuffer(usr._sizeOfBuffer)
{
	_buffer = new char[_sizeOfBuffer];
	memcpy(_buffer, usr._buffer, _sizeOfBuffer);
}

// operator=
User User::operator=(User usr)
{
	char* temp = _buffer;
	_buffer = usr._buffer;
	usr._buffer = temp;
	return *this;
}


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
			response["Command"] = "Acception";
			response["Status"] = "Successful"; // or Failure.
			response["UserID"] = _socket;
			
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
	static std::string lastWord = "";

	if (_requestQueue.size() == 0)
	{
		size_t receivedBytes = get_information();

		if (receivedBytes > 0)
		{
			auto begin = _buffer;
			auto end   = _buffer + receivedBytes;
			do
			{
				auto it = std::find_if(begin, end, [](char x) { return x == '\0';});
				if (it == end)
				{
					*end = 0;
					try
					{
						std::string result(begin);
						json::parse(result);
						_requestQueue.push(result);
					}
					catch (const json::parse_error& e)
					{
						if (lastWord != "")
							lastWord += std::string(begin);
						else
							lastWord = std::string(begin);
					}
					break;
				}
				else
				{
					if (lastWord != "")
					{
						if (lastWord.size() > 16'000)
						{
							lastWord = "";
							begin = it + 1;
							continue;
						}
						lastWord += std::string(begin);
						try
						{
							json::parse(lastWord);
							_requestQueue.push(lastWord);
							lastWord = "";
						}
						catch(const json::parse_error& e)
						{
							std::cerr << "Can't parse last word : " << e.what() << '\n';
						}
						begin = it + 1;
						continue;
					}
					_requestQueue.push(std::string(begin));
					begin = it + 1;
				}
			} while(1);
		}
	}

	if (_requestQueue.size() == 0)
		return "";

	std::string result = _requestQueue.front();
	_requestQueue.pop();
	return result;
}

void User::send_information(const std::string& str)
{
	struct pollfd fds[1];
	fds[0].fd = _socket;
	fds[0].events = POLLOUT;

	int poll_count = poll(fds, 1, 100);
	if (poll_count < 0) {
		std::cerr << "Error in poll()" << std::endl;
		return;
	}
	else if (poll_count == 0) {
		std::cout << "Timeout, no activity" << std::endl;
		return;
	}

	if (!(fds[0].revents & POLLOUT))
	{
		std::cerr << "Can't send data to user : " << _socket << '\n';
	       return;
	}
	
	size_t totalSent = 0;
	while (totalSent < str.size())
	{
		int bytesSent = send(_socket, str.c_str() + totalSent, str.size() - totalSent + 1, 0);
		if (bytesSent < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				std::cout << "Send would block, try again later." << std::endl;
			}
			else {
				std::cerr << "Error sending data: " << strerror(errno) << std::endl;
			}
			break;
		}
		else
			totalSent += bytesSent;
	}
}

// get information from socket and put it in _buffer.
size_t User::get_information()
{
	int receivedBytes = 0;

	struct pollfd fds[1];
	fds[0].fd = _socket;
	fds[0].events = POLLIN;

	int poll_count = poll(fds, 1, 100);
	
	if (poll_count < 0) {
            std::cerr << "Error in poll()\n";
	    return 0;
        }
	else if (poll_count == 0) {
	    return 0;
        }
	
	if (fds[0].revents && POLLIN)
	{
		receivedBytes = recv(_socket, _buffer, _sizeOfBuffer - 1, 0);
	}
	else
	{
		std::cerr << "Can't receive data from user : " << _socket << '\n';
		return 0;
	}

	if (receivedBytes < 0)
	{
		std::cerr << "GOT < 0 data! User : " << _socket << '\n';
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			std::cerr << "recv failed: " << strerror(errno) << std::endl;
			return 0;
		}
		return 0;
	}
	else if (receivedBytes == 0)
	{
		std::cerr << "Client is unavailable.\n";
		return 0;
	}
	
	return receivedBytes;
}

// make ready.
void User::make_ready()
{
	_isReady = true;
}

// make non-ready.
void User::make_non_ready()
{
	_isReady = false;
}

// is user ready?
bool User::is_ready() const
{
	return _isReady;
}

U64 User::get_score() const
{
	return _score;
}

void User::set_score(U64 score)
{
	_score = score;
}

U32 User::get_lines() const
{
	return _lines;
}

void User::set_lines(U32 lines)
{
	_lines = lines;
}

U32 User::get_level() const
{
	return _level;
}

void User::set_level(U32 level)
{
	_level = level;
}

// is socket valid
bool User::is_socket_valid() const
{
	struct pollfd pfd;
	pfd.fd = _socket;
	pfd.events = POLLIN;

	int ret = poll(&pfd, 1, 0);
	if (ret == -1)
	{
		std::cerr << "Error poll: " << strerror(errno) << std::endl;
		return true;
	}

	if (ret > 0 && (pfd.revents & POLLIN))
	{
		char buffer[1];
		ssize_t bytesReceived = recv(_socket, buffer, sizeof(buffer), MSG_PEEK);
		if (bytesReceived == 0) {
			return false;
		}
	}
	return true;
}

// is user connected?
bool User::is_user_connected() const
{
	return is_socket_valid();
}

void User::close_socket()
{
	close(_socket);
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







