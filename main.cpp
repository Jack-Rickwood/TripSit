/* This file contains the main logic for my program, including all UI code. */
#include <string>
#include <chrono>
#include <ctime>
#include <iterator>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/listctrl.h>
#include <wx/statline.h>
#include <wx/generic/stattextg.h>
#include <wx/dataview.h>
#include <wx/treectrl.h>
#include "tripview_api.h"
#include "tripsit_gui.h"

// Define custom event IDs
enum
{
    ID_Hello = 1,
    ID_OS = 2,
    ID_DS = 3,
    ID_ADD_TRIP = 4,
    ID_SELECT_TRIP = 5,
    ID_VIEW_JOURNEY = 6,
    ID_REMOVE_TRIP = 7
};

// Link event IDs to their handler functions
wxBEGIN_EVENT_TABLE(GUIFrame, wxFrame)
    EVT_TEXT(ID_OS,                             GUIFrame::OnOS)
    EVT_TEXT(ID_DS,                             GUIFrame::OnDS)
    EVT_BUTTON(ID_ADD_TRIP,                     GUIFrame::OnAddTrip)
    EVT_LIST_ITEM_ACTIVATED(ID_SELECT_TRIP,     GUIFrame::OnSelectTrip)
    EVT_LIST_ITEM_RIGHT_CLICK(ID_SELECT_TRIP,   GUIFrame::OnRightClick)
    EVT_MENU(ID_REMOVE_TRIP,                    GUIFrame::OnDeleteTrip)
    EVT_MENU(wxID_EXIT,                         GUIFrame::OnExit)
    EVT_MENU(wxID_ABOUT,                        GUIFrame::OnAbout)
wxEND_EVENT_TABLE()
wxBEGIN_EVENT_TABLE(TripDialog, wxDialog)
    EVT_LIST_ITEM_SELECTED(ID_VIEW_JOURNEY,     TripDialog::OnViewTrip)
wxEND_EVENT_TABLE()

// Tell the GUI library the main GUI class
wxIMPLEMENT_APP(TripSitGUI);

// Main function
bool TripSitGUI::OnInit() {
    // Create the main window object
    GUIFrame *frame = new GUIFrame("TripSit", wxPoint(50, 50), wxSize(900, 680));
    // Show the UI
    frame->Show(true);

    return true;
}

// Constructor for the main window
// Takes the window title, position and size as arguments
GUIFrame::GUIFrame(const wxString &title, const wxPoint &pos, const wxSize &size) : wxFrame(NULL, wxID_ANY, title, pos, size,  wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
    // Create the 'File' dropdown
    wxMenu *menu_file = new wxMenu;
    menu_file->AppendSeparator();
    menu_file->Append(wxID_EXIT);

    // Create the 'Help' dropdown
    wxMenu *menu_help = new wxMenu;
    menu_help->Append(wxID_ABOUT);

    // Create the menu bar to hold these dropdowns
    wxMenuBar *menu_bar = new wxMenuBar;
    menu_bar->Append(menu_file, "&File");
    menu_bar->Append(menu_help, "&Help");
    SetMenuBar(menu_bar);

    // Create the status bar
    CreateStatusBar();
    SetStatusText("Tripsit");
    Centre();

    // Add UI elements
    CreateLayout();
    // Load the list of saved trips from the disk
    LoadTripList();
}

// Constructor for the main window
// Takes the window title, position, size, and origin / destination stations as arguments
TripDialog::TripDialog(const wxString &title, const wxPoint &pos, const wxSize &size, const wxString &origin_name, const wxString &destination_name) : wxDialog(NULL, wxID_ANY, title, pos, size,  wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
    wxPanel *panel = new wxPanel(this, wxID_ANY);
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxGridSizer *gs = new wxGridSizer(2, 2, 2, 2);
    Centre();

    // Create the text object to display the title
    wxGenericStaticText *title_text = new wxGenericStaticText(panel, wxID_ANY, origin_name + " to " + destination_name, wxPoint(50, 0), wxSize(250, 90), wxST_NO_AUTORESIZE | wxALIGN_CENTER_HORIZONTAL);
    title_text->SetLabelMarkup("<u><span size=\"xx-large\">" + origin_name + " to " + destination_name + "</span></u>");

    // Create the table for displaying the list of trips arriving soon
    journeys_list = new wxListCtrl(panel, ID_VIEW_JOURNEY, wxPoint(10, 120), wxSize(340, 580), wxLC_REPORT | wxLC_SINGLE_SEL);
    wxColourDatabase *colours = new wxColourDatabase();
    // Create the table's columns
    journeys_list->InsertColumn(0, "Line");
    journeys_list->SetColumnWidth(0, 85);
    journeys_list->InsertColumn(1, "Departure");
    journeys_list->SetColumnWidth(0, 85);
    journeys_list->InsertColumn(2, "Arrival");
    journeys_list->SetColumnWidth(0, 85);
    journeys_list->InsertColumn(3, "Legs");
    journeys_list->SetColumnWidth(1, 85);
    // Set the table's color
    journeys_list->SetBackgroundColour(colours->Find("LIGHT GREY"));

    // Create the tree object for displaying info about the trip
    trip_viewer = new wxTreeCtrl(panel, wxID_ANY, wxPoint(360, 10), wxSize(330, 730));
    // Set the object's background color
    trip_viewer->SetBackgroundColour(colours->Find("LIGHT BLUE"));

    // Get the current system time
    auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    // Call the function to contact the transportnsw api, and return info about the trips
    trip = TNSW::API::GetTripInfo(origin_name.ToStdString(), destination_name.ToStdString(), time);
    // Loop through each journey in the object
    for (auto &journey : trip) {
        // Read info about the trip to display in the table
        string line = journey.legs[0].line_name;
        string departure = PadInt(to_string(journey.legs[0].departure_time_estimated.tm_hour), 2) + ":" + PadInt(to_string(journey.legs[0].departure_time_estimated.tm_min), 2);
        string arrival = PadInt(to_string(journey.legs[0].arrival_time_estimated.tm_hour), 2) + ":" + PadInt(to_string(journey.legs[0].arrival_time_estimated.tm_min), 2);
        int legs = journey.legs.size();

        // Append the trip info to the table
        long index = journeys_list->InsertItem(journeys_list->GetItemCount(), line);
        journeys_list->SetItem(index, 1, departure);
        journeys_list->SetItem(index, 2, arrival);
        journeys_list->SetItem(index, 3, to_string(legs));
    }
}

// Pad an integer to by adding 0's
// Takes two arguments, a string and the desired size
string TripDialog::PadInt(string s, int size) {
    std::stringstream ss;
    ss << std::setw(size) << std::setfill('0') << s;
    return ss.str();
}

// Event handler for the 'About' button in the menu bar
void GUIFrame::OnExit(wxCommandEvent &event) {
    // Closes the application
    Close(true);
}

// Event handler for the 'About' button in the menu bar
void GUIFrame::OnAbout(wxCommandEvent &event) {
    // Displays a message box
    wxMessageBox("This is a wxWidgets Hello world sample", "About Hello World", wxOK | wxICON_INFORMATION);
}

// Event handler
// Handles the event when the user enters a search term into the origin input box
void GUIFrame::OnOS(wxCommandEvent &event) {
    // Get the refined list
    const wxArrayString refined_list = RefineList(station_list, event.GetString().ToStdString());
    unsigned int pos = 0;
    // Clear the list
    this->origin_list->Clear();
    if (!refined_list.IsEmpty())
        // Add the refined list to the list display
        this->origin_list->InsertItems(refined_list, pos);
}

// Event handler
// Handles the event when the user enters a search term into the destination input box
void GUIFrame::OnDS(wxCommandEvent &event) {
    // Get the refined list
    const wxArrayString refined_list = RefineList(station_list, event.GetString().ToStdString());
    unsigned int pos = 0;
    // Clear the list
    this->destination_list->Clear();
    if (!refined_list.IsEmpty())
        // Add the refined list to the list display
        this->destination_list->InsertItems(refined_list, pos);
}

// Event handler
// Handles the event when the user presses the add trip button
void GUIFrame::OnAddTrip(wxCommandEvent &event) {
    // Checks if the user has selected an origin and a destination
    if (origin_list->GetSelection() == wxNOT_FOUND || destination_list->GetSelection() == wxNOT_FOUND) {
        wxMessageBox("You have not selected an origin and a destination", "Error!", wxOK | wxICON_INFORMATION);
    } else {
        // Gets the origin and destination station names and adds them to the list of trips
        long trip_index = trip_list->InsertItem(trip_list->GetItemCount(), origin_list->GetString(origin_list->GetSelection()));
        trip_list->SetItem(trip_index, 1, destination_list->GetString(destination_list->GetSelection()));
    }
    // Saves the trip list to the file
    SaveTripList();
}

// Event handler
// Handles the event when the user chooses a trip they want to view info on, from the list
void GUIFrame::OnSelectTrip(wxListEvent &event) {
    long index = -1;
    string origin_name, destination_name;
    // Get the index of the selected item
    while ((index = trip_list->GetNextItem(index, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != wxNOT_FOUND) {
        // Get the origin and destination names
        origin_name = trip_list->GetItemText(index).ToStdString();
        destination_name = trip_list->GetItemText(index, 1).ToStdString();
    }
    // Create a new trip info dialog
    try {
        TripDialog dlg("Trip", wxPoint(50, 50), wxSize(700, 750), origin_name, destination_name);
        dlg.ShowModal();
    }
    catch(exception & e) {
        wxMessageBox("TripSit could not find any valid trips with this criteria. Try a different origin and destination.", "Error!", wxOK | wxICON_ERROR);
    }
}

// Event handler
// Handles the event when the user right clicks a saved trip from the list
void GUIFrame::OnRightClick(wxListEvent &event) {
    // Create a context menu
    wxMenu *context = new wxMenu;
    // Add a menu item to delete the selected trip
    context->Append(ID_REMOVE_TRIP, "Delete Item");
    PopupMenu(context, event.GetPoint() + wxPoint(460, 10));
}

// Event handler
// Handles the event when the user clicks on the remove trip button
void GUIFrame::OnDeleteTrip(wxCommandEvent &event) {
    long index = trip_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    // Remove the selected item
    trip_list->DeleteItem(index);
    // Save the new list
    SaveTripList();
}

// Event handler
// Handles the event when the user selects a trip from the timetable list
void TripDialog::OnViewTrip(wxListEvent& event) {
    long index = journeys_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    trip_viewer->DeleteAllItems();
    wxTreeItemId root_item = trip_viewer->AddRoot(trip[index].legs[0].origin_name + " to " + trip[index].legs[0].destination_name);
    wxTreeItemId origin_item = trip_viewer->AppendItem(root_item, "Origin");
    wxTreeItemId destination_item = trip_viewer->AppendItem(root_item, "Destination");
    wxTreeItemId duration_item = trip_viewer->AppendItem(root_item, "Trip Duration: " + to_string((trip[index].legs[0].duration / 60) + 1) + " mins");
    wxTreeItemId line_item = trip_viewer->AppendItem(root_item, "Line: " + trip[index].legs[0].line_name);
    wxTreeItemId stops_item = trip_viewer->AppendItem(root_item, "Stops");
    wxTreeItemId alerts_item = trip_viewer->AppendItem(root_item, "Alerts");

    trip_viewer->AppendItem(origin_item, "Name: " + trip[index].legs[0].origin_name);
    trip_viewer->AppendItem(origin_item, "Platform: " + to_string(trip[index].legs[0].origin_platform));
    trip_viewer->AppendItem(origin_item, "Arrival Time: " + PadInt(to_string(trip[index].legs[0].departure_time_estimated.tm_hour), 2) + ":" + PadInt(to_string(trip[index].legs[0].departure_time_estimated.tm_min), 2));

    trip_viewer->AppendItem(destination_item, "Name: " + trip[index].legs[0].destination_name);
    trip_viewer->AppendItem(destination_item, "Platform: " + to_string(trip[index].legs[0].destination_platform));
    trip_viewer->AppendItem(destination_item, "Arrival Time: " + PadInt(to_string(trip[index].legs[0].arrival_time_estimated.tm_hour), 2) + ":" + PadInt(to_string(trip[index].legs[0].arrival_time_estimated.tm_min), 2));

    int i = 1;
    for (auto &stop : trip[index].legs[0].stops) {
        wxTreeItemId stop_item = trip_viewer->AppendItem(stops_item, "Stop " + to_string(i));
        trip_viewer->AppendItem(stop_item, "Name: " + stop.name);
        trip_viewer->AppendItem(stop_item, "Platform: " + stop.platform);
        i++;
    }

    int j = 1;
    for (auto &alert : trip[index].legs[0].alerts) {
        wxTreeItemId alert_item = trip_viewer->AppendItem(alerts_item, "Alert " + to_string(j) + " - " + alert.priority + " priority");
        trip_viewer->AppendItem(alert_item, alert.title);
        trip_viewer->AppendItem(alert_item, "");
        istringstream lines(alert.content);
        string line;
        while (getline(lines, line)) {
            trip_viewer->AppendItem(alert_item, line);
        }
        trip_viewer->AppendItem(alert_item, "");
        trip_viewer->AppendItem(alert_item, alert.url);
        j++;
    }

    trip_viewer->AppendItem(root_item, "Number of Carriages: " + trip[index].legs[0].carriages);
    trip_viewer->AppendItem(root_item, "Adult Price: $" + to_string(trip[index].adult_price));
    trip_viewer->AppendItem(root_item, "Child Price: $" + to_string(trip[index].child_price));
}

// Creates the layout that is displayed in the main window
// Takes no arguments and returns nothing
void GUIFrame::CreateLayout() {
    // Create the main sizer
    wxPanel *panel = new wxPanel(this, wxID_ANY);
    // Create the origin and destination heading text
    wxGenericStaticText *origin_text = new wxGenericStaticText(panel, wxID_ANY, wxT("Origin"), wxPoint(150, 0), wxSize(150, 90), wxST_NO_AUTORESIZE | wxALIGN_CENTER_HORIZONTAL);
    wxGenericStaticText *destination_text = new wxGenericStaticText(panel, wxID_ANY, wxT("Destination"), wxPoint(150, 260), wxSize(150, 90), wxST_NO_AUTORESIZE | wxALIGN_CENTER_HORIZONTAL);
    origin_text->SetLabelMarkup("<u><span size=\"xx-large\">Origin</span></u>");
    destination_text->SetLabelMarkup("<u><span size=\"xx-large\">Destination</span></u>");

    // Create the origin search box
    wxTextCtrl *origin_input = new wxTextCtrl(panel, ID_OS, wxT(""), wxPoint(10, 80), wxSize(430, 30));
    origin_input->SetHint("(e.g. Hornsby)");
    // Loads the file listing all possible stations
    std::ifstream input("stations.txt");
    // Reads the file line by line, and adds each line to a list
    for(string line; getline(input, line);) {
        station_list.Add(line);
    }
    // Create the origin list display using the list of all possible stations
    origin_list = new wxListBox(panel, wxID_ANY, wxPoint(10, 110), wxSize(430, 140), station_list);

    // Create the destination search box
    wxTextCtrl *destination_input = new wxTextCtrl(panel, ID_DS, wxT(""), wxPoint(10, 340), wxSize(430, 30));
    destination_input->SetHint("(e.g. Gosford)");
    // Create the destination list display using the list of all possible stations
    destination_list = new wxListBox(panel, wxID_ANY, wxPoint(10, 370), wxSize(430, 140), station_list);

    // Create some visual separators
    wxStaticLine *seperator_vert = new wxStaticLine(panel, wxID_ANY, wxPoint(450, 0), wxSize(0, 680), wxLI_VERTICAL);
    wxStaticLine *seperator_hor1 = new wxStaticLine(panel, wxID_ANY, wxPoint(0, 260), wxSize(450, 0));
    wxStaticLine *seperator_hor2 = new wxStaticLine(panel, wxID_ANY, wxPoint(0, 520), wxSize(450, 0));

    wxButton *button = new wxButton(panel, ID_ADD_TRIP, wxT("Add Trip"), wxPoint(10, 530), wxSize(430, 60));
    colours = new wxColourDatabase();

    // Create the display for the list of saved trips
    trip_list = new wxListCtrl(panel, ID_SELECT_TRIP, wxPoint(460, 10), wxSize(430, 580), wxLC_REPORT | wxLC_SINGLE_SEL);
    trip_list->InsertColumn(0, "Origin");
    trip_list->SetColumnWidth(0, 210);
    trip_list->InsertColumn(1, "Destination");
    trip_list->SetColumnWidth(1, 210);
    trip_list->SetBackgroundColour(colours->Find("LIGHT GREY"));
}

// Refines an array based on a search term
// Takes two arguments: a list and a search term
// The list is searched for any strings that contain that search term
// The results are then returned in a new array
wxArrayString GUIFrame::RefineList(wxArrayString& station_list, string search_term) {
    // Create a new array to store the refined results
    wxArrayString refined_list;
    // Iterate through the list
    for(const auto& station : station_list) {
        string search_term_lower = search_term;
        // Convert the search term to lowercase
        for(char &c : search_term_lower)
            c = tolower(c);
        string station_lower = station.ToStdString();
        // Convert the station to lowercase
        for(char &c : station_lower)
            c = tolower(c);
        // If the station contains the search term, add it to the refined list
        std::size_t found = station_lower.find(search_term_lower);
        if (found != string::npos) {
            refined_list.Add(station);
        }
    }
    return refined_list;
}

// Save the trip list to a file
// Takes no arguments and returns nothing
void GUIFrame::SaveTripList() {
    // Open the file where the trips will be saved
    ofstream trip_file("trips.txt");
    
    // Check for an error opening the file
    if (trip_file.is_open()) {
        // Write the trips to the file
        long index = -1;
        while ((index = trip_list->GetNextItem(index, wxLIST_NEXT_ALL)) != wxNOT_FOUND) {
            // Get the origin and destination of the trip
            string origin_text = trip_list->GetItemText(index, 0).ToStdString();
            string destination_text = trip_list->GetItemText(index, 1).ToStdString();
            // Write the trip
            trip_file << origin_text << "," << destination_text << endl;
        }
        // Close the file
        trip_file.close();
    } else {
        cerr << "File open error";
    }
}

// Load the trip list from a file
// Takes no arguments and returns nothing
void GUIFrame::LoadTripList() {
    // Open the file where the saved trips are stored
    ifstream trip_file("trips.txt");
    string line;

    // Check for an error opening the file
    if (trip_file.good()) {
        // Read the file line by line
        int i = 0;
        while(getline(trip_file, line)) {
            string origin;
            string destination;
            // Split the line into origin and destination
            origin = line.substr(0, line.find(","));
            destination = line.substr(line.find(",") + 1);

            // Add the origin and destination to the list
            long index = trip_list->InsertItem(i, origin);
            trip_list->SetItem(index, 1, destination);
            i++;
        }
        // Close the file
        trip_file.close();
    }
}