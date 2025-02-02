#ifndef _USER_HXX_
#define _USER_HXX_

#include <string>
#include <queue>

class User
{
	int _socket;
	std::string _username;
	std::queue<std::string> _requestQueue;
public:
	// send information to user.
	void send_information(const std::string&);
	// receive information.
	char* get_information();
	// constructor.
	// paramaters: socket, username.
	User(int, const std::string&);
	// get user's name.
	std::string get_username() const;
	// get socket.
	int get_socket() const;
	// try to identify.
	bool try_identify();
	// dequeue request.
	std::string dequeue_request();
};

#endif
