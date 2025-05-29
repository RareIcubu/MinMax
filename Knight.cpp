#include "Knight.h"
#include "Board.h"

Knight::Knight(PieceColor color) : Piece(PieceType::KNIGHT, color) {}

wxString Knight::GetSymbol() const {
    return (GetColor() == PieceColor::WHITE) ? L"♘" : L"♞";
}

std::string Knight::GetName() const {
    return "Knight";
}

std::vector<wxPoint> Knight::GetPossibleMoves(const Board& board, wxPoint position) const {
    std::vector<wxPoint> moves;
    const int moveset[8][2] = {
        {2, 1}, {1, 2}, {-1, 2}, {-2, 1},
        {-2, -1}, {-1, -2}, {1, -2}, {2, -1}
    };
    
    for (const auto& move : moveset) {
        int nx = position.x + move[0];
        int ny = position.y + move[1];
        
        if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
            if (board.IsEmpty(nx, ny) || board.IsEnemy(nx, ny, GetColor())) {
                moves.push_back(wxPoint(nx, ny));
            }
        }
    }
    
    return moves;
}
