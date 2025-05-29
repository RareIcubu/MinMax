#include "Queen.h"
#include "Board.h"
Queen::Queen(PieceColor color) 
    : Piece(PieceType::QUEEN, color) {}

wxString Queen::GetSymbol() const {
    return (GetColor() == PieceColor::WHITE) ? L"♕" : L"♛";
}

std::string Queen::GetName() const {
    return "Queen";
}

std::vector<wxPoint> Queen::GetPossibleMoves(const Board& board, wxPoint pos) const {
    std::vector<wxPoint> moves;
    const std::vector<wxPoint> directions = {
        {1,0}, {-1,0}, {0,1}, {0,-1},
        {1,1}, {-1,1}, {1,-1}, {-1,-1}
    };

    for (auto dir : directions) {
        wxPoint p = pos;
        while (true) {
            p.x += dir.x;
            p.y += dir.y;
            if (!board.IsInsideBoard(p)) break;

            auto* piece = board.GetPieceAt(p);
            if (!piece) {
                moves.push_back(p);
            } else {
                if (piece->GetColor() != color)
                    moves.push_back(p);
                break;
            }
        }
    }

    return moves;
}
