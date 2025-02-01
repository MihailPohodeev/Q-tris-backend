#include "User.hxx"

User::User(int socket, const std::string& username) : _socket(socket), _username(username) {}

std::string User::get_username() const
{
	return _username;
}
