#include "Board.h"
#include <wx/dcbuffer.h>
#include <wx/gdicmn.h> // Add this include
#include <map>

wxBEGIN_EVENT_TABLE(Board, wxPanel)
    EVT_PAINT(Board::OnPaint)
    EVT_LEFT_DOWN(Board::OnLeftDown)
wxEND_EVENT_TABLE()

Board::Board(wxWindow* parent) : wxPanel(parent) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    InitNewGame();
}

void Board::InitNewGame() {
    // Initialize with proper syntax
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            board[x][y] = {PieceType::NONE, PieceColor::NONE};
        }
    }

    for (int x = 0; x < 8; x++) {
        board[x][1] = {PieceType::PAWN, PieceColor::BLACK};
        board[x][6] = {PieceType::PAWN, PieceColor::WHITE};
    }

    PieceType backRow[8] = {
        PieceType::ROOK, PieceType::KNIGHT, PieceType::BISHOP, PieceType::QUEEN,
        PieceType::KING, PieceType::BISHOP, PieceType::KNIGHT, PieceType::ROOK
    };
    
    for (int x = 0; x < 8; x++) {
        board[x][0] = {backRow[x], PieceColor::BLACK};
        board[x][7] = {backRow[x], PieceColor::WHITE};
    }
}

void Board::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    wxSize size = GetClientSize();
    wxBrush lightBrush(wxColour(200, 200, 200));  // Light grey
    wxBrush darkBrush(wxColour(100, 100, 100));    // Dark grey

    tileSize = wxSize(size.GetWidth()/8, size.GetHeight()/8);

     wxFont pieceFont(tileSize.GetHeight() * 0.8,  // 80% of tile height
                    wxFONTFAMILY_DEFAULT,
                    wxFONTSTYLE_NORMAL,
                    wxFONTWEIGHT_BOLD);
    dc.SetFont(pieceFont);

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            wxRect tile(x * tileSize.x, y * tileSize.y, tileSize.x, tileSize.y);
                       // Then in the loop:
            dc.SetBrush(((x + y) % 2) ? lightBrush : darkBrush);      dc.DrawRectangle(tile);

           if (board[x][y].type != PieceType::NONE) {
                dc.SetTextForeground(board[x][y].color == PieceColor::WHITE ? *wxBLACK : *wxWHITE);
                
                wxString symbol = GetPieceSymbol(board[x][y]);
                
                // Calculate centered position
                wxSize textSize = dc.GetTextExtent(symbol);
                int textX = tile.x + (tileSize.x - textSize.GetWidth()) / 2;
                int textY = tile.y + (tileSize.y - textSize.GetHeight()) / 2;
                
                dc.DrawText(symbol, textX, textY);
            
           }
        }
    }
}

wxString Board::GetPieceSymbol(const Piece& piece) const {
    static std::map<PieceType, wxString> symbols = {
        {PieceType::KING,   L"♔"}, {PieceType::QUEEN,  L"♕"},
        {PieceType::ROOK,   L"♖"}, {PieceType::BISHOP, L"♗"},
        {PieceType::KNIGHT, L"♘"}, {PieceType::PAWN,   L"♙"}
    };
    return symbols[piece.type];
}

void Board::OnLeftDown(wxMouseEvent& event) {
    wxPoint clickPos = event.GetPosition();
    int x = clickPos.x / tileSize.x;
    int y = clickPos.y / tileSize.y;

    if (x < 0 || x >= 8 || y < 0 || y >= 8) return;

    if (selectedPiece.x == -1) {
        if (board[x][y].type != PieceType::NONE) {
            selectedPiece = wxPoint(x, y);
        }
    } else {
        if (x != selectedPiece.x || y != selectedPiece.y) {
            // Manual swap instead of std::swap
            Piece temp = board[selectedPiece.x][selectedPiece.y];
            board[selectedPiece.x][selectedPiece.y] = board[x][y];
            board[x][y] = temp;
        }
        selectedPiece = wxPoint(-1, -1);
    }
    Refresh();
}
