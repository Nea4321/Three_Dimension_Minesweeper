#pragma once
#include <array>
#include <algorithm>
#include <random>
#include <time.h>

#include "Resource.h"


class Func
{
public:
	Func();
	~Func();

	static struct cell
	{
		bool isMine = false;
		int stuckMines = 0;
		int status = 0; // 0 = ´ÝÈû 1 = ±ê¹ß 2 = ? 3 = ¿­¸²
	};
	
	std::array<std::array<std::array<cell, 9>, 9>, 9> board;

	static int mineCount;

	void placeMines(int __mCnt = 20);
	void calcStuckMines();
	bool openCell(int __z, int __x, int __y);
private:
	bool isOpen = false;

};

