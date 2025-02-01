#ifndef _USER_HXX_
#define _USER_HXX_

#include <string>

class User
{
	int _socket;
	std::string _username;
public:
	// constructor.
	// paramaters: socket, username.
	User(int, const std::string&);
	// get user's name.
	std::string get_username() const;
};

#endif
