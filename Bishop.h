#ifndef BISHOP_H
#define BISHOP_H

#include "Piece.h"

class Bishop : public Piece {
public:
    explicit Bishop(PieceColor color);
    wxString GetSymbol() const override;
    std::string GetName() const override;
    std::vector<wxPoint> GetPossibleMoves(const Board& board, wxPoint pos) const override;
};

#endif // BISHOP_H

