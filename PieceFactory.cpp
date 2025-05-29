#include "PieceFactory.h"
#include "Pawn.h"
#include "Rook.h"
#include "Knight.h"
#include "Bishop.h"
#include "Queen.h"
#include "King.h"

std::unique_ptr<Piece> PieceFactory::CreatePiece(PieceType type, PieceColor color) {
    switch (type) {
        case PieceType::PAWN:   return std::make_unique<Pawn>(color);
        case PieceType::ROOK:   return std::make_unique<Rook>(color);
        case PieceType::KNIGHT: return std::make_unique<Knight>(color);
        case PieceType::BISHOP: return std::make_unique<Bishop>(color);
        case PieceType::QUEEN:  return std::make_unique<Queen>(color);
        case PieceType::KING:   return std::make_unique<King>(color);
        default:                return nullptr;
    }
}
