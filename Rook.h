#ifndef ROOK_H
#define ROOK_H

#include "Piece.h"

class Rook : public Piece {
public:
    Rook(PieceColor color);
    wxString GetSymbol() const override;
    std::string GetName() const override;
    std::vector<wxPoint> GetPossibleMoves(const Board& board, wxPoint position) const override;
};

#endif
