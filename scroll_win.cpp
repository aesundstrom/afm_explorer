#include "scroll_win.h"

BEGIN_EVENT_TABLE(WindowScroll,wxScrolledWindow)
	EVT_SCROLLWIN(WindowScroll::OnScroll)
	EVT_ENTER_WINDOW(WindowScroll::OnMouseEnter)
	EVT_LEAVE_WINDOW(WindowScroll::OnMouseLeave)
	EVT_MOTION(WindowScroll::OnMouseMove)
	EVT_LEFT_DOWN(WindowScroll::OnMouseClick)
END_EVENT_TABLE()

WindowScroll::WindowScroll(wxWindow *parent)
            : wxScrolledWindow( parent, wxID_ANY, wxDefaultPosition, wxSize(-1,-1), wxHSCROLL|wxVSCROLL )
{
	GetViewStart( &x, &y );
	xm=0; ym=0;
	xc=0; yc=0;
}	


void WindowScroll::OnScroll(wxScrollWinEvent& event)
{
        if (event.GetOrientation() == wxHORIZONTAL) {
		x=-event.GetPosition();
        } else if (event.GetOrientation() == wxVERTICAL) {
		y=-event.GetPosition();
        }

        // For active scrolling, the window repaint might go here.
}

void WindowScroll::OnMouseEnter(wxMouseEvent& event)
{
	xm=event.m_x;
	ym=event.m_y;
}

void WindowScroll::OnMouseLeave(wxMouseEvent& event)
{
	xm=0; ym=0;
	xc=0; yc=0;
}

void WindowScroll::OnMouseMove(wxMouseEvent& event)
{
	xc=0; yc=0;
	if(xm!=0 && ym!=0)	{
		xm=event.m_x;
		ym=event.m_y;
	}
}

void WindowScroll::OnMouseClick(wxMouseEvent& event)	{
	
	if(xm!=0 && ym!=0)	{
		xc=event.m_x;
		yc=event.m_y;
	}
}
