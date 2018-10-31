 // For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/wx.h"
#include "afm_explorer.h"

// Define a new application type, each program should derive a class from wxApp
class main : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------
	AFMExplorer *frame; 
	
    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
	wxDocManager *fh;
};

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)

IMPLEMENT_APP(main)

bool main::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;

    // create the main application window
    frame = new AFMExplorer(NULL,"AFM Explorer");

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame ->Show(); 
    SetTopWindow( frame );
    return true;

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}
