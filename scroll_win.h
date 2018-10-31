#include "wx/wx.h"
#include "wx/scrolwin.h"
#include <iostream>

using namespace std;

class WindowScroll : public wxScrolledWindow
{

public:
	WindowScroll( wxWindow *parent );
	void OnScroll(wxScrollWinEvent& event);
	void OnMouseEnter(wxMouseEvent& event);
	void OnMouseLeave(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnMouseClick(wxMouseEvent& event);
	int x,y;
	int xm,ym;
	int xc,yc;
	
	DECLARE_EVENT_TABLE()
};



