/* This file contains the declaration of the custom data type to contain trip info received from the API */
#include <string>
#include <iostream>
#include <vector>

using namespace std;

// The main TNSW API parent class
class TNSW {
    public:
        // The standard structure for storing basic information about train alerts / notifications.
        struct Alert {
            string priority;
            string title;
            string content;
            string url;
        };

        // The standard structure for storing basic information about a train stop.
        struct BasicStopInfo {
            string name;
            string platform;
            string global_id;
        };

        // The structure to contain information about a particular leg of a journey.
        struct Leg {
            int duration;

            string origin_name;
            string origin_type;
            int origin_platform;
            string origin_global_id;
            struct tm departure_time_planned;
            struct tm departure_time_estimated;

            string destination_name;
            string destination_type;
            int destination_platform;
            string destination_global_id;
            struct tm arrival_time_planned;
            struct tm arrival_time_estimated;

            string line_name;
            string final_station;
            vector<BasicStopInfo> stops;
            vector<Alert> alerts;
            string carriages;
        };

        // The overarching class to contain all info received from the API.
        // Stores a vector of 'leg' objects, as well as pricing information.
        struct Journey {
            vector<Leg> legs;
            float adult_price;
            float child_price;
        };

        // The class to conatin declarations for API functions
        class API {
            private:
                static BasicStopInfo StopFinder(string, string, string, string, string);
                static vector<Journey> TripPlanner(string, string, string, string, string, string, string, string);
            
            public:
                static vector<Journey> GetTripInfo(string, string, time_t);
        };
};