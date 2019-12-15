#ifndef DEFINES_H
#define DEFINES_H

enum class WndEvents {
	NOTHING	=  0x0,
	MOVE	=  0x1,
	MINIMIZE=  0x2,
	MAXIMIZE=  0x4,
	RESIZE	=  0x8,
	DESTROY	= 0x10,
};

#endif