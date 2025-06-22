#include "Board.h"
#include "PieceFactory.h"
#include "Pawn.h"
#include "King.h"
#include "Rook.h"
#include "Queen.h"
#include "Knight.h"
#include "Bishop.h"
#include <wx/dcbuffer.h>
#include <wx/msgdlg.h>
#include <algorithm>
#include <random>
#include <map>
#include <climits>
#include <cmath>
#include <unordered_set>
#include <chrono>

wxBEGIN_EVENT_TABLE(Board, wxPanel)
    EVT_PAINT(Board::OnPaint)
    EVT_LEFT_DOWN(Board::OnLeftDown)
wxEND_EVENT_TABLE()

Board::Board(wxWindow* parent) : wxPanel(parent) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    InitNewGame();
    
    wxTheApp->CallAfter([this]() {
        if (IsComputerTurn()) {
            ComputerMove();
        }
    });
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
    
    // Early exit for invalid moves
    if (board[to.x][to.y] && 
        board[to.x][to.y]->GetColor() == board[from.x][from.y]->GetColor()) {
        return false;
    }
    
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
    state.promotionSquare = promotionSquare;
    
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
    promotionSquare = state.promotionSquare;
    
    possibleMoves.clear();
    gameOver = false;
    gameResult = "";
}

void Board::GetCurrentState(MoveState& state) const {
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            if (board[x][y]) {
                state.board[x][y] = PieceFactory::CreatePiece(
                    board[x][y]->GetType(), 
                    board[x][y]->GetColor()
                );
            } else {
                state.board[x][y].reset();
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
    state.promotionSquare = promotionSquare;
}

void Board::DoMove(wxPoint from, wxPoint to) {
    PieceType movedType = board[from.x][from.y]->GetType();
    PieceColor movedColor = board[from.x][from.y]->GetColor();
    
    if (movedType == PieceType::PAWN && to == enPassantTarget) {
        int captureY = (movedColor == PieceColor::WHITE) ? to.y + 1 : to.y - 1;
        board[to.x][captureY].reset();
    }
    
    if (movedType == PieceType::KING) {
        int deltaX = to.x - from.x;
        
        if (deltaX == 2) {
            board[5][to.y] = std::move(board[7][to.y]);
            board[7][to.y].reset();
            SetRookMoved(7, to.y);
        }
        else if (deltaX == -2) {
            board[3][to.y] = std::move(board[0][to.y]);
            board[0][to.y].reset();
            SetRookMoved(0, to.y);
        }
        SetKingMoved(movedColor);
        
        if (movedColor == PieceColor::WHITE) {
            whiteKingPos = to;
        } else {
            blackKingPos = to;
        }
    }
    
    if (movedType == PieceType::ROOK) {
        SetRookMoved(from.x, from.y);
    }
    
    if (movedType == PieceType::PAWN && abs(to.y - from.y) == 2) {
        int epY = (from.y + to.y) / 2;
        enPassantTarget = wxPoint(to.x, epY);
    } else {
        enPassantTarget = wxPoint(-1, -1);
    }
    
    board[to.x][to.y] = std::move(board[from.x][from.y]);
    
    // Sprawdź promocję pionka
    if (movedType == PieceType::PAWN && (to.y == 0 || to.y == 7)) {
        promotionSquare = to;
        HandlePawnPromotion(to);
    }
    
    currentTurn = (currentTurn == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;
}

void Board::PromotePawn(wxPoint pos, PieceType promotionType) {
    if (board[pos.x][pos.y] && board[pos.x][pos.y]->GetType() == PieceType::PAWN) {
        PieceColor color = board[pos.x][pos.y]->GetColor();
        board[pos.x][pos.y] = PieceFactory::CreatePiece(promotionType, color);
    }
    promotionSquare = wxPoint(-1, -1);
}

void Board::HandlePawnPromotion(wxPoint pos) {
    // Dla AI zawsze promuj do hetmana
    if (IsComputerTurn()) {
        PromotePawn(pos, PieceType::QUEEN);
    }
    // Dla gracza pokaż okno dialogowe (do zaimplementowania)
    else {
        // W tej wersji zawsze promuj do hetmana
        PromotePawn(pos, PieceType::QUEEN);
    }
}

int Board::EvaluateMaterial() const {
    int score = 0;
    std::map<PieceType, int> pieceValues = {
        {PieceType::PAWN, 100},
        {PieceType::KNIGHT, 320},
        {PieceType::BISHOP, 330},
        {PieceType::ROOK, 500},
        {PieceType::QUEEN, 900},
        {PieceType::KING, 20000}
    };

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if (board[x][y]) {
                Piece* piece = board[x][y].get();
                int value = pieceValues[piece->GetType()];
                if (piece->GetColor() == playerColor) {
                    score += value;
                } else {
                    score -= value;
                }
            }
        }
    }
    return score;
}

int Board::EvaluateMobility(PieceColor color) const {
    int mobility = 0;
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if (board[x][y] && board[x][y]->GetColor() == color) {
                auto moves = board[x][y]->GetPossibleMoves(*this, wxPoint(x, y));
                mobility += moves.size();
            }
        }
    }
    return mobility;
}

int Board::EvaluateKingSafety(PieceColor color) const {
    int safety = 0;
    wxPoint kingPos = GetKingPosition(color);
    
    // Kara za brak roszady
    if (color == PieceColor::WHITE && !whiteKingMoved) {
        safety -= 20;
    }
    if (color == PieceColor::BLACK && !blackKingMoved) {
        safety -= 20;
    }
    
    // Bonus za bezpieczne pozycje króla
    if (color == PieceColor::WHITE) {
        if (kingPos.y == 7 && kingPos.x == 4) safety += 10;
        if (kingPos.y == 7 && (kingPos.x == 6 || kingPos.x == 2)) safety += 5;
    } else {
        if (kingPos.y == 0 && kingPos.x == 4) safety += 10;
        if (kingPos.y == 0 && (kingPos.x == 6 || kingPos.x == 2)) safety += 5;
    }
    
    // Kara za szach
    if (IsKingInCheck(color)) {
        safety -= 50;
    }
    
    return safety;
}

int Board::EvaluateCenterControl(PieceColor color) const {
    int control = 0;
    const std::vector<wxPoint> centerSquares = {{3,3}, {3,4}, {4,3}, {4,4}};
    
    for (const auto& square : centerSquares) {
        if (IsSquareUnderAttack(square, color)) {
            control += 5;
        }
    }
    
    // Bonus za figury w centrum
    for (int x = 2; x <= 5; x++) {
        for (int y = 2; y <= 5; y++) {
            if (board[x][y] && board[x][y]->GetColor() == color) {
                if (board[x][y]->GetType() != PieceType::KING) {
                    control += 3;
                }
            }
        }
    }
    
    return control;
}

int Board::EvaluatePawnStructure(PieceColor color) const {
    int structure = 0;
    int doubledPawns = 0;
    int isolatedPawns = 0;
    int passedPawns = 0;
    
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if (board[x][y] && board[x][y]->GetType() == PieceType::PAWN && 
                board[x][y]->GetColor() == color) {
                
                // Sprawdź podwójne pionki
                for (int y2 = 0; y2 < 8; y2++) {
                    if (y2 != y && board[x][y2] && board[x][y2]->GetType() == PieceType::PAWN && 
                        board[x][y2]->GetColor() == color) {
                        doubledPawns++;
                    }
                }
                
                // Sprawdź izolowane pionki
                bool hasNeighbor = false;
                for (int dx = -1; dx <= 1; dx += 2) {
                    if (x + dx >= 0 && x + dx < 8) {
                        for (int y2 = 0; y2 < 8; y2++) {
                            if (board[x+dx][y2] && board[x+dx][y2]->GetType() == PieceType::PAWN && 
                                board[x+dx][y2]->GetColor() == color) {
                                hasNeighbor = true;
                                break;
                            }
                        }
                    }
                }
                if (!hasNeighbor) isolatedPawns++;
                
                // Sprawdź przechodnie pionki
                bool isPassed = true;
                int direction = (color == PieceColor::WHITE) ? -1 : 1;
                int startY = (color == PieceColor::WHITE) ? y - 1 : y + 1;
                int endY = (color == PieceColor::WHITE) ? 0 : 7;
                
                for (int y2 = startY; y2 != endY; y2 += direction) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (x + dx >= 0 && x + dx < 8) {
                            if (board[x+dx][y2] && board[x+dx][y2]->GetType() == PieceType::PAWN && 
                                board[x+dx][y2]->GetColor() != color) {
                                isPassed = false;
                                break;
                            }
                        }
                    }
                    if (!isPassed) break;
                }
                
                if (isPassed) passedPawns++;
            }
        }
    }
    
    structure -= doubledPawns * 10;
    structure -= isolatedPawns * 15;
    structure += passedPawns * 20;
    return structure;
}

int Board::EvaluateBoard() const {
    int score = EvaluateMaterial();
    
    // Ocena mobilności
    int mobilityWhite = EvaluateMobility(PieceColor::WHITE);
    int mobilityBlack = EvaluateMobility(PieceColor::BLACK);
    if (playerColor == PieceColor::WHITE) {
        score += mobilityWhite - mobilityBlack;
    } else {
        score += mobilityBlack - mobilityWhite;
    }
    
    // Ocena bezpieczeństwa króla
    int kingSafetyWhite = EvaluateKingSafety(PieceColor::WHITE);
    int kingSafetyBlack = EvaluateKingSafety(PieceColor::BLACK);
    if (playerColor == PieceColor::WHITE) {
        score += kingSafetyWhite - kingSafetyBlack;
    } else {
        score += kingSafetyBlack - kingSafetyWhite;
    }
    
    // Ocena kontroli centrum
    int centerControlWhite = EvaluateCenterControl(PieceColor::WHITE);
    int centerControlBlack = EvaluateCenterControl(PieceColor::BLACK);
    if (playerColor == PieceColor::WHITE) {
        score += centerControlWhite - centerControlBlack;
    } else {
        score += centerControlBlack - centerControlWhite;
    }
    
    // Ocena struktury pionków
    int pawnStructureWhite = EvaluatePawnStructure(PieceColor::WHITE);
    int pawnStructureBlack = EvaluatePawnStructure(PieceColor::BLACK);
    if (playerColor == PieceColor::WHITE) {
        score += pawnStructureWhite - pawnStructureBlack;
    } else {
        score += pawnStructureBlack - pawnStructureWhite;
    }
    
    // Bonus za aktywne figury
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if (board[x][y] && board[x][y]->GetColor() == playerColor) {
                PieceType type = board[x][y]->GetType();
                if (type != PieceType::PAWN && type != PieceType::KING) {
                    if (y < 4 || y > 3) {
                        score += 5;
                    }
                }
            }
        }
    }
    
    return score;
}

int Board::GetPieceValue(PieceType type) const {
    static std::map<PieceType, int> values = {
        {PieceType::PAWN, 100},
        {PieceType::KNIGHT, 320},
        {PieceType::BISHOP, 330},
        {PieceType::ROOK, 500},
        {PieceType::QUEEN, 900},
        {PieceType::KING, 20000}
    };
    return values[type];
}

int Board::ScoreMove(const wxPoint& from, const wxPoint& to) const {
    int score = 0;
    if (Piece* captured = board[to.x][to.y].get()) {
        score += GetPieceValue(captured->GetType()) * 10;
        score -= GetPieceValue(board[from.x][from.y]->GetType());
    }
    if (board[from.x][from.y]->GetType() == PieceType::PAWN && 
        (to.y == 0 || to.y == 7)) {
        score += 800; // Promotion bonus
    }
    return score;
}

void Board::StartSearchTimer() {
    searchTimeout = false;
    searchStartTime = std::chrono::steady_clock::now();
}

bool Board::IsTimeOut() const {
    if (searchTimeout) return true;
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - searchStartTime);
    return elapsed.count() > searchTimeLimit;
}

void Board::CheckTime() {
    if (IsTimeOut()) {
        searchTimeout = true;
    }
}

int Board::MinMax(int depth, int alpha, int beta, bool maximizingPlayer) {
    if (depth == 0 || gameOver) {
        return EvaluateBoard();
    }

    CheckTime();
    if (IsTimeOut()) {
        return maximizingPlayer ? INT_MIN : INT_MAX;
    }

    PieceColor currentColor = maximizingPlayer ? playerColor : 
        (playerColor == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;

    using ScoredMove = std::pair<wxPoint, wxPoint>;
    std::vector<std::pair<ScoredMove, int>> scoredMoves;

    // Generuj i oceniaj wszystkie ruchy
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if (board[x][y] && board[x][y]->GetColor() == currentColor) {
                wxPoint from(x, y);
                auto moves = board[x][y]->GetPossibleMoves(*this, from);
                
                for (const auto& to : moves) {
                    if (IsMoveLegal(from, to)) {
                        scoredMoves.push_back({{from, to}, ScoreMove(from, to)});
                    }
                }
            }
        }
    }

    // Sortuj ruchy według oceny (najlepsze na początku)
    std::sort(scoredMoves.begin(), scoredMoves.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    int bestValue = maximizingPlayer ? INT_MIN : INT_MAX;
    bool foundMove = false;

    for (const auto& [move, score] : scoredMoves) {
        if (IsTimeOut()) break;
        
        MoveState savedState;
        GetCurrentState(savedState);
        DoMove(move.first, move.second);

        int value = MinMax(depth - 1, alpha, beta, !maximizingPlayer);
        
        RestoreState(savedState);

        if (maximizingPlayer) {
            if (value > bestValue) {
                bestValue = value;
            }
            alpha = std::max(alpha, bestValue);
        } else {
            if (value < bestValue) {
                bestValue = value;
            }
            beta = std::min(beta, bestValue);
        }

        // Przycinanie alfa-beta
        if (beta <= alpha) {
            break;
        }
        foundMove = true;
    }

    return foundMove ? bestValue : 
        (maximizingPlayer ? INT_MIN + 100 : INT_MAX - 100);
}

std::pair<wxPoint, wxPoint> Board::FindBestMove(int depth) {
    StartSearchTimer();
    int bestValue = INT_MIN;
    std::pair<wxPoint, wxPoint> bestMove = {{-1, -1}, {-1, -1}};

    // Generuj wszystkie ruchy na poziomie głównym
    std::vector<std::pair<std::pair<wxPoint, wxPoint>, int>> scoredRootMoves;
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if (board[x][y] && board[x][y]->GetColor() != playerColor) {
                wxPoint from(x, y);
                auto moves = board[x][y]->GetPossibleMoves(*this, from);
                for (const auto& to : moves) {
                    if (IsMoveLegal(from, to)) {
                        scoredRootMoves.push_back({{from, to}, ScoreMove(from, to)});
                    }
                }
            }
        }
    }

    // Sortuj ruchy według oceny
    std::sort(scoredRootMoves.begin(), scoredRootMoves.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    // Przeszukuj ruchy w kolejności od najlepszego do najgorszego
    for (const auto& [move, score] : scoredRootMoves) {
        if (IsTimeOut()) {
            break;
        }

        MoveState savedState;
        GetCurrentState(savedState);
        DoMove(move.first, move.second);

        int value = MinMax(depth - 1, INT_MIN, INT_MAX, false);

        RestoreState(savedState);

        if (value > bestValue) {
            bestValue = value;
            bestMove = move;
        }
    }

    return bestMove;
}

void Board::ComputerMove() {
    if (!gameOver && IsComputerTurn() && promotionSquare.x == -1) {
        auto move = FindBestMove(aiDepth);
        if (move.first.x != -1) {
            SaveState();
            DoMove(move.first, move.second);

            // Sprawdź stan gry po ruchu
            PieceColor opponent = (currentTurn == PieceColor::WHITE) ? 
                PieceColor::BLACK : PieceColor::WHITE;
                
            if (IsCheckmate(opponent)) {
                ShowGameOverDialog("Checkmate! " + wxString(opponent == PieceColor::WHITE ? "White" : "Black") + " wins!");
            } else if (IsStalemate(opponent)) {
                ShowGameOverDialog("Stalemate! Game drawn!");
            }
        }
        Refresh();
    }
}

void Board::UndoLastMove() {
    if (moveHistory.size() <= 1) return;
    
    moveHistory.pop();
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
    promotionSquare = wxPoint(-1, -1);
    
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

    // Podświetl pole promocji
    if (promotionSquare.x != -1) {
        wxRect promoRect(promotionSquare.x * tileSize.x, promotionSquare.y * tileSize.y,
                         tileSize.x, tileSize.y);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(wxPen(*wxBLUE, 4));
        dc.DrawRectangle(promoRect);
    }

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

    // Jeśli trwa promocja, obsłuż wybór figury
    if (promotionSquare.x != -1) {
        wxPoint clickPos = event.GetPosition();
        int x = clickPos.x / tileSize.x;
        int y = clickPos.y / tileSize.y;
        
        if (x >= 0 && x < 8 && y >= 0 && y < 8) {
            // Dla uproszczenia zawsze promuj do hetmana
            PromotePawn(promotionSquare, PieceType::QUEEN);
            Refresh();
            
            // Po promocji sprawdź stan gry
            PieceColor opponent = (currentTurn == PieceColor::WHITE) ? 
                PieceColor::BLACK : PieceColor::WHITE;
                
            if (IsCheckmate(opponent)) {
                ShowGameOverDialog("Checkmate! " + wxString(currentTurn == PieceColor::WHITE ? "White" : "Black") + " wins!");
            } else if (IsStalemate(opponent)) {
                ShowGameOverDialog("Stalemate! Game drawn!");
            }
        }
        return;
    }

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
            DoMove(selectedPiece, dest);
            
            PieceColor movedColor = board[dest.x][dest.y]->GetColor();
            PieceColor opponent = (movedColor == PieceColor::WHITE) ? 
                PieceColor::BLACK : PieceColor::WHITE;
                
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
    if (!gameOver && IsComputerTurn() && promotionSquare.x == -1) {
        ComputerMove();
    }
}
