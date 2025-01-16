#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <unordered_map>

namespace reader {
/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = reader::ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}
    
std::string_view SeparatingCoordinatesFromStops(std::string_view coordinates) {
    size_t second_comma = coordinates.find(',', coordinates.find(',') + 1);
    std::string_view result = coordinates.substr(0, second_comma);     
    return result;   
}    
       

void ParseStopsAndDistances(const std::vector<std::string_view>& request, std::unordered_map<std::string_view, unsigned int>& stops) {
    for( auto iter = request.begin() + 2; iter != request.end(); ++iter ) {
        auto pos_m = (*iter).find('m');
        auto pos_space = (*iter).find(' ', (*iter).find(' ') + 1);
        stops[(*iter).substr(pos_space + 1)] = std::stoi(std::string((*iter).substr(0, pos_m + 1)));        
    }               
}        
    
void InputReader::ApplyCommands([[maybe_unused]] transport::TransportCatalogue& catalogue) const {
    for ( const CommandDescription& command_description : commands_) {
        if ( command_description.command == "Stop") {
            catalogue.AddStop(command_description.id, ParseCoordinates(SeparatingCoordinatesFromStops(command_description.description)));            
        }
    }
    for ( const CommandDescription& command_description : commands_) {
        if ( command_description.command == "Bus") {
            std::vector<std::string_view> stops_v = reader::ParseRoute(command_description.description);
            std::vector<std::string> stops(stops_v.begin(), stops_v.end());
            catalogue.AddBus (command_description.id, stops);
        } else if ( command_description.command == "Stop") {
            if ( size_t second_comma = command_description.command.find(',', command_description.command.find(',') + 1); second_comma != std::string::npos){
                continue;
            } 
            std::vector<std::string_view> request = Split(command_description.description, ',');
            std::unordered_map<std::string_view, unsigned int> stops_and_distance;
            ParseStopsAndDistances(request, stops_and_distance);
            for(const auto& [key, value]: stops_and_distance) {
                catalogue.AddDistance(catalogue.GetStop(std::string(key)), catalogue.GetStop(command_description.id), value);
            }
        }
    }
}
    
}//namespace reader
