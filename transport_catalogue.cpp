#include "transport_catalogue.h"

namespace transport {

    using namespace std;

    void TransportCatalogue::AddBus(const string& id, const vector<string>& stops) {
        buses_.push_back({ id, stops });
        buses_info_[buses_.back().name] = &buses_.back();
        for (const string& stop : stops) {
            buses_on_stops_[stops_info_[stop]->name].insert(GetBus(id)->name);
            //buses_on_stops_[stop].insert(id);
        }
    }

    void TransportCatalogue::AddStop(const string& id, const geo::Coordinates& coordinates) {
        stops_.push_back({ id, coordinates });
        stops_info_[stops_.back().name] = &stops_.back();
    }

    const BusInfo* TransportCatalogue::GetBus(const string& id) const {
        return buses_info_.count(id) ? buses_info_.at(id) : nullptr;
    }

    const StopInfo* TransportCatalogue::GetStop(const string& id) const {
        return stops_info_.count(id) ? stops_info_.at(id) : nullptr;
    }

    StatRoute TransportCatalogue::GetStatBus(const string& id) const {
        const BusInfo* bus_info = GetBus(id);
        if (!bus_info) {
            return {};
        }
        StatRoute stat_route;
        stat_route.number_of_stops = bus_info->stops.size();

        unordered_set<string> unique_stops((*bus_info).stops.begin(), (*bus_info).stops.end());
        stat_route.unique_stops = unique_stops.size();

        for (size_t i = 0; i < stat_route.number_of_stops - 1; ++i) {
            stat_route.route_length += ComputeDistance(GetStop((*bus_info).stops[i])->coordinates, GetStop((*bus_info).stops[i + 1])->coordinates);
        }
        return stat_route;
    }

    set<string_view> TransportCatalogue::GetStatStop(const string& id) const {
        if (buses_on_stops_.count(id)) {
            return buses_on_stops_.at(id);
        }
        else {
            return {};
        }
    }

}// namespace tranpsort
