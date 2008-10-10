#include <wx/wx.h>
#include "wx/aui/aui.h"


class MyFrame : public wxFrame {
 public:
   MyFrame(wxWindow* parent) : wxFrame(parent, -1, _("wxAUI Test"),
                      wxDefaultPosition, wxSize(800,600),
                      wxDEFAULT_FRAME_STYLE)                              
   {
    m_mgr.SetFlags(wxAUI_MGR_DEFAULT | wxAUI_MGR_TRANSPARENT_DRAG);

     // notify wxAUI which frame to use
     m_mgr.SetManagedWindow(this);
 
     // create several text controls
     wxTextCtrl* text1 = new wxTextCtrl(this, -1, _("Pane 1 - sample text"),
                      wxDefaultPosition, wxSize(200,150),
                      wxNO_BORDER | wxTE_MULTILINE);
                                        
     wxTextCtrl* text2 = new wxTextCtrl(this, -1, _("Pane 2 - sample text"),
                      wxDefaultPosition, wxSize(200,150),
                      wxNO_BORDER | wxTE_MULTILINE);
                                        
     wxTextCtrl* text3 = new wxTextCtrl(this, -1, _("Main content window"),
                      wxDefaultPosition, wxSize(200,150),
                      wxNO_BORDER | wxTE_MULTILINE);
         
   // create the notebook off-window to avoid flicker
   wxSize client_size = GetClientSize();
   long m_notebook_style = wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER;
   wxAuiNotebook* notebookCtrl = new wxAuiNotebook(this, wxID_ANY,
                                    wxPoint(client_size.x, client_size.y),
                                    wxSize(430,200),
                                    m_notebook_style);
   notebookCtrl->AddPage( new wxTextCtrl( notebookCtrl, wxID_ANY, wxT("Some text #1"),
                                          wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxNO_BORDER) , 
                          wxT("wxTextCtrl 1"), false /*, page_bmp*/ );

   notebookCtrl->AddPage( new wxTextCtrl( notebookCtrl, wxID_ANY, wxT("Some text #2"),
                                          wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxNO_BORDER) , 
                          wxT("wxTextCtrl 2"), false /*, page_bmp*/ );

     // add the panes to the manager
     m_mgr.AddPane(text1, wxLEFT, wxT("Pane Number One"));
     m_mgr.AddPane(text2, wxBOTTOM, wxT("Pane Number Two"));
     m_mgr.AddPane(text3, wxCENTER);
    //m_mgr.AddPane(notebookCtrl, wxAuiPaneInfo().
    //              Caption(wxT("Notebook")).
    //              Float().
    //              //FloatingPosition(GetStartPosition()).
    //              //FloatingSize(300,200).
    //              CloseButton(true).MaximizeButton(true));

    m_mgr.AddPane(notebookCtrl, wxAuiPaneInfo().Name(wxT("notebook_content")).
                  CenterPane().PaneBorder(false));

     // tell the manager to "commit" all the changes just made
     m_mgr.Update();
   }
 
   ~MyFrame()
   {
     // deinitialize the frame manager
     m_mgr.UnInit();
   }
 
 private:
     wxAuiManager m_mgr;
 };
 
  // our normal wxApp-derived class, as usual
 class MyApp : public wxApp {
 public:
 
   bool OnInit()
   {
     wxFrame* frame = new MyFrame(NULL);
     SetTopWindow(frame);
     frame->Show();
     return true;                    
   }
 };
 
  DECLARE_APP(MyApp);
  IMPLEMENT_APP(MyApp);
