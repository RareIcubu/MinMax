#ifndef PIECE_FACTORY_H
#define PIECE_FACTORY_H

#include "Piece.h"
#include <memory>

class PieceFactory {
public:
    static std::unique_ptr<Piece> CreatePiece(PieceType type, PieceColor color);
};

#endif
