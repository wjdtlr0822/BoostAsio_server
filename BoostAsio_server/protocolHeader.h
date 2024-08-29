#pragma once

#define CREATE_ROOM 0x0001
#define DELETE_ROOM 0x0002
#define SEND_MESSAGE 0x0003


/*
	protocol ±¸Á¶
	Header {
		Length : uint16;
		code : uint16;
	}
	Message{
		message : string;
	}
*/