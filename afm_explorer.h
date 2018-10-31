#ifndef __length_calculation__
#define __length_calculation__

#include "afm_explorer_gui.h"
#include "wx/wx.h"
#include "wx/wxprec.h"
#include "wx/image.h"
#include "wx/file.h"
#include "wx/scrolwin.h"
#include "wx/scrolbar.h"
#include "wx/docview.h"
#include "wx/config.h"
#include "wx/textctrl.h"
#include "wx/numdlg.h"
#include "wx/textdlg.h"
#include "wx/choicdlg.h"
#include "process_image.h"

class AFMExplorer : public AFMExplorerGui
{
private:
	void SetFitOriginal();
	void SetFitFiltered();
	void SetFitBackbone();

protected:
	// Handlers for LengthGui events.
	void OpenFile( wxCommandEvent& event );
	void DrawImage1( wxUpdateUIEvent& event );
	void DrawImage2( wxUpdateUIEvent& event );
	void DrawImage3( wxUpdateUIEvent& event );
	void Exit( wxCommandEvent& event );
	void ProcessImage( wxCommandEvent& event );
	void SetImageProcessingParameters( wxCommandEvent& event );
	void SetConversionFactor( wxCommandEvent& event );
	void SetFit( wxCommandEvent& event );
	void SetNoZoom( wxCommandEvent& event );
	void SetZoom2x( wxCommandEvent& event );
	void SetZoom3x( wxCommandEvent& event );
	void SetZoom4x( wxCommandEvent& event );
	void SetZoom5x( wxCommandEvent& event );
	void About( wxCommandEvent& event );
	void Zoom(int zoom_val);
	void PerformResize( wxSizeEvent& event );
	void SetStatusValue( wxUpdateUIEvent& event );
	void RecentSelected(wxCommandEvent& event);
	void HighlightImage( wxListEvent& event );
	void SortComponents( wxListEvent& event );
	void SaveFiltered( wxCommandEvent& event );
	void SaveBackbones( wxCommandEvent& event );
	void SaveComponents( wxCommandEvent& event );
	void SetImageProperties();
	void SetImageProcessingProperties();
	void SetComponentProperties();
	void SetProgramStatus( const wxString& str );
	void SelectItem(int ind);

	int zoom;
	short *label,*length;
	wxFileHistory *fh;
	wxConfig *config;
	bool wxOriginalActive, wxFilteredActive, wxBackboneActive;

public:
	wxImage *wxOriginal, *wxOriginalZoom, *wxFiltered, *wxFilteredZoom, *wxBackbone, *wxBackboneZoom;
	IplImage *cvOriginal;
	FilterImages flt;

	AFMExplorer( wxWindow* parent, wxString title );
};

#endif // __length_calculation__
