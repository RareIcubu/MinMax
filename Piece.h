#include <wx/wx.h>
#include <vector>
#ifndef PIECE_H
#define PIECE_H

enum class PieceType {NONE, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING};
enum class PieceColor {NONE, BLACK, WHITE};

struct Piece{
    PieceType type = PieceType::NONE;
    PieceColor color = PieceColor::NONE;
};

#endif
