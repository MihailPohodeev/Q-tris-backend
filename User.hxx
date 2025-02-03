#ifndef _USER_HXX_
#define _USER_HXX_

#include <string>
#include <queue>

#include "setup.hxx"

class User
{
	// personal user's score data.
	U64 _score;
	U32 _lines;
	U32 _level;

	// is user ready to play?
	bool _isReady;
	// socket for communication with client, also ID.
	int _socket;
	// username of player.
	std::string _username;
	// queue of requests.
	std::queue<std::string> _requestQueue;

	// is socket valid
	bool is_socket_valid() const;
public:
	// send information to user.
	void send_information(const std::string&);
	// receive information.
	std::string get_information();
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
	// make ready.
	void make_ready();
	// make non-ready.
	void make_non_ready();
	// is user ready?
	bool is_ready() const;
	// is user connected?
	bool is_user_connected() const;
	// disconnect.
	void close_socket();

	// getters and setters.
	U64 get_score() const;
	void set_score(U64);
	U32 get_lines() const;
	void set_lines(U32);
	U32 get_level() const;
	void set_level(U32);
};

#endif
