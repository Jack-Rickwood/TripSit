/* This file contains code for contacting the transportnsw API, and parsing the information into a custom data structure. */
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <time.h>
#include "tripview_api.h"
using namespace std;
using namespace curlpp::options;
using json = nlohmann::json;

#define UTC (10)

string api_key = "Uj5Y6CW0qnAZ59UagrFadFDHO8ZSx76LGWhy";
string base_url = "https://api.transport.nsw.gov.au/v1/tp";

extern "C" char* strptime(const char* s, const char* f, struct tm* tm) {
    std::istringstream input(s);
    input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
    input >> std::get_time(tm, f);
    if (input.fail()) {
        return nullptr;
    }
    return (char*)(s + input.tellg());
}

TNSW::BasicStopInfo TNSW::API::StopFinder(string name_sf, string output_format = "rapidJson", string type_sf = "stop", string coord_output_format = "EPSG:4326", string tnsw_options = "true") {
    TNSW::BasicStopInfo stop;
    try {
        curlpp::Cleanup cleanup;
        curlpp::Easy request;

        // Set URL
        string url = base_url + "/stop_finder?outputFormat=" + output_format + "&type_sf=" + type_sf + "&name_sf=" + curlpp::escape(name_sf) + "&coordOutputFormat=" + coord_output_format + "&TfNSWSF=" + tnsw_options;
        request.setOpt<curlpp::options::Url>(url);

        // Set HTTP headers
        std::list<std::string> headers;
        headers.push_back("Accept: application/json");
        headers.push_back("Authorization: apikey " + api_key);
        request.setOpt(new HttpHeader(headers));

        // Output stream
        std::ostringstream os;
		curlpp::options::WriteStream ws(&os);
		request.setOpt(ws);
        request.perform();

        // Parse response
        auto response = json::parse(os.str());
        stop.name = response["locations"][0]["assignedStops"][0]["name"];
        stop.global_id = response["locations"][0]["assignedStops"][0]["id"];
    }
    catch(curlpp::RuntimeError & e) {
		std::cout << e.what() << std::endl;
	}
	catch(curlpp::LogicError & e) {
		std::cout << e.what() << std::endl;
	}
    return stop;
}

vector<TNSW::Journey> TNSW::API::TripPlanner(string itd_date, string itd_time, string name_origin, string name_destination, string output_format = "rapidJson", string coord_output_format = "EPSG:4326", string dep_arr_macro = "dep", string calc_num_trips = "15") {
    vector<TNSW::Journey> trips;
    try {
        curlpp::Cleanup cleanup;
        curlpp::Easy request;

        // Set URL
        string url = base_url + "/trip?outputFormat=" + output_format + "&coordOutputFormat=" + coord_output_format + "&depArrMacro=" + dep_arr_macro + "&itdDate=" + itd_date + "&itdTime=" + itd_time + "&type_origin=any&name_origin=" + name_origin + "&type_destination=any&name_destination=" + name_destination + "&calcNumberOfTrips=" + calc_num_trips;
        request.setOpt<curlpp::options::Url>(url);

        // Set HTTP headers
        std::list<std::string> headers;
        headers.push_back("Accept: application/json");
        headers.push_back("Authorization: apikey " + api_key);
        request.setOpt(new HttpHeader(headers));

        // Output stream
        std::ostringstream os;
		curlpp::options::WriteStream ws(&os);
		request.setOpt(ws);
        request.perform();

        // Parse response
        auto response = json::parse(os.str());
        cout << response.dump() << endl;
        for (auto &journey_json : response["journeys"]) {
            if (journey_json["legs"][0]["transportation"]["product"]["class"] == 1 && journey_json["legs"].size() == 1) {
                TNSW::Journey journey;
                vector<TNSW::Leg> legs;

                for (auto &leg_json : journey_json["legs"]) {
                    TNSW::Leg leg;

                    leg.duration = leg_json["duration"];

                    leg.origin_type = leg_json["origin"]["type"];
                    if (leg.origin_type == "stop") {
                        leg.origin_name = leg_json["origin"]["name"];
                    } else {
                        string name_full_o = leg_json["origin"]["disassembledName"];
                        char oname[100];
                        sscanf(name_full_o.c_str(), "%[^,]", oname);
                        leg.origin_name = string(oname);
                        string platform_full_o = leg_json["origin"]["properties"]["platform"];
                        leg.origin_platform = stoi(platform_full_o.erase(0, 3));
                    }

                    leg.origin_global_id = leg_json["origin"]["id"];

                    struct tm dtp;
                    string dtp_str = leg_json["origin"]["departureTimePlanned"];
                    strptime(dtp_str.c_str(), "%Y-%m-%dT%H:%M:%SZ", &dtp);
                    dtp.tm_hour += UTC;
                    mktime(&dtp);
                    leg.departure_time_planned = dtp;

                    struct tm dte;
                    string dte_str = leg_json["origin"]["departureTimeEstimated"];
                    strptime(dte_str.c_str(), "%Y-%m-%dT%H:%M:%SZ", &dte);
                    dte.tm_hour += UTC;
                    mktime(&dte);
                    leg.departure_time_estimated = dte;

                    leg.destination_type = leg_json["destination"]["type"];
                    if (leg.destination_type == "stop") {
                        leg.destination_name = leg_json["destination"]["name"];
                    } else {
                        string name_full_d = leg_json["destination"]["disassembledName"];
                        char dname[100];
                        sscanf(name_full_d.c_str(), "%[^,]", dname);
                        leg.destination_name = string(dname);
                        string platform_full_d = leg_json["destination"]["properties"]["platform"];
                        leg.destination_platform = stoi(platform_full_d.erase(0, 3));
                    }

                    leg.destination_global_id = leg_json["destination"]["id"];

                    struct tm atp;
                    string atp_str = leg_json["destination"]["arrivalTimePlanned"];
                    strptime(atp_str.c_str(), "%Y-%m-%dT%H:%M:%SZ", &atp);
                    atp.tm_hour += UTC;
                    mktime(&atp);
                    leg.arrival_time_planned = atp;

                    struct tm ate;
                    string ate_str = leg_json["destination"]["arrivalTimeEstimated"];
                    strptime(ate_str.c_str(), "%Y-%m-%dT%H:%M:%SZ", &ate);
                    ate.tm_hour += UTC;
                    mktime(&ate);
                    leg.arrival_time_estimated = ate;

                    leg.line_name = leg_json["transportation"]["name"];
                    leg.final_station = leg_json["transportation"]["destination"]["name"];

                    vector<TNSW::BasicStopInfo> stops;
                    for (auto &stop_json : leg_json["stopSequence"]) {
                        TNSW::BasicStopInfo stop;
                        string name_full = stop_json["disassembledName"];
                        char name[100];
                        sscanf(name_full.c_str(), "%[^,]", name);
                        stop.name = name;
                        string platform = stop_json["properties"]["platform"];
                        stop.platform = platform.erase(0, 3);
                        stop.global_id = stop_json["id"];
                        stops.push_back(stop);
                    }
                    leg.stops = stops;

                    vector<TNSW::Alert> alerts;
                    for (auto &info_json : leg_json["infos"]) {
                        TNSW::Alert alert;
                        alert.priority = info_json["priority"];
                        alert.url = info_json["url"];
                        alert.title = info_json["subtitle"];
                        alert.content = info_json["content"];
                        alerts.push_back(alert);
                    }
                    leg.alerts = alerts;

                    if (leg_json["origin"]["properties"].count("NumberOfCars") != 0) {
                        leg.carriages = leg_json["origin"]["properties"]["NumberOfCars"];
                    }

                    legs.push_back(leg);
                }

                journey.legs = legs;
                if (journey_json["fare"]["tickets"].size() > 1) {
                    journey.adult_price = journey_json["fare"]["tickets"][0]["priceBrutto"];
                    journey.child_price = journey_json["fare"]["tickets"][1]["priceBrutto"];
                }
                trips.push_back(journey);
            }
        }
    }
    catch(curlpp::RuntimeError & e) {
		std::cout << e.what() << std::endl;
	}
	catch(curlpp::LogicError & e) {
		std::cout << e.what() << std::endl;
	}
    return trips;
}

vector<TNSW::Journey> TNSW::API::GetTripInfo(string origin_name, string destination_name, time_t time) {
    TNSW::BasicStopInfo origin = StopFinder(origin_name);
    TNSW::BasicStopInfo destination = StopFinder(destination_name);
    cout << "Origin: " << origin.name << endl << "Destination: " << destination.name << endl;

    struct tm *tmp = gmtime(&time);
    tmp->tm_hour += UTC;
    mktime(tmp);
    char date_str[9];
    char time_str[5];
    sprintf(date_str, "%04d%02d%02d", tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday);
    sprintf(time_str, "%02d%02d", tmp->tm_hour, tmp->tm_min);

    return TripPlanner(date_str, time_str, origin.global_id, destination.global_id);
}