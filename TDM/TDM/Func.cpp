#include "Func.h"

Func::Func() {
    mineCount = 0;
}

Func::~Func() {

}

void Func::placeMines(const int __mCnt) {
    mineCount = __mCnt;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, SIZE - 1);

    for (int z = 0; z < SIZE; z++) {
        for (int x = 0; x < SIZE; x++) {
            for (int y = 0; y < SIZE; y++) {
                board[z][x][y].isMine = false;
                board[z][x][y].stuckMines = 0;
                board[z][x][y].status = 0;
            }
        }
    }

    int placedMines = 0;
    while (placedMines < mineCount) {
        int x = dis(gen);
        int y = dis(gen);
        int z = dis(gen);

        if (!board[z][x][y].isMine) {
            board[z][x][y].isMine = true;
            placedMines++;
        }
    }

}

void Func::calcStuckMines() {
    for (int z = 0; z < SIZE; z++) {
        for (int x = 0; x < SIZE; x++) {
            for (int y = 0; y < SIZE; y++) {
                
                if (board[z][x][y].isMine) continue;

                int stuckCnt = 0;
                
                for (int sz = -1; sz <= 1; sz++) {
                    for (int sx = -1; sx <= 1; sx++) {
                        for (int sy = -1; sy <= 1; sy++) {
                            if (sz == 0 && sx == 0 && sy == 0) continue;

                            try {
                                if (board.at(z + sz).at(x + sx).at(y + sy).isMine == true) stuckCnt++;
                            }
                            catch(const std::out_of_range&) {}
                            
                        }
                    }
                }

                board[z][x][y].stuckMines = stuckCnt;


            }
        }
    }


}



bool Func::openCell(const int __z, const int __x, const int __y) {
    try {
        thisCell = &board.at(__z).at(__y).at(__x);

        // 이미 열려있거나 깃발이 꽂힌 셀이면 아무 것도 하지 않음
        if (thisCell->status % 2) {
            return false;
        }

        thisCell->status = 3;

        if (thisCell->isMine) {
            //아직 생각중
            return true;
        }

        if (!thisCell->stuckMines) {
            for (int oz = -1; oz <= 1; oz++) {
                for (int ox = -1; ox <= 1; ox++) {
                    for (int oy = -1; oy <= 1; oy++) {
                        openCell(__z + oz, __x + ox, __y + oy);
                    }
                }
            }
        }
        return false;

    }
    catch (const std::out_of_range&) { 
        return false; 
    }

}

void Func::setFlag(const int __z, const int __x, const int __y) {
    thisCell = &board.at(__z).at(__y).at(__x);
    if (thisCell->status == 0) thisCell->status++;
    else if (thisCell->status == 1) thisCell->status++;
    else if (thisCell->status == 2)thisCell->status = 0;
}


bool Func::isMineAt(const int __z, const int __x, const int __y) const {
    return board[__z][__x][__y].isMine;
}
