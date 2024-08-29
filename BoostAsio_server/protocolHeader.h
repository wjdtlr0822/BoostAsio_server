#pragma once

#define CREATE_ROOM 0x0001
#define DELETE_ROOM 0x0002
#define SEND_MESSAGE 0x0003

/*
* 모든 프로코톨 header
* Header {
		Length : uint16;
		code : uint16;
	}

----------------------------------------------------------------------
* CREATE_ROOM 
(client -> server)
	body{
		room name : char[]
	}
(server -> client)
	body{
		result : bool;
	}
----------------------------------------------------------------------
	DELETE_ROOM
(client -> server)
	body{
		room name : char[]
	}
(server -> client)
	body{
		result : bool;
	}
----------------------------------------------------------------------
	SEND_MESSAGE
(server <-> client)
	body{
		message : char[];
	}
----------------------------------------------------------------------
*/