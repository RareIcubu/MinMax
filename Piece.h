#ifndef PIECE_H
#define PIECE_H

#include <wx/wx.h>
#include <vector>

class Board; // Forward declaration

enum class PieceType { NONE, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING };
enum class PieceColor { NONE, BLACK, WHITE };

class Piece {
public:
    Piece(PieceType type, PieceColor color) : type(type), color(color) {}
    virtual ~Piece() = default;
    
    virtual wxString GetSymbol() const = 0;
    virtual std::string GetName() const = 0;
    virtual std::vector<wxPoint> GetPossibleMoves(const Board& board, wxPoint position) const = 0;
    
    PieceType GetType() const { return type; }
    PieceColor GetColor() const { return color; }

protected:
    PieceType type;
    PieceColor color;
};

#endif
