#include <wx/wx.h>
#include "Board.h"

class Chess : public wxApp {
    public:
        virtual bool OnInit();
};

class BaseFrame : public wxFrame {
    public:
        BaseFrame(const wxString& title);
    private:
        Board* board;
        void OnReset(wxCommandEvent& event);
        void OnRandomColor(wxCommandEvent& event);
};

wxIMPLEMENT_APP(Chess);

bool Chess::OnInit() {
    BaseFrame *frame = new BaseFrame("Chess");
    frame->Show(true);
    return true;
}

BaseFrame::BaseFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(500, 500)) {
    wxPanel* mainPanel = new wxPanel(this);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Create button panel
    wxPanel* buttonPanel = new wxPanel(mainPanel);
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxButton* resetButton = new wxButton(buttonPanel, wxID_ANY, "Reset Game");
    wxButton* randomColorButton = new wxButton(buttonPanel, wxID_ANY, "Random Color");
    
    resetButton->Bind(wxEVT_BUTTON, &BaseFrame::OnReset, this);
    randomColorButton->Bind(wxEVT_BUTTON, &BaseFrame::OnRandomColor, this);
    
    buttonSizer->Add(resetButton, 0, wxALL, 5);
    buttonSizer->Add(randomColorButton, 0, wxALL, 5);
    buttonPanel->SetSizer(buttonSizer);
    
    // Create chess board
    board = new Board(mainPanel);
    
    // Add to main sizer
    mainSizer->Add(buttonPanel, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);
    mainSizer->Add(board, 1, wxEXPAND | wxALL, 5);
    mainPanel->SetSizer(mainSizer);
    
    Centre();
}

void BaseFrame::OnReset(wxCommandEvent& event) {
    board->ResetGame();
}

void BaseFrame::OnRandomColor(wxCommandEvent& event) {
    board->SetRandomColor();
}
