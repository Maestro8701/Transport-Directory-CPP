#include "transport_catalogue.h"

namespace transport{

using namespace std;

    void TransportCatalogue::AddBus (const string& id, const vector<string>& stops) {
        buses_.push_back({id, stops});
        buses_info_[buses_.back().name] = &buses_.back();
        
        for (const string& stop : stops) {
             buses_on_stops_[stops_info_[stop]->name].insert(GetBus(id)->name);
        }
    }

    void TransportCatalogue::AddDistance(const StopInfo* lhs, const StopInfo* rhs, unsigned int distance) {
        distances_.insert({ {lhs, rhs}, distance});
    }
    
    void TransportCatalogue::AddStop (const string& id, const geo::Coordinates& coordinates) {
        stops_.push_back({id, coordinates});
        stops_info_[stops_.back().name] = &stops_.back();
    }
    
    const BusInfo* TransportCatalogue::GetBus (const string& id) const {
        return buses_info_.count(id) ? buses_info_.at(id) : nullptr; 
    }
    
    const StopInfo* TransportCatalogue::GetStop (const string& id) const {
        return stops_info_.count(id)? stops_info_.at(id) : nullptr;
    }
    
    unsigned int TransportCatalogue::GetDistance(const StopInfo* lhs, const StopInfo* rhs) const {
        if(distances_.count({ lhs, rhs})) {
            return distances_.at({ lhs, rhs});
        } else if (distances_.count({ rhs, lhs})) {
            return distances_.at({ rhs, lhs});
        }
        return 0;
    }
    
    StatRoute TransportCatalogue::GetStatBus (const string& id) const {
        const BusInfo* bus_info = GetBus(id);
        if (!bus_info) {
            return {};
        }
        StatRoute stat_route;        
        unordered_set<string> unique_stops((*bus_info).stops.begin(), (*bus_info).stops.end());
                        
        double route_length_f = 0.0; 
        for (size_t i = 0; i < (*bus_info).stops.size() - 1; ++i) {
            route_length_f += ComputeDistance(GetStop((*bus_info).stops[i])->coordinates, GetStop((*bus_info).stops[i+1])->coordinates);
            stat_route.curvature += GetDistance(GetStop(bus_info->stops[i]), GetStop(bus_info->stops[i + 1]));
        }
        
        stat_route.number_of_stops = bus_info->stops.size();
        stat_route.unique_stops = unique_stops.size();
        stat_route.route_length = stat_route.curvature;
        stat_route.curvature = stat_route.curvature / route_length_f;
        return stat_route;
    }

    set<string_view> TransportCatalogue::GetStatStop (const string& id) const {
        if (buses_on_stops_.count(id)) {
            return buses_on_stops_.at(id);
        } else {
            return {};
        }
   }
    
}// namespace transport
