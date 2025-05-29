#ifndef BOARD_H
#define BOARD_H

#include <wx/wx.h>
#include <vector>
#include <memory>
#include "Piece.h"

class Board : public wxPanel {
public:
    explicit Board(wxWindow* parent);
    void InitNewGame();

    bool IsEmpty(int x, int y) const;
    bool IsEnemy(int x, int y, PieceColor color) const;
    bool IsValidMove(int fromX, int fromY, int toX, int toY) const;
    void SetEnPassantTarget(wxPoint target);
    bool IsEnPassantTarget(int x, int y) const;
    bool IsInsideBoard(wxPoint p) const { return p.x >= 0 && p.x < 8 && p.y >= 0 && p.y < 8; }
    Piece* GetPieceAt(wxPoint p) const;
    bool IsRook(int x, int y, PieceColor color) const; // Needed for castling check

private:
    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);

    wxSize tileSize = wxSize(60, 60);
    std::unique_ptr<Piece> board[8][8];
    wxPoint selectedPiece = wxPoint(-1, -1);
    PieceColor currentTurn = PieceColor::WHITE;
    std::vector<wxPoint> possibleMoves;
    wxPoint enPassantTarget = wxPoint(-1, -1);

    wxDECLARE_EVENT_TABLE();
};

#endif // BOARD_H

