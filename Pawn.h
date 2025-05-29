#ifndef PAWN_H
#define PAWN_H

#include "Piece.h"

// Inherit from Piece struct
struct Pawn : public Piece {
    Pawn(PieceColor color) {
        type = PieceType::PAWN;
        this->color = color;
        isMoved = false;
    }
    
    void setIsMoved(bool moved) { isMoved = moved; }
    bool getIsMoved() const { return isMoved; }
    
private:
    bool isMoved;
};

#endif
