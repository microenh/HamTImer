#pragma once

#ifndef NDEBUG
	#include <stdio.h>
	#define Debug(__info,...) printf("Debug: " __info,##__VA_ARGS__)
#else
	#define Debug(__info,...)  
#endif
