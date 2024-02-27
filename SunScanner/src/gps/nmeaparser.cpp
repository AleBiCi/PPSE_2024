#include <string>
#include <string.h>
#include <sstream> // for stringstream
#include <time.h>
#include <iostream>
#include "gps/NMEAParser.h"
#include <math.h>

bool validateChecksum(std::string mess_s, std::string& check_s) {
    /*
    Workflow to calculate NMEA checksum for a given message:
        1. compute XOR of every char of the message between '$' and '*' with themselves
        2. divide XOR into two nibbles (4 bit fields) (they correspond to the two characters of the checksum)
        3. convert the nibbles into hexadecimal characters checkByte1 and checkByte2
        4. compare the two checksum characters with the computed checkBytes
    */
    
    uint8_t XOR = 0;
    size_t limit = mess_s.find('*'); // limit points to first occurence of '*'

    for (size_t i = 1; i < limit; ++i) {    // start at i = 1 to skip '$'
        XOR^=mess_s[i];                     // computes XOR
    }

    uint8_t nibble1 = (XOR & 0xF0) >> 4;    // separate the nibbles
    uint8_t nibble2 = XOR & 0x0F;

    uint8_t checkByte1 = (nibble1 <= 0x09) ? (nibble1 + '0') : (nibble1 - 10 + 'A');    // convert the nibbles into hex values
    uint8_t checkByte2 = (nibble2 <= 0x09) ? (nibble2 + '0') : (nibble2 - 10 + 'A');

    return (checkByte1 == check_s[0] || checkByte2 == check_s[1]); // returns true if XOR is same as checksum
}

bool parseRMC(std::string& line, MessageRMC& mess) {
    bool error = false;

    // String versions of the various fields
    std::string ID_s;
    std::string time_s;
    std::string status_s;
    std::string lat_s;
    std::string lat_dir_s;
    std::string lon_s;
    std::string lon_dir_s;
    std::string tmp;
    std::string date_s;
    std::string mode_s;
    std::string checksum_s;

    // Sample GNRMC sentence for debugging
    line.assign("$GNRMC,140212.00,A,4604.18179,N,01108.18041,E,0.234,,210224,,,A*69\n");

    // Creates input string stream from which to gather values of each field
    std::istringstream inputStr(line);

    // One getline for each field
    std::getline(inputStr, ID_s, ',');

    std::getline(inputStr, time_s, ',');
    
    std::getline(inputStr, status_s, ',');
    
    std::getline(inputStr, lat_s, ',');
    
    std::getline(inputStr, lat_dir_s, ',');
    
    std::getline(inputStr, lon_s, ',');
    
    std::getline(inputStr, lon_dir_s, ',');
    
    // Throwaway calls to skip unwanted fields
    std::getline(inputStr, tmp, ',');
    std::getline(inputStr, tmp, ',');

    std::getline(inputStr, date_s, ',');
    
    // Throwaway calls to skip unwanted fields
    std::getline(inputStr, tmp, ',');
    std::getline(inputStr, tmp, ',');

    std::getline(inputStr, mode_s, '*');

    std::getline(inputStr, checksum_s, '\n');

    // Casting and assignment of checksum and status fields to check for sentence validity
    mess.checksum = checksum_s[0] + checksum_s[1]; // using array notation to access specific characters in string
    mess.status = status_s[0];

    // Checking for validity : calculated checksum chars have to match the originals + status field has to be valid (A = valid mess, V = not valid)
    if (!validateChecksum(line, checksum_s) || strcmp(&mess.status, "A")!=0) {
        // Serial.println("ERRORE: messaggio non valido");
        error = true; // if checksum comparison and validity status check didn't give positive result, return false
    } else return false;

    // Cast and Assign values to class attributes
    mess.message_ID = ID_s.data();


    if (!lat_s.empty()) mess.latitude = std::stod(lat_s);

    double degrees = floor(mess.latitude / 100.0);
    double minutes = mess.latitude - degrees * 100.0;
    mess.latitude = degrees + minutes / 60.0;

    (!lat_dir_s.empty()) ? mess.lat_dir = lat_dir_s[0] : mess.lat_dir = ' ';
    
    if (!lon_s.empty()) mess.longitude = std::stod(lon_s);
    degrees = floor(mess.longitude / 100.0);
    minutes = mess.longitude - degrees * 100.0;
    mess.longitude = degrees + minutes / 60.0;
    
    (!lon_dir_s.empty()) ? mess.lon_dir = lon_dir_s[0] : mess.lon_dir = ' ';
    
    if(!time_s.empty()) {
        std::string hour;
        hour.push_back(time_s[0]); hour.push_back(time_s[1]);
        mess.time.tm_hour = std::stoi(hour, nullptr, 10);

        std::string minutes;
        minutes.push_back(time_s[2]); minutes.push_back(time_s[3]);
        mess.time.tm_min = std::stoi(minutes, nullptr, 10);

        std::string seconds;
        seconds.push_back(time_s[4]); seconds.push_back(time_s[5]);
        mess.time.tm_sec = std::stoi(seconds, nullptr, 10);
    }

    if(!date_s.empty()) {
        std::string month;
        month.push_back(date_s[2]); month.push_back(date_s[3]);
        mess.time.tm_mon = std::stoi(month, nullptr, 10) - 1;

        std::string day;
        day.push_back(date_s[0]); day.push_back(date_s[1]);
        mess.time.tm_mday = std::stoi(day, nullptr, 10);

        std::string year;
        year.push_back(date_s[4]); year.push_back(date_s[5]);
        mess.time.tm_year = std::stoi(year, nullptr, 10) + 100;
    }
    
    (!mode_s.empty()) ? mess.mode = mode_s[0] : mess.mode = ' ';

    return (error);
}
