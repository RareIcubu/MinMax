#ifndef KING_H
#define KING_H

#include "Piece.h"

class King : public Piece {
public:
    King(PieceColor color);
    bool CanCastle() const { return !hasMoved; }
    wxString GetSymbol() const override;
    std::string GetName() const override;
    std::vector<wxPoint> GetPossibleMoves(const Board& board, wxPoint position) const override;
private:
    bool hasMoved = false;
};

#endif
