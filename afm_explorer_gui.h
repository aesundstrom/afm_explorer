#ifndef __afm_explorer_gui__
#define __afm_explorer_gui__

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include <wx/menu.h>
#include <wx/scrolwin.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/listbox.h>
#include <wx/listctrl.h>
#include <wx/statusbr.h>
#include <wx/frame.h>
#include "scroll_win.h"

///////////////////////////////////////////////////////////////////////////////
/// Class AFMExplorerGui
///////////////////////////////////////////////////////////////////////////////
class AFMExplorerGui : public wxFrame 
{
	private:
		DECLARE_EVENT_TABLE()

	protected:
		wxString imageFileName;
		int imageWidth;
		int imageHeight;
		int conversionNanometers;
		double conversionFactor;
                
		wxString defaultPath;

		int selectedItemIndex;
		int selectedItemId;

		int threshold_method;
		int fixed_thresh_too_bright_elim;
		int fixed_thresh_too_dim_elim;
		int adaptive_thresh_too_dim_elim;
		int adaptive_thresh_method;
		int adaptive_thresh_box_dim;

		wxMenuBar* m_menubar2;
		wxMenu* file;
		wxMenu* m_menu21;
		wxMenu* m_menu2;
		wxMenu* m_menu4;
		wxMenu* m_menu3;
		wxNotebook* m_notebook1;
		WindowScroll* scroll_win_original;
		WindowScroll* scroll_win_filtered;
		WindowScroll* scroll_win_backbone;
		wxStaticText* m_staticText1;
		wxListCtrl* m_listCtrl1;
		wxStaticText* m_staticText2;
		wxListBox* m_listBox2;
		wxStaticText* m_staticText3;
		wxListBox* m_listBox3;
		wxStatusBar* m_statusBar1;
		wxMenuItem *menuItem3;
		wxMenuItem *menuItem4;
		wxMenuItem *menuItem5;
		wxMenuItem *menuItemSaveComponents;
		wxMenuItem *menuItem11;
		wxMenuItem *menuItemSetImageProcessingParameters;
		wxBoxSizer* bSizer11;

		// Virtual event handlers, overide them in your derived class
		virtual void PerformResize( wxSizeEvent& event ){ event.Skip(); }
		virtual void OpenFile( wxCommandEvent& event ){ event.Skip(); }
		virtual void Exit( wxCommandEvent& event ){ event.Skip(); }
		virtual void SetImageProcessingParameters( wxCommandEvent& event ){ event.Skip(); }
		virtual void SetConversionFactor( wxCommandEvent& event ){ event.Skip(); }
		virtual void ProcessImage( wxCommandEvent& event ){ event.Skip(); }
		virtual void SetFit( wxCommandEvent& event ){ event.Skip(); }
		virtual void SetNoZoom( wxCommandEvent& event ){ event.Skip(); }
		virtual void SetZoom2x( wxCommandEvent& event ){ event.Skip(); }
		virtual void SetZoom3x( wxCommandEvent& event ){ event.Skip(); }
		virtual void SetZoom4x( wxCommandEvent& event ){ event.Skip(); }
		virtual void SetZoom5x( wxCommandEvent& event ){ event.Skip(); }
		virtual void About( wxCommandEvent& event ){ event.Skip(); }
		virtual void HighlightImage( wxListEvent& event ){ event.Skip(); }
		virtual void SortComponents( wxListEvent& event ){ event.Skip(); }
		virtual void SaveFiltered( wxCommandEvent& event ){ event.Skip(); }
		virtual void SaveBackbones( wxCommandEvent& event ){ event.Skip(); }
		virtual void SaveComponents( wxCommandEvent& event ){ event.Skip(); }
		virtual void SetStatusValue( wxUpdateUIEvent& event ){ event.Skip(); }
	
	public:
		AFMExplorerGui(
			wxWindow* parent,
			int id = wxID_ANY,
			wxString title = wxT( "AFM Explorer Tool" ),
			wxPoint pos = wxPoint( 0, 22 ),
			wxSize size = wxSize( 850, 600 ),
			int style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL
		);
		virtual void DrawImage1( wxUpdateUIEvent& event ){ event.Skip(); }
		virtual void DrawImage2( wxUpdateUIEvent& event ){ event.Skip(); }
		virtual void DrawImage3( wxUpdateUIEvent& event ){ event.Skip(); }
	
};

#endif //__afm_explorer_gui__
