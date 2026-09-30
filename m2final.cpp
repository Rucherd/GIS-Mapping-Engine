// milestone 2 final
// Richard Liu, Jason Tai, Kalvin Cui

#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <algorithm>
#include <math.h>
#include <dataStructure.h>
#include <helperFunctions.h>
#include <numeric>
#include <chrono>
#include <thread>
#include "m2.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/color.hpp"
#include <limits>
#include <regex>
//#include "ezgl/control.hpp"
//#include "ezgl/camera.hpp"
//#include "ezgl/canvas.hpp"
//#include "rectangle.hpp"

using namespace std;

void clearMap();
void reDrawMap(ezgl::application *app);
void draw_main_canvas(ezgl::renderer *g);

void initial_setup(ezgl::application *application, bool new_window);
void initializeIntersections();
void initializePOIs();
void initializeFeatures();
void initializeStreetSegment_data();
void initializeOSMFeatures();
void initializePathnames();
void initializeFont();

void draw_street_segment(ezgl::renderer *g, StreetSegmentIdx street_segment_id);
void draw_all_street_segments(ezgl::renderer *g);
void draw_street_segment_one_way(ezgl::renderer *g);
void draw_intersections(ezgl::renderer *g);
void draw_features(ezgl::renderer *g);
void draw_icon(ezgl::renderer *g);
void draw_scale(ezgl::renderer *g);
void draw_straight_one_way(ezgl::renderer *g, StreetSegmentIdx street_segment_id);
void drawArrow(double xStart, double xEnd, double yStart, double yEnd, double distance, ezgl::renderer *g);

//DIPSLAY FUNCTIONS
void display_pois(ezgl::renderer *g);
void display_all_street_names(ezgl::renderer *g);
void display_street_name(ezgl::renderer *g, StreetIdx street_id);
void display_street_segment_name(ezgl::renderer *g, StreetSegmentIdx street_segment_id);
void display_sub_street_segment(ezgl::renderer *g, StreetSegmentIdx street_segment_id);


//HELPER FUNCTIONS
void change_map(string locationname, ezgl::application* app);
void findXYAdd(double *addX, double *addY, double xDiff, double yDiff, int arrowSize);
void highlight_intersection(int idx);
void calculate_latAvg();
double calculateAngle(ezgl::point2d p1, ezgl::point2d p2);
void highlight_intersections_between_2_streets(string street1, string street2, ezgl::application* app);
bool world_contains_point(ezgl::rectangle world, vector<ezgl::point2d> points);
void findZoomLevel(double zoomFactor);
void setZoomLevel(double left, double right);
string getOSMWayTagValue(OSMID OSMid, string key);


// UI callback
void highlight_intersection_cbk(GtkSwitch */*self*/, gboolean state, ezgl::application *application);
void create_find_button_cbk(GtkWidget* /*widget*/, ezgl::application* app);
void dialog_cbk(GtkDialog* self, gint response_id, ezgl::application* app);
void create_change_map_button_cbk(GtkWidget* /*widget*/, ezgl::application* app);
void clear_button_cbk(GtkWidget* /*widget*/, ezgl::application* app);
void nightModeSwitch_cbk(GtkSwitch */*self*/, gboolean state, ezgl::application *application);
void display_icon_cbk(GtkSwitch */*self*/, gboolean state, ezgl::application *application);

// Event Callback
void act_on_mouse_press(ezgl::application *application, GdkEventButton *event, double x, double y);

//structs

struct Inter_data {
    ezgl::point2d xy_loc;
    string name;
    bool highlight;
};

struct POI_data {
    string type;
    string name;
    ezgl::point2d xy_loc;
};

struct Feature_data {
    string name;
    FeatureType type;
    int points;
    bool open;
    double length;
    vector <ezgl::point2d> xy_loc;
    ezgl::point2d center;
};

struct Street_segment_data {
    string type;
    string name;
    IntersectionIdx from;
    IntersectionIdx to;
    OSMID wayOSMID;
    bool oneWay;
    int numCurvePoints;
    float speedLimit;
    StreetIdx streetID;
    vector <ezgl::point2d> curvePointsPoint2d;
    ezgl::point2d start;
    ezgl::point2d end;
};

//GLOBAL VARIABLES 

//  colors
//  daytime colors
ezgl::color BACKGROUND{226, 223, 211};
ezgl::color WATER{151, 192, 223};
ezgl::color NATURE{198, 216, 162};
ezgl::color BEACHES{193, 178, 132};
ezgl::color BUILDINGS{227, 209, 184};
ezgl::color LIGHTER_NATURE{208, 217, 184};
ezgl::color HIGHWAY{250, 214, 124};
ezgl::color ROAD{0xFF, 0xFF, 0xFF};
ezgl::color STREETNAMES{30, 30, 30}; // change
ezgl::color STORENAMES{30, 30, 30}; // change
ezgl::color SCALE{0, 0, 0};

string font = "OpenSans";

ezgl::rectangle current_world;

double min_x;

double max_x;

double min_y;

double max_y;

double x_diff;

multimap <double, ezgl::point2d> RestaurantMap;

multimap <double, ezgl::point2d> SupermarketMap;

multimap <double, ezgl::point2d> ShopMap;

multimap <double, ezgl::point2d> HotelMap;

multimap <double, ezgl::point2d> SchoolMap;

multimap <double, ezgl::point2d> HospitalMap;

vector <int> xMaxText;

vector <int> xMinText;

vector <int> yMaxText;

vector <int> yMinText;

double ZOOMLEVEL = numeric_limits<int>::max();

double screenX;

double screenY;

int STREETNAMEDENSITY;

vector <Inter_data> intersections;

vector <POI_data> pois;

multimap <double, POIIdx> PoiMap;

double latAvg = 0;

vector <Street_segment_data> streetSegmentData;

multimap <double, StreetSegmentIdx> MotorWayMinXMap;

multimap <double, StreetSegmentIdx> MotorWayMaxXMap;

multimap <double, StreetSegmentIdx> PrimaryRoadMinXMap;

multimap <double, StreetSegmentIdx> PrimaryRoadMaxXMap;

multimap <double, StreetSegmentIdx> SecondaryRoadMinXMap;

multimap <double, StreetSegmentIdx> SecondaryRoadMaxXMap;

multimap <double, StreetSegmentIdx> TertiaryRoadMinXMap;

multimap <double, StreetSegmentIdx> TertiaryRoadMaxXMap;

multimap <double, StreetSegmentIdx> ResidentialRoadMinXMap;

multimap <double, StreetSegmentIdx> ResidentialRoadMaxXMap;

multimap <double, StreetSegmentIdx> OtherRoadMinXMap;

multimap <double, StreetSegmentIdx> OtherRoadMaxXMap;

double longestMotorWay = 0;

double longestPrimary = 0;

double longestSecondary = 0;

double longestTertiary = 0;

double longestResidential = 0;

double longestOther = 0;

vector <Street_segment_data> MotorwayAndPrimarySegments;

vector <Street_segment_data> SecondarySegments;

vector <Street_segment_data> TertiarySegments;

vector <Street_segment_data> OtherSegments;

bool highlightintersectiontoggle = false;

vector <IntersectionIdx> highlightintersectionidx;

vector <string> mapnames;

bool nightModeOn = false;

bool displayicontoggle = false;

vector <Feature_data> features;

unordered_map <string, string> pathnames;

string current_map_path;

set <StreetSegmentIdx> streets_to_draw;

//Initializing Global Variables and Data Structures

void calculate_latAvg() {
    double latSum = 0;
    for (int i = 0; i < getNumIntersections(); i++) {
        latSum += fastData.intersection_latlon[i].latitude();
    }
    latAvg = latSum / getNumIntersections();
}

double calculateAngle(ezgl::point2d p1, ezgl::point2d p2) {
    double deltaX = p2.x - p1.x;
    double deltaY = p2.y - p1.y;

    if (deltaX == 0) {
        return 0;
    }

    return atan(deltaY / deltaX)* (1 / kDegreeToRadian);
}

void initializeIntersections() {
    intersections.resize(getNumIntersections());
    for (int i = 0; i < getNumIntersections(); i++) {
        double x = getXCoord(fastData.intersection_latlon[i].longitude(), latAvg);
        double y = getYCoord(fastData.intersection_latlon[i].latitude());
        min_x = min(min_x, x);
        max_x = max(max_x, x);
        min_y = min(min_y, y);
        max_y = max(max_y, y);
        intersections[i].xy_loc = ezgl::point2d(x, y);
        intersections[i].name = getIntersectionName(i);
        intersections[i].highlight = false;
    }
}

void initializePOIs() {
    pois.resize(getNumPointsOfInterest());
    for (int i = 0; i < getNumPointsOfInterest(); i++) {
        double x = getXCoord(getPOIPosition(i).longitude(), latAvg);
        double y = getYCoord(getPOIPosition(i).latitude());
        min_x = min(min_x, x);
        max_x = max(max_x, x);
        min_y = min(min_y, y);
        max_y = max(max_y, y);
        pois[i].type = getPOIType(i);
        pois[i].name = getPOIName(i);
        pois[i].xy_loc = ezgl::point2d(x, y);
        PoiMap.insert(pair<double, POIIdx> (x, i));
    }
}

void initializeFeatures() {
    features.resize(getNumFeatures());
    for (int i = 0; i < getNumFeatures(); i++) {
        double sumx = 0;
        double sumy = 0;
        features[i].name = getFeatureName(i);
        features[i].type = getFeatureType(i);
        features[i].points = getNumFeaturePoints(i);
        for (int j = 0; j < features[i].points; j++) {
            LatLon featurepoint = getFeaturePoint(i, j);
            double x = getXCoord(featurepoint.longitude(), latAvg);
            double y = getYCoord(featurepoint.latitude());
            min_x = min(min_x, x);
            max_x = max(max_x, x);
            min_y = min(min_y, y);
            max_y = max(max_y, y);
            sumx += x;
            sumy += y;
            features[i].xy_loc.push_back(ezgl::point2d(x, y));
        }

        if (findFeatureArea(i) == 0) {
            features[i].open = true;
            //features[i].center = NULL;
            double totallength = 0;
            for (int j = 0; j < features[i].points - 1; j++) {
                LatLon featurepoint1 = getFeaturePoint(i, j);
                LatLon featurepoint2 = getFeaturePoint(i, j + 1);
                totallength += findDistanceBetweenTwoPoints(featurepoint1, featurepoint2);
            }
            features[i].length = totallength;
        } else {
            features[i].open = false;
            features[i].length = -1;
            sumx = sumx / features[i].points;
            sumy = sumy / features[i].points;
            features[i].center = ezgl::point2d(sumx, sumy);
        }
        if (features[i].type == PARK && findFeatureArea(i) >= 4000 && features[i].points < 20) { //ignore if the park is too small or if its a thin curvy strip
            fastData.parks.push_back(features[i].center);
        }
    }
}

void initializeStreetSegment_data() {
    streetSegmentData.resize(getNumStreetSegments());

    for (int i = 0; i < getNumStreetSegments(); i++) {
        double segmentMinX;
        double segmentMaxX;
        StreetSegmentInfo segment = fastData.street_segment_info[i];
        streetSegmentData[i].from = segment.from;
        streetSegmentData[i].to = segment.to;
        streetSegmentData[i].wayOSMID = segment.wayOSMID;
        streetSegmentData[i].name = getOSMWayTagValue(streetSegmentData[i].wayOSMID, "name");

        streetSegmentData[i].type = getOSMWayTagValue(streetSegmentData[i].wayOSMID, "highway");
        streetSegmentData[i].oneWay = segment.oneWay;
        streetSegmentData[i].numCurvePoints = segment.numCurvePoints;
        streetSegmentData[i].speedLimit = segment.speedLimit;
        streetSegmentData[i].streetID = segment.streetID;

        LatLon startLatLon = fastData.intersection_latlon[streetSegmentData[i].from];
        LatLon endLatLon = fastData.intersection_latlon[streetSegmentData[i].to];

        segmentMinX = getXCoord(startLatLon.longitude(), latAvg);
        segmentMaxX = getXCoord(startLatLon.longitude(), latAvg);

        streetSegmentData[i].start = ezgl::point2d(getXCoord(startLatLon.longitude(), latAvg), getYCoord(startLatLon.latitude()));
        streetSegmentData[i].end = ezgl::point2d(getXCoord(endLatLon.longitude(), latAvg), getYCoord(endLatLon.latitude()));

        segmentMinX = min(segmentMinX, getXCoord(endLatLon.longitude(), latAvg));
        segmentMaxX = max(segmentMaxX, getXCoord(endLatLon.longitude(), latAvg));

        for (int j = 0; j < streetSegmentData[i].numCurvePoints; j++) {
            LatLon point = fastData.street_segment_curve_points[i][j];
            double x = getXCoord(point.longitude(), latAvg);
            double y = getYCoord(point.latitude());

            segmentMinX = min(segmentMinX, x);
            segmentMaxX = max(segmentMaxX, y);

            min_x = min(min_x, x);
            max_x = max(max_x, x);
            min_y = min(min_y, y);
            max_y = max(max_y, y);
            streetSegmentData[i].curvePointsPoint2d.push_back(ezgl::point2d(x, y));
        }
        if (streetSegmentData[i].type == "motorway" || streetSegmentData[i].type == "trunk") {
            MotorWayMinXMap.insert(pair<double, StreetSegmentIdx>(segmentMinX, i));
            MotorWayMaxXMap.insert(pair<double, StreetSegmentIdx>(segmentMaxX, i));
            longestMotorWay = max(longestMotorWay, fastData.street_segment_length[i]);
        } else if (streetSegmentData[i].type == "primary" || streetSegmentData[i].type == "primary_link") {
            PrimaryRoadMinXMap.insert(pair<double, StreetSegmentIdx>(segmentMinX, i));
            PrimaryRoadMaxXMap.insert(pair<double, StreetSegmentIdx>(segmentMaxX, i));
            longestPrimary = max(longestPrimary, fastData.street_segment_length[i]);
        } else if (streetSegmentData[i].type == "secondary" || streetSegmentData[i].type == "secondary_link") {
            SecondaryRoadMinXMap.insert(pair<double, StreetSegmentIdx>(segmentMinX, i));
            SecondaryRoadMaxXMap.insert(pair<double, StreetSegmentIdx>(segmentMaxX, i));
            longestSecondary = max(longestSecondary, fastData.street_segment_length[i]);
        } else if (streetSegmentData[i].type == "tertiary" || streetSegmentData[i].type == "tertiary_link") {
            TertiaryRoadMinXMap.insert(pair<double, StreetSegmentIdx>(segmentMinX, i));
            TertiaryRoadMaxXMap.insert(pair<double, StreetSegmentIdx>(segmentMaxX, i));
            longestTertiary = max(longestTertiary, fastData.street_segment_length[i]);
        } else if (streetSegmentData[i].type == "motorway_link" || streetSegmentData[i].type == "trunk_link") {
            TertiaryRoadMinXMap.insert(pair<double, StreetSegmentIdx>(segmentMinX, i));
            TertiaryRoadMaxXMap.insert(pair<double, StreetSegmentIdx>(segmentMaxX, i));
            longestTertiary = max(longestTertiary, fastData.street_segment_length[i]);
        } else if (streetSegmentData[i].type == "residential") {
            ResidentialRoadMinXMap.insert(pair<double, StreetSegmentIdx>(segmentMinX, i));
            ResidentialRoadMaxXMap.insert(pair<double, StreetSegmentIdx>(segmentMaxX, i));
            longestResidential = max(longestResidential, fastData.street_segment_length[i]);
        } else {
            OtherRoadMinXMap.insert(pair<double, StreetSegmentIdx>(segmentMinX, i));
            OtherRoadMaxXMap.insert(pair<double, StreetSegmentIdx>(segmentMaxX, i));
            longestOther = max(longestOther, fastData.street_segment_length[i]);
        }
    }
}

void initializeOSMFeatures() {
    for (int i = 0; i < fastData.latlon_osm_restaurants.size(); i++) {
        ezgl::point2d point = getxyPoint(fastData.latlon_osm_restaurants[i], latAvg);
        fastData.osm_restaurants.push_back(point);
        RestaurantMap.insert(pair <double, ezgl::point2d>(point.x, point));
    }

    for (int i = 0; i < fastData.latlon_osm_supermarkets.size(); i++) {
        ezgl::point2d point = getxyPoint(fastData.latlon_osm_supermarkets[i], latAvg);
        fastData.osm_supermarkets.push_back(getxyPoint(fastData.latlon_osm_supermarkets[i], latAvg));
        SupermarketMap.insert(pair <double, ezgl::point2d>(point.x, point));
    }

    for (int i = 0; i < fastData.latlon_osm_shops.size(); i++) {
        ezgl::point2d point = getxyPoint(fastData.latlon_osm_shops[i], latAvg);
        fastData.osm_shops.push_back(getxyPoint(fastData.latlon_osm_shops[i], latAvg));
        ShopMap.insert(pair <double, ezgl::point2d>(point.x, point));
    }

    for (int i = 0; i < fastData.latlon_osm_hotels.size(); i++) {
        ezgl::point2d point = getxyPoint(fastData.latlon_osm_hotels[i], latAvg);
        fastData.osm_hotels.push_back(getxyPoint(fastData.latlon_osm_hotels[i], latAvg));
        HotelMap.insert(pair <double, ezgl::point2d>(point.x, point));
    }

    for (int i = 0; i < fastData.latlon_osm_schools.size(); i++) {
        ezgl::point2d point = getxyPoint(fastData.latlon_osm_schools[i], latAvg);
        fastData.osm_schools.push_back(getxyPoint(fastData.latlon_osm_schools[i], latAvg));
        SchoolMap.insert(pair <double, ezgl::point2d>(point.x, point));
    }

    for (int i = 0; i < fastData.latlon_osm_hospitals.size(); i++) {
        ezgl::point2d point = getxyPoint(fastData.latlon_osm_hospitals[i], latAvg);
        fastData.osm_hospitals.push_back(getxyPoint(fastData.latlon_osm_hospitals[i], latAvg));
        HospitalMap.insert(pair <double, ezgl::point2d>(point.x, point));
    }
}

void initializePathnames() {
    pathnames = {
        {"Beijing, China", "/cad2/ece297s/public/maps/beijing_china.streets.bin"},
        {"Cairo, Egypt", "/cad2/ece297s/public/maps/cairo_egypt.streets.bin"},
        {"Cape Town, South Africa", "/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin"},
        {"Golden Horseshoe Canada", "/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin"},
        {"Hamilton, Canada", "/cad2/ece297s/public/maps/hamilton_canada.streets.bin"},
        {"Hong Kong, China", "/cad2/ece297s/public/maps/hong-kong_china.streets.bin"},
        {"Iceland", "/cad2/ece297s/public/maps/iceland.streets.bin"},
        {"Interlaken, Switzerland", "/cad2/ece297s/public/maps/interlaken_switzerland.streets.bin"},
        {"Kyiv, Ukraine", "/cad2/ece297s/public/maps/kyiv_ukraine.streets.bin"},
        {"London, England", "/cad2/ece297s/public/maps/london_england.streets.bin"},
        {"New Delhi, India", "/cad2/ece297s/public/maps/new-delhi_india.streets.bin"},
        {"New York, USA", "/cad2/ece297s/public/maps/new-york_usa.streets.bin"},
        {"Saint Helena", "/cad2/ece297s/public/maps/saint-helena.streets.bin"},
        {"Singapore", "/cad2/ece297s/public/maps/singapore.streets.bin"},
        {"Sydney, Australia", "/cad2/ece297s/public/maps/sydney_australia.streets.bin"},
        {"Tokyo, Japan", "/cad2/ece297s/public/maps/tokyo_japan.streets.bin"},
        {"Toronto, Canada", "/cad2/ece297s/public/maps/toronto_canada.streets.bin"},
        {"Rio De Janeiro, Brazil", "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin"}
    };
}

void initializeFont() {
    if (current_map_path == "/cad2/ece297s/public/maps/cairo_egypt.streets.bin") {
        font = "Noto Sans Arabic";
    } else if (current_map_path == "/cad2/ece297s/public/maps/tokyo_japan.streets.bin" || current_map_path == "/cad2/ece297s/public/maps/beijing_china.streets.bin" || current_map_path == "/cad2/ece297s/public/maps/hong-kong_china.streets.bin") {
        font = "Noto Sans CJK SC";
    } else {
        font = "OpenSans";
    }
}

//MAIN FUCNTION

void drawMap() {
    ezgl::application::settings settings;

    // Path to the "main.ui" file that contains an XML description of the UI.
    // Edit this file with glade if you want to change the UI layout
    settings.main_ui_resource = "libstreetmap/resources/modifiedautocompletewithguitest.ui";

    // Note: the "main.ui" file has a GtkWindow called "MainWindow".
    settings.window_identifier = "MainWindow";

    // Note: the "main.ui" file has a GtkDrawingArea called "MainCanvas".
    settings.canvas_identifier = "MainCanvas";

    // Create our EZGL application.
    ezgl::application application(settings);

    // Set some parameters for the main sub-window (MainCanvas), where 
    // visualization graphics are draw. Set the callback function that will be 
    // called when the main window needs redrawing, and define the (world) 
    // coordinate system we want to draw in.
    calculate_latAvg();
    max_x = getXCoord(fastData.intersection_latlon[0].longitude(), latAvg);
    min_x = getXCoord(fastData.intersection_latlon[0].longitude(), latAvg);
    max_y = getYCoord(fastData.intersection_latlon[0].latitude());
    min_y = getYCoord(fastData.intersection_latlon[0].latitude());
    initializeIntersections();
    initializePOIs();
    initializeFeatures();
    initializeStreetSegment_data();
    initializeOSMFeatures();



    x_diff = max_x - min_x;

    ezgl::rectangle initial_world({min_x, min_y},
    {
        max_x, max_y
    });
    current_world = initial_world;
    current_map_path = fastData.pathname;
    initializeFont();
    initializePathnames();
    ZOOMLEVEL = max_x - min_x;
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);

    // Run the application until the user quits.
    // This hands over all control to the GTK runtime---after this point
    // you will only regain control based on callbacks you have setup.
    // Three callbacks can be provided to handle mouse button presses,
    // mouse movement and keyboard button presses in the graphics area,
    // respectively. Also, an initial_setup function can be passed that will
    // be called before the activation of the application and can be used
    // to create additional buttons, initialize the status message, or
    // connect added widgets to their callback functions.
    // Those callbacks are optional, so we can pass nullptr if
    // we don't need to take any action on those events

    application.run(initial_setup, act_on_mouse_press, nullptr, nullptr);

}

//Initial Setup for DrawMap

void initial_setup(ezgl::application *application, bool /*new_window*/) {

    //Setting our starting row for insertion at 6 (Default zoom/pan buttons created by EZGL take up first five rows);
    //We will increment row each time we insert a new element. 
    //int row = 6;

    //application->create_label(row++, "Map Tools: ");

    //Find Button in Glade
    GObject *findButton = application->get_object("FindButton");
    g_signal_connect(findButton, "pressed", G_CALLBACK(create_find_button_cbk), application);


    //GObject *entrygtk = application->get_object("GtkEntry");

    //GObject *entrycompletion = application ->get_object("GtkEntryCompletion");

    GtkListStore* list = GTK_LIST_STORE(application->get_object("GtkListStore"));
    //g_signal_connect (application->get_object("GtkListStore"),nullptr, nullptr, application);

    set<string> streets;
    for (int i = 0; i < streetSegmentData.size(); i++) {
        streets.insert(streetSegmentData[i].name);
    }

    GtkTreeIter iter;
    set<string>::iterator it;
    for (it = streets.begin(); it != streets.end(); it++) {

        gtk_list_store_append(list, &iter);

        gtk_list_store_set(list, &iter, 0, (*it).c_str(), -1);


    }


    GObject *highlightSwitch = application->get_object("HighlightSwitch");
    g_signal_connect(highlightSwitch, "state-set", G_CALLBACK(highlight_intersection_cbk), application);

    GObject *nightModeSwitch = application->get_object("NightModeSwitch");
    g_signal_connect(nightModeSwitch, "state-set", G_CALLBACK(nightModeSwitch_cbk), application);

    GObject *iconSwitch = application->get_object("IconSwitch");
    g_signal_connect(iconSwitch, "state-set", G_CALLBACK(display_icon_cbk), application);


    //Change Map Button in Glade
    GObject *changeMapButton = application->get_object("changeMapButton");
    g_signal_connect(changeMapButton, "pressed", G_CALLBACK(create_change_map_button_cbk), application);

    //Clear Button in Glade
    GObject *clearButton = application->get_object("clearButton");
    g_signal_connect(clearButton, "pressed", G_CALLBACK(clear_button_cbk), application);


}

//Clear Map

void clearMap() {
    intersections.clear();
    pois.clear();
    streetSegmentData.clear();
    features.clear();
    highlightintersectionidx.clear();
    pathnames.clear();
    MotorWayMinXMap.clear();
    MotorWayMaxXMap.clear();
    PrimaryRoadMinXMap.clear();
    PrimaryRoadMaxXMap.clear();
    SecondaryRoadMinXMap.clear();
    SecondaryRoadMaxXMap.clear();
    TertiaryRoadMinXMap.clear();
    TertiaryRoadMaxXMap.clear();
    ResidentialRoadMinXMap.clear();
    ResidentialRoadMaxXMap.clear();
    OtherRoadMinXMap.clear();
    OtherRoadMaxXMap.clear();
    RestaurantMap.clear();
    SupermarketMap.clear();
    ShopMap.clear();
    HotelMap.clear();
    SchoolMap.clear();
    HospitalMap.clear();
    xMaxText.clear();
    xMinText.clear();
    yMaxText.clear();
    yMinText.clear();
    PoiMap.clear();
}

//ReDraw Map

void reDrawMap(ezgl::application *app) {
    calculate_latAvg();
    max_x = getXCoord(fastData.intersection_latlon[0].longitude(), latAvg);
    min_x = getXCoord(fastData.intersection_latlon[0].longitude(), latAvg);
    max_y = getYCoord(fastData.intersection_latlon[0].latitude());
    min_y = getYCoord(fastData.intersection_latlon[0].latitude());
    initializeIntersections();
    initializePOIs();
    initializeFeatures();
    initializeStreetSegment_data();
    initializeOSMFeatures();
    initializePathnames();
    ezgl::rectangle initial_world({min_x, min_y},
    {
        max_x, max_y
    });

    x_diff = max_x - min_x;
    current_world = initial_world;
    current_map_path = fastData.pathname;
    initializeFont();
    ZOOMLEVEL = max_x - min_x;

    //RELOAD AUTOCOMPLETE
    GtkListStore* list = GTK_LIST_STORE(app->get_object("GtkListStore"));
    //g_signal_connect (application->get_object("GtkListStore"),nullptr, nullptr, application);
    gtk_list_store_clear (list);
    
    set<string> streets;
    for (int i = 0; i < streetSegmentData.size(); i++) {
        streets.insert(streetSegmentData[i].name);
    }

    GtkTreeIter iter;
    set<string>::iterator it;
    for (it = streets.begin(); it != streets.end(); it++) {

        gtk_list_store_append(list, &iter);

        gtk_list_store_set(list, &iter, 0, (*it).c_str(), -1);
    }


    std::string main_canvas_id = app->get_main_canvas_id();
    auto canvas = app->get_canvas(main_canvas_id);
    app->change_canvas_world_coordinates(main_canvas_id, initial_world);
    canvas->get_camera().set_world(initial_world);

    draw_main_canvas(app->get_renderer());
}

/**
 * The redrawing function for still pictures
 */
void draw_main_canvas(ezgl::renderer *g) {

    current_world = g->get_visible_world();

    screenX = g->get_visible_screen().right();

    screenY = g->get_visible_screen().top();

    draw_features(g);

    draw_all_street_segments(g);

    draw_street_segment_one_way(g);

    display_all_street_names(g);

    display_pois(g);

    xMaxText.clear();

    xMinText.clear();

    yMaxText.clear();

    yMinText.clear();

    draw_intersections(g);

    draw_icon(g);

    draw_scale(g);

    streets_to_draw.clear();
}

//DRAW FUNCTIONS

void draw_scale(ezgl::renderer *g) {


    double displayed_distance = ZOOMLEVEL / 10;
    double screen_distance = screenX / 10;
    int width = 5;

    g->set_coordinate_system(ezgl::SCREEN);

    //cout << screenY << endl;

    g->set_color(SCALE);
    g->draw_line({screenX - (screenX / 10)*2, screenY - 100},
    {
        screenX - (screenX / 10)*2 + screen_distance, screenY - 100
    });
    g->draw_line({screenX - (screenX / 10)*2, screenY - 100 - width},
    {
        screenX - (screenX / 10)*2, screenY - 100 + width
    });
    g->draw_line({screenX - (screenX / 10)*2 + screen_distance, screenY - 100 - width},
    {
        screenX - (screenX / 10)*2 + screen_distance, screenY - 100 + width
    });

    g->set_text_rotation(0);
    g->set_font_size(15);
    g->set_color(SCALE);
    g->draw_text({screenX - (screenX / 10)*2 + screenX / 20, screenY - 100 + 20}, to_string(((int) (round(displayed_distance)))) + " m");

    g->set_coordinate_system(ezgl::WORLD);

}

void draw_icon(ezgl::renderer *g) {
    if (displayicontoggle) {
        int ICONDENSITY;
        if (ZOOMLEVEL <= x_diff / 15) {
            ICONDENSITY = 30;
        }
        if (ZOOMLEVEL <= x_diff / 50) {
            ICONDENSITY = 10;
        }
        if (ZOOMLEVEL <= x_diff / 75) {
            ICONDENSITY = 2;
        }
        if (ZOOMLEVEL <= x_diff / 100) {
            ICONDENSITY = 1;
        }

        if (ZOOMLEVEL < x_diff / 15) {
            int count = 0;
            for (auto i = RestaurantMap.lower_bound(current_world.left()); i != RestaurantMap.end(); i++) {
                if (i->first > current_world.right()) {
                    break;
                }
                if (current_world.contains(i->second)) {
                    count++;
                    if (count == ICONDENSITY) {
                        count = 0;
                        g -> draw_surface(fastData.restaurantIcon, i->second, 0.5);
                    }
                }
            }

            count = 0;
            for (auto i = SupermarketMap.lower_bound(current_world.left()); i != SupermarketMap.end(); i++) {
                if (i->first > current_world.right()) {
                    break;
                }
                if (current_world.contains(i->second)) {
                    count++;
                    if (count == ICONDENSITY) {
                        count = 0;
                        g -> draw_surface(fastData.supermarketIcon, i->second, 0.75);
                    }
                }

            }

            count = 0;
            for (auto i = ShopMap.lower_bound(current_world.left()); i != ShopMap.end(); i++) {
                if (i->first > current_world.right()) {
                    break;
                }
                if (current_world.contains(i->second)) {
                    count++;
                    if (count == ICONDENSITY) {
                        count = 0;
                        g -> draw_surface(fastData.shopsIcon, i->second, 0.75);
                    }
                }

            }

            count = 0;
            for (auto i = HotelMap.lower_bound(current_world.left()); i != HotelMap.end(); i++) {
                if (i->first > current_world.right()) {
                    break;
                }
                if (current_world.contains(i->second)) {
                    count++;
                    if (count == ICONDENSITY) {
                        count = 0;
                        g -> draw_surface(fastData.hotelIcon, i->second, 0.8);
                    }
                }


            }

            count = 0;

            for (auto i = SchoolMap.lower_bound(current_world.left()); i != SchoolMap.end(); i++) {
                if (i->first > current_world.right()) {
                    break;
                }
                if (current_world.contains(i->second)) {
                    count++;
                    if (count == ICONDENSITY) {
                        count = 0;
                        g -> draw_surface(fastData.schoolIcon, i->second, 0.75);
                    }
                }

            }

            count = 0;
            for (int i = 0; i < fastData.parks.size(); i++) {
                count++;
                if (count == ICONDENSITY) {
                    count = 0;
                    ezgl::point2d tempPoint = fastData.parks[i];
                    g -> draw_surface(fastData.parkIcon, tempPoint, 0.75);
                }

            }

            count = 0;
            for (auto i = HospitalMap.lower_bound(current_world.left()); i != HospitalMap.end(); i++) {

                if (i->first > current_world.right()) {
                    break;
                }
                if (current_world.contains(i->second)) {

                    g -> draw_surface(fastData.hospitalIcon, i->second, 0.9);
                }
            }
        }
    }
}


//Draw all highlighted street segments

void draw_intersections(ezgl::renderer *g) {

    //Radius of the circle
    float radius = x_diff / 100;
    //Transculent Red
    g->set_color(255, 0, 0, 100);

    //Adjust radius of circle depending on zoom level
    if (ZOOMLEVEL < x_diff / 1.2) {
        radius = x_diff / (2 * 120);
    }
    if (ZOOMLEVEL < x_diff / 2) {
        radius = x_diff / (2 * 200);
    }

    if (ZOOMLEVEL < x_diff / 7.5) {
        radius = x_diff / (2 * 750);
    }
    if (ZOOMLEVEL < x_diff / 15) {
        radius = x_diff / (2 * 1500);
    }
    if (ZOOMLEVEL < x_diff / 30) {
        radius = x_diff / (2 * 3000);
    }

    //Loop through all intersections that are to be highlighted to highlight them
    for (int idx = 0; idx < highlightintersectionidx.size(); idx++) {
        if (intersections[highlightintersectionidx[idx]].highlight) {
            g->fill_arc(intersections[highlightintersectionidx[idx]].xy_loc, radius, 0, 360);
        }
    }

}


//Function to determine which street segments to draw based on current screen size and zoom level

void draw_all_street_segments(ezgl::renderer *g) {
    //DRAW HIGHWAYS REGARDLESS OF ZOOM
    auto itr = MotorWayMinXMap.lower_bound(current_world.left() - longestMotorWay);
    //cout << itr->first;
    while (itr != MotorWayMinXMap.end()) {
        if (itr->first > current_world.right()) {
            break;
        }
        streets_to_draw.insert(itr->second);
        itr++;
    }
    itr = MotorWayMaxXMap.lower_bound(current_world.right() + longestMotorWay);
    while (itr != MotorWayMaxXMap.begin()) {
        if (itr->first < current_world.left()) {
            break;
        }
        streets_to_draw.insert(itr->second);
        itr--;
    }
    //DRAW PRIMARY ROADS IF ZOOM LEVEL <= x_diff / 1.2
    if (ZOOMLEVEL < x_diff / 1.2) {
        itr = PrimaryRoadMinXMap.lower_bound(current_world.left() - longestPrimary);
        while (itr != PrimaryRoadMinXMap.end()) {
            if (itr->first > current_world.right()) {
                break;
            }
            streets_to_draw.insert(itr->second);
            itr++;
        }
        itr = PrimaryRoadMaxXMap.lower_bound(current_world.right() + longestPrimary);
        while (itr != PrimaryRoadMaxXMap.begin()) {
            if (itr->first < current_world.left()) {
                break;
            }
            streets_to_draw.insert(itr->second);
            itr--;
        }
    }

    //DRAW SECONDARY ROADS IF ZOOM LEVEL <= x_diff / 2
    if (ZOOMLEVEL < x_diff / 2) {
        itr = SecondaryRoadMinXMap.lower_bound(current_world.left() - longestSecondary);
        while (itr != SecondaryRoadMinXMap.end()) {
            if (itr->first > current_world.right()) {
                break;
            }
            streets_to_draw.insert(itr->second);
            itr++;
        }
        itr = SecondaryRoadMaxXMap.lower_bound(current_world.right() + longestSecondary);
        while (itr != SecondaryRoadMaxXMap.begin()) {
            if (itr->first < current_world.left()) {
                break;
            }
            streets_to_draw.insert(itr->second);
            itr--;
        }
    }

    //DRAW TERTIARY ROADS IF ZOOM LEVEL <= x_diff / 7.5
    if (ZOOMLEVEL < x_diff / 7.5) {
        itr = TertiaryRoadMinXMap.lower_bound(current_world.left() - longestTertiary);
        while (itr != TertiaryRoadMinXMap.end()) {
            if (itr->first > current_world.right()) {
                break;
            }
            streets_to_draw.insert(itr->second);
            itr++;
        }
        itr = TertiaryRoadMaxXMap.lower_bound(current_world.right() + longestTertiary);
        while (itr != TertiaryRoadMaxXMap.begin()) {
            if (itr->first < current_world.left()) {
                break;
            }
            streets_to_draw.insert(itr->second);
            itr--;
        }
    }

    //DRAW RESIDENTIAL ROADS IF ZOOM LEVEL <= x_diff / 15
    if (ZOOMLEVEL < x_diff / 15) {
        itr = ResidentialRoadMinXMap.lower_bound(current_world.left() - longestResidential);
        while (itr != MotorWayMinXMap.end()) {
            if (itr->first > current_world.right()) {
                break;
            }
            streets_to_draw.insert(itr->second);
            itr++;
        }
        itr = ResidentialRoadMaxXMap.lower_bound(current_world.right() + longestResidential);
        while (itr != MotorWayMaxXMap.begin()) {
            if (itr->first < current_world.left()) {
                break;
            }
            streets_to_draw.insert(itr->second);
            itr--;
        }
    }

    //DRAW ALL OTHER ROADS IF ZOOM LEVEL <= x_diff / 30
    if (ZOOMLEVEL < x_diff / 30) {
        itr = OtherRoadMinXMap.lower_bound(current_world.left() - longestOther);
        while (itr != OtherRoadMinXMap.end()) {
            if (itr->first > current_world.right()) {
                break;
            }
            streets_to_draw.insert(itr->second);
            itr++;
        }
        itr = OtherRoadMaxXMap.lower_bound(current_world.right() + longestOther);
        while (itr != OtherRoadMaxXMap.begin()) {
            if (itr->first < current_world.left()) {
                break;
            }
            streets_to_draw.insert(itr->second);
            itr--;
        }
    }
    //Iterate across set, drawing all street segments in the set
    for (auto it = streets_to_draw.begin(); it != streets_to_draw.end(); it++) {
        draw_street_segment(g, *it);
    }
}

//Draw street segment

void draw_street_segment(ezgl::renderer *g, StreetSegmentIdx street_segment_id) {


    int width;
    g->set_color(ROAD);

    //Set color and width of street segment depending on it's type
    if (streetSegmentData[street_segment_id].type == "motorway" || streetSegmentData[street_segment_id].type == "trunk") {
        width = 5;
        g->set_color(HIGHWAY);
    } else if (streetSegmentData[street_segment_id].type == "motorway_link" || streetSegmentData[street_segment_id].type == "trunk_link") {
        width = 2;
    } else if (streetSegmentData[street_segment_id].type == "primary" || streetSegmentData[street_segment_id].type == "primary_link") {
        width = 4;
    } else if (streetSegmentData[street_segment_id].type == "secondary" || streetSegmentData[street_segment_id].type == "secondary_link") {
        width = 3;
    } else if (streetSegmentData[street_segment_id].type == "tertiary" || streetSegmentData[street_segment_id].type == "tertiary_link") {
        width = 2;
    } else if (streetSegmentData[street_segment_id].type == "residential") {
        width = 2;
    } else {
        width = 2;
    }

    int widthfactor = 0;

    if (ZOOMLEVEL < x_diff / 70) {
        widthfactor = 5;
    } else if (ZOOMLEVEL < x_diff / 30) {
        widthfactor = 3;
    } else if (ZOOMLEVEL < x_diff / 10) {
        widthfactor = 2;
    } else {
        widthfactor = 1;
    }



    g->set_line_cap(ezgl::line_cap::round);
    g->set_line_width(width * widthfactor);

    if (streetSegmentData[street_segment_id].curvePointsPoint2d.size() == 0) {
        g->draw_line(streetSegmentData[street_segment_id].start, streetSegmentData[street_segment_id].end);

    } else {
        for (int pointNum = 0; pointNum < streetSegmentData[street_segment_id].curvePointsPoint2d.size() - 1; ++pointNum) {
            //Check if either of the points lie in the current screen. If so draw the line connecting them
            if (world_contains_point(current_world,{streetSegmentData[street_segment_id].curvePointsPoint2d[pointNum], streetSegmentData[street_segment_id].curvePointsPoint2d[pointNum + 1]})) {
                g->draw_line(streetSegmentData[street_segment_id].curvePointsPoint2d[pointNum], streetSegmentData[street_segment_id].curvePointsPoint2d[pointNum + 1]);
            }
        }

        //Check if either of the points lie in the current screen. If so draw the line connecting start to first curve point and last curve point to end

        if (world_contains_point(current_world,{streetSegmentData[street_segment_id].start, streetSegmentData[street_segment_id].curvePointsPoint2d[0]})) {
            g->draw_line(streetSegmentData[street_segment_id].start, streetSegmentData[street_segment_id].curvePointsPoint2d[0]);
        }
        if (world_contains_point(current_world,{streetSegmentData[street_segment_id].curvePointsPoint2d[streetSegmentData[street_segment_id].curvePointsPoint2d.size() - 1], streetSegmentData[street_segment_id].end})) {
            g->draw_line(streetSegmentData[street_segment_id].curvePointsPoint2d[streetSegmentData[street_segment_id].curvePointsPoint2d.size() - 1], streetSegmentData[street_segment_id].end);
        }
    }
}

void drawArrow(double xStart, double xEnd, double yStart, double yEnd, double distance, ezgl::renderer *g) {
    double spacing = 4; //consider putting as CONSTS
    double arrowSize = 3.5;

    ezgl::point2d start = ezgl::point2d(xStart, yStart);
    ezgl::point2d end = ezgl::point2d(xEnd, yEnd);
    if (!world_contains_point(current_world,{start, end})) {
        return;
    }
    double lengthOfArrows = 25;

    double ratio = lengthOfArrows / distance;
    double addX, addY;

    if (xEnd - xStart > 0 && yEnd - yStart > 0) { // Q1
        double xDiff = (xEnd - xStart) * ratio;
        double yDiff = (yEnd - yStart) * ratio;
        xStart += xDiff;
        yStart += yDiff;
        while (xStart + xDiff * spacing < xEnd && yStart + yDiff * spacing < yEnd) {
            g->draw_line(ezgl::point2d(xStart, yStart), ezgl::point2d(xStart + xDiff, yStart + yDiff));

            findXYAdd(&addX, &addY, xDiff, yDiff, arrowSize);

            g->fill_poly({
                {xStart + xDiff, yStart + yDiff},
                {xStart + 0.7 * xDiff + addX, yStart + 0.7 * yDiff - addY},
                {xStart + xDiff * 0.7 - addX, yStart + yDiff * 0.7 + addY}
            });

            xStart += xDiff * (spacing - 1);
            yStart += yDiff * (spacing - 1);
        }

    } else if (xEnd - xStart > 0 && yEnd - yStart < 0) { //Q2
        double xDiff = (xEnd - xStart) * ratio;
        double yDiff = (yStart - yEnd) * ratio;
        xStart += xDiff;
        yStart -= yDiff;
        while (xStart + xDiff * spacing < xEnd && yStart - yDiff * spacing > yEnd) {
            g->draw_line(ezgl::point2d(xStart, yStart), ezgl::point2d(xStart + xDiff, yStart - yDiff));

            findXYAdd(&addX, &addY, xDiff, yDiff, arrowSize);

            g->fill_poly({
                {xStart + xDiff, yStart - yDiff},
                {xStart + 0.7 * xDiff + addX, yStart - 0.7 * yDiff + addY},
                {xStart + xDiff * 0.7 - addX, yStart - yDiff * 0.7 - addY}
            });

            xStart += xDiff * (spacing - 1);
            yStart -= yDiff * (spacing - 1);
        }
    } else if (xEnd - xStart < 0 && yEnd - yStart < 0) { //Q3
        double xDiff = (xStart - xEnd) * ratio;
        double yDiff = (yStart - yEnd) * ratio;
        xStart -= xDiff;
        yStart -= yDiff;
        while (xStart - xDiff * spacing > xEnd && yStart - yDiff * spacing > yEnd) {
            g->draw_line(ezgl::point2d(xStart, yStart), ezgl::point2d(xStart - xDiff, yStart - yDiff));

            findXYAdd(&addX, &addY, xDiff, yDiff, arrowSize);

            g->fill_poly({
                {xStart - xDiff, yStart - yDiff},
                {xStart - 0.7 * xDiff - addX, yStart - 0.7 * yDiff + addY},
                {xStart - xDiff * 0.7 + addX, yStart - yDiff * 0.7 - addY}
            });

            xStart -= xDiff * (spacing - 1);
            ;
            yStart -= yDiff * (spacing - 1);
            ;
        }
    } else { //Q4
        double xDiff = (xStart - xEnd) * ratio;
        double yDiff = (yEnd - yStart) * ratio;
        xStart -= xDiff;
        yStart += yDiff;
        while (xStart - xDiff * spacing > xEnd && yStart + yDiff * spacing < yEnd) {
            g->draw_line(ezgl::point2d(xStart, yStart), ezgl::point2d(xStart - xDiff, yStart + yDiff));

            findXYAdd(&addX, &addY, xDiff, yDiff, arrowSize);

            g->fill_poly({
                {xStart - xDiff, yStart + yDiff},
                {xStart - 0.7 * xDiff - addX, yStart + 0.7 * yDiff - addY},
                {xStart - xDiff * 0.7 + addX, yStart + yDiff * 0.7 + addY}
            });

            xStart -= xDiff * (spacing - 1);
            ;
            yStart += yDiff * (spacing - 1);
            ;

        }
    }
}

void draw_straight_one_way(ezgl::renderer *g, StreetSegmentIdx street_segment_id) {
    IntersectionIdx from = streetSegmentData[street_segment_id].from;
    IntersectionIdx to = streetSegmentData[street_segment_id].to;

    double xStart = intersections[from].xy_loc.x;
    double xEnd = intersections[to].xy_loc.x;
    double yStart = intersections[from].xy_loc.y;
    double yEnd = intersections[to].xy_loc.y;

    drawArrow(xStart, xEnd, yStart, yEnd, findStreetSegmentLength(street_segment_id), g);
}

void draw_street_segment_one_way(ezgl::renderer *g) {
    g->set_line_width(2);
    g->set_color(ezgl::RED);
    if (ZOOMLEVEL < x_diff / 25) {
        for (auto itr = streets_to_draw.begin(); itr != streets_to_draw.end(); itr++) {
            bool oneWay = streetSegmentData[*itr].oneWay;
            if (oneWay) {
                if (streetSegmentData[*itr].numCurvePoints == 0) {
                    draw_straight_one_way(g, *itr);
                } else {
                    for (int pointNum = 0; pointNum < streetSegmentData[*itr].numCurvePoints - 1; pointNum++) {
                        double distance = pow(pow(streetSegmentData[*itr].curvePointsPoint2d[pointNum].x - streetSegmentData[*itr].curvePointsPoint2d[pointNum + 1].x, 2) + pow(streetSegmentData[*itr].curvePointsPoint2d[pointNum].y - streetSegmentData[*itr].curvePointsPoint2d[pointNum + 1].y, 2), 0.5);
                        drawArrow(streetSegmentData[*itr].curvePointsPoint2d[pointNum].x, streetSegmentData[*itr].curvePointsPoint2d[pointNum + 1].x, streetSegmentData[*itr].curvePointsPoint2d[pointNum].y, streetSegmentData[*itr].curvePointsPoint2d[pointNum + 1].y, distance, g);
                    }
                }
            }
        }
    }
}

//Draw features

void draw_features(ezgl::renderer *g) {
    //  set background color
    ezgl::rectangle visible_world = g->get_visible_world();
    g->set_color(BACKGROUND);
    g->fill_rectangle(visible_world);
    //Loop through every feature
    for (size_t id = 0; id < features.size(); id++) {
        //If feature is only 1 point or less, do not draw it
        if (features[id].xy_loc.size() <= 1) {
            continue;
            //Check to see if feature is on the screen or not, if not do not draw
        } else if (!world_contains_point(current_world, features[id].xy_loc)) {
            continue;
            //Set color of feature depending on what it is
        } else {
            if (features[id].type == PARK) {
                g->set_color(NATURE);
            } else if (features[id].type == BEACH) {
                g->set_color(BEACHES);
            } else if (features[id].type == LAKE) {
                g->set_color(WATER);
            } else if (features[id].type == RIVER) {
                g->set_color(WATER);
            } else if (features[id].type == ISLAND) {
                g->set_color(NATURE);
            } else if (features[id].type == BUILDING) {
                g->set_color(BUILDINGS);
            } else if (features[id].type == GREENSPACE) {
                g->set_color(NATURE);
            } else if (features[id].type == GOLFCOURSE) {
                g->set_color(LIGHTER_NATURE);
            } else if (features[id].type == STREAM) {
                g->set_color(WATER);
            } else if (features[id].type == GLACIER) {
                g->set_color(ezgl::CYAN);
            } else {
                g->set_color(ezgl::WHITE);
            }
            //Draw open features depending on zoom level and length
            if (features[id].open) {
                if (features[id].length < 1000 && ZOOMLEVEL > x_diff / 24) {
                    continue;
                }
                if (features[id].length < 5000 && ZOOMLEVEL > x_diff / 8) {
                    continue;
                }
                if (features[id].length < 10000 && ZOOMLEVEL > x_diff / 3) {
                    continue;
                }
                if (features[id].length < 50000 && ZOOMLEVEL > x_diff / 1.2) {
                    continue;
                }
                for (int i = 0; i < features[id].xy_loc.size() - 1; i++) {
                    g->draw_line(features[id].xy_loc[i], features[id].xy_loc[i + 1]);
                }
                //Draw closed features depending on it's type, size, and current zoom level
            } else {
                if (features[id].type == BUILDING) {
                    if (ZOOMLEVEL < x_diff / 50) {
                        g->fill_poly(features[id].xy_loc);
                    }
                } else {
                    if (fastData.featureArea[id] < 1000) {
                        if (ZOOMLEVEL < x_diff / 20) {
                            g->fill_poly(features[id].xy_loc);
                        }
                        continue;
                    }
                    if (fastData.featureArea[id] < 5000) {
                        if (ZOOMLEVEL < x_diff / 12) {
                            g->fill_poly(features[id].xy_loc);
                        }
                        continue;
                    }
                    if (fastData.featureArea[id] < 10000) {
                        if (ZOOMLEVEL < x_diff / 6) {
                            g->fill_poly(features[id].xy_loc);
                        }
                        continue;
                    }
                    if (fastData.featureArea[id] < 50000) {
                        if (ZOOMLEVEL < x_diff / 2) {
                            g->fill_poly(features[id].xy_loc);
                        }
                        continue;
                    }
                    if (fastData.featureArea[id] < 100000) {
                        if (ZOOMLEVEL < x_diff / 1.1) {
                            g->fill_poly(features[id].xy_loc);
                        }
                        continue;
                    }
                    g->fill_poly(features[id].xy_loc);
                }

            }
        }
    }
}


//DISPLAY FUNCTIONS

//Display names of point of interests ex. store names

void display_pois(ezgl::renderer *g) {
    //Set the font, color, size, and rotation
    g->format_font(font, ezgl::font_slant::normal, ezgl::font_weight::normal, 10);
    g->set_color(STORENAMES);
    g->set_font_size(12);
    g->set_text_rotation(0);


    //Only display POIs past a certain zoom level
    if (ZOOMLEVEL < x_diff / 75) {
        //Iterate across PoiMap to ensure POI is in the current visible world
        auto itr = PoiMap.lower_bound(current_world.left());
        while (itr != PoiMap.end()) {
            //Once X-coordinate of PoiMap is past the maximum x coordinate, stop iterating the map
            if (itr->first > current_world.right()) {
                break;
            }
            //If the current visible world contains the POI location, display it
            if (current_world.contains(pois[itr->second].xy_loc)) {
                g->draw_text_unclipped_bounded(pois[itr->second].xy_loc, pois[itr->second].name, 30, xMaxText, xMinText, yMaxText, yMinText);
            }
            //Increment iterator
            itr++;
        }

    }
}

void display_all_street_names(ezgl::renderer *g) {
    for (auto itr = streets_to_draw.begin(); itr != streets_to_draw.end(); itr++) {
        display_sub_street_segment(g, *itr);
    }
}

void display_sub_street_segment(ezgl::renderer *g, StreetSegmentIdx street_segment_id) {


    g->format_font(font, ezgl::font_slant::normal, ezgl::font_weight::normal);
    g->set_color(STREETNAMES);
    if (streetSegmentData[street_segment_id].curvePointsPoint2d.size() == 0) {
        //  draw text from start to end

        double angle = calculateAngle(streetSegmentData[street_segment_id].start, streetSegmentData[street_segment_id].end);
        if (angle > 90) {
            return;
        }

        int length = getDistanceBetweenTwoPoints(streetSegmentData[street_segment_id].start, streetSegmentData[street_segment_id].end);

        g->set_text_rotation(angle);
        ezgl::point2d midpoint = getMidpoint(streetSegmentData[street_segment_id].start, streetSegmentData[street_segment_id].end);
        g->draw_text_unclipped_bounded(midpoint, streetSegmentData[street_segment_id].name, length, xMaxText, xMinText, yMaxText, yMinText);


    } else if (streetSegmentData[street_segment_id].curvePointsPoint2d.size() > 1) {

        g->set_color(STREETNAMES);
        ezgl::point2d midpointStart = getMidpoint(streetSegmentData[street_segment_id].start, streetSegmentData[street_segment_id].curvePointsPoint2d[0]);
        double angleStart = calculateAngle(streetSegmentData[street_segment_id].start, streetSegmentData[street_segment_id].curvePointsPoint2d[0]);
        g->set_text_rotation(angleStart);

        int lengthStart = getDistanceBetweenTwoPoints(streetSegmentData[street_segment_id].start, streetSegmentData[street_segment_id].curvePointsPoint2d[0]);
        if (lengthStart <= 10) {
            return;
        }

        if (streetSegmentData[street_segment_id].type != "motorway" && streetSegmentData[street_segment_id].type != "primary" && streetSegmentData[street_segment_id].type != "secondary") {

            g->draw_text_unclipped_bounded(midpointStart, streetSegmentData[street_segment_id].name, lengthStart, xMaxText, xMinText, yMaxText, yMinText);
        } else {
            g->draw_text_unclipped(midpointStart, streetSegmentData[street_segment_id].name, xMaxText, xMinText, yMaxText, yMinText);
        }


        g->set_color(STREETNAMES);
        ezgl::point2d midpointEnd = getMidpoint(streetSegmentData[street_segment_id].end, streetSegmentData[street_segment_id].curvePointsPoint2d[streetSegmentData[street_segment_id].curvePointsPoint2d.size() - 1]);
        double angleEnd = calculateAngle(streetSegmentData[street_segment_id].end, streetSegmentData[street_segment_id].curvePointsPoint2d[streetSegmentData[street_segment_id].curvePointsPoint2d.size() - 1]);
        g->set_text_rotation(angleEnd);

        int lengthEnd = getDistanceBetweenTwoPoints(streetSegmentData[street_segment_id].end, streetSegmentData[street_segment_id].curvePointsPoint2d[streetSegmentData[street_segment_id].curvePointsPoint2d.size() - 1]);
        if (lengthEnd <= 10) {
            return;
        }

        if (streetSegmentData[street_segment_id].type != "motorway" && streetSegmentData[street_segment_id].type != "primary" && streetSegmentData[street_segment_id].type != "secondary") {

            g->draw_text_unclipped_bounded(midpointEnd, streetSegmentData[street_segment_id].name, lengthEnd, xMaxText, xMinText, yMaxText, yMinText);
        } else {
            g->draw_text_unclipped(midpointEnd, streetSegmentData[street_segment_id].name, xMaxText, xMinText, yMaxText, yMinText);
        }

        g->set_color(STREETNAMES);
        for (int pointNum = 0; pointNum < streetSegmentData[street_segment_id].curvePointsPoint2d.size() - 1; ++pointNum) {
            // draw text from curve point to curve point
            double angle = calculateAngle(streetSegmentData[street_segment_id].curvePointsPoint2d[pointNum], streetSegmentData[street_segment_id].curvePointsPoint2d[pointNum + 1]);
            g->set_text_rotation(angle);
            int length = getDistanceBetweenTwoPoints(streetSegmentData[street_segment_id].curvePointsPoint2d[pointNum], streetSegmentData[street_segment_id].curvePointsPoint2d[pointNum + 1]);
            ezgl::point2d midpoint = getMidpoint(streetSegmentData[street_segment_id].curvePointsPoint2d[pointNum], streetSegmentData[street_segment_id].curvePointsPoint2d[pointNum + 1]);

            if (streetSegmentData[street_segment_id].type != "motorway" && streetSegmentData[street_segment_id].type != "primary") {
                g->draw_text_unclipped_bounded(midpoint, streetSegmentData[street_segment_id].name, length, xMaxText, xMinText, yMaxText, yMinText);
            } else {
                g->draw_text_unclipped(midpoint, streetSegmentData[street_segment_id].name, xMaxText, xMinText, yMaxText, yMinText);
            }


        }
    }
}


//HELPER FUNCTIONS


//Helper function for drawing one way arrows

void findXYAdd(double *addX, double *addY, double xDiff, double yDiff, int arrowSize) {
    double angle = atan(yDiff / xDiff);
    angle = M_PI / 2.0 - angle;
    *addY = arrowSize * sin(angle);
    *addX = arrowSize * cos(angle);
}

//Highlight street intersection from it's index

void highlight_intersection(int idx) {
    intersections[idx].highlight = true;
    //Check to see if intersection idx is already highlighted, if not add it to the highlight vector
    if (find(highlightintersectionidx.begin(), highlightintersectionidx.end(), idx) != highlightintersectionidx.end()) {
        return;
    }
    highlightintersectionidx.push_back(idx);
}


//Function that will highlight all intersections between 2 streets

void highlight_intersections_between_2_streets(string street1, string street2, ezgl::application* app) {
    //Vectors to store the streetIDX of the 2 respective streets
    vector <StreetIdx> street1Idx;
    vector <StreetIdx> street2Idx;
    //Check if either of the entered streets are empty (User has not typed in search entry)
    if (street1.empty() || street2.empty()) {
        app->create_popup_message("Error", " Please Enter 2 Streets");
        return;
    }
    //Get all streetIds based on the entrys in the search entrys
    street1Idx = findStreetIdsFromPartialStreetName(street1);
    street2Idx = findStreetIdsFromPartialStreetName(street2);

    //If either of the vectors are empty, that means the user entered a street that does not exists
    if (street1Idx.empty() || street2Idx.empty()) {
        app->create_popup_message("Error", " Street does not exist");
        return;
    }


    //Make sure the street entered in the search bar is unique, as the search bar works with partial street name as well
    set <string> uniqueStreet1Checker;
    set <string> uniqueStreet2Checker;
    for (int i = 0; i < street1Idx.size(); i++) {
        string temp = getStreetName(street1Idx[i]);
        if (uniqueStreet1Checker.find(temp) == uniqueStreet1Checker.end()) {
            uniqueStreet1Checker.insert(temp);
        }
    }
    for (int i = 0; i < street2Idx.size(); i++) {
        string temp = getStreetName(street2Idx[i]);
        if (uniqueStreet2Checker.find(temp) == uniqueStreet2Checker.end()) {
            uniqueStreet2Checker.insert(temp);
        }
    }

    //If there are more than 1 street that starts with what the user entered, street is not unique
    if (uniqueStreet1Checker.size() != 1 || uniqueStreet2Checker.size() != 1) {
        //cout << street1.size() << endl;
        //cout << street2.size() << endl;
        app->create_popup_message("Error", "Please Enter Unique Street Names");
        return;
    }
    //Get all intersections between the 2 streets and highlight them
    for (int street1Intersection = 0; street1Intersection < street1Idx.size(); street1Intersection++) {
        for (int street2Intersection = 0; street2Intersection < street2Idx.size(); street2Intersection++) {
            vector <IntersectionIdx> intersecs = findIntersectionsOfTwoStreets(street1Idx[street1Intersection], street2Idx[street2Intersection]);
            if (intersecs.empty()) {
                continue;
            }
            for (int IntersectionNum = 0; IntersectionNum < intersecs.size(); IntersectionNum++) {
                highlight_intersection(intersecs[IntersectionNum]);
            }
        }
    }
    app->refresh_drawing();
}

//Helper Function to change the map

void change_map(string locationname, ezgl::application* app) {
    //Get the pathname from the pathnames map that was previously initialized
    string newpathname = pathnames.find(locationname)->second;
    //Check if map that user wants to switch to is the current map
    if (newpathname == current_map_path) {
        app->create_popup_message("Error", "You are already on this map!");
        return;
    }
    //Clear and close current map
    clearMap();
    app->update_message("Changing Maps...");
    closeMap();

    //cout << font << endl;
    //Load new map and update path
    loadMap(newpathname);
    current_map_path = newpathname;

    //Change font
    initializeFont();
    //Redraw to display new map
    reDrawMap(app);
    app->refresh_drawing();
    //Update application message
    app->update_message("Successfully Changed Map to " + locationname);
}

void findZoomLevel(double zoomFactor) {

    ZOOMLEVEL *= zoomFactor;

    //cout << ZOOMLEVEL << endl;

}

void setZoomLevel(double left, double right) {
    ZOOMLEVEL = right - left;
    //cout << ZOOMLEVEL << endl;
}

string getOSMWayTagValue(OSMID OSMid, string key) {

    // using the ODMid as the index, find the corresponding OSMNode
    auto it = fastData.OSMWay_IDs.find(OSMid);
    // if OSMid is not found, return empty string
    if (it == fastData.OSMWay_IDs.end()) {
        return "";
    }

    const OSMWay *osmway = it->second;

    // separate and store the key and value pairs in two different variables
    for (int i = 0; i < getTagCount(osmway); ++i) {
        string osmkey, value;
        tie(osmkey, value) = getTagPair(osmway, i);
        if (osmkey == key) {
            return value;
        }
    }

    // if nothing was returned, that means key was not found, so return empty string
    return "";
}

bool world_contains_point(ezgl::rectangle world, vector<ezgl::point2d> points) {
    for (int i = 0; i < points.size(); i++) {
        if (world.contains(points[i])) {
            return true;
        }
    }
    return false;
}


//CALLBACK FUNCTIONS

//A callback function for the highlight_intersection switch

void highlight_intersection_cbk(GtkSwitch */*self*/, gboolean state, ezgl::application *application) {
    //Set the state of the global toggle variable 
    highlightintersectiontoggle = state;
    //Refresh the screen
    application->refresh_drawing();
}

void clear_button_cbk(GtkWidget */*widget*/, ezgl::application *application) {
    //Clear all the intersections which were previously selected to be highlighted
    highlightintersectionidx.clear();
    //Refresh the screen
    application->refresh_drawing();
}

void display_icon_cbk(GtkSwitch */*self*/, gboolean state, ezgl::application *application) {
    displayicontoggle = state;
    application->refresh_drawing();
}

void nightModeSwitch_cbk(GtkSwitch */*self*/, gboolean state, ezgl::application *application) {
    nightModeOn = state;
    if (state == false) {
        BACKGROUND.red = 226;
        BACKGROUND.green = 223;
        BACKGROUND.blue = 211;

        WATER.red = 151;
        WATER.green = 192;
        WATER.blue = 223;

        NATURE.red = 198;
        NATURE.green = 216;
        NATURE.blue = 162;

        BEACHES.red = 193;
        BEACHES.green = 178;
        BEACHES.blue = 132;

        BUILDINGS.red = 227;
        BUILDINGS.green = 209;
        BUILDINGS.blue = 184;

        LIGHTER_NATURE.red = 208;
        LIGHTER_NATURE.green = 217;
        LIGHTER_NATURE.blue = 184;

        HIGHWAY.red = 250;
        HIGHWAY.green = 214;
        HIGHWAY.blue = 124;

        ROAD.red = 0xFF;
        ROAD.green = 0xFF;
        ROAD.blue = 0xFF;

        STREETNAMES.red = 30;
        STREETNAMES.green = 30;
        STREETNAMES.blue = 30;

        STORENAMES.red = 30;
        STORENAMES.green = 30;
        STORENAMES.blue = 30;

        SCALE.red = 0;
        SCALE.green = 0;
        SCALE.blue = 0;

    } else {
        BACKGROUND.red = 27;
        BACKGROUND.green = 38;
        BACKGROUND.blue = 53;

        WATER.red = 7;
        WATER.green = 8;
        WATER.blue = 11;

        NATURE.red = 25;
        NATURE.green = 63;
        NATURE.blue = 67;

        BEACHES.red = 116;
        BEACHES.green = 107;
        BEACHES.blue = 79;

        BUILDINGS.red = 39;
        BUILDINGS.green = 52;
        BUILDINGS.blue = 77;

        LIGHTER_NATURE.red = 37;
        LIGHTER_NATURE.green = 79;
        LIGHTER_NATURE.blue = 83;

        HIGHWAY.red = 173;
        HIGHWAY.green = 148;
        HIGHWAY.blue = 86;

        ROAD.red = 63;
        ROAD.green = 65;
        ROAD.blue = 72;

        STREETNAMES.red = 0xFF;
        STREETNAMES.green = 0xFF;
        STREETNAMES.blue = 0xFF;

        STORENAMES.red = 0xFF;
        STORENAMES.green = 0xFF;
        STORENAMES.blue = 0xFF;

        SCALE.red = 0xFF;
        SCALE.green = 0xFF;
        SCALE.blue = 0xFF;

    }
    application->refresh_drawing();
}

//Callback function for find button

void create_find_button_cbk(GtkWidget* /*widget*/, ezgl::application* app) {
    GObject *search1 = app->get_object("Search1");
    GObject *search2 = app->get_object("Search2");

    GtkEntry* search1_entry = GTK_ENTRY(search1);
    GtkEntry* search2_entry = GTK_ENTRY(search2);

    //Get text from each of the search entrys and passes those strings to a function that will highlight 
    //all intersections of those 2 streets
    const gchar* search1text = gtk_entry_get_text(search1_entry);
    const gchar* search2text = gtk_entry_get_text(search2_entry);
    string search1string = search1text;
    string search2string = search2text;
    highlight_intersections_between_2_streets(search1string, search2string, app);

}

//Callback function for Change Map Button

void create_change_map_button_cbk(GtkWidget* /*widget*/, ezgl::application* app) {


    //Get text from selected combobox
    GtkComboBoxText* combobox = GTK_COMBO_BOX_TEXT(app->get_object("ChangeMapDropDown"));
    const gchar* comboboxtext = gtk_combo_box_text_get_active_text(combobox);
    string comboboxstring;
    //Pass in combobox string of map name to change_map function
    if (comboboxtext == NULL) {
        comboboxstring = "";
        return;
    } else {
        comboboxstring = comboboxtext;
    }
    change_map(comboboxstring, app);

}

/**
 * Function to handle mouse press event
 * The current mouse position in the main canvas' world coordinate system is returned
 * A pointer to the application and the entire GDK event are also returned
 */
void act_on_mouse_press(ezgl::application *application, GdkEventButton *event, double x, double y) {
    if (event->button == 1) {
        if (highlightintersectiontoggle) {
            LatLon pos = LatLon(getLat(y), getLon(x, latAvg));
            int idx = findClosestIntersection(pos);
            highlight_intersection(idx);
            application->update_message("Closest Intersection: " + intersections[idx].name);
        }
    }
    application->refresh_drawing();
}


