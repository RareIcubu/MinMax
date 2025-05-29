#include <wx/wx.h>
#include "Board.h"

class Chess : public wxApp {
    public:
        virtual bool OnInit();
};

class BaseFrame : public wxFrame {
    public:
        BaseFrame(const wxString& title);
};

wxIMPLEMENT_APP(Chess);

bool Chess::OnInit() {
    BaseFrame *frame = new BaseFrame("Chess");
    frame->Show(true);
    return true;
}

BaseFrame::BaseFrame(const wxString& title) : wxFrame(NULL,wxID_ANY,title, wxDefaultPosition, wxSize(500,500)){
    Board* board = new Board(this);
    Centre();
}
