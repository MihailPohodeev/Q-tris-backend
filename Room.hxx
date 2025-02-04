#ifndef _ROOM_HXX_
#define _ROOM_HXX_

#include <list>
#include <mutex>

#include "GameParameter.hxx"
#include "User.hxx"

class Room
{
	mutable std::mutex _usersListGuard;
	bool _isReady;
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
	// is game can start?
	bool is_start_game() const;
	// start game.
	void notify_users_about_game_start();

	// handle waiting room.
	void handle_waiting_room();
	// handle process of game
	void handle_process_room();
};

#endif
