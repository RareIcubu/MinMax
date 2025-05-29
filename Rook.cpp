#include "Rook.h"
#include "Board.h"
Rook::Rook(PieceColor color) 
    : Piece(PieceType::ROOK, color) {}

wxString Rook::GetSymbol() const {
    return (GetColor() == PieceColor::WHITE) ? L"♖" : L"♜";
}

std::string Rook::GetName() const {
    return "Rook";
}

std::vector<wxPoint> Rook::GetPossibleMoves(const Board& board, wxPoint pos) const {
    std::vector<wxPoint> moves;
    const wxPoint dirs[] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    for (auto d : dirs) {
        wxPoint p = pos;
        while (true) {
            p.x += d.x; p.y += d.y;
            if (!board.IsInsideBoard(p)) break;
            auto *piece = board.GetPieceAt(p);
            if (!piece) {
                moves.push_back(p);
            } else {
                if (piece->GetColor() != GetColor())
                    moves.push_back(p);
                break;
            }
        }
    }
    return moves;
}
