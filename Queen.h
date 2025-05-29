#ifndef QUEEN_H
#define QUEEN_H

#include "Piece.h"

class Queen : public Piece {
public:
    Queen(PieceColor color);
    wxString GetSymbol() const override;
    std::string GetName() const override;
    std::vector<wxPoint> GetPossibleMoves(const Board& board, wxPoint pos) const override;
};

#endif
