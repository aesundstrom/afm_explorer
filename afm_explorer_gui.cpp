#include "wx/wx.h"
#include "wx/filedlg.h"
#include "afm_explorer_gui.h"
#include "image_processing_control.h"

AFMExplorerGui::AFMExplorerGui( wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style )
              : wxFrame( parent, id, title, pos, size, style )
{
	imageFileName = wxString();
	imageWidth = 0;
	imageHeight = 0;
	conversionNanometers = 2000;
	conversionFactor = -1.0;

	defaultPath = wxString( "./" );

	selectedItemIndex = -1;
	selectedItemId = -1;

	threshold_method = __THRESHOLD_METHOD__;
	fixed_thresh_too_bright_elim = __FIXED_THRESH_TOO_BRIGHT_ELIM__;
	fixed_thresh_too_dim_elim = __FIXED_THRESH_TOO_DIM_ELIM__;
	adaptive_thresh_too_dim_elim = __ADAPTIVE_THRESH_TOO_DIM_ELIM__;
	adaptive_thresh_method = __ADAPTIVE_THRESH_METHOD__;
	adaptive_thresh_box_dim = __ADAPTIVE_THRESH_BOX_DIM__;

	this->SetSizeHints( wxSize( 600,400 ), wxDefaultSize );
	this->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNTEXT ) );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );
	
	m_menubar2 = new wxMenuBar( 0 );
	//m_menubar2->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BACKGROUND ) );
	
	file = new wxMenu();
	wxMenuItem* menuItem1 = new wxMenuItem( file, wxID_ANY, wxString( wxT("Open...") ) , wxEmptyString, wxITEM_NORMAL );
	file->Append( menuItem1 );

	menuItem4 = new wxMenuItem( file, wxID_ANY, wxString( wxT("Save Filtered Image...") ) , wxEmptyString, wxITEM_NORMAL );
	file->Append( menuItem4 );
	menuItem4->Enable( false );

	menuItem5 = new wxMenuItem( file, wxID_ANY, wxString( wxT("Save Backbones Image...") ) , wxEmptyString, wxITEM_NORMAL );
	file->Append( menuItem5 );
	menuItem5->Enable( false );

	menuItemSaveComponents = new wxMenuItem( file, wxID_ANY, wxString( wxT("Save Connected Components...") ) , wxEmptyString, wxITEM_NORMAL );
	file->Append( menuItemSaveComponents );
	menuItemSaveComponents->Enable( false );

	m_menu21 = new wxMenu();
	file->Append( -1, wxT("Most Recently Opened"), m_menu21 );
	
	file->AppendSeparator();
	wxMenuItem* menuItem51 = new wxMenuItem( file, wxID_EXIT, wxString( wxT("Exit") ) , wxEmptyString, wxITEM_NORMAL );
	file->Append( menuItem51 );
	m_menubar2->Append( file, wxT("File") );
	
	m_menu2 = new wxMenu();
	menuItemSetImageProcessingParameters = new wxMenuItem( m_menu2, wxID_ANY, wxString( wxT("Set Image Processing Parameters...") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu2->Append( menuItemSetImageProcessingParameters );
	menuItem11 = new wxMenuItem( m_menu2, wxID_ANY, wxString( wxT("Set Conversion Factor...") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu2->Append( menuItem11 );
	menuItem11->Enable( false );
	menuItem3 = new wxMenuItem( m_menu2, wxID_ANY, wxString( wxT("Process Image") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu2->Append( menuItem3 );
	menuItem3->Enable( false );

	m_menubar2->Append( m_menu2, wxT("Run") );
	
	m_menu4 = new wxMenu();
        
	wxMenuItem* menuItemFit = new wxMenuItem( m_menu4, 1, wxString( wxT("Fit to Window") ) , wxEmptyString, wxITEM_CHECK );
	m_menu4->Append( menuItemFit );
	menuItemFit->Check( true );

	m_menu4->AppendSeparator();

	wxMenuItem* menuItem6 = new wxMenuItem( m_menu4, 2, wxString( wxT("Actual Size") ) , wxEmptyString, wxITEM_CHECK );
	m_menu4->Append( menuItem6 );
	
	m_menu4->AppendSeparator();

	wxMenuItem* menuItem7 = new wxMenuItem( m_menu4, 3, wxString( wxT("Zoom 2x") ) , wxEmptyString, wxITEM_CHECK );
	m_menu4->Append( menuItem7 );

	wxMenuItem* menuItem8 = new wxMenuItem( m_menu4, 4, wxString( wxT("Zoom 3x") ) , wxEmptyString, wxITEM_CHECK );
	m_menu4->Append( menuItem8 );

	wxMenuItem* menuItem9 = new wxMenuItem( m_menu4, 5, wxString( wxT("Zoom 4x") ) , wxEmptyString, wxITEM_CHECK );
	m_menu4->Append( menuItem9 );

	wxMenuItem* menuItem10 = new wxMenuItem( m_menu4, 6, wxString( wxT("Zoom 5x") ) , wxEmptyString, wxITEM_CHECK );
	m_menu4->Append( menuItem10 );

	m_menubar2->Append( m_menu4, wxT("Zoom") );
	
	m_menu3 = new wxMenu();
	wxMenuItem* menuItem12 = new wxMenuItem( m_menu3, wxID_ABOUT, wxString( wxT("About...") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu3->Append( menuItem12 );
	m_menubar2->Append( m_menu3, wxT("Info") );
	
	this->SetMenuBar( m_menubar2 );
	
	wxFlexGridSizer* fgSizer14;
	fgSizer14 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer14->AddGrowableCol( 0 );
	fgSizer14->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer14->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	bSizer11 = new wxBoxSizer( wxVERTICAL );
	
	m_notebook1 = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_notebook1->SetMinSize( wxSize( 400,300 ) );

	bSizer11->Add( m_notebook1, 1, wxALL|wxEXPAND, 5 );

	fgSizer14->Add( bSizer11, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );

	// BOX 2 BEGIN
	m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("Image Properties"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	bSizer10->Add( m_staticText2, 0, wxALL, 5 );
	
	m_listBox2 = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( 300,-1 ), 0, NULL, 0 ); 
	m_listBox2->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNTEXT ) );
	m_listBox2->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );
	
	bSizer10->Add( m_listBox2, 1, wxALL|wxEXPAND, 5 );
    // BOX 2 END

	// BOX 3 BEGIN
	m_staticText3 = new wxStaticText( this, wxID_ANY, wxT("Image Processing Parameters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	bSizer10->Add( m_staticText3, 0, wxALL, 5 );
	
	m_listBox3 = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( 300,-1 ), 0, NULL, 0 ); 
	m_listBox3->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNTEXT ) );
	m_listBox3->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );
	
	bSizer10->Add( m_listBox3, 1, wxALL|wxEXPAND, 5 );	

	fgSizer14->Add( bSizer10, 0, wxEXPAND, 5 );
	// BOX 3 END
	
	// CTRL 1 BEGIN
	m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("Component Data"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	bSizer10->Add( m_staticText1, 0, wxALL, 5 );
	
	m_listCtrl1 = new wxListCtrl( this, 9990, wxDefaultPosition, wxSize( 300,-1 ), wxLC_REPORT ); 
	m_listCtrl1->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNTEXT ) );
	m_listCtrl1->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );

	wxListItem itemCol;
    itemCol.SetText( wxT( "ID" ) );
    m_listCtrl1->InsertColumn( 0, itemCol );
    m_listCtrl1->SetColumnWidth( 0, 40 );
    itemCol.SetText( wxT( "Length (nm)" ) );
    m_listCtrl1->InsertColumn( 1, itemCol );
    m_listCtrl1->SetColumnWidth( 1, 80 );
	
	bSizer10->Add( m_listCtrl1, 1, wxALL|wxEXPAND, 5 );
	// CTRL 1 END

	this->SetSizer( fgSizer14 );
	this->Layout();
	m_statusBar1 = this->CreateStatusBar( 1, wxST_SIZEGRIP, wxID_ANY );
	
	// Connect Events
	this->Connect( wxEVT_SIZE, wxSizeEventHandler( AFMExplorerGui::PerformResize ) );
	this->Connect( menuItem1->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AFMExplorerGui::OpenFile ) );
        this->Connect( menuItem4->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AFMExplorerGui::SaveFiltered ) );
        this->Connect( menuItem5->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AFMExplorerGui::SaveBackbones ) );
        this->Connect( menuItemSaveComponents->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AFMExplorerGui::SaveComponents ) );
	this->Connect( menuItem51->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AFMExplorerGui::Exit ) );
	this->Connect( menuItem3->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AFMExplorerGui::ProcessImage ) );
	this->Connect( menuItemSetImageProcessingParameters->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AFMExplorerGui::SetImageProcessingParameters ) );
	this->Connect( menuItem11->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AFMExplorerGui::SetConversionFactor ) );
	this->Connect( menuItemFit->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AFMExplorerGui::SetFit ) );
	this->Connect( menuItem6->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AFMExplorerGui::SetNoZoom ) );
	this->Connect( menuItem7->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AFMExplorerGui::SetZoom2x ) );
	this->Connect( menuItem8->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AFMExplorerGui::SetZoom3x ) );
	this->Connect( menuItem9->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AFMExplorerGui::SetZoom4x ) );
	this->Connect( menuItem10->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AFMExplorerGui::SetZoom5x ) );
        this->Connect( menuItem12->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AFMExplorerGui::About ) );
	m_statusBar1->Connect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( AFMExplorerGui::SetStatusValue ), NULL, this );
}

BEGIN_EVENT_TABLE( AFMExplorerGui, wxFrame )
	EVT_LIST_ITEM_SELECTED( 9990, AFMExplorerGui::HighlightImage )
	EVT_LIST_COL_CLICK( 9990, AFMExplorerGui::SortComponents )
END_EVENT_TABLE()

