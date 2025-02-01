#ifndef _USER_HXX_
#define _USER_HXX_

#include <string>

class User
{
	int _socket;
	std::string _username;
public:
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
};

#endif
