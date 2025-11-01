#pragma once
#include "Game.h"

class Connect4 : public Game
{
public:
    Connect4();
    ~Connect4();

    void        setUpBoard() override;
    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder &holder) override;
    bool        canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool        canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    void        stopGame() override;

    void        updateAI() override;
    bool        gameHasAI() override { return true; }
    Grid*       getGrid() override { return _grid; }

private:
    // Board helpers
    Bit*        pieceForPlayer(int playerNumber);
    bool        dropInColumn(int col, int playerNumber, bool animate = true);
    int         lowestEmptyRow(int col) const;
    Player*     ownerAt(int x, int y) const;
    bool        fourInARow(int sx, int sy, int dx, int dy, Player* p) const;

    // AI helpers
    int         chooseAIMove();
    bool        wouldWin(int col, int playerNumber);

    Grid*       _grid;
};
