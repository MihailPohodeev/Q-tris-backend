#ifndef _ROOM_HXX_
#define _ROOM_HXX_

#include <list>
#include <mutex>

#include "GameParameter.hxx"
#include "User.hxx"

class Room
{
	mutable std::mutex _usersListGuard;
	// is room play of wait?
	bool _isPlay;
	// room's id.
	int _ID;
	// container of users.
	std::list<User> _users;
	// parameters of room.
	GameParameter _gameParameter;
public:
	// constructor.
	// parameter: count of players in Room.
	Room(int, const struct GameParameter&);
	// add user to room.
	void add_user(const User&);
	// get room's id.
	int get_ID() const;
	// get count of user.
	int get_players_count() const;
	// get capacity of room.
	int get_players_capacity() const;
	// start game.
	bool start_game();
	// handle.
	void handle_waiting_room();
};

#endif
