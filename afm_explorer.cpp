#include "afm_explorer.h"
#include "the_statics.h"
	
AFMExplorer::AFMExplorer( wxWindow* parent, wxString title )
           : AFMExplorerGui( parent, 0, title )
{
	char *history_file = "history.txt";
	fh = new wxFileHistory( 7, wxID_FILE1 );
	FILE *f = fopen( history_file, "r" );
	if (f) {
		while (! feof( f )) {
			char *filename = (char *)malloc( 100 * sizeof( char ) );
			fscanf( f, "%s", filename );	
			if (strlen( filename ) > 0) {
				fh->AddFileToHistory( wxT( filename ) );
			}
		}
		fclose( f );
		fh->AddFilesToMenu( m_menu21 );
		for (int i = 0; i < m_menu21->GetMenuItemCount(); i++) {
			this->Connect(
				m_menu21->FindItemByPosition(i)->GetId(),
				wxEVT_COMMAND_MENU_SELECTED,
				wxCommandEventHandler( AFMExplorer::RecentSelected )
			);
		}
	}
	
	if (wxOriginalActive)	wxOriginal		= NULL;
	if (wxFilteredActive)	wxFiltered		= NULL;
	if (wxBackboneActive)	wxBackbone		= NULL;
	if (wxOriginalZoom)		wxOriginalZoom	= NULL;
	if (wxFilteredZoom)		wxFilteredZoom	= NULL;
	if (wxBackboneZoom)		wxBackboneZoom	= NULL;
	
	wxOriginalActive = false;
	wxFilteredActive = false;
	wxBackboneActive = false;

	idSortAscending = false;
	lengthSortAscending = true;

	zoom = 1;
	m_menu4->Check( 1, true );
	for (int i = 2; i <= 6; i++)	{
		m_menu4->Enable(i,false);
		m_menu4->Check(i,false);
	}

	SetImageProperties();
	SetImageProcessingProperties();
	SetProgramStatus( wxString( "Ready to load image." ) );
}

void AFMExplorer::OpenFile( wxCommandEvent& event )
{	
	wxFileDialog dialog(
		this,
		wxT( "Choose a file" ),
		wxT( defaultPath ),
		wxEmptyString,
		wxT( "BMP files (*.bmp)|*.bmp|GIF files (*.gif)|*.gif" ),
		wxFD_OPEN
	);
	
	if (dialog.ShowModal() == wxID_OK) {
		SetProgramStatus( wxString( "Loading image..." ) );

		// TODO: fix dependency (Windows uses '\\', Unix uses '/').
		defaultPath = dialog.GetPath().BeforeLast( wxChar( '/' ) ) + wxChar( '/' );
		
		// Add the new path/filename to the file history.
		//fh->AddFileToHistory( dialog.GetPath() );
		// Add the file history to the "Most Recently Opened" submenu of the "File" menu.
		//fh->AddFilesToMenu( m_menu21 );
		// Dynamically construct an event handler for the new path/filename menu item.
		//this->Connect(
		//	m_menu21->FindItemByPosition( m_menu21->GetMenuItemCount() )->GetId(),
		//	wxEVT_COMMAND_MENU_SELECTED,
		//	wxCommandEventHandler( AFMExplorer::RecentSelected )
		//);
		// Update "File" menu that owns "Most Recently Opened" submenu.
		//m_menu21->UpdateUI();

		wxString path = dialog.GetPath();
                flt.LoadImage( (char *)path.c_str() );
		
		if (wxOriginalActive)	{
			wxOriginal->Destroy();
			wxOriginalZoom->Destroy();
			wxOriginalActive = false;

			scroll_win_original->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( AFMExplorerGui::DrawImage1 ), NULL, this );
			scroll_win_original = NULL;

			if (wxFilteredActive)	{
				wxFiltered->Destroy();
				wxFilteredZoom->Destroy();
				wxFilteredActive = false;

				scroll_win_filtered->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( AFMExplorerGui::DrawImage2 ), NULL, this );
				scroll_win_filtered = NULL;
			}
			if (wxBackboneActive)	{
				wxBackbone->Destroy();
				wxBackboneZoom->Destroy();
				wxBackboneActive = false;

				scroll_win_backbone->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( AFMExplorerGui::DrawImage3 ), NULL, this );
				scroll_win_backbone = NULL;
			}
                        
			m_notebook1->DeleteAllPages();
		}
		
		wxOriginal = new wxImage();
		wxOriginal->LoadFile(path,wxBITMAP_TYPE_BMP,-1);
		wxOriginalZoom = new wxImage(*wxOriginal);
		wxOriginalActive = true;

		scroll_win_original = new WindowScroll( m_notebook1 );
		scroll_win_original->SetScrollRate( 5, 5 );
		m_notebook1->AddPage( scroll_win_original, wxT("Original Image"), false );
		scroll_win_original->Connect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( AFMExplorerGui::DrawImage1 ), NULL, this );

		scroll_win_original->SetScrollbars(1,1,wxOriginal->GetWidth(),wxOriginal->GetHeight());
		scroll_win_original->Refresh();
		for (int i = 1; i <= 6; i++) {
			m_menu4->Enable(i,true);
		}
		
		fh->AddFileToHistory(path);

		m_listCtrl1->DeleteAllItems();

		wxString s("");
		m_listBox2->Set(1,&s,NULL);

		menuItem11->Enable( true );
		menuItem4->Enable( false );
		menuItem5->Enable( false );
		menuItemSaveComponents->Enable( false );

		imageFileName = path;
		imageWidth = wxOriginal->GetWidth();
		imageHeight = wxOriginal->GetHeight();
		SetImageProperties();

		if (flt.conversion_factor == -1.0) {
			SetProgramStatus( wxString( "Ready to set conversion factor." ) );
		}
		else {
			SetProgramStatus( wxString( "Ready to process image." ) );
		}

		idSortAscending = false;
		lengthSortAscending = true;
		
		selectedItemIndex = -1;
		selectedItemId = -1;
	}
}

void AFMExplorer::DrawImage1( wxUpdateUIEvent& event )
{
	if (wxOriginalZoom && m_notebook1->GetCurrentPage()==scroll_win_original)	{
		wxClientDC dc(scroll_win_original);
		scroll_win_original->DoPrepareDC(dc); 

                if (m_menu4->IsChecked(1)) {
                  SetFitOriginal();
                }

		dc.DrawBitmap(*wxOriginalZoom,scroll_win_original->x,scroll_win_original->y,true);		
		scroll_win_original->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );
	}
}	

void AFMExplorer::DrawImage2( wxUpdateUIEvent& event )
{
	if (wxFilteredZoom && m_notebook1->GetCurrentPage()==scroll_win_filtered)	{
		wxClientDC dc(scroll_win_filtered);
		scroll_win_filtered->DoPrepareDC(dc);

                if (m_menu4->IsChecked(1)) {
                  SetFitFiltered();
                }

		dc.DrawBitmap(*wxFilteredZoom,scroll_win_filtered->x,scroll_win_filtered->y,true);
	}
	if (!wxFilteredZoom && m_notebook1->GetCurrentPage()==scroll_win_filtered)
		scroll_win_filtered->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );
}

void AFMExplorer::DrawImage3( wxUpdateUIEvent& event )
{	
	if (wxBackboneZoom && m_notebook1->GetCurrentPage()==scroll_win_backbone)	{
		wxClientDC dc(scroll_win_backbone);
		scroll_win_backbone->DoPrepareDC(dc);

                if (m_menu4->IsChecked(1)) {
                  SetFitBackbone();
                }

		dc.DrawBitmap(*wxBackboneZoom,scroll_win_backbone->x,scroll_win_backbone->y,true);
	}
	if (!wxBackboneZoom && m_notebook1->GetCurrentPage()==scroll_win_backbone)
		scroll_win_backbone->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );
	
}

void AFMExplorer::Exit( wxCommandEvent& event )
{       
        wxFile f( "history.txt", wxFile::write );
	for (int i = 0; i < (int)fh->GetCount(); i++) {
                f.Write( " ", wxConvUTF8 );
                f.Write( fh->GetHistoryFile( i ), wxConvUTF8 );	
	}
	Close();
}

void AFMExplorer::SetImageProcessingParameters( wxCommandEvent& event )
{
        // Set priors (defined in afm_explorer_gui.h, initialized in afm_explorer_gui.cpp).
        int prior_threshold_method = threshold_method;
        int prior_fixed_thresh_too_bright_elim = fixed_thresh_too_bright_elim;
        int prior_fixed_thresh_too_dim_elim = fixed_thresh_too_dim_elim;
        int prior_adaptive_thresh_too_dim_elim = adaptive_thresh_too_dim_elim;
        int prior_adaptive_thresh_method = adaptive_thresh_method;
        int prior_adaptive_thresh_box_dim = adaptive_thresh_box_dim;

        wxString msg;

        // Set thresholding method: fixed or adaptive.
        msg.Printf( "Select the thresholding method..." );
        wxArrayString strings;
        strings.Add( wxT( "Fixed" ) );
        strings.Add( wxT( "Adaptive" ) );
        wxSingleChoiceDialog ThresholdMethodDialog(
            this,
            msg,
            wxT( "Thresholding method" ),
            strings
        );
        ThresholdMethodDialog.SetSelection( threshold_method );
        if (ThresholdMethodDialog.ShowModal() == wxID_OK) {
          threshold_method = ThresholdMethodDialog.GetSelection();
        }

        // Fixed or adaptive?
        if (threshold_method == 0) { // fixed

          // Set fixed thresh too bright elim.
          msg.Printf( "Select the fixed threshold level for eliminating pixels that are too bright..." );
          wxNumberEntryDialog FixedThreshTooBrightElimDialog(
              this,
              msg,
              wxT( "[1,255]:" ),
              wxT( "Eliminate pixels that are too bright" ),
              fixed_thresh_too_bright_elim,
              1,
              255
          );
          if (FixedThreshTooBrightElimDialog.ShowModal() == wxID_OK) {
            fixed_thresh_too_bright_elim = FixedThreshTooBrightElimDialog.GetValue();
          }

          // Set fixed thresh too dim elim.
          wxString range;
          range.Printf( "[%d,%d]:", 0, fixed_thresh_too_bright_elim - 1 );
          msg.Printf( "Select the fixed threshold level for eliminating pixels that are too dim..." );
          wxNumberEntryDialog FixedThreshTooDimElimDialog(
              this,
              msg,
              range,
              wxT( "Eliminate pixels that are too dim" ),
              fixed_thresh_too_dim_elim,
              0,
              fixed_thresh_too_bright_elim - 1
          );
          if (FixedThreshTooDimElimDialog.ShowModal() == wxID_OK) {
            fixed_thresh_too_dim_elim = FixedThreshTooDimElimDialog.GetValue();
          }

        }
        else if (threshold_method == 1) { // adaptive

          // Set adaptive thresh too dim elim.
          msg.Printf( "Select the adaptive threshold level for eliminating pixels that are too dim..." );
          wxNumberEntryDialog AdaptiveThreshTooDimElimDialog(
              this,
              msg,
              wxT( "[0,254]:" ),
              wxT( "Eliminate pixels that are too dim" ),
              adaptive_thresh_too_dim_elim,
              0,
              254
          );
          if (AdaptiveThreshTooDimElimDialog.ShowModal() == wxID_OK) {
            adaptive_thresh_too_dim_elim = AdaptiveThreshTooDimElimDialog.GetValue();
          }

          // Set adaptive thresholding method: mean or gaussian.
          msg.Printf( "Select the adaptive thresholding method..." );
          wxArrayString strings;
          strings.Add( wxT( "Mean" ) );
          strings.Add( wxT( "Gaussian" ) );
          wxSingleChoiceDialog AdaptiveThresholdMethodDialog(
              this,
              msg,
              wxT( "Adaptive thresholding method" ),
              strings
          );
          ThresholdMethodDialog.SetSelection( adaptive_thresh_method );
          if (AdaptiveThresholdMethodDialog.ShowModal() == wxID_OK) {
            adaptive_thresh_method = AdaptiveThresholdMethodDialog.GetSelection();
          }

          // Set adaptive thresh box dim.
          msg.Printf( "Select the adaptive threshold box dimension for retaining pixels that are locally bright enough..." );
          wxNumberEntryDialog AdaptiveThreshBoxDimDialog(
              this,
              msg,
              wxT( "[3,999], must be odd:" ),
              wxT( "Retain pixels that are locally bright enough" ),
              adaptive_thresh_box_dim,
              3,
              999
          );
          if (AdaptiveThreshBoxDimDialog.ShowModal() == wxID_OK) {
            adaptive_thresh_box_dim = AdaptiveThreshBoxDimDialog.GetValue();
          }

        }
        
        SetImageProcessingProperties();

        // Prompt to reprocess if any change from priors.
        if (wxFilteredActive && (
            prior_threshold_method != threshold_method ||
            prior_fixed_thresh_too_bright_elim != fixed_thresh_too_bright_elim ||
            prior_fixed_thresh_too_dim_elim != fixed_thresh_too_dim_elim ||
            prior_adaptive_thresh_too_dim_elim != adaptive_thresh_too_dim_elim ||
            prior_adaptive_thresh_method != adaptive_thresh_method ||
            prior_adaptive_thresh_box_dim != adaptive_thresh_box_dim
        ) ) {
          wxMessageDialog ReprocessDialog(
              this,
              wxT( "Displayed component lengths do not reflect image processing using the current parameters.\nWould you like to reprocess the image using the new values?" ),
              wxT( "Reprocess image?" ),
              wxYES_NO
          );
          if (ReprocessDialog.ShowModal() == wxID_YES) {
            ProcessImage( event );
          }
        }

        if (! wxOriginalActive) {
          SetProgramStatus( wxString( "Ready to load image." ) );
        }
        else if (wxOriginalActive && flt.conversion_factor == -1.0) {
          SetProgramStatus( wxString( "Ready to set conversion factor." ) );
        }
        else if (wxOriginalActive && flt.conversion_factor != -1.0) {
          SetProgramStatus( wxString( "Ready to process image." ) );
        }
}

void AFMExplorer::SetConversionFactor( wxCommandEvent& event )
{
        double priorConversionFactor = flt.conversion_factor;
        int longestDim = 1;
        if (wxOriginal->GetWidth() > wxOriginal->GetHeight()) {
          longestDim = wxOriginal->GetWidth();
        }
        else {
          longestDim = wxOriginal->GetHeight();
        }
        wxString msg;
        msg.Printf( wxT( "The longest dimension (%d pixels) of the current image represents..." ), longestDim );
        wxNumberEntryDialog ConversionDialog(
            this,
            msg,
            wxT( "[100,10000] nm:" ),
            wxT( "Pixel to Metric Conversion" ),
            conversionNanometers,
            100,
            10000
        );
        if (ConversionDialog.ShowModal() == wxID_OK) {
          conversionNanometers = ConversionDialog.GetValue();
          flt.conversion_factor = (double)conversionNanometers / (double)longestDim;
          menuItem3->Enable( true );
          SetImageProperties();
          SetProgramStatus( wxString( "Ready to process image." ) );
        }
        
        if (wxFilteredActive && priorConversionFactor != flt.conversion_factor) {
          wxMessageDialog ReprocessDialog(
              this,
              wxT( "Displayed component lengths do not reflect the current conversion factor.\nWould you like to reprocess the image using the new value?" ),
              wxT( "Reprocess image?" ),
              wxYES_NO
          );
          if (ReprocessDialog.ShowModal() == wxID_YES) {
            ProcessImage( event );
          }
        }
}

void AFMExplorer::ProcessImage( wxCommandEvent& event )
{
	if (wxOriginalActive){
                wxOriginalActive = true;
                if (wxBackboneActive) {
                  wxBackboneActive = false;
                  scroll_win_backbone->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( AFMExplorerGui::DrawImage3 ), NULL, this );
                  scroll_win_backbone = NULL;
                  m_notebook1->DeletePage( 2 );
                }
                if (wxFilteredActive) {
                  wxFilteredActive = false;
                  scroll_win_filtered->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( AFMExplorerGui::DrawImage2 ), NULL, this );
                  scroll_win_filtered = NULL;
                  m_notebook1->DeletePage( 1 );
                }

                SetProgramStatus( wxString( "Applying filters to image..." ) );
		flt.ApplyFilters(
                    threshold_method,
                    fixed_thresh_too_bright_elim,
                    fixed_thresh_too_dim_elim,
                    adaptive_thresh_too_dim_elim,
                    adaptive_thresh_method,
                    adaptive_thresh_box_dim
                );
		IplImage *cvFilteredCol = cvCreateImage( cvGetSize( flt.inputImage ), IPL_DEPTH_8U, 3 );
		cvCvtColor( flt.filteredImage, cvFilteredCol, CV_GRAY2RGB );
		unsigned char *data;
		int step;
		CvSize size;
		cvGetRawData( cvFilteredCol, &data, &step, &size );

                SetProgramStatus( wxString( "Creating filtered image..." ) );
		wxFiltered = new wxImage(size.width,size.height,data,true);
		wxFilteredZoom = new wxImage();
		*wxFilteredZoom = wxFiltered->Scale(wxFiltered->GetWidth()*zoom,wxFiltered->GetHeight()*zoom);
                wxFilteredActive = true;

                scroll_win_filtered = new WindowScroll( m_notebook1 );
                scroll_win_filtered->SetScrollRate( 5, 5 );
                m_notebook1->AddPage( scroll_win_filtered, wxT("Filtered Image"), false );
                scroll_win_filtered->Connect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( AFMExplorerGui::DrawImage2 ), NULL, this );

		scroll_win_filtered->SetScrollbars(1,1,wxFilteredZoom->GetWidth(),wxFilteredZoom->GetHeight());
	
                SetProgramStatus( wxString( "Finding backbones in image..." ) );
		flt.FindBackbones();
		IplImage *cvBackboneCol = cvCreateImage( cvGetSize( flt.inputImage ), IPL_DEPTH_8U, 3 );
		cvCvtColor( flt.backboneImage, cvBackboneCol, CV_GRAY2RGB );
		cvGetRawData( cvBackboneCol, &data, &step, &size );

                SetProgramStatus( wxString( "Creating backbone image..." ) );
		wxBackbone = new wxImage(size.width,size.height,data,true);
		wxBackboneZoom = new wxImage();
		*wxBackboneZoom = wxBackbone->Scale(wxBackbone->GetWidth()*zoom,wxBackbone->GetHeight()*zoom);
                wxBackboneActive = true;

                scroll_win_backbone = new WindowScroll( m_notebook1 );
                scroll_win_backbone->SetScrollRate( 5, 5 );
                m_notebook1->AddPage( scroll_win_backbone, wxT("Backbone Image"), false );
                scroll_win_backbone->Connect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( AFMExplorerGui::DrawImage3 ), NULL, this );

		scroll_win_backbone->SetScrollbars(1,1,wxBackboneZoom->GetWidth(),wxBackboneZoom->GetHeight());
	
                SetProgramStatus( wxString( "Outputting connected components..." ) );

                m_listCtrl1->DeleteAllItems();

                for (int
                     i = 0;
                     i < flt.backbones.size();
                     i++) {

                    wxString s;

                    s.Printf( "%d", i + 1 );
                    m_listCtrl1->InsertItem( i, s );

                    s.Printf( "%.2f", flt.backbones[i].length * flt.conversion_factor );

                    m_listCtrl1->SetItem( i, 1, s );

                    sortStruct *itemData = new sortStruct;
                    itemData->id = i;
                    itemData->length = flt.backbones[i].length * flt.conversion_factor;
                    m_listCtrl1->SetItemPtrData( i, (wxUIntPtr)itemData );
		}
	
                menuItem4->Enable( true );
                menuItem5->Enable( true );
                menuItemSaveComponents->Enable( true );

                SetProgramStatus( wxString( "Ready." ) );
	}
}

void AFMExplorer::SetFit( wxCommandEvent& event )
{
	if (wxOriginalZoom) {
          SetFitOriginal();
          scroll_win_original->SetScrollbars( zoom, zoom, wxOriginal->GetWidth(), wxOriginal->GetHeight() );
          scroll_win_original->Refresh();
	}
	
	if (wxFilteredZoom) {
          SetFitFiltered();
          scroll_win_filtered->SetScrollbars( zoom, zoom, wxFiltered->GetWidth(), wxFiltered->GetHeight() );
          scroll_win_filtered->Refresh();
	}
	
	if (wxBackboneZoom) {
          SetFitBackbone();
          scroll_win_backbone->SetScrollbars( zoom, zoom, wxBackbone->GetWidth(), wxBackbone->GetHeight() );
          scroll_win_backbone->Refresh();
	}

	m_menu4->Check(1,true);
	for (int i=2;i<=6;i++)
		m_menu4->Check(i,false);
}

void AFMExplorer::SetNoZoom( wxCommandEvent& event )
{
	if (wxOriginalZoom)	{
		zoom=1;
		*wxOriginalZoom=wxOriginal->Scale(wxOriginal->GetWidth()*zoom,wxOriginal->GetHeight()*zoom);
		scroll_win_original->SetScrollbars(zoom, zoom,wxOriginal->GetWidth(),wxOriginal->GetHeight());
                scroll_win_original->Refresh();
	}
	
	if (wxFilteredZoom)	{
		zoom=1;
		*wxFilteredZoom=wxFiltered->Scale(wxFiltered->GetWidth()*zoom,wxFiltered->GetHeight()*zoom);
		scroll_win_filtered->SetScrollbars(zoom, zoom,wxFiltered->GetWidth(),wxFiltered->GetHeight());
                scroll_win_filtered->Refresh();
	}
	
	if (wxBackboneZoom)	{
		zoom=1;
		*wxBackboneZoom=wxBackbone->Scale(wxBackbone->GetWidth()*zoom,wxBackbone->GetHeight()*zoom);
		scroll_win_backbone->SetScrollbars(zoom, zoom,wxBackbone->GetWidth(),wxBackbone->GetHeight());
                scroll_win_backbone->Refresh();
	}

        m_menu4->Check(1,false);
	m_menu4->Check(2,true);
	for (int i=3;i<=6;i++)
		m_menu4->Check(i,false);
}

void AFMExplorer::Zoom(int zoom_val)
{
	if (wxOriginalZoom)	{
		*wxOriginalZoom=wxOriginal->Scale(wxOriginal->GetWidth()*zoom_val,wxOriginal->GetHeight()*zoom_val);
		scroll_win_original->SetScrollbars(1, 1,wxOriginalZoom->GetWidth(),wxOriginalZoom->GetHeight());
                scroll_win_original->Refresh();
	}
	
	if (wxFilteredZoom){
		*wxFilteredZoom=wxFiltered->Scale(wxFiltered->GetWidth()*zoom_val,wxFiltered->GetHeight()*zoom_val);
		scroll_win_filtered->SetScrollbars(1, 1,wxFilteredZoom->GetWidth(),wxFilteredZoom->GetHeight());
                scroll_win_filtered->Refresh();
	}
	
	if (wxBackboneZoom){
		*wxBackboneZoom=wxBackbone->Scale(wxBackbone->GetWidth()*zoom_val,wxBackbone->GetHeight()*zoom_val);
		scroll_win_backbone->SetScrollbars(1, 1,wxBackboneZoom->GetWidth(),wxBackboneZoom->GetHeight());
                scroll_win_backbone->Refresh();
	}

        m_menu4->Check(1,false);
        m_menu4->Check(2,false);
	for (int i=3;i<=6;i++)
		if (i-1==zoom_val) m_menu4->Check(i,true);
		else	           m_menu4->Check(i,false);
	
}

void AFMExplorer::SetZoom2x( wxCommandEvent& event )
{
	zoom=2;
	Zoom(zoom);
}

void AFMExplorer::SetZoom3x( wxCommandEvent& event )
{
	zoom=3;
	Zoom(zoom);
}

void AFMExplorer::SetZoom4x( wxCommandEvent& event )
{
	zoom=4;
	Zoom(zoom);
}

void AFMExplorer::SetZoom5x( wxCommandEvent& event )
{
	zoom=5;
	Zoom(zoom);
}

void AFMExplorer::PerformResize( wxSizeEvent& event )
{
        if (m_menu4->IsChecked(1)) {
          SetFit( (wxCommandEvent&)event );
        }
	this->Layout();
}


void AFMExplorer::SetImageProperties()
{
	wxArrayString strings;
	wxString str;
	if (wxOriginalActive) {
		// TODO: fix dependency (Windows uses '\\', Unix uses '/').
		wxString justFileName = imageFileName.AfterLast( wxChar( '/' ) );
		str.Printf( "Filename = " + justFileName );
		strings.Add( wxT( str ) );
		str.Printf( "Width = %d", imageWidth );
		strings.Add( wxT( str ) );
		str.Printf( "Height = %d", imageHeight );
		strings.Add( wxT( str ) );
		if (flt.conversion_factor == -1.0) {
			str.Printf( "No conversion factor set." );
		}
		else {
			str.Printf( "Conversion factor = %.2f nm/pixel", flt.conversion_factor );
		}
		strings.Add( wxT( str ) );
		m_listBox2->Set( strings, NULL );
	}
	else {
		str.Printf( "No image loaded." );
		strings.Add( wxT( str ) );
		m_listBox2->Set( strings, NULL );
	}
	strings.Clear();
}

void AFMExplorer::SetImageProcessingProperties()
{
		wxArrayString strings;
		wxString str;
		wxString t_meth;
        
        if (threshold_method == 0) {
          t_meth = "fixed";
          
          str.Printf( "threshold method = " + t_meth );
          strings.Add( wxT( str ) );
          
          str.Printf( "fixed thresh too bright elim = %d", fixed_thresh_too_bright_elim );
          strings.Add( wxT( str ) );
          
          str.Printf( "fixed thresh too dim elim = %d", fixed_thresh_too_dim_elim );
          strings.Add( wxT( str ) );
        }
        if (threshold_method == 1) {
          t_meth = "adaptive";
          
          str.Printf( "threshold method = " + t_meth );
          strings.Add( wxT( str ) );
          
          str.Printf( "adaptive thresh too dim elim = %d", adaptive_thresh_too_dim_elim );
          strings.Add( wxT( str ) );
          
          wxString at_meth;
          if (adaptive_thresh_method == 0) {
            at_meth = "mean";
            str.Printf( "adaptive thresh method = " + at_meth );
          }
          if (adaptive_thresh_method == 1) {
            at_meth = "gaussian";
            str.Printf( "adaptive thresh method = " + at_meth );
          }
          strings.Add( wxT( str ) );
          
          str.Printf( "adaptive thresh box dim = %d", adaptive_thresh_box_dim );
          strings.Add( wxT( str ) );
        }
        
        m_listBox3->Set( strings, NULL );

        strings.Clear();
}

void AFMExplorer::SetProgramStatus( const wxString& str )
{
        m_statusBar1->SetStatusText( wxT( str ), 0 );

        //wxArrayString strings;
        //strings.Add( wxT( str ) );
        //m_listBox3->Set( strings, NULL );
        //strings.Clear();
}

void AFMExplorer::SelectItem( int idx )
{
	unsigned char *data_orig, *data_filt, *data_back;

        if (selectedItemIndex != -1) {
          m_listCtrl1->SetItemState( selectedItemIndex, 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED );
        }
        bool found = false;
        for (int i = 0; i < m_listCtrl1->GetItemCount(); i++) {
          sortStruct *itemData = (sortStruct *)m_listCtrl1->GetItemData( i );
          if (idx == itemData->id) {
            selectedItemIndex = i;
            found = true;
            break;
          }
        }
        if (! found) {
          SetProgramStatus( wxString( "Could not determine the list control item index from selected image component index value." ) );
        }
	m_listCtrl1->SetItemState( selectedItemIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
        m_listCtrl1->EnsureVisible( selectedItemIndex );
	
	data_orig = wxOriginal->GetData();
	data_filt = wxFiltered->GetData();
	data_back = wxBackbone->GetData();

	flt.HighlightBackboneId( data_orig, idx, true );
	flt.HighlightBackboneId( data_filt, idx, false );
	flt.HighlightBackboneId( data_back, idx, false );

	*wxOriginalZoom = wxOriginal->Scale( wxOriginal->GetWidth() * zoom, wxOriginal->GetHeight() * zoom );
	*wxFilteredZoom = wxFiltered->Scale( wxFiltered->GetWidth() * zoom, wxFiltered->GetHeight() * zoom );
	*wxBackboneZoom = wxBackbone->Scale( wxBackbone->GetWidth() * zoom, wxBackbone->GetHeight() * zoom );
}

void AFMExplorer::SetStatusValue( wxUpdateUIEvent& event )
{
	wxString s;
	
	if (wxOriginalZoom && m_notebook1->GetCurrentPage()==scroll_win_original && scroll_win_original->xm!=0 && scroll_win_original->ym!=0)
		if ((scroll_win_original->xm-scroll_win_original->x)/zoom<=wxOriginal->GetWidth() && (scroll_win_original->ym-scroll_win_original->y)/zoom <= wxOriginal->GetHeight())	{
                        //s.Printf("X= %d  , Y= %d",(scroll_win_original->xm-scroll_win_original->x)/zoom,(scroll_win_original->ym-scroll_win_original->y)/zoom);
			//m_statusBar1->SetStatusText(wxT(s), 0);
			if (scroll_win_original->xc!=0 && scroll_win_original->yc!=0 && m_listCtrl1->GetItemCount()>0)	{
				int idx = flt.NearBackboneId(
                                    scroll_win_original->xc / zoom - scroll_win_original->x,
                                    scroll_win_original->yc / zoom - scroll_win_original->y
                                );
				if (idx > -1)	{
					SelectItem(idx);
				}
			}
		}
	
	if (wxFilteredZoom && m_notebook1->GetCurrentPage()==scroll_win_filtered && scroll_win_filtered->xm!=0 && scroll_win_filtered->ym!=0)
		if ((scroll_win_filtered->xm-scroll_win_filtered->x)/zoom <=wxFiltered->GetWidth() && (scroll_win_filtered->ym-scroll_win_filtered->y)/zoom <= wxFiltered->GetHeight())	{
                        //s.Printf("X= %d  , Y= %d",(scroll_win_filtered->xm-scroll_win_filtered->x)/zoom,(scroll_win_filtered->ym-scroll_win_filtered->y)/zoom);
			//m_statusBar1->SetStatusText(wxT(s), 0);
			if (scroll_win_filtered->xc!=0 && scroll_win_filtered->yc!=0)	{
				int idx = flt.NearBackboneId(
                                    scroll_win_filtered->xc / zoom - scroll_win_filtered->x,
                                    scroll_win_filtered->yc / zoom - scroll_win_filtered->y
                                );
				if (idx > -1)	{
					SelectItem(idx);
				}
			}
		}
	
	if (wxBackboneZoom && m_notebook1->GetCurrentPage()==scroll_win_backbone && scroll_win_backbone->xm!=0 && scroll_win_backbone->ym!=0)
		if ((scroll_win_backbone->xm-scroll_win_backbone->x)/zoom <=wxBackbone->GetWidth() && (scroll_win_backbone->ym-scroll_win_backbone->y)/zoom <= wxBackbone->GetHeight())	{
                        //s.Printf("X= %d  , Y= %d",(scroll_win_backbone->xm-scroll_win_backbone->x)/zoom,(scroll_win_backbone->ym-scroll_win_backbone->y)/zoom);
			//m_statusBar1->SetStatusText(wxT(s), 0);
			if (scroll_win_backbone->xc!=0 && scroll_win_backbone->yc!=0)	{
				int idx =flt.NearBackboneId(
                                    scroll_win_backbone->xc / zoom - scroll_win_backbone->x,
                                    scroll_win_backbone->yc / zoom - scroll_win_backbone->y
                                );
				if (idx > -1)	{
					SelectItem(idx);
				}
			}
		}
}

void AFMExplorer::RecentSelected(wxCommandEvent& event )
{
	SetProgramStatus( wxString( "Loading image..." ) );
	// TODO: fix dependency (Windows uses '\\', Unix uses '/').
	defaultPath = m_menu21->GetLabel( event.GetId() ).Remove( 0, 3 ).BeforeLast( wxChar( '/' ) ) + wxChar( '/' );

        flt.LoadImage( (char *)(m_menu21->GetLabel(event.GetId())).Remove(0,3).c_str() );
		
	if (wxOriginalActive)	{
		wxOriginal->Destroy();
		wxOriginalZoom->Destroy();
		wxOriginalActive = false;

                scroll_win_original->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( AFMExplorerGui::DrawImage1 ), NULL, this );
                scroll_win_original = NULL;

		if (wxFilteredActive)	{
			wxFiltered->Destroy();
			wxFilteredZoom->Destroy();
                        wxFilteredActive = false;

                        scroll_win_filtered->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( AFMExplorerGui::DrawImage2 ), NULL, this );
                        scroll_win_filtered = NULL;
		}
		if (wxBackboneActive)	{
			wxBackbone->Destroy();
			wxBackboneZoom->Destroy();
                        wxBackboneActive = false;

                        scroll_win_backbone->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( AFMExplorerGui::DrawImage3 ), NULL, this );
                        scroll_win_backbone = NULL;
		}

                m_notebook1->DeleteAllPages();
	}
	
	wxOriginal = new wxImage();
	wxOriginal->LoadFile( m_menu21->GetLabel( event.GetId() ).Remove( 0, 3 ), wxBITMAP_TYPE_BMP, -1 );
	wxOriginalZoom = new wxImage(*wxOriginal);
        wxOriginalActive = true;

        //
	scroll_win_original = new WindowScroll( m_notebook1 );
	scroll_win_original->SetScrollRate( 5, 5 );
	m_notebook1->AddPage( scroll_win_original, wxT("Original Image"), false );
        scroll_win_original->Connect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( AFMExplorerGui::DrawImage1 ), NULL, this );
        
	scroll_win_original->SetScrollbars(1,1,wxOriginal->GetWidth(),wxOriginal->GetHeight());
        scroll_win_original->Refresh();
	for (int i=1;i<=6;i++)
          m_menu4->Enable(i,true);

        m_listCtrl1->DeleteAllItems();

        wxString s("");
        m_listBox2->Set(1,&s,NULL);

        menuItem11->Enable( true );
        menuItem4->Enable( false );
        menuItem5->Enable( false );
        menuItemSaveComponents->Enable( false );

        imageFileName = m_menu21->GetLabel( event.GetId() ).Remove( 0, 3 );
        imageWidth = wxOriginal->GetWidth();
        imageHeight = wxOriginal->GetHeight();
        SetImageProperties();

        if (flt.conversion_factor == -1.0) {
          SetProgramStatus( wxString( "Ready to set conversion factor." ) );
        }
        else {
          SetProgramStatus( wxString( "Ready to process image." ) );
        }

        idSortAscending = false;
        lengthSortAscending = true;
        selectedItemIndex = -1;
        selectedItemId = -1;
}

void AFMExplorer::HighlightImage( wxListEvent& event )
{
        selectedItemIndex = event.GetIndex();
        sortStruct *itemData = (sortStruct *)m_listCtrl1->GetItemData( selectedItemIndex );
        selectedItemId = itemData->id;

	unsigned char *data_orig,*data_filt,*data_back;
	if (wxOriginalZoom)	{
		data_orig = wxOriginal->GetData();
		flt.HighlightBackboneId( data_orig, selectedItemId, true);
		*wxOriginalZoom = wxOriginal->Scale( wxOriginal->GetWidth() * zoom, wxOriginal->GetHeight() * zoom );
	}
	
	if (wxFilteredZoom)	{
		data_filt = wxFiltered->GetData();
		flt.HighlightBackboneId( data_filt, selectedItemId, false );
		*wxFilteredZoom = wxFiltered->Scale( wxFiltered->GetWidth() * zoom, wxFiltered->GetHeight() * zoom );
	}
	
	if (wxBackboneZoom)	{
		data_back = wxBackbone->GetData();
		flt.HighlightBackboneId( data_back, selectedItemId, false );
		*wxBackboneZoom = wxBackbone->Scale( wxBackbone->GetWidth() * zoom, wxBackbone->GetHeight() * zoom );
	}	
}

void AFMExplorer::SortComponents( wxListEvent& event )
{
  m_listCtrl1->SortItems( CompareItems, (long)event.GetColumn() );
  if (event.GetColumn() == 0) {
    if (idSortAscending) {
      idSortAscending = false;
    }
    else {
      idSortAscending = true;
    }
    lengthSortAscending = true;
  }
  if (event.GetColumn() == 1) {
    if (lengthSortAscending) {
      lengthSortAscending = false;
    }
    else {
      lengthSortAscending = true;
    }
    idSortAscending = true;
  }
  if (selectedItemIndex != -1) {
    m_listCtrl1->SetItemState( selectedItemIndex, 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED );
    bool found = false;
    for (int i = 0; i < m_listCtrl1->GetItemCount(); i++) {
      sortStruct *itemData = (sortStruct *)m_listCtrl1->GetItemData( i );
      if (selectedItemId == itemData->id) {
        selectedItemIndex = i;
        found = true;
        break;
      }
    }
    if (! found) {
      SetProgramStatus( wxString( "Could not determine the list control item index from selected image component index value." ) );
    }
    m_listCtrl1->SetItemState( selectedItemIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    m_listCtrl1->EnsureVisible( selectedItemIndex );
  }
}

int wxCALLBACK CompareItems( long item1, long item2, long sortData )
{
  sortStruct *a = (sortStruct *)item1;
  sortStruct *b = (sortStruct *)item2; 
  
  if (sortData == 0) {
    if (a->id == b->id)  return  0;
    if (idSortAscending) {
      if (a->id < b->id) return -1;
      if (a->id > b->id) return  1;
    }
    else {
      if (a->id < b->id) return  1;
      if (a->id > b->id) return -1;
    }
  }
  if (sortData == 1) {
    if (a->length == b->length)  return  0;
    if (lengthSortAscending) {
      if (a->length < b->length) return -1;
      if (a->length > b->length) return  1;
    }
    else {
      if (a->length < b->length) return  1;
      if (a->length > b->length) return -1;
    }
  }
}

void AFMExplorer::SaveFiltered( wxCommandEvent& event )
{
    bool readyToSave = false;
    wxString path;
    while (! readyToSave) {
        wxFileDialog dialog(
            this,
            wxT( "Save filtered image as..." ),
            wxT( defaultPath ),
            wxEmptyString,
            wxT( "BMP files (*.bmp)|*.bmp|GIF files (*.gif)|*.gif" ),
            wxFD_SAVE
        );
        if (dialog.ShowModal() == wxID_OK) {
            path = dialog.GetPath();
            if (! path.Contains( wxString( ".bmp" ) ) &&
                ! path.Contains( wxString( ".gif" ) )) {
                wxString msg = wxT( "You must end your filename with a proper file extension: '.bmp' or '.gif'." );
                wxMessageBox( msg, wxT( "File name error" ), wxOK, this );
            }
            else {
                readyToSave = true;
            }
        }
    }
    SetProgramStatus( wxString( "Saving filtered image..." ) );
    cvSaveImage( path, flt.filteredImage );
    SetProgramStatus( wxString( "Ready." ) );
}

void AFMExplorer::SaveBackbones( wxCommandEvent& event )
{
    bool readyToSave = false;
    wxString path;
    while (! readyToSave) {
        wxFileDialog dialog(
            this,
            wxT( "Save backbones image as..." ),
            wxT( defaultPath ),
            wxEmptyString,
            wxT( "BMP files (*.bmp)|*.bmp|GIF files (*.gif)|*.gif" ),
            wxFD_SAVE
        );
        if (dialog.ShowModal() == wxID_OK) {
            path = dialog.GetPath();
            if (! path.Contains( wxString( ".bmp" ) ) &&
                ! path.Contains( wxString( ".gif" ) )) {
                wxString msg = wxT( "You must end your filename with a proper file extension: '.bmp' or '.gif'." );
                wxMessageBox( msg, wxT( "File name error" ), wxOK, this );
            }
            else {
                readyToSave = true;
            }
        }
    }
    SetProgramStatus( wxString( "Saving backbones image..." ) );
    cvSaveImage( path, flt.backboneImage );
    SetProgramStatus( wxString( "Ready." ) );
}

void AFMExplorer::SetFitOriginal()
{
        int image_width  = wxOriginal->GetWidth();
        int image_height = wxOriginal->GetHeight();
        double image_aspect_ratio = (double)image_width / (double)image_height;

        int *client_width = new int;
        int *client_height = new int;
        scroll_win_original->GetClientSize( client_width, client_height );
        double client_aspect_ratio = (double)*client_width / (double)*client_height;

        if      (client_aspect_ratio > image_aspect_ratio) {
          *wxOriginalZoom = wxOriginal->Scale(
            (int)((double)image_width * ((double)*client_height / (double)image_height)),
            *client_height
          );
        }
        else if (client_aspect_ratio < image_aspect_ratio) {
          *wxOriginalZoom = wxOriginal->Scale(
            *client_width,
            (int)((double)image_height * ((double)*client_width / (double)image_width))
          );
        }
        else {
          *wxOriginalZoom = wxOriginal->Scale( *client_width, *client_height );
        }
}

void AFMExplorer::SetFitFiltered()
{
        int image_width  = wxFiltered->GetWidth();
        int image_height = wxFiltered->GetHeight();
        double image_aspect_ratio = (double)image_width / (double)image_height;

        int *client_width = new int;
        int *client_height = new int;
        scroll_win_filtered->GetClientSize( client_width, client_height );
        double client_aspect_ratio = (double)*client_width / (double)*client_height;

        if      (client_aspect_ratio > image_aspect_ratio) {
          *wxFilteredZoom = wxFiltered->Scale(
            (int)((double)image_width * ((double)*client_height / (double)image_height)),
            *client_height
          );
        }
        else if (client_aspect_ratio < image_aspect_ratio) {
          *wxFilteredZoom = wxFiltered->Scale(
            *client_width,
            (int)((double)image_height * ((double)*client_width / (double)image_width))
          );
        }
        else {
          *wxFilteredZoom = wxFiltered->Scale( *client_width, *client_height );
        }
}

void AFMExplorer::SetFitBackbone()
{
        int image_width  = wxBackbone->GetWidth();
        int image_height = wxBackbone->GetHeight();
        double image_aspect_ratio = (double)image_width / (double)image_height;

        int *client_width = new int;
        int *client_height = new int;
        scroll_win_backbone->GetClientSize( client_width, client_height );
        double client_aspect_ratio = (double)*client_width / (double)*client_height;

        if      (client_aspect_ratio > image_aspect_ratio) {
          *wxBackboneZoom = wxBackbone->Scale(
            (int)((double)image_width * ((double)*client_height / (double)image_height)),
            *client_height
          );
        }
        else if (client_aspect_ratio < image_aspect_ratio) {
          *wxBackboneZoom = wxBackbone->Scale(
            *client_width,
            (int)((double)image_height * ((double)*client_width / (double)image_width))
          );
        }
        else {
          *wxBackboneZoom = wxBackbone->Scale( *client_width, *client_height );
        }
}

void AFMExplorer::About( wxCommandEvent& event )
{
        // TODO: Ugly and static -- at least use a wxStringArray and #defines
        wxString msg = wxT( "AFM Explorer\n\nVersion: 0.22\nRelease: 24-Mar-2008\n\nAuthors:\n\nAndrew Sundstrom, Research Scientist, Courant Institute of Mathematical Sciences, NYU Bioinformatics Group\nSilvio Cirrone, University of Catania, Sicily\n" );
        wxMessageBox( msg, wxT( "About AFM Explorer" ), wxOK, this );
}

void AFMExplorer::SaveComponents( wxCommandEvent& event )
{
	wxFileDialog dialog(
		this,
		wxT( "Save connected components as..." ),
		wxT( defaultPath ),
		wxEmptyString,
		wxT( "*.*" ),
		wxFD_SAVE
	);
	if (dialog.ShowModal() == wxID_OK) {
		SetProgramStatus( wxString( "Saving connected components..." ) );
		wxString path = dialog.GetPath();
		ofstream componentStream( (char *)path.char_str() );
		if (! componentStream) {
			SetProgramStatus( wxString( "Error writing output to '" + path + "'." ) );
			return;
		}
		for (int i = 0; i < m_listCtrl1->GetItemCount(); i++) {
			sortStruct *itemData = (sortStruct *)m_listCtrl1->GetItemData( i );
			char *data = (char *)malloc(20 * sizeof(char));
			sprintf( data, "%d\t%.2f", itemData->id + 1, (double)itemData->length );
			componentStream << data << endl;;
		}
		componentStream.close();
	}
	SetProgramStatus( wxString( "Ready." ) );
}
