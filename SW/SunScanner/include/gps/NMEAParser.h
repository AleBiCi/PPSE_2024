/*
*   Functions to parse and acquire data from standard format NMEA 0183 messages
*
*   Basic sentence structure: $aabbb,df1,df2,df3*hh<CR><LF>
*   where
*       aabbb -> sentence ID; aa is talker (GNSS) ID, bbb are type code (most important ones are GGA, GLL, RMC) 
*       df1,df2... -> data fields, number of them depends on type of sentence; delimited by comma ','
*       hh -> 2-hexadecimal checksum
*       <CR><LF> -> carriage return and line feed, helps with identifying end of message and visualization
*   The messages are at most 80 characters long, not counting cr and lf
*
*   We focus on RMC (Recommended Minimum Navigation Information) messages as they provide the essential data we need (Time, date, position, course, speed data)
*/

#ifndef __NMEAPARSER_H__
#define __NMEAPARSER_H__

#include <time.h>
#include <string>

class MessageRMC {
public:
    char* m; // full message
    char* message_ID;
    tm time; // time in tm format -> see time.h library for specifics
    char status;
    double latitude;
    char lat_dir; // N or S
    double longitude;
    char lon_dir; // E or W
    // We ignore SOG (Speed Over Ground) and CMG (Course Made Good) fields as we are not moving, this info would be useless
    // We ignore magnetic variation + magnetic variation dir
    char mode;
    char checksum; // 2 hexadecimal (0-F) values
};

bool validateChecksum(std::string mess_s, std::string& check_s);    // validates the checksum (XOR of all chars between '$' and '*' including ',')
bool parseRMC(std::string& line, MessageRMC& mess); // parses the RMC sentences (if valid) and assigns values to struct attributes; returns result of complete validation

#endif // __NMEAPARSER.H__
