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

void Func::resetBoard() {
    board = { 0 };
    placeMines();
    calcStuckMines();
}

bool Func::safeFirstClick(const int __z, const int __x, const int __y) {
    try {
        bool boardChanged = false;

        // 클릭한 셀과 주변 셀 검사
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int nz = __z + dz, nx = __x + dx, ny = __y + dy;
                    if (nz >= 0 && nz < SIZE && nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE) {
                        if (board.at(nz).at(nx).at(ny).isMine) {
                            // 지뢰를 다른 곳으로 옮기기
                            board.at(nz).at(nx).at(ny).isMine = false;
                            placeSingleMine(nz, nx, ny);
                            boardChanged = true;
                        }
                    }
                }
            }
        }

        if (boardChanged) {
            calcStuckMines();  // 지뢰 위치가 변경되었으므로 주변 지뢰 수 재계산
        }

        return true;
    }
    catch (const std::out_of_range&) {
        return false; // 잘못된 좌표
    }
}

// 새로운 함수: 단일 지뢰 배치
void Func::placeSingleMine(int excludeZ, int excludeX, int excludeY) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, SIZE - 1);

    while (true) {
        int z = dis(gen);
        int x = dis(gen);
        int y = dis(gen);

        // 원래 위치와 그 주변을 제외하고 지뢰 배치
        if (abs(z - excludeZ) > 1 || abs(x - excludeX) > 1 || abs(y - excludeY) > 1) {
            if (!board[z][x][y].isMine) {
                board[z][x][y].isMine = true;
                break;
            }
        }
    }
}