#include "Board.h"
#include "PieceFactory.h"
#include "Pawn.h"
#include "King.h"
#include "Rook.h"
#include <wx/dcbuffer.h>
#include <wx/msgdlg.h>
#include <algorithm>
#include <random>

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
        return false;
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
    if (y == 0) {
        if (x == 0) blackRookQMoved = true;
        else if (x == 7) blackRookKMoved = true;
    } else if (y == 7) {
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
                
                if (type == PieceType::KING) {
                    if (abs(square.x - x) <= 1 && abs(square.y - y) <= 1) {
                        return true;
                    }
                } 
                else if (type == PieceType::PAWN) {
                    int direction = (attackerColor == PieceColor::WHITE) ? -1 : 1;
                    if (y + direction == square.y && 
                        (x - 1 == square.x || x + 1 == square.x)) {
                        return true;
                    }
                } 
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
    wxPoint kingPos = GetKingPosition(color);
    PieceColor attacker = (color == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;
    return IsSquareUnderAttack(kingPos, attacker);
}

wxPoint Board::GetKingPosition(PieceColor color) const {
    return (color == PieceColor::WHITE) ? whiteKingPos : blackKingPos;
}

std::vector<wxPoint> Board::GetCheckingPieces(PieceColor color) const {
    std::vector<wxPoint> checkers;
    wxPoint kingPos = GetKingPosition(color);
    PieceColor attackerColor = (color == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            Piece* piece = board[x][y].get();
            if (piece && piece->GetColor() == attackerColor) {
                auto moves = piece->GetPossibleMoves(*this, wxPoint(x, y));
                if (std::find(moves.begin(), moves.end(), kingPos) != moves.end()) {
                    checkers.push_back(wxPoint(x, y));
                }
            }
        }
    }
    return checkers;
}

void Board::HighlightChecks(wxAutoBufferedPaintDC& dc) const {
    if (IsKingInCheck(currentTurn)) {
        wxPoint kingPos = GetKingPosition(currentTurn);
        wxRect kingRect(kingPos.x * tileSize.x, kingPos.y * tileSize.y, 
                         tileSize.x, tileSize.y);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(wxPen(*wxRED, 4));
        dc.DrawRectangle(kingRect);

        std::vector<wxPoint> attackers = GetCheckingPieces(currentTurn);
        for (const auto& attacker : attackers) {
            wxRect attackerRect(attacker.x * tileSize.x, attacker.y * tileSize.y, 
                                tileSize.x, tileSize.y);
            dc.SetPen(wxPen(*wxRED, 3));
            dc.DrawRectangle(attackerRect);
        }
    }
}

bool Board::IsCheckmate(PieceColor color) {
    if (!IsKingInCheck(color)) return false;
    return !HasLegalMoves(color);
}

bool Board::IsStalemate(PieceColor color) {
    if (IsKingInCheck(color)) return false;
    return !HasLegalMoves(color);
}

bool Board::HasLegalMoves(PieceColor color) {
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if (board[x][y] && board[x][y]->GetColor() == color) {
                auto moves = board[x][y]->GetPossibleMoves(*this, wxPoint(x, y));
                for (const auto& move : moves) {
                    if (IsMoveLegal(wxPoint(x, y), move)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool Board::IsMoveLegal(wxPoint from, wxPoint to) {
    if (!board[from.x][from.y]) return false;
    
    PieceType movedType = board[from.x][from.y]->GetType();
    PieceColor movedColor = board[from.x][from.y]->GetColor();
    bool isEnPassant = (movedType == PieceType::PAWN && to == enPassantTarget);
    bool isCastling = (movedType == PieceType::KING && abs(to.x - from.x) == 2);

    std::unique_ptr<Piece> backup[3];
    wxPoint originalWhiteKingPos = whiteKingPos;
    wxPoint originalBlackKingPos = blackKingPos;
    
    backup[0] = std::move(board[from.x][from.y]);
    backup[1] = std::move(board[to.x][to.y]);
    
    if (isEnPassant) {
        int captureY = (movedColor == PieceColor::WHITE) ? to.y + 1 : to.y - 1;
        backup[2] = std::move(board[to.x][captureY]);
        board[to.x][captureY].reset();
    } else if (isCastling) {
        if (to.x > from.x) {
            backup[2] = std::move(board[7][from.y]);
            board[5][from.y] = std::move(backup[2]);
        } else {
            backup[2] = std::move(board[0][from.y]);
            board[3][from.y] = std::move(backup[2]);
        }
    }

    board[to.x][to.y] = std::move(backup[0]);
    
    if (movedType == PieceType::KING) {
        if (movedColor == PieceColor::WHITE) {
            whiteKingPos = to;
        } else {
            blackKingPos = to;
        }
    }
    
    bool inCheck = IsKingInCheck(movedColor);
    
    board[from.x][from.y] = std::move(board[to.x][to.y]);
    board[to.x][to.y] = std::move(backup[1]);
    
    if (movedType == PieceType::KING) {
        if (movedColor == PieceColor::WHITE) {
            whiteKingPos = originalWhiteKingPos;
        } else {
            blackKingPos = originalBlackKingPos;
        }
    }
    
    if (isEnPassant) {
        int captureY = (movedColor == PieceColor::WHITE) ? to.y + 1 : to.y - 1;
        board[to.x][captureY] = std::move(backup[2]);
    } else if (isCastling) {
        if (to.x > from.x) {
            board[7][from.y] = std::move(board[5][from.y]);
            board[5][from.y].reset();
        } else {
            board[0][from.y] = std::move(board[3][from.y]);
            board[3][from.y].reset();
        }
    }
    
    return !inCheck;
}

void Board::SaveState() {
    MoveState state;
    
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            if (board[x][y]) {
                state.board[x][y] = PieceFactory::CreatePiece(
                    board[x][y]->GetType(), 
                    board[x][y]->GetColor()
                );
            }
        }
    }
    
    state.selectedPiece = selectedPiece;
    state.currentTurn = currentTurn;
    state.enPassantTarget = enPassantTarget;
    state.whiteKingPos = whiteKingPos;
    state.blackKingPos = blackKingPos;
    state.whiteKingMoved = whiteKingMoved;
    state.blackKingMoved = blackKingMoved;
    state.whiteRookKMoved = whiteRookKMoved;
    state.whiteRookQMoved = whiteRookQMoved;
    state.blackRookKMoved = blackRookKMoved;
    state.blackRookQMoved = blackRookQMoved;
    
    moveHistory.push(std::move(state));
}

void Board::RestoreState(const MoveState& state) {
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            if (state.board[x][y]) {
                board[x][y] = PieceFactory::CreatePiece(
                    state.board[x][y]->GetType(), 
                    state.board[x][y]->GetColor()
                );
            } else {
                board[x][y].reset();
            }
        }
    }
    
    selectedPiece = state.selectedPiece;
    currentTurn = state.currentTurn;
    enPassantTarget = state.enPassantTarget;
    whiteKingPos = state.whiteKingPos;
    blackKingPos = state.blackKingPos;
    whiteKingMoved = state.whiteKingMoved;
    blackKingMoved = state.blackKingMoved;
    whiteRookKMoved = state.whiteRookKMoved;
    whiteRookQMoved = state.whiteRookQMoved;
    blackRookKMoved = state.blackRookKMoved;
    blackRookQMoved = state.blackRookQMoved;
    
    possibleMoves.clear();
    gameOver = false;
    gameResult = "";
}

void Board::UndoLastMove() {
    if (moveHistory.size() <= 1) return; // Keep initial state
    
    moveHistory.pop(); // Remove current state
    RestoreState(moveHistory.top());
    Refresh();
}

void Board::ShowGameOverDialog(wxString message) {
    gameOver = true;
    gameResult = message;
    wxMessageDialog dialog(this, message, "Game Over", wxOK | wxCENTRE);
    dialog.ShowModal();
}

void Board::ResetGame() {
    InitNewGame();
    gameOver = false;
    gameResult = "";
    Refresh();
}

void Board::SetRandomColor() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 1);
    playerColor = distrib(gen) ? PieceColor::WHITE : PieceColor::BLACK;
    ResetGame();
}

void Board::InitNewGame() {
    whiteKingMoved = false;
    blackKingMoved = false;
    whiteRookKMoved = false;
    whiteRookQMoved = false;
    blackRookKMoved = false;
    blackRookQMoved = false;
    enPassantTarget = wxPoint(-1, -1);
    selectedPiece = wxPoint(-1, -1);
    possibleMoves.clear();
    currentTurn = PieceColor::WHITE;
    whiteKingPos = wxPoint(4, 7);
    blackKingPos = wxPoint(4, 0);
    gameOver = false;
    gameResult = "";
    playerColor = PieceColor::WHITE;
    
    while (!moveHistory.empty()) {
        moveHistory.pop();
    }

    for (int x = 0; x < 8; ++x)
        for (int y = 0; y < 8; ++y)
            board[x][y].reset();

    for (int x = 0; x < 8; ++x) {
        board[x][1] = PieceFactory::CreatePiece(PieceType::PAWN, PieceColor::BLACK);
        board[x][6] = PieceFactory::CreatePiece(PieceType::PAWN, PieceColor::WHITE);
    }

    PieceType backRow[8] = {
        PieceType::ROOK, PieceType::KNIGHT, PieceType::BISHOP, PieceType::QUEEN,
        PieceType::KING, PieceType::BISHOP, PieceType::KNIGHT, PieceType::ROOK
    };

    for (int x = 0; x < 8; ++x) {
        board[x][0] = PieceFactory::CreatePiece(backRow[x], PieceColor::BLACK);
        board[x][7] = PieceFactory::CreatePiece(backRow[x], PieceColor::WHITE);
    }
    
    SaveState();
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

    for (const auto& move : possibleMoves) {
        wxRect highlight(move.x * tileSize.x, move.y * tileSize.y,
                         tileSize.x, tileSize.y);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(wxPen(*wxGREEN, 3));
        dc.DrawRectangle(highlight);
    }

    HighlightChecks(dc);

    if (gameOver) {
        dc.SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        dc.SetTextForeground(*wxBLUE);
        wxSize textSize = dc.GetTextExtent(gameResult);
        int textX = (size.GetWidth() - textSize.GetWidth()) / 2;
        int textY = (size.GetHeight() - textSize.GetHeight()) / 2;
        dc.DrawText(gameResult, textX, textY);
    }
}

void Board::OnLeftDown(wxMouseEvent& event) {
    if (gameOver) return;

    wxPoint clickPos = event.GetPosition();
    int x = clickPos.x / tileSize.x;
    int y = clickPos.y / tileSize.y;

    if (x < 0 || x >= 8 || y < 0 || y >= 8)
        return;

    if (selectedPiece.x == -1) {
        if (board[x][y] && board[x][y]->GetColor() == currentTurn) {
            selectedPiece = wxPoint(x, y);
            possibleMoves = board[x][y]->GetPossibleMoves(*this, selectedPiece);
            
            std::vector<wxPoint> legalMoves;
            for (const auto& move : possibleMoves) {
                if (IsMoveLegal(selectedPiece, move)) {
                    legalMoves.push_back(move);
                }
            }
            possibleMoves = legalMoves;
        }
    } else {
        wxPoint dest(x, y);
        auto it = std::find(possibleMoves.begin(), possibleMoves.end(), dest);
        if (it != possibleMoves.end()) {
            SaveState();
            
            PieceType movedType = board[selectedPiece.x][selectedPiece.y]->GetType();
            PieceColor movedColor = board[selectedPiece.x][selectedPiece.y]->GetColor();
            
            if (movedType == PieceType::PAWN && dest == enPassantTarget) {
                int captureY = (movedColor == PieceColor::WHITE) ? dest.y + 1 : dest.y - 1;
                board[dest.x][captureY].reset();
            }
            
            if (movedType == PieceType::KING) {
                int deltaX = dest.x - selectedPiece.x;
                
                if (deltaX == 2) {
                    board[5][dest.y] = std::move(board[7][dest.y]);
                    board[7][dest.y].reset();
                    SetRookMoved(7, dest.y);
                }
                else if (deltaX == -2) {
                    board[3][dest.y] = std::move(board[0][dest.y]);
                    board[0][dest.y].reset();
                    SetRookMoved(0, dest.y);
                }
                SetKingMoved(movedColor);
                
                if (movedColor == PieceColor::WHITE) {
                    whiteKingPos = dest;
                } else {
                    blackKingPos = dest;
                }
            }
            
            if (movedType == PieceType::ROOK) {
                SetRookMoved(selectedPiece.x, selectedPiece.y);
            }
            
            if (movedType == PieceType::PAWN && abs(dest.y - selectedPiece.y) == 2) {
                int epY = (selectedPiece.y + dest.y) / 2;
                enPassantTarget = wxPoint(dest.x, epY);
            } else {
                enPassantTarget = wxPoint(-1, -1);
            }
            
            board[dest.x][dest.y] = std::move(board[selectedPiece.x][selectedPiece.y]);
            currentTurn = (currentTurn == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;
            
            PieceColor opponent = (movedColor == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;
            if (IsCheckmate(opponent)) {
                ShowGameOverDialog("Checkmate! " + wxString(movedColor == PieceColor::WHITE ? "White" : "Black") + " wins!");
            } else if (IsStalemate(opponent)) {
                ShowGameOverDialog("Stalemate! Game drawn!");
            }
        } 
        selectedPiece = wxPoint(-1, -1);
        possibleMoves.clear();
    }   
    Refresh();
}
