#include <string>

#include "json.hpp"
#include "Room.hxx"

using json = nlohmann::json;

Room::Room(int id, const GameParameter& gameParameter) : _ID(id), _gameParameter(gameParameter), _isPlay(false), _isReady(false) {}

// add user to room.
void Room::add_user(const User& user)
{
	std::lock_guard<std::mutex> lock(_usersListGuard);
	_users.push_back(user);
}

int Room::get_ID() const
{
	return _ID;
}

// get count of user.
int Room::get_players_count() const
{
	std::lock_guard<std::mutex> lock(_usersListGuard);
	return _users.size();
}

// get capacity of room.
int Room::get_players_capacity() const
{
	return _gameParameter.playersCount;
}

// is game can start?
bool Room::is_start_game() const
{
	return _isReady;
}

// start the game.
void Room::notify_users_about_game_start()
{
	json notification;
	notification["Command"] = "CanStartGame";
	std::string result = notification.dump();
	for (auto it = _users.begin(); it != _users.end(); ++it)
	{
		it->send_information(result);
	}
}

// handle waiting room.
void Room::handle_waiting_room()
{
	bool roomIsReady = true;
	std::lock_guard<std::mutex> lock(_usersListGuard);
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
		{
			++it;
			continue;
		}
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
	_isReady = roomIsReady;
}

// handle process room.
void Room::handle_process_room()
{
	json dataFrame;
	dataFrame["Command"] = "DataFrame";
	dataFrame["Data"] = json::array();

	for (auto it = _users.begin(); it != _users.end(); ++it)
	{
		if( !it->is_user_connected() )
		{
			std::cout << "LIQUIDIROVAN!\n";
			it->close_socket();
			it = _users.erase(it);
			--it;
			continue;
		}

		std::string request = it->dequeue_request();
		if (request == "")
			continue;
		
		json requestJSON;
		try
		{
			requestJSON = json::parse(request);
			std::string command = requestJSON["Command"];
			if (command == "GetRoomParameters")
			{
				json response;
				response["Command"] = "RoomParameters";
				response["Users"] = json::array();
				for (auto jt = _users.begin(); jt != _users.end(); ++jt)
				{
					if (it == jt)
						continue;
					json userData;
					userData["ID"] = jt->get_socket();
					userData["Username"] = jt->get_username();
					response["Users"].push_back(userData);
					it->send_information(response.dump());
				}
			}
			else if (command == "GameFrame")
			{
				json data;
				data["UserID"] = it->get_socket();
				data["Data"] = requestJSON["Data"];
				dataFrame["Data"].push_back(data);
			}
		}
		catch(const json::parse_error& e)
		{
			std::cerr << "Can't handle user in process ; Parse error at byte : " << e.byte << " : " << e.what() << '\n';
		}
		catch (const json::type_error& e)
		{
			std::cerr << "Can't handle user in process ; Type error : " << e.what() << '\n';
		}
		catch (const json::out_of_range& e)
		{
			std::cerr << "Can't handle user in process ; Out of range error : " << e.what() << '\n';
		}
	}

	for (auto it = _users.begin(); it != _users.end(); ++it)
	{
		std::cout << "Data : " << dataFrame.dump() << '\n';;
		it->send_information(dataFrame.dump());
	}
}



