#pragma once

namespace game_event{
    enum {
        INVALID = -1,
        SERVER_CLOSED = 0,
        SERVER_INFO_REQUEST = 1,
        SERVER_INFO_UPDATE = 2,

        PLAYER_CONNECTION = 3,
        PLAYER_DISCONNECTION = 4,
        PLAYER_ID_SET = 5,

        START_MATCH = 6,

        MATCH_MAP_REQUEST = 7,
        MATCH_MAP_RESPONSE = 8,
        MATCH_HOLE_CHANGE = 9,

        MATCH_FORCED_TERMINATION = 10,
    };
}
