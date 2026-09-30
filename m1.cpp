// milestone 1
// Richard Liu, Jason Tai, Kalvin Cui

#include <iostream>
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <algorithm>
#include <math.h>
#include <dataStructure.h>
#include <helperFunctions.h>

using namespace std;


// Initialize global dataStructures struct

struct dataStructures fastData;

// Loads a map streets.bin and the corresponding osm.bin file 
// Returns true if successful, false if an error prevents map loading.
bool loadMap(std::string map_streets_database_filename)
{
    std::string osm_filename = map_streets_database_filename;
    if (osm_filename.find(".streets") != -1)
    {
        osm_filename = osm_filename.replace(osm_filename.find(".streets"), sizeof(".streets") - 1, ".osm");
    }
    bool load_successful = loadStreetsDatabaseBIN(map_streets_database_filename) && loadOSMDatabaseBIN(osm_filename);
    // Indicates whether the map has loaded successfully

    std::cout << "loadMap: " << map_streets_database_filename << std::endl;

    if (load_successful)
    {
        fastData.pathname = map_streets_database_filename;
        // Initializing fastData.intersection_street_segments vector
        fastData.intersection_street_segments.resize(getNumIntersections());
        for (int intersection = 0; intersection < getNumIntersections(); intersection++)
        {
            fastData.intersection_latlon.push_back(getIntersectionPosition(intersection));

            for (int i = 0; i < getNumIntersectionStreetSegment(intersection); i++)
            {
                int ss_id = getIntersectionStreetSegment(intersection, i);
                fastData.intersection_street_segments[intersection].push_back(ss_id);
            }
        }

        // loop through all street segments to load/compute required data into fastData data structures
        std::set<int> uniqueStreetIDChecker;
        fastData.intersections_on_street.resize(getNumStreets());
        fastData.street_segment_of_street.resize(getNumStreets());
        for (int streetSegment = 0; streetSegment < getNumStreetSegments(); streetSegment++)
        {

            StreetSegmentInfo ssi = getStreetSegmentInfo(streetSegment);

            int streetID = ssi.streetID;

            // loading intersection id data of the from and to of a street segment into their respective fastData data structure
            fastData.street_segment_start.push_back(ssi.from);
            fastData.street_segment_end.push_back(ssi.to);

            // loading street segment id into fastData data structure
            fastData.intersections_on_street[streetID].push_back(fastData.street_segment_start[streetSegment]);
            fastData.intersections_on_street[streetID].push_back(fastData.street_segment_end[streetSegment]);

            // temporary set to check for duplicate streetID
            if (uniqueStreetIDChecker.find(streetID) == uniqueStreetIDChecker.end())
            {
                uniqueStreetIDChecker.insert(streetID);
                std::string modifiedstreetname = getStreetName(streetID);
                std::transform(modifiedstreetname.begin(), modifiedstreetname.end(), modifiedstreetname.begin(), ::tolower);
                modifiedstreetname.erase(remove(modifiedstreetname.begin(), modifiedstreetname.end(), ' '), modifiedstreetname.end());
                fastData.partial_street_name_and_ids.insert(std::pair<std::string, int>{modifiedstreetname, streetID});
            }

            // load street segment info into fastData data structure
            fastData.street_segment_info.push_back(ssi);

            // load curve points for each street segment into fastData data structure
            std::vector<LatLon> points;
            if (fastData.street_segment_info[streetSegment].numCurvePoints != 0)
            {
                for (int curvePoint = 0; curvePoint < fastData.street_segment_info[streetSegment].numCurvePoints; ++curvePoint)
                {
                    points.push_back(getStreetSegmentCurvePoint(streetSegment, curvePoint));
                }
            }

            fastData.street_segment_curve_points.push_back(points);

            // load all street segment id of each street into fastData data structure (vector of vector)
            fastData.street_segment_of_street[fastData.street_segment_info[streetSegment].streetID].push_back(streetSegment);

            // preload reciprocal speed limits into fastData vector
            fastData.street_segment_speed_limit.push_back(1 / fastData.street_segment_info[streetSegment].speedLimit);

            // preload segment lengths into vector fastData.street_segment_length
            fastData.street_segment_length.push_back(findStreetSegmentLength(streetSegment));

            // preload segment travel time into fastData data structure
            fastData.street_segment_travel_time.push_back(findStreetSegmentTravelTime(streetSegment));
        }

        // sorts intersection_on_street vector by intersectionIdx index and removes dupicates
        for (int i = 0; i < getNumStreets(); i++)
        {
            std::sort(fastData.intersections_on_street[i].begin(), fastData.intersections_on_street[i].end());
            fastData.intersections_on_street[i].erase(std::unique(fastData.intersections_on_street[i].begin(), fastData.intersections_on_street[i].end()), fastData.intersections_on_street[i].end());
        }

        //  stores feature areas into vector
        for (int i = 0; i < getNumFeatures(); ++i){
            fastData.featureArea.push_back(findFeatureArea(i));
        }

        // stores all OSMNodes in the city in an unordered hashmap with OSMid as the key
        for (int i = 0; i < getNumberOfNodes(); i++)
        {
            // Gets the ith OSM Node in this city
            const OSMNode *currNode = getNodeByIndex(i);
            OSMID id = currNode->id();
            // pairs and stores in unordered hashmap
            fastData.OSMNode_IDs.insert(std::make_pair(id, currNode));

            //const OSMNode *currNode = getNodeByIndex(i);
            //OSMID id = currNode -> id();
            if (getOSMNodeTagValue(id, "amenity") == "restaurant") {
                fastData.latlon_osm_restaurants.push_back(getNodeCoords(currNode));
            } else if (getOSMNodeTagValue(id, "shop") == "supermarket") {
                fastData.latlon_osm_supermarkets.push_back(getNodeCoords(currNode));
            } else if (getOSMNodeTagValue(id, "shop") == "mall") {
                //else if (getOSMNodeTagValue(id, "shop") == "department_store" || getOSMNodeTagValue(id, "shop") == "mall" || getOSMNodeTagValue(id, "shop") == "general" || getOSMNodeTagValue(id, "shop") == "clothes"){
                fastData.latlon_osm_shops.push_back(getNodeCoords(currNode));
            } else if (getOSMNodeTagValue(id, "building") == "hotel" || getOSMNodeTagValue(id, "tourism") == "hotel") {
                fastData.latlon_osm_hotels.push_back(getNodeCoords(currNode));
            } else if (getOSMNodeTagValue(id, "building") == "school" || getOSMNodeTagValue(id, "amenity") == "school") {
                fastData.latlon_osm_schools.push_back(getNodeCoords(currNode));
            } else if (getOSMNodeTagValue(id, "amenity") == "hospital" || getOSMNodeTagValue(id, "amenity") == "doctors" || getOSMNodeTagValue(id, "amenity") == "clinic") {
                fastData.latlon_osm_hospitals.push_back(getNodeCoords(currNode));
            }
        }

        for (int i = 0; i < getNumberOfWays(); i++){
            // Gets the ith OSM Way in this city
            const OSMWay *e = getWayByIndex(i);
            OSMID id = e->id();
            // pairs and stores in unordered hashmap
            fastData.OSMWay_IDs.insert(std::make_pair(id, e));
        }

        for (int i = 0; i < getNumberOfRelations(); i++){
            const OSMRelation *currRel = getRelationByIndex(i);
            //OSMID id = currRel->id();

            for (unsigned j = 0; j < getTagCount(currRel); j++) {
                pair <string, string> tagPair = getTagPair(currRel, j);

                if (tagPair.first == "route" && tagPair.second == "subway") {

                    fastData.subway_lines.push_back(currRel);
                    break;
                }
            }
        }

        fastData.subway_station_names.resize(fastData.subway_lines.size());
        fastData.subway_stations.resize(fastData.subway_lines.size());
        fastData.subway_lines.resize(fastData.subway_lines.size());
        fastData.subway_tracks.resize(fastData.subway_lines.size());
        fastData.osm_subway_stations.resize(fastData.subway_lines.size());
        fastData.osm_subway_tracks.resize(fastData.subway_lines.size());

        for (unsigned i = 0; i < fastData.subway_lines.size(); i++) {

            for (unsigned j = 0; j < getTagCount(fastData.subway_lines[i]); j++) {
                pair<string, string> tagPair = getTagPair(fastData.subway_lines[i], j);

                if (tagPair.first == "colour") {
                    fastData.subway_colour.push_back(tagPair.second);
                    cout << "Subway line color: " << tagPair.second << endl;
                } else if (tagPair.first == "name") {
                    fastData.subway_name.push_back(tagPair.second);
                    cout << "Subway line name: " << tagPair.second << endl;
                }
            }

            vector <TypedOSMID> route_members = getRelationMembers(fastData.subway_lines[i]);

            cout << "Subway line stations:" << endl;

            vector <const OSMWay*> ways;

            for(unsigned j = 0; j < route_members.size(); j++) {
                
                // A member of type node represents a subway station
                if(route_members[j].type() == TypedOSMID::Node) {

                    fastData.subway_station_names[i].push_back(getOSMNodeTagValue(route_members[j], "name"));
                   // cout << getOSMNodeTagValue(route_members[j], "name") << endl;

                    auto it = fastData.OSMNode_IDs.find(route_members[j]);
                    const OSMNode *tempNode = it->second;

                    fastData.subway_stations[i].push_back(getNodeCoords(tempNode));
                    //cout << getNodeCoords(tempNode) << endl;

                }else{
                    auto it = fastData.OSMWay_IDs.find(route_members[j]);
                    if (it == fastData.OSMWay_IDs.end()){
                        continue;
                    }
                    const OSMWay *tempWay = it->second;
                    ways.push_back(tempWay);
                }
            }

            fastData.subway_tracks[i].resize(ways.size());
            fastData.osm_subway_tracks[i].resize(ways.size());
            for (unsigned int j = 0; j < ways.size(); j++){

                const vector<OSMID>& way_members = getWayMembers(ways[j]);
                //fastData.subway_tracks[i].resize(ways.size());

                for (int k = 0; k < way_members.size(); k++){
                    auto it = fastData.OSMNode_IDs.find(way_members[k]);
                    const OSMNode *tempNode = it->second;
                    fastData.subway_tracks[i][j].push_back(getNodeCoords(tempNode));
                    //cout << getNodeCoords(tempNode) << endl;

                }
            }
            route_members.clear();
            ways.clear();
        }
        
        fastData.restaurantIcon = ezgl::renderer::load_png("libstreetmap/resources/restaurantIcon.png");
        fastData.supermarketIcon = ezgl::renderer::load_png("libstreetmap/resources/supermarketIcon.png");
        fastData.shopsIcon = ezgl::renderer::load_png("libstreetmap/resources/shopIcon.png");
        fastData.hotelIcon = ezgl::renderer::load_png("libstreetmap/resources/hotelIcon.png");
        fastData.schoolIcon = ezgl::renderer::load_png("libstreetmap/resources/schoolIcon.png");
        fastData.parkIcon = ezgl::renderer::load_png("libstreetmap/resources/parkIcon.png");
        fastData.hospitalIcon = ezgl::renderer::load_png("libstreetmap/resources/hospitalIcon.png");
    }

    return load_successful;
}

// Close the map (if loaded)
void closeMap()
{
    // Clean-up map related data structures here

    ezgl::renderer::free_surface(fastData.restaurantIcon);
    ezgl::renderer::free_surface(fastData.supermarketIcon);
    ezgl::renderer::free_surface(fastData.shopsIcon);
    ezgl::renderer::free_surface(fastData.hotelIcon);
    ezgl::renderer::free_surface(fastData.schoolIcon);
    ezgl::renderer::free_surface(fastData.parkIcon);
    ezgl::renderer::free_surface(fastData.hospitalIcon);

    closeStreetDatabase();

    closeOSMDatabase();

    fastData.intersection_street_segments.clear();

    fastData.intersection_latlon.clear();

    fastData.intersections_on_street.clear();

    fastData.partial_street_name_and_ids.clear();

    fastData.street_segment_info.clear();

    fastData.street_segment_start.clear();

    fastData.street_segment_end.clear();

    fastData.street_segment_curve_points.clear();

    fastData.street_segment_of_street.clear();

    fastData.street_segment_speed_limit.clear();

    fastData.street_segment_length.clear();

    fastData.street_segment_travel_time.clear();
    
    fastData.featureArea.clear();
    
    fastData.OSMNode_IDs.clear();
    
    fastData.OSMWay_IDs.clear();

    fastData.latlon_osm_restaurants.clear();
    fastData.latlon_osm_supermarkets.clear();
    fastData.latlon_osm_shops.clear();
    fastData.latlon_osm_hotels.clear();
    fastData.latlon_osm_schools.clear();
    fastData.latlon_osm_hospitals.clear();

    fastData.osm_restaurants.clear();
    fastData.osm_supermarkets.clear();
    fastData.osm_shops.clear();
    fastData.osm_hotels.clear();
    fastData.osm_schools.clear();
    fastData.osm_hospitals.clear();

    fastData.subway_tracks.clear();
    fastData.subway_stations.clear();
    fastData.subway_station_names.clear();
    fastData.subway_colour.clear();
    fastData.subway_lines.clear();
    fastData.subway_name.clear();

    fastData.osm_subway_stations.clear();
    fastData.osm_subway_tracks.clear();

    fastData.pathname.clear();
}

// Returns the distance between two (lattitude,longitude) coordinates in meters
double findDistanceBetweenTwoPoints(LatLon point_1, LatLon point_2)
{
    double distance = 0;

    double point_1Lat = point_1.latitude();
    double point_1Lon = point_1.longitude();

    double point_2Lat = point_2.latitude();
    double point_2Lon = point_2.longitude();

    
    double latAvg = (point_1Lat * kDegreeToRadian + point_2Lat * kDegreeToRadian) / 2; // compute latAvg


    
    double point_1X = getXCoord(point_1Lon, latAvg); // compute x from latitude
    double point_1Y = getYCoord(point_1Lat); //compute y from longitude

    double point_2X = getXCoord(point_2Lon, latAvg);;
    double point_2Y = getYCoord(point_2Lat);

    
    distance = sqrt(pow(point_2Y - point_1Y, 2) + pow(point_2X - point_1X, 2)); // compute distance between 2 points 
    return abs(distance);
}


// Returns the length of the given street segment in meters
double findStreetSegmentLength(StreetSegmentIdx street_segment_id)
{

    // goes into this if statement to retrieve data after loadmaps completed. Otherwise, will compute data for loadmaps
    if (fastData.street_segment_length.size() == getNumStreetSegments())
    {
        return fastData.street_segment_length[street_segment_id];
    }

    // calculations
    double currentLen = 0;

    LatLon startLatLon = fastData.intersection_latlon[fastData.street_segment_start[street_segment_id]];
    LatLon endLatLon = fastData.intersection_latlon[fastData.street_segment_end[street_segment_id]];

    int currentNumOfCurvePoints = fastData.street_segment_curve_points[street_segment_id].size();

    //  If street segment is not curved return distance between start and end;
    if (currentNumOfCurvePoints == 0)
    {
        currentLen = findDistanceBetweenTwoPoints(startLatLon, endLatLon);
        return currentLen;
    }

    // if street segment is curved, add up distance between each point and start and end together
    for (int i = 0; i < currentNumOfCurvePoints - 1; ++i)
    {
        currentLen += findDistanceBetweenTwoPoints(fastData.street_segment_curve_points[street_segment_id][i], fastData.street_segment_curve_points[street_segment_id][i + 1]);
    }

    currentLen += findDistanceBetweenTwoPoints(startLatLon, fastData.street_segment_curve_points[street_segment_id][0]);
    currentLen += findDistanceBetweenTwoPoints(endLatLon, fastData.street_segment_curve_points[street_segment_id][currentNumOfCurvePoints - 1]);

    return currentLen;
}

// Returns the travel time to drive from one end of a street segment to the other, in seconds, when driving at the speed limit
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id)
{

    // goes into this if statement to retrieve data after loadmaps is finished, otherwise will compute data for loadmaps
    if (fastData.street_segment_travel_time.size() == getNumStreetSegments())
    {
        return fastData.street_segment_travel_time[street_segment_id];
    }

    // multiply street segment length with 1/speed limit
    return fastData.street_segment_length[street_segment_id] * fastData.street_segment_speed_limit[street_segment_id];
}


// Returns all intersections reachable by traveling down one street segment from the given intersection 
std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id)
{
    std::vector<IntersectionIdx> adj_intersections;

    // loops to the number of street segment in the given intersection
    for (int i = 0; i < getNumIntersectionStreetSegment(intersection_id); i++)
    {

        StreetSegmentInfo ssi = getStreetSegmentInfo(findStreetSegmentsOfIntersection(intersection_id)[i]);

        // if not one way street, store the index of the other intersection
        if (!ssi.oneWay)
        {
            if (intersection_id == ssi.from)
            {
                adj_intersections.push_back(ssi.to);
            }
            else
            {
                adj_intersections.push_back(ssi.from);
            }
            
        }

        // if one way street, only store the index of the other intersection if the "to" is not the given intersection id
        else
        {
            if (intersection_id == ssi.from)
            {
                adj_intersections.push_back(ssi.to);
            }
        }
    }
    //if a street loops back to itself, we can just ignore it because the next commands sorts the vector and erases duplicates
    std::sort( adj_intersections.begin(), adj_intersections.end() );
    adj_intersections.erase(std::unique( adj_intersections.begin(), adj_intersections.end() ), adj_intersections.end());

    return adj_intersections;
}


// Returns the geographically nearest intersection (i.e. as the crow flies) to the given position
IntersectionIdx findClosestIntersection(LatLon my_position)
{
    double closest = std::numeric_limits<double>::max(); // initalize closest to largest possible double
    IntersectionIdx closestIndex = 0;

    for (unsigned int intersection = 0; intersection < getNumIntersections(); intersection++)
    {
        // compute distance between my position and every intersection (this function doesn't have a time requirement)
        double current = findDistanceBetweenTwoPoints(my_position, fastData.intersection_latlon[intersection]);
        if (current < closest)
        {
            // if a smaller value is found, store the index
            closest = current;
            closestIndex = intersection;
        }
    }
    return closestIndex;
}

// Returns the street segments that connect to the given intersection
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id)
{
    // already have a datastructure that implements this
    return fastData.intersection_street_segments[intersection_id];
}

// Returns all intersections along the a given street
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id)
{
    // already have a datastructure that implements this
    return fastData.intersections_on_street[street_id];
}

// Return all intersection ids at which the two given streets intersect. This function will typically return one intersection id 
// for streets that intersect and a length 0 vector for streets that do not. 
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(StreetIdx street_id1, StreetIdx street_id2)
{

    std::vector<IntersectionIdx> intersections_of_two_streets;

    // store all intersections on the two given streets in two different vectors
    std::vector<IntersectionIdx> v1 = fastData.intersections_on_street[street_id1];
    std::vector<IntersectionIdx> v2 = fastData.intersections_on_street[street_id2];

    int index1 = 0;
    int index2 = 0;

    // make sure that index doesn't go out of bounds
    // algorithm logic: since these vectors are sorted, compare elements in parallel
    // if they are the same (intersection exists in both streets), store the intersection
    // if they are different, increment the index of the lower valued intersectionIdx
    while ((index1 < v1.size()) && (index2 < v2.size()))
    {
        if (v1[index1] == v2[index2])
        {
            intersections_of_two_streets.push_back(v1[index1]);
            index1++;
            index2++;
        }
        else if (v1[index1] < v2[index2])
        {
            index1++;
        }
        else
        {
            index2++;
        }
    }

    return intersections_of_two_streets;
}

// Returns all street ids corresponding to street names that start with the given prefix
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix)
{
    
    std::vector<StreetIdx> streetIds; // Vector to store streetIds
    // Remove all spaces and make street_prefix lowercase
    std::transform(street_prefix.begin(), street_prefix.end(), street_prefix.begin(), ::tolower);
    street_prefix.erase(remove(street_prefix.begin(), street_prefix.end(), ' '), street_prefix.end());

    // checking to see if the map is empty
    if (fastData.partial_street_name_and_ids.empty())
    {
        return streetIds;
    }
    // checking to see if the prefix is empty
    if (street_prefix.empty())
    {
        return streetIds;
    }
    // Using an iterator to check multimap for potential matches
    auto it = fastData.partial_street_name_and_ids.lower_bound(street_prefix);
    while (it != fastData.partial_street_name_and_ids.end())
    {
        // If a match is found, push into the vector, otherwise break out of the loop
        if (street_prefix == it->first.substr(0, street_prefix.length()))
        {
            streetIds.push_back(it->second);
        }
        else
        {
            break;
        }
        it++;
    }
    // Return the streetIds vector
    return streetIds;
}

// Returns the length of a given street in meters
double findStreetLength(StreetIdx street_id)
{

    double streetlength = 0;

    // loop through vector of street_segment_of_street to add all segment lengths of street together
    for (auto it = fastData.street_segment_of_street[street_id].begin(); it != fastData.street_segment_of_street[street_id].end(); ++it)
    {
        streetlength += findStreetSegmentLength(*it);
    }

    // return streetlength
    return streetlength;
}


// Returns the nearest point of interest of the given type (e.g. "restaurant") to the given position
POIIdx findClosestPOI(LatLon my_position, std::string POItype)
{
    
    double closestdistance = std::numeric_limits<double>::max(); // Initializing to be the greatest possible double value
    
    POIIdx closestPOI = -1; // Creating the closestPOI holder
    // Loop through all POIs, comparing their distance to the closest distance
    for (int POIIndex = 0; POIIndex < getNumPointsOfInterest(); POIIndex++)
    {
        // if the POIType matches, goes into this if statement
        if (getPOIType(POIIndex) == POItype)
        {
            double distance = findDistanceBetweenTwoPoints(my_position, getPOIPosition(POIIndex));
            // Recording closestPOI if current closest distance is less than previous closest distance
            if (distance < closestdistance)
            {
                closestdistance = distance;
                closestPOI = POIIndex;
            }
        }
    }
    // Will return NULL if there is no POI that matches POItype, otherwise will return the closestPOIidx
    return closestPOI;
}

// Returns the area of the given closed feature in square meters
// Assume a non self-intersecting polygon (i.e. no holes)
// Return 0 if this feature is not a closed polygon.
double findFeatureArea(FeatureIdx feature_id)
{
     
    if (fastData.featureArea.size() == getNumFeatures()){
        return fastData.featureArea[feature_id];
    }
    
    std::vector<LatLon> featurePointVertices; // Create a vector to store the LatLon of the vertices

    
    int numFeaturePoints = getNumFeaturePoints(feature_id); // Get the number of vertices
    
    double currentArea = 0; // Initialize the area
    
    double latAvg= 0; //Initialize the latitude average that will be used for calculations
    for (int pointNum = 0; pointNum < numFeaturePoints; pointNum ++){
        
        LatLon featurePoint = getFeaturePoint(feature_id, pointNum); //Record the latitude and longitude of each vertice
        
        featurePointVertices.push_back(featurePoint); // Pushing the latitude and longitude of the vertice
        
        latAvg += featurePoint.latitude() * kDegreeToRadian; // Adding the latitude to the average
    }
    // Making sure there are atleast 1 vertice
    if (featurePointVertices.empty())
    {
        return currentArea;
    }
    // Ensuring the vertices form a closed polygon
    if (featurePointVertices[0].latitude() != featurePointVertices[featurePointVertices.size() - 1].latitude())
    {
        return currentArea;
    }
    if (featurePointVertices[0].longitude() != featurePointVertices[featurePointVertices.size() - 1].longitude())
    {
        return currentArea;
    }
    
    latAvg = latAvg / numFeaturePoints; // Calculating the latitude average 
    
    std::vector<double> xVertices; // Vector to store X coordinate of vertices
    
    std::vector<double> yVertices; // Vector to store Y coordinate of vertices
    // For loop to populate X and Y coordinate vectors (converting Lat and Lon to Cartesian)
    for (int pointNum = 0; pointNum < numFeaturePoints; pointNum++)
    {
        double Lat = featurePointVertices[pointNum].latitude();
        double Lon = featurePointVertices[pointNum].longitude();
        double X = kEarthRadiusInMeters * Lat * kDegreeToRadian * cos(latAvg);
        double Y = kEarthRadiusInMeters * Lon * kDegreeToRadian;
        xVertices.push_back(X);
        yVertices.push_back(Y);
    }
    // Using the shoelace formula to calculate the area of a closed polygon
    int point2 = numFeaturePoints - 1;
    for (int point1 = 0; point1 < numFeaturePoints; point1++)
    {
        currentArea += (xVertices[point2] + xVertices[point1]) * (yVertices[point2] - yVertices[point1]);
        point2 = point1;
    }
    // Returning the featureare, will return 0 if featureArea is not a closed polygon
    return abs(currentArea / 2.0);
}


// Return the value associated with this key on the specified OSMNode.
// If this OSMNode does not exist in the current map, or the specified key is
// not set on the specified OSMNode, return an empty string.
std::string getOSMNodeTagValue(OSMID OSMid, std::string key)
{

    // using the ODMid as the index, find the corresponding OSMNode
    auto it = fastData.OSMNode_IDs.find(OSMid);
    // if OSMid is not found, return empty string
    if (it == fastData.OSMNode_IDs.end())
    {
        return "";
    }

    const OSMNode *osmnode = it->second;

    // separate and store the key and value pairs in two different variables
    for (int i = 0; i < getTagCount(osmnode); ++i)
    {
        std::string osmkey, value;
        std::tie(osmkey, value) = getTagPair(osmnode, i);
        if (osmkey == key)
        {
            return value;
        }
    }

    // if nothing was returned, that means key was not found, so return empty string
    return "";
}
