#ifndef _FUNCTIONS_HXX_
#define _FUNCTIONS_HXX_

#include "GameParameter.hxx"
#include "Room.hxx"
#include "User.hxx"

// create new room.
int create_room(const GameParameter&);

// add user to room.
bool add_user_to_room(Room*, const User&);

// translate room from waiting state to playing state.
bool start_game(Room*);

#endif
