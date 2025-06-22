#ifndef BOARD_H
#define BOARD_H

#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/msgdlg.h>
#include <vector>
#include <memory>
#include <random>
#include <stack>
#include <map>
#include <climits>
#include <algorithm>
#include <unordered_set>
#include <atomic>
#include <chrono>
#include <future>
#include <mutex>
#include "Piece.h"

class Board : public wxPanel {
public:
    explicit Board(wxWindow* parent);
    void InitNewGame();
    void ResetGame();
    void SetRandomColor();
    void UndoLastMove();

    bool IsEmpty(int x, int y) const;
    bool IsEnemy(int x, int y, PieceColor color) const;
    bool IsValidMove(int fromX, int fromY, int toX, int toY) const;
    bool IsInsideBoard(wxPoint p) const { return p.x >= 0 && p.x < 8 && p.y >= 0 && p.y < 8; }
    Piece* GetPieceAt(wxPoint p) const;
    bool IsRook(int x, int y, PieceColor color) const;
    wxPoint GetEnPassantTarget() const { return enPassantTarget; }
    void SetEnPassantTarget(wxPoint target) { enPassantTarget = target; }
    bool IsEnPassantTarget(int x, int y) const { return enPassantTarget == wxPoint(x, y); }
    bool CanCastleKingside(PieceColor color) const;
    bool CanCastleQueenside(PieceColor color) const;
    void SetKingMoved(PieceColor color);
    void SetRookMoved(int x, int y);
    bool IsSquareUnderAttack(wxPoint square, PieceColor attackerColor) const;
    bool IsKingInCheck(PieceColor color) const;
    bool IsCheckmate(PieceColor color);
    bool IsStalemate(PieceColor color);
    bool HasLegalMoves(PieceColor color);
    void ShowGameOverDialog(wxString message);

    PieceColor GetCurrentTurn() const { return currentTurn; }

    wxPoint GetKingPosition(PieceColor color) const;
    std::vector<wxPoint> GetCheckingPieces(PieceColor color) const;
    bool IsMoveLegal(wxPoint from, wxPoint to);
    bool IsComputerTurn() const { return currentTurn != playerColor; }
    void PromotePawn(wxPoint pos, PieceType promotionType = PieceType::QUEEN);

private:
    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void HighlightChecks(wxAutoBufferedPaintDC& dc) const;
    void DoMove(wxPoint from, wxPoint to);
    void ComputerMove();
    void HandlePawnPromotion(wxPoint pos);
    
    struct MoveState {
        std::unique_ptr<Piece> board[8][8];
        wxPoint selectedPiece;
        PieceColor currentTurn;
        wxPoint enPassantTarget;
        wxPoint whiteKingPos;
        wxPoint blackKingPos;
        bool whiteKingMoved;
        bool blackKingMoved;
        bool whiteRookKMoved;
        bool whiteRookQMoved;
        bool blackRookKMoved;
        bool blackRookQMoved;
        wxPoint promotionSquare;
    };
    
    void SaveState();
    void RestoreState(const MoveState& state);
    void GetCurrentState(MoveState& state) const;
    
    std::pair<wxPoint, wxPoint> FindBestMove(int depth);
    int MinMax(int depth, int alpha, int beta, bool maximizingPlayer);
    int EvaluateBoard() const;
    int EvaluateMaterial() const;
    int EvaluateMobility(PieceColor color) const;
    int EvaluateKingSafety(PieceColor color) const;
    int EvaluateCenterControl(PieceColor color) const;
    int EvaluatePawnStructure(PieceColor color) const;
    
    // Time management functions
    void StartSearchTimer();
    bool IsTimeOut() const;
    void CheckTime();
    
    // Move scoring
    int ScoreMove(const wxPoint& from, const wxPoint& to) const;
    int GetPieceValue(PieceType type) const;

    wxSize tileSize = wxSize(60, 60);
    std::unique_ptr<Piece> board[8][8];
    wxPoint selectedPiece = wxPoint(-1, -1);
    PieceColor currentTurn = PieceColor::WHITE;
    PieceColor playerColor = PieceColor::WHITE;
    std::vector<wxPoint> possibleMoves;
    wxPoint enPassantTarget = wxPoint(-1, -1);
    wxPoint promotionSquare = wxPoint(-1, -1);

    // King positions for quick access
    wxPoint whiteKingPos = wxPoint(4, 7);
    wxPoint blackKingPos = wxPoint(4, 0);

    // Game state flags
    bool gameOver = false;
    wxString gameResult = "";

    // Castling flags
    bool whiteKingMoved = false;
    bool blackKingMoved = false;
    bool whiteRookKMoved = false;
    bool whiteRookQMoved = false;
    bool blackRookKMoved = false;
    bool blackRookQMoved = false;

    // AI settings
    int aiDepth = 4;

    // Time management
    std::atomic<bool> searchTimeout{false};
    std::chrono::steady_clock::time_point searchStartTime;
    int searchTimeLimit = 3000; // 5-second limit

    // Move history
    std::stack<MoveState> moveHistory;

    wxDECLARE_EVENT_TABLE();
};

#endif // BOARD_H
