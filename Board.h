#ifndef BOARD_H
#define BOARD_H

#include <wx/wx.h>
#include <vector>
#include "Piece.h"

class Board : public wxPanel {
public:
    Board(wxWindow* parent);
    void InitNewGame();

private:
    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    wxString GetPieceSymbol(const Piece& piece) const;  // Add declaration

    wxSize tileSize = wxSize(60, 60);
    Piece board[8][8];
    wxPoint selectedPiece = wxPoint(-1, -1);

    DECLARE_EVENT_TABLE();
};
#endif
