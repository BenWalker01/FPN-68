#pragma once

#include <map>
#include <string>

#define FPN_NAME "FPN-68 PAR"

#define SETTINGS_OBJECT_MAIN_CONTROLS 0
#define SETTINGS_OBJECT_RANGE_CONTROLS 1
#define SETTINGS_OBJECT_GLIDE_CONTROLS 2
#define SETTINGS_OBJECT_DISPLAY_CONTROLS 3
#define SETTINGS_OBJECT_RADAR_CONTROLS 4
#define SETTINGS_OBJECT_RUNWAY_CONTROLS 5

#define AXES_COLOUR RGB(236, 181, 117)
#define GLIDESLOPE_COLOUR RGB(137, 251, 247)
#define TRACK_DEVIATION_COLOUR RGB(105, 217, 255)
#define BACKGROUND_COLOUR RGB(41, 44, 66)
#define PRIMARY_COLOUR RGB(255, 255, 255)

#define X_AXIS_OFFSET 20

#define AIRPORT_ELEVATION std::map<std::string, std::map<std::string, double>>{ \
    { "EGAA", {{"07", 200.1}, {"25", 259.2}, {"17", 193.6}, {"35", 252.6} }},\
    { "EGAC", {{"04", 6.6}, {"22", 9.8}} },\
    { "EGAE", {{"08", 16.4}, {"26", 3.3}} },\
    { "EGBB", {{"15", 292.0}, {"33", 324.8}} },\
    { "EGBE", {{"05", 275.6}, {"23", 255.9}} },\
    { "EGBJ", {{"04", 82.0}, {"22", 82.0}, {"09", 68.9}, {"27", 82.0}} },\
    { "EGBP", {{"08", 439.6}, {"26", 403.5}} },\
    { "EGCC", {{"05L", 213.3}, {"23R", 255.9}, {"05R", 187.0}, {"23L", 232.9}} },\
    { "EGCJ", {{"01", 26.2}, {"19", 26.2}, {"06", 23.0}, {"24", 26.2}, {"10", 26.2}, {"28", 26.2}, {"10G", 19.7}, {"28G", 26.2}} },\
    { "EGCM", {{"06", 32.8}, {"24", 26.2}} },\
    { "EGCN", {{"02", 62.3}, {"20", 29.5}} },\
    { "EGDI", {{"03", 121.4}, {"21", 111.5}, {"09", 134.5}, {"27", 101.7}, {"16", 121.4}, {"34", 105.0}} },\
    { "EGDM", {{"05", 400.3}, {"23", 370.7}} },\
    { "EGDO", {{"05", 278.9}, {"23", 285.4}, {"09", 282.2}, {"27", 282.2}, {"13", 288.7}, {"31", 275.6}, {"18", 295.3}, {"36", 278.9}} },\
    { "EGDR", {{"11", 249.3}, {"29", 265.7}, {"06", 239.5}, {"24", 265.7}, {"18", 259.2}, {"36", 246.1}} },\
    { "EGDY", {{"04", 68.9}, {"22", 72.2}, {"08", 55.8}, {"26", 55.8}} },\
    { "EGEC", {{"11", 32.8}, {"29", 39.4}} },\
    { "EGFF", {{"12", 206.7}, {"30", 213.3}} },\
    { "EGGD", {{"09", 603.7}, {"27", 593.8}} },\
    { "EGGP", {{"09", 55.8}, {"27", 75.5}} },\
    { "EGGW", {{"07", 508.5}, {"25", 511.8}} },\
    { "EGHC", {{"02", 370.7}, {"20", 374.0}, {"07", 347.8}, {"25", 387.1}, {"11", 387.1}, {"29", 383.9}, {"16", 374.0}, {"34", 383.9}} },\
    { "EGHE", {{"09", 114.8}, {"27", 101.7}, {"14", 91.9}, {"32", 62.3}} },\
    { "EGHG", {{"09", 200.1}, {"27", 164.0}} },\
    { "EGHH", {{"08", 23.0}, {"26", 26.2}} },\
    { "EGHI", {{"02", 29.5}, {"20", 39.4}} },\
    { "EGHL", {{"09", 574.1}, {"27", 613.5}} },\
    { "EGHQ", {{"12", 315.0}, {"30", 374.0}} },\
    { "EGJA", {{"03", 269.0}, {"21", 278.9}, {"08", 269.0}, {"26", 278.9}, {"13", 255.9}, {"31", 269.0}} },\
    { "EGJB", {{"09", 295.3}, {"27", 331.4}} },\
    { "EGJJ", {{"08", 272.3}, {"26", 262.5}} },\
    { "EGKA", {{"02", 3.3}, {"20", 0.0}, {"02G", 0.0}, {"20G", 0.0}, {"06", 0.0}, {"24", 0.0}, {"13", 0.0}, {"31", 0.0}} },\
    { "EGKB", {{"03", 570.9}, {"21", 511.8}} },\
    { "EGKK", {{"08L", 183.7}, {"26R", 180.4}, {"08R", 183.7}, {"26L", 164.0}} },\
    { "EGKR", {{"07L", 203.4}, {"25R", 203.4}, {"07R", 213.3}, {"25L", 196.9}, {"18", 210.0}, {"36", 200.1}} },\
    { "EGKT", {{"05L", 682.4}, {"23R", 649.6}, {"05R", 689.0}, {"23L", 646.3}, {"12", 620.1}, {"30", 649.6}} },\
    { "EGLC", {{"09", 9.8}, {"27", 16.4}} },\
    { "EGLF", {{"06", 223.1}, {"24", 203.4}} },\
    { "EGLK", {{"07", 321.5}, {"25", 311.7}} },\
    { "EGLL", {{"09L", 72.2}, {"27R", 72.2}, {"09R", 65.6}, {"27L", 75.5}} },\
    { "EGLW", {{"02", 13.1}, {"20", 13.1}} },\
    { "EGMC", {{"05", 49.2}, {"23", 42.7}} },\
    { "EGMD", {{"03", 9.8}, {"21", 3.3}} },\
    { "EGNE", {{"02", 88.6}, {"20", 75.5}} },\
    { "EGNH", {{"10", 39.4}, {"28", 19.7}, {"13", 29.5}, {"31", 29.5}} },\
    { "EGNJ", {{"02", 121.4}, {"20", 75.5}, {"08", 88.6}, {"26", 85.3}} },\
    { "EGNL", {{"05", 32.8}, {"23", 19.7}, {"17", 36.1}, {"35", 49.2}} },\
    { "EGNM", {{"14", 672.6}, {"32", 662.7}} },\
    { "EGNO", {{"07", 32.8}, {"25", 62.3}} },\
    { "EGNR", {{"04", 29.5}, {"22", 13.1}} },\
    { "EGNS", {{"03", 23.0}, {"21", 59.1}, {"08", 39.4}, {"26", 29.5}} },\
    { "EGNT", {{"07", 255.9}, {"25", 226.4}} },\
    { "EGNV", {{"05", 114.8}} },\
    { "EGNX", {{"09", 311.7}, {"27", 288.7}} },\
    { "EGOE", {{"04", 259.2}, {"22", 262.5}, {"10", 262.5}, {"28", 269.0}, {"17", 255.9}, {"35", 262.5}} },\
    { "EGOQ", {{"04", 180.4}, {"22", 193.6}} },\
    { "EGOS", {{"05", 255.9}, {"23", 226.4}, {"18", 236.2}, {"36", 229.7}} },\
    { "EGOV", {{"01", 13.1}, {"19", 29.5}, {"13", 23.0}, {"31", 19.7}} },\
    { "EGOW", {{"03", 29.5}, {"21", 32.8}, {"08", 39.4}, {"26", 26.2}} },\
    { "EGPA", {{"09", 42.7}, {"27", 49.2}, {"14", 26.2}, {"32", 49.2}} },\
    { "EGPB", {{"06", 9.8}, {"24", 26.2}, {"09", 19.7}, {"27", 0.0}, {"15", 6.6}, {"33", 9.8}} },\
    { "EGPC", {{"13", 111.5}, {"31", 111.5}} },\
    { "EGPD", {{"16", 210.0}, {"34", 203.4}} },\
    { "EGPE", {{"05", 23.0}, {"23", 19.7}, {"11", 29.5}, {"29", 26.2}} },\
    { "EGPF", {{"05", 19.7}, {"23", 19.7}} },\
    { "EGPH", {{"06", 101.7}, {"24", 91.9}} },\
    { "EGPI", {{"07", 29.5}, {"25", 42.7}, {"12", 16.4}, {"30", 36.1}} },\
    { "EGPK", {{"02", 42.7}, {"20", 52.5}, {"12", 36.1}, {"30", 68.9}} },\
    { "EGPL", {{"06", 16.4}, {"24", 13.1}, {"17", 19.7}, {"35", 16.4}} },\
    { "EGPN", {{"09", 23.0}, {"27", 19.7}} },\
    { "EGPO", {{"06", 29.5}, {"24", 19.7}, {"18", 13.1}, {"36", 13.1}} },\
    { "EGPR", {{"07", 0.0}, {"25", 0.0}, {"11", 6.6}, {"29", 0.0}, {"15", 13.1}, {"33", 0.0}} },\
    { "EGPU", {{"05", 23.0}, {"23", 13.1}, {"11", 32.8}, {"29", 19.7}} },\
    { "EGQL", {{"08", 32.8}, {"26", 23.0}} },\
    { "EGQS", {{"05", 29.5}, {"23", 36.1}, {"10", 39.4}, {"28", 36.1}} },\
    { "EGSC", {{"05", 42.7}, {"23", 52.5}} },\
    { "EGSH", {{"09", 114.8}, {"27", 105.0}} },\
    { "EGSS", {{"04", 344.5}, {"22", 341.2}} },\
    { "EGSY", {{"07", 154.2}, {"25", 134.5}} },\
    { "EGTC", {{"03", 354.3}, {"21", 357.6}} },\
    { "EGTD", {{"07", 173.9}, {"25", 160.8}} },\
    { "EGTE", {{"08", 98.4}, {"26", 101.7}} },\
    { "EGTF", {{"06", 75.5}, {"24", 68.9}} },\
    { "EGTK", {{"01", 242.8}, {"19", 259.2}} },\
    { "EGTO", {{"02L", 413.4}, {"20R", 383.9}, {"02R", 413.4}, {"20L", 397.0}} },\
    { "EGUB", {{"01", 173.9}, {"19", 200.1}} },\
    { "EGUL", {{"06", 13.1}, {"24", 19.7}} },\
    { "EGUN", {{"10", 13.1}, {"28", 32.8}} },\
    { "EGUW", {{"05", 285.4}, {"23", 272.3}} },\
    { "EGVA", {{"09", 285.4}, {"27", 259.2}} },\
    { "EGVH", {{"08", 229.7}, {"26", 232.9}} },\
    { "EGVN", {{"07", 292.0}, {"25", 246.1}} },\
    { "EGVO", {{"09", 387.1}, {"27", 390.4}} },\
    { "EGVP", {{"04", 265.7}, {"22", 262.5}, {"08", 278.9}, {"26", 259.2}, {"17", 272.3}, {"35", 249.3}} },\
    { "EGWC", {{"06", 262.5}, {"24", 265.7}, {"06G", 262.5}, {"24G", 265.7}} },\
    { "EGWU", {{"07", 118.1}, {"25", 124.7}} },\
    { "EGXC", {{"07", 19.7}, {"25", 13.1}} },\
    { "EGXE", {{"03", 114.8}, {"21", 108.3}, {"16", 108.3}, {"34", 128.0}} },\
    { "EGXH", {{"08", 170.6}, {"26", 154.2}} },\
    { "EGWX", {{"02", 239.5},{"20", 226.4}} },\
    { "EGXZ", {{"02", 78.7}, {"20", 95.1},{ "07", 75.5}, {"25", 82.0}, {"13", 95.1}, {"31", 82.0}}},\
    { "EGYD", {{"01", 183.7}, {"19", 183.7}, {"08", 226.4}, {"26", 177.2}}},\
    { "EGYE", {{"06", 364.2}, {"24", 321.5}, {"10", 374.0}, {"28", 347.8}, {"18", 331.4}, {"36", 347.8}}},\
    { "EGYM", {{"01", 52.5}, {"19", 85.3}, {"05", 52.5}, {"23", 55.8}}}}
