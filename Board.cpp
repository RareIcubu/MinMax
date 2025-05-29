#include "Board.h"
#include "PieceFactory.h"
#include "Pawn.h"
#include <wx/dcbuffer.h>
#include <algorithm>

wxBEGIN_EVENT_TABLE(Board, wxPanel)
    EVT_PAINT(Board::OnPaint)
    EVT_LEFT_DOWN(Board::OnLeftDown)
wxEND_EVENT_TABLE()

Board::Board(wxWindow* parent) : wxPanel(parent) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    InitNewGame();
}

bool Board::IsEmpty(int x, int y) const {
    if (x < 0 || x >= 8 || y < 0 || y >= 8) return false;
    return !board[x][y];
}

bool Board::IsEnemy(int x, int y, PieceColor color) const {
    if (x < 0 || x >= 8 || y < 0 || y >= 8) return false;
    return board[x][y] && board[x][y]->GetColor() != color;
}

void Board::SetEnPassantTarget(wxPoint target) {
    enPassantTarget = target;
}

bool Board::IsEnPassantTarget(int x, int y) const {
    return enPassantTarget == wxPoint(x, y);
}

bool Board::IsValidMove(int fromX, int fromY, int toX, int toY) const {
    if (!board[fromX][fromY]) return false;
    if (board[toX][toY] && board[toX][toY]->GetColor() == board[fromX][fromY]->GetColor()) {
        return false; // Can't capture own piece
    }
    return true;
}

Piece* Board::GetPieceAt(wxPoint p) const {
    if (p.x < 0 || p.x >= 8 || p.y < 0 || p.y >= 8) return nullptr;
    return board[p.x][p.y].get();
}

bool Board::IsRook(int x, int y, PieceColor color) const {
    if (x < 0 || x >= 8 || y < 0 || y >= 8) return false;
    auto piece = board[x][y].get();
    return piece && piece->GetType() == PieceType::ROOK && piece->GetColor() == color;
}

void Board::InitNewGame() {
    for (int x = 0; x < 8; ++x)
        for (int y = 0; y < 8; ++y)
            board[x][y].reset();

    // Setup pawns
    for (int x = 0; x < 8; ++x) {
        board[x][1] = PieceFactory::CreatePiece(PieceType::PAWN, PieceColor::BLACK);
        board[x][6] = PieceFactory::CreatePiece(PieceType::PAWN, PieceColor::WHITE);
    }

    // Setup back rank pieces
    PieceType backRow[8] = {
        PieceType::ROOK, PieceType::KNIGHT, PieceType::BISHOP, PieceType::QUEEN,
        PieceType::KING, PieceType::BISHOP, PieceType::KNIGHT, PieceType::ROOK
    };

    for (int x = 0; x < 8; ++x) {
        board[x][0] = PieceFactory::CreatePiece(backRow[x], PieceColor::BLACK);
        board[x][7] = PieceFactory::CreatePiece(backRow[x], PieceColor::WHITE);
    }
}

void Board::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    wxSize size = GetClientSize();

    tileSize = wxSize(size.GetWidth() / 8, size.GetHeight() / 8);

    wxFont pieceFont(tileSize.GetHeight() * 0.8,
                    wxFONTFAMILY_DEFAULT,
                    wxFONTSTYLE_NORMAL,
                    wxFONTWEIGHT_BOLD);
    dc.SetFont(pieceFont);

    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            wxRect tile(x * tileSize.x, y * tileSize.y, tileSize.x, tileSize.y);
            dc.SetBrush(((x + y) % 2) ? *wxLIGHT_GREY : *wxWHITE);
            dc.DrawRectangle(tile);

            if (board[x][y]) {
                dc.SetTextForeground(*wxBLACK);

                wxString symbol = board[x][y]->GetSymbol();
                wxSize textSize = dc.GetTextExtent(symbol);
                int textX = tile.x + (tileSize.x - textSize.GetWidth()) / 2;
                int textY = tile.y + (tileSize.y - textSize.GetHeight()) / 2;

                dc.DrawText(symbol, textX, textY);
            }
        }
    }

    // Draw possible moves highlights
    for (const auto& move : possibleMoves) {
        wxRect highlight(move.x * tileSize.x, move.y * tileSize.y,
                         tileSize.x, tileSize.y);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(wxPen(*wxGREEN, 3));
        dc.DrawRectangle(highlight);
    }
}

void Board::OnLeftDown(wxMouseEvent& event) {
    wxPoint clickPos = event.GetPosition();
    int x = clickPos.x / tileSize.x;
    int y = clickPos.y / tileSize.y;

    if (x < 0 || x >= 8 || y < 0 || y >= 8)
        return;

    if (selectedPiece.x == -1) {
        // First click - select a piece
        if (board[x][y] && board[x][y]->GetColor() == currentTurn) {
            selectedPiece = wxPoint(x, y);
            possibleMoves = board[x][y]->GetPossibleMoves(*this, selectedPiece);
        }
    } 
    else {
    // Second click - try to move
    wxPoint dest(x, y);
    auto it = std::find(possibleMoves.begin(), possibleMoves.end(), dest);
    if (it != possibleMoves.end()) {
        board[dest.x][dest.y] = std::move(board[selectedPiece.x][selectedPiece.y]);
        board[selectedPiece.x][selectedPiece.y].reset();
        currentTurn = (currentTurn == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;
    } 
        selectedPiece = wxPoint(-1, -1);
        possibleMoves.clear();
    }   

    Refresh();

} 
