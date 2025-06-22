#include "Bishop.h"
#include "Board.h"

Bishop::Bishop(PieceColor color)
    : Piece(PieceType::BISHOP, color) {}

wxString Bishop::GetSymbol() const {
    return (GetColor() == PieceColor::WHITE) ? L"♗" : L"♝";
}

std::string Bishop::GetName() const {
    return "Bishop";
}

std::vector<wxPoint> Bishop::GetPossibleMoves(const Board& board, wxPoint pos) const {
    std::vector<wxPoint> moves;
    const std::vector<wxPoint> directions = { {1, 1}, {-1, 1}, {1, -1}, {-1, -1} };

    for (const auto& dir : directions) {
        wxPoint p = pos;
        while (true) {
            p.x += dir.x;
            p.y += dir.y;
            if (!board.IsInsideBoard(p)) break;

            if (board.IsEmpty(p.x, p.y)) {
                moves.push_back(p);
            } else {
                if (board.IsEnemy(p.x, p.y, GetColor())) {
                    moves.push_back(p);
                }
                break;
            }
        }
    }
    return moves;
}
