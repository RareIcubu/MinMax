#include "Board.h"
#include "PieceFactory.h"
#include "Pawn.h"
#include "King.h"
#include "Rook.h"
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

void Board::SetKingMoved(PieceColor color) {
    if (color == PieceColor::WHITE) whiteKingMoved = true;
    else blackKingMoved = true;
}

void Board::SetRookMoved(int x, int y) {
    if (y == 0) {  // Black pieces
        if (x == 0) blackRookQMoved = true;
        else if (x == 7) blackRookKMoved = true;
    } else if (y == 7) {  // White pieces
        if (x == 0) whiteRookQMoved = true;
        else if (x == 7) whiteRookKMoved = true;
    }
}

bool Board::CanCastleKingside(PieceColor color) const {
    if (color == PieceColor::WHITE) 
        return !whiteKingMoved && !whiteRookKMoved;
    else 
        return !blackKingMoved && !blackRookKMoved;
}

bool Board::CanCastleQueenside(PieceColor color) const {
    if (color == PieceColor::WHITE) 
        return !whiteKingMoved && !whiteRookQMoved;
    else 
        return !blackKingMoved && !blackRookQMoved;
}

bool Board::IsSquareUnderAttack(wxPoint square, PieceColor attackerColor) const {
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            Piece* piece = board[x][y].get();
            if (piece && piece->GetColor() == attackerColor) {
                PieceType type = piece->GetType();
                
                // Handle king attacks separately (adjacent squares)
                if (type == PieceType::KING) {
                    if (abs(square.x - x) <= 1 && abs(square.y - y) <= 1) {
                        return true;
                    }
                } 
                // Handle pawn attacks separately (different capture pattern)
                else if (type == PieceType::PAWN) {
                    int direction = (attackerColor == PieceColor::WHITE) ? -1 : 1;
                    if (y + direction == square.y && 
                        (x - 1 == square.x || x + 1 == square.x)) {
                        return true;
                    }
                } 
                // For other pieces, check if they can move to the square
                else {
                    auto moves = piece->GetPossibleMoves(*this, wxPoint(x, y));
                    if (std::find(moves.begin(), moves.end(), square) != moves.end()) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool Board::IsKingInCheck(PieceColor color) const {
    // Find king position
    wxPoint kingPos(-1, -1);
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if (board[x][y] && 
                board[x][y]->GetType() == PieceType::KING && 
                board[x][y]->GetColor() == color) {
                kingPos = wxPoint(x, y);
                break;
            }
        }
    }
    if (kingPos.x == -1) return false;

    PieceColor attacker = (color == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;
    return IsSquareUnderAttack(kingPos, attacker);
}

void Board::InitNewGame() {
    // Reset castling flags
    whiteKingMoved = false;
    blackKingMoved = false;
    whiteRookKMoved = false;
    whiteRookQMoved = false;
    blackRookKMoved = false;
    blackRookQMoved = false;
    enPassantTarget = wxPoint(-1, -1);

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
                dc.SetTextForeground(board[x][y]->GetColor() == PieceColor::WHITE ? *wxBLACK : *wxRED);

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
    } else {
        // Second click - try to move
        wxPoint dest(x, y);
        auto it = std::find(possibleMoves.begin(), possibleMoves.end(), dest);
        if (it != possibleMoves.end()) {
            PieceType movedType = board[selectedPiece.x][selectedPiece.y]->GetType();
            PieceColor movedColor = board[selectedPiece.x][selectedPiece.y]->GetColor();
            
            // Handle en passant capture
            if (movedType == PieceType::PAWN && dest == enPassantTarget) {
                int captureY = (movedColor == PieceColor::WHITE) ? dest.y + 1 : dest.y - 1;
                board[dest.x][captureY].reset();
            }
            
            // Handle castling
            if (movedType == PieceType::KING) {
                int deltaX = dest.x - selectedPiece.x;
                
                // Kingside castling (king moves 2 right)
                if (deltaX == 2) {
                    // Move the rook from H to F
                    board[5][dest.y] = std::move(board[7][dest.y]);
                    board[7][dest.y].reset();
                    SetRookMoved(7, dest.y);
                }
                // Queenside castling (king moves 2 left)
                else if (deltaX == -2) {
                    // Move the rook from A to D
                    board[3][dest.y] = std::move(board[0][dest.y]);
                    board[0][dest.y].reset();
                    SetRookMoved(0, dest.y);
                }
                SetKingMoved(movedColor);
            }
            
            // Update rook moved status
            if (movedType == PieceType::ROOK) {
                SetRookMoved(selectedPiece.x, selectedPiece.y);
            }
            
            // Handle pawn double move for en passant
            if (movedType == PieceType::PAWN && abs(dest.y - selectedPiece.y) == 2) {
                int epY = (selectedPiece.y + dest.y) / 2;
                enPassantTarget = wxPoint(dest.x, epY);
            } else {
                enPassantTarget = wxPoint(-1, -1);
            }
            
            // Move the piece
            board[dest.x][dest.y] = std::move(board[selectedPiece.x][selectedPiece.y]);
            currentTurn = (currentTurn == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;
        } 
        selectedPiece = wxPoint(-1, -1);
        possibleMoves.clear();
    }   
    Refresh();
}
