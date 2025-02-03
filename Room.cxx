#include <string>

#include "json.hpp"
#include "Room.hxx"

using json = nlohmann::json;

Room::Room(int id, const GameParameter& gameParameter) : _ID(id), _gameParameter(gameParameter), _isPlay(false) {}

// add user to room.
void Room::add_user(const User& user)
{
	_users.push_back(user);
}

int Room::get_ID() const
{
	return _ID;
}

// get count of user.
int Room::get_players_count() const
{
	return _users.size();
}

// get capacity of room.
int Room::get_players_capacity() const
{
	return _gameParameter.playersCount;
}

// start the game.
bool Room::start_game()
{
	if (_users.size() < _gameParameter.playersCount)
		return false;
	return true;
}

// handle waiting room.
void Room::handle_waiting_room()
{
	bool roomIsReady = true;
	if (_users.size() < _gameParameter.playersCount)
		roomIsReady = false;
	for (std::list<User>::iterator it = _users.begin(); it != _users.end();)
	{
		std::cout << "handle " << it->get_username() << " user.\n";
		if( !it->is_user_connected() )
		{
			std::cout << "LIQUIDIROVAN!\n";
			it->close_socket();
			it = _users.erase(it);
			continue;
		}
		std::string request = it->dequeue_request();
		if (request == "")
			continue;
		std::cout << "User : " << it->get_username() << " - request : " << request << '\n';	
		json requestJSON;
		try
		{
			requestJSON = json::parse(request);
			std::string command = requestJSON["Command"];
			if (command == "BeReady")
			{
				if (requestJSON["IsReady"] == "Yes")
				{
					it->make_ready();
					std::cout << "User " << it->get_username() << " is ready.\n";
				}
				else
					it->make_non_ready();
			}
		}
		catch (const json::parse_error& e)
                {
                        std::cerr << "Waiting room User's request : Parse error at byte : " << e.byte << " ; string : " << request << "; exception : " << e.what() << '\n';
                }
                catch (const json::type_error& e)
                {
		std::cerr << "Waiting room User's request : Type error ; string : " << request << "; exception : " << e.what() << '\n';
                }
                catch (const json::out_of_range& e)
                {
                        std::cerr << "Waiting room User's request : Out of range error; string : " << request << "; exception : " << e.what() << '\n';
                }
		if (!it->is_ready())
			roomIsReady = false;
		++it;
	}
	if (roomIsReady)
		std::cout << "Room is ready!!!\n";
}
