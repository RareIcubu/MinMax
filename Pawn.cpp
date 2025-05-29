#include "Pawn.h"
#include "Board.h"

Pawn::Pawn(PieceColor color) : Piece(PieceType::PAWN, color) {}

wxString Pawn::GetSymbol() const {
    return (GetColor() == PieceColor::WHITE) ? L"♙" : L"♟";
}

std::string Pawn::GetName() const {
    return "Pawn";
}

std::vector<wxPoint> Pawn::GetPossibleMoves(const Board& board, wxPoint position) const {
    std::vector<wxPoint> moves;
    int direction = (GetColor() == PieceColor::WHITE) ? -1 : 1;
    int startRow = (GetColor() == PieceColor::WHITE) ? 6 : 1;

    // Single move forward
    if (board.IsEmpty(position.x, position.y + direction)) {
        moves.push_back(wxPoint(position.x, position.y + direction));
        
        // Double move from start position
        if (position.y == startRow && board.IsEmpty(position.x, position.y + 2 * direction)) {
            moves.push_back(wxPoint(position.x, position.y + 2 * direction));
        }
    }

    // Captures
    for (int dx : {-1, 1}) {
        int nx = position.x + dx;
        int ny = position.y + direction;
        if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
            if (board.IsEnemy(nx, ny, GetColor()) || board.IsEnPassantTarget(nx, ny)) {
                moves.push_back(wxPoint(nx, ny));
            }
        }
    }

    return moves;
}
