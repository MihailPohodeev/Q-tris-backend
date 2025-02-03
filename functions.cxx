#include <unordered_map>

#include "functions.hxx"
#include "User.hxx"
#include "Room.hxx"

extern std::unordered_map<int, Room*> waitingRooms;
extern int roomIDGenerator;

int create_room(const GameParameter& gameParameter)
{
	waitingRooms[roomIDGenerator] = new Room(roomIDGenerator, gameParameter);
	int result = roomIDGenerator;
	roomIDGenerator++;
	return result;
}

bool add_user_to_room(Room* room, const User& user)
{
	if (!room)
		return false;
	if (room->get_players_count() >= room->get_players_capacity())
		return false;
	room->add_user(user);
	return true;
}
