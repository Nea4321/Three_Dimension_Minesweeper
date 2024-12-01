#pragma once
#include <array>
#include <algorithm>
#include <random>
#include <time.h>

#include "Resource.h"

struct mineCell
{
	bool isMine = false;
	int stuckMines = 0;
	int status = 0; // 0 = ´ÝÈû 1 = ±ê¹ß 2 = ? 3 = ¿­¸²
};

class Func
{
public:
	Func();
	~Func();


	std::array<std::array<std::array<mineCell, 9>, 9>, 9> board;
	mineCell* thisCell = nullptr;
	int mineCount;

	void placeMines(const int __mCnt = 50);
	void calcStuckMines();
	bool openCell(const int __z, const int __x, const int __y);
	void setFlag(const int __z, const int __x, const int __y);
	bool isMineAt(const int __z, const int __x, const int __y) const;
	void resetBoard();
	bool safeFirstClick(const int __z, const int __x, const int __y);
	void placeSingleMine(int excludeZ, int excludeX, int excludeY);
private:
	bool isOpen = false;
};

