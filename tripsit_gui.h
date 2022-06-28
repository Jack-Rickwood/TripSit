/* THis file contains declarations for the classes pertaining to the program's UI */
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

// Overarching GUI class
class TripSitGUI: public wxApp {
public:
    virtual bool OnInit();
};

// Main window class
class GUIFrame: public wxFrame {
public:
    // Constructor
    GUIFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

private:
    // Event handler functions
    void OnOS(wxCommandEvent& event);
    void OnDS(wxCommandEvent& event);
    void OnAddTrip(wxCommandEvent& event);
    void OnSelectTrip(wxListEvent& event);
    void OnRightClick(wxListEvent& event);
    void OnDeleteTrip(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void CreateLayout();
    wxDECLARE_EVENT_TABLE();

    // Variables that can be accessed by any member function
    // List objects
    wxArrayString station_list;
    wxListBox *origin_list;
    wxListBox *destination_list;
    wxListCtrl *trip_list;
    wxColourDatabase *colours;
    wxPanel *panel;
    // Search function
    wxArrayString RefineList(wxArrayString& station_list, string search_term);
    // Saving and loading function declarations
    void SaveTripList();
    void LoadTripList();
};

// Trip info window class
class TripDialog: public wxDialog {
public:
    // Constructor
    TripDialog(const wxString &title, const wxPoint &pos, const wxSize &size, const wxString &origin_name, const wxString &destination_name);
    // Helper function
    static string PadInt(string s, int size);

private:
    void OnViewTrip(wxListEvent& event);
    wxDECLARE_EVENT_TABLE();

    // Variables that can be accessed by any member function
    // List objects and trip info object
    wxListCtrl *journeys_list;
    wxTreeCtrl *trip_viewer;
    vector<TNSW::Journey> trip;
};