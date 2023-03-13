#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <map>
#include <set>
#include <vector>
#include <unordered_map>
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/color.hpp"

struct dataStructures{
    std::string pathname;
    
    ezgl::surface *restaurantIcon;
    ezgl::surface *supermarketIcon;
    ezgl::surface *shopsIcon;
    ezgl::surface *hotelIcon;
    ezgl::surface *schoolIcon;
    ezgl::surface *parkIcon;
    ezgl::surface *hospitalIcon;
    // A 2D vector that stores the street segments connecting to each intersection
    std::vector<std::vector<StreetSegmentIdx>> intersection_street_segments;

    // A vector that stores the coordinates of each intersection
    std::vector <LatLon> intersection_latlon;

    // A 2D vector that stores all intersections on each street
    std::vector<std::vector<IntersectionIdx>> intersections_on_street;

    std::multimap<std::string, int> partial_street_name_and_ids;

    // A vector stores street segment info for each street segment
    std::vector <StreetSegmentInfo> street_segment_info;

    // vector that stores intersection id of street segment start point
    std::vector <int> street_segment_start;

    // vector that stores intersection id of street segment end point
    std::vector <int> street_segment_end;

    // vector of vector of each street segment's curve points
    std::vector<std::vector <LatLon>> street_segment_curve_points;

    //A 2D vector that stores all street segments on each street
    std::vector<std::vector<StreetSegmentIdx>> street_segment_of_street;

    // vector that holds 1/speed limit of street segment
    std::vector <float> street_segment_speed_limit;

    // vector that holds street segment lengths
    std::vector <double> street_segment_length;

    // vector that holds street segment travel times
    std::vector <double> street_segment_travel_time;

    std::vector <double> featureArea;

    //An unordered map that stores OSMNOdes with OSMID as the key
    std::unordered_map <OSMID, const OSMNode*> OSMNode_IDs;

    //An unordered map that stores OSMWays with OSMID as the key
    std::unordered_map <OSMID, const OSMWay*> OSMWay_IDs;

    std::vector <LatLon> latlon_osm_restaurants;
    std::vector <LatLon> latlon_osm_supermarkets;
    std::vector <LatLon> latlon_osm_shops;
    std::vector <LatLon> latlon_osm_hotels;
    std::vector <LatLon> latlon_osm_schools;
    //std::vector <LatLon> latlon_parks;
    std::vector <LatLon> latlon_osm_hospitals;

    std::vector <ezgl::point2d> osm_restaurants;
    std::vector <ezgl::point2d> osm_supermarkets;
    std::vector <ezgl::point2d> osm_shops;
    std::vector <ezgl::point2d> osm_hotels;
    std::vector <ezgl::point2d> osm_schools;
    std::vector <ezgl::point2d> parks;
    std::vector <ezgl::point2d> osm_hospitals;

    std::vector<const OSMRelation*> subway_lines;
    std::vector<std::string> subway_name;
    std::vector<std::string> subway_colour;

    std::vector<std::vector<std::vector<LatLon>>> subway_tracks; //vector of relations, each relation holds a vector of ways, each way holds a vector of nodes
    std::vector<std::vector<std::vector<ezgl::point2d>>> osm_subway_tracks;

    std::vector<std::vector<LatLon>> subway_stations; //cponsider changing to latlon
    std::vector<std::vector<ezgl::point2d>> osm_subway_stations;
    std::vector<std::vector<std::string>> subway_station_names;
    
    
};

extern struct dataStructures fastData;

#endif
