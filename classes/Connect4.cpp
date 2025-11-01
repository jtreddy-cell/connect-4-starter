#include "Connect4.h"

Connect4::Connect4()
{
    _grid = new Grid(7, 6);
}

Connect4::~Connect4()
{
    delete _grid;
}

Bit* Connect4::pieceForPlayer(int playerNumber)
{
    Bit* bit = new Bit();
    // playerNumber: 0 -> human, 1 -> AI (following TicTacToe convention)
    // Use yellow for player 1, red for player 0 (arbitrary but consistent with resources)
    const char* tex = (playerNumber == 0) ? "red.png" : "yellow.png";
    bit->LoadTextureFromFile(tex);
    bit->setOwner(getPlayerAt(playerNumber));
    return bit;
}

void Connect4::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 7;
    _gameOptions.rowY = 6;
    _grid->initializeSquares(80, "boardsquare.png");

    startGame();
}

bool Connect4::actionForEmptyHolder(BitHolder &holder)
{
    // Drop a piece in the column of the clicked square
    ChessSquare* square = static_cast<ChessSquare*>(&holder);
    int col = square->getColumn();
    int current = getCurrentPlayer()->playerNumber();
    return dropInColumn(col, current, true);
}

bool Connect4::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // Pieces don't move after placement in Connect 4
    return false;
}

bool Connect4::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    return false;
}

void Connect4::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int, int){
        square->destroyBit();
    });
}

int Connect4::lowestEmptyRow(int col) const
{
    for (int y = _grid->getHeight() - 1; y >= 0; --y) {
        ChessSquare* s = _grid->getSquare(col, y);
        if (s && !s->bit()) return y;
    }
    return -1;
}

bool Connect4::dropInColumn(int col, int playerNumber, bool animate)
{
    if (col < 0 || col >= _grid->getWidth()) return false;
    int row = lowestEmptyRow(col);
    if (row < 0) return false;

    ChessSquare* dst = _grid->getSquare(col, row);
    Bit* bit = pieceForPlayer(playerNumber);

    if (animate) {
        // Spawn above the board and animate down
        ImVec2 above = dst->getPosition();
        above.y = 0.0f; // top of window; simple drop
        bit->setCenterPosition(above);
        dst->setBit(bit);
        bit->moveTo(dst->getPosition());
    } else {
        bit->setPosition(dst->getPosition());
        dst->setBit(bit);
    }

    endTurn();
    return true;
}

Player* Connect4::ownerAt(int x, int y) const
{
    ChessSquare* s = _grid->getSquare(x, y);
    if (!s || !s->bit()) return nullptr;
    return s->bit()->getOwner();
}

bool Connect4::fourInARow(int sx, int sy, int dx, int dy, Player* p) const
{
    for (int k = 0; k < 4; ++k) {
        int x = sx + dx * k;
        int y = sy + dy * k;
        if (x < 0 || x >= _grid->getWidth() || y < 0 || y >= _grid->getHeight()) return false;
        if (ownerAt(x, y) != p) return false;
    }
    return true;
}

Player* Connect4::checkForWinner()
{
    // Check all possible start points and directions
    for (int y = 0; y < _grid->getHeight(); ++y) {
        for (int x = 0; x < _grid->getWidth(); ++x) {
            Player* p = ownerAt(x, y);
            if (!p) continue;
            if (fourInARow(x, y, 1, 0, p)) return p;      // horizontal
            if (fourInARow(x, y, 0, 1, p)) return p;      // vertical
            if (fourInARow(x, y, 1, 1, p)) return p;      // diag down-right
            if (fourInARow(x, y, 1, -1, p)) return p;     // diag up-right
        }
    }
    return nullptr;
}

bool Connect4::checkForDraw()
{
    bool full = true;
    _grid->forEachSquare([&](ChessSquare* s, int x, int y){
        if (!s->bit()) full = false;
    });
    return full && (checkForWinner() == nullptr);
}

std::string Connect4::initialStateString()
{
    // 7x6 zeros
    return std::string(42, '0');
}

std::string Connect4::stateString()
{
    std::string s(42, '0');
    for (int y = 0; y < _grid->getHeight(); ++y) {
        for (int x = 0; x < _grid->getWidth(); ++x) {
            ChessSquare* sq = _grid->getSquare(x, y);
            if (sq && sq->bit()) {
                int idx = y * _grid->getWidth() + x;
                s[idx] = char('1' + sq->bit()->getOwner()->playerNumber());
            }
        }
    }
    return s;
}

void Connect4::setStateString(const std::string &s)
{
    if ((int)s.size() != _grid->getWidth() * _grid->getHeight()) return;
    _grid->forEachSquare([](ChessSquare* square, int, int){ square->destroyBit(); });
    for (int y = 0; y < _grid->getHeight(); ++y) {
        for (int x = 0; x < _grid->getWidth(); ++x) {
            int idx = y * _grid->getWidth() + x;
            int pn = s[idx] - '0';
            if (pn == 1 || pn == 2) {
                ChessSquare* dst = _grid->getSquare(x, y);
                Bit* bit = pieceForPlayer(pn - 1);
                bit->setPosition(dst->getPosition());
                dst->setBit(bit);
            }
        }
    }
}

bool Connect4::wouldWin(int col, int playerIndex)
{
    int row = lowestEmptyRow(col);
    if (row < 0) return false;

    // Temporarily place
    ChessSquare* dst = _grid->getSquare(col, row);
    Bit* bit = pieceForPlayer(playerIndex);
    bit->setPosition(dst->getPosition());
    dst->setBit(bit);

    Player* p = getPlayerAt(playerIndex);
    bool win = (checkForWinner() == p);

    // Undo
    // destroyBit already deletes the held Bit
    dst->destroyBit();

    return win;
}

int Connect4::chooseAIMove()
{
    int aiIndex = getAIPlayer();
    int humanIndex = getHumanPlayer();
    // Try to win
    for (int c = 0; c < _grid->getWidth(); ++c) if (wouldWin(c, aiIndex)) return c;
    // Block opponent
    for (int c = 0; c < _grid->getWidth(); ++c) if (wouldWin(c, humanIndex)) return c;
    // Prefer center columns
    static int order[] = {3,2,4,1,5,0,6};
    for (int i = 0; i < 7; ++i) {
        int c = order[i];
        if (lowestEmptyRow(c) >= 0) return c;
    }
    return -1;
}

void Connect4::updateAI()
{
    int col = chooseAIMove();
    if (col >= 0) {
        dropInColumn(col, getAIPlayer(), true);
    }
}
