#include "King.h"
#include "Board.h"

King::King(PieceColor color) 
    : Piece(PieceType::KING, color) {}

wxString King::GetSymbol() const {
    return (GetColor() == PieceColor::WHITE) ? L"♔" : L"♚";
}

std::string King::GetName() const {
    return "King";
}

std::vector<wxPoint> King::GetPossibleMoves(const Board& board, wxPoint position) const {
    std::vector<wxPoint> moves;
    
    // Regular moves
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            int nx = position.x + dx;
            int ny = position.y + dy;
            if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                if (board.IsEmpty(nx, ny) || board.IsEnemy(nx, ny, GetColor())) {
                    moves.push_back(wxPoint(nx, ny));
                }
            }
        }
    }
    
    // Castling
    if (position.x == 4 && !board.IsKingInCheck(GetColor())) {
        // Kingside
        if (board.CanCastleKingside(GetColor()) &&
            board.IsEmpty(5, position.y) && 
            board.IsEmpty(6, position.y)) {
            moves.push_back(wxPoint(6, position.y));
        }
        // Queenside
        if (board.CanCastleQueenside(GetColor()) &&
            board.IsEmpty(3, position.y) && 
            board.IsEmpty(2, position.y) && 
            board.IsEmpty(1, position.y)) {
            moves.push_back(wxPoint(2, position.y));
        }
    }
    
    return moves;
}
