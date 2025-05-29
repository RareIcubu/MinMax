#ifndef KNIGHT_H
#define KNIGHT_H

#include "Piece.h"

class Board; // Forward declaration

class Knight : public Piece {
public:
    Knight(PieceColor color);
    wxString GetSymbol() const override;
    std::string GetName() const override;
    std::vector<wxPoint> GetPossibleMoves(const Board& board, wxPoint position) const override;
};

#endif
