#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <math.h>
#include "dwm_api.h"
#include "hal.h"
#include <iomanip>
#include <cmath>
#include <ctime>
#include <fstream>
#include<chrono>
#include<thread>
#include<boost/program_options.hpp>
/** 
 * @brief   Structure definitions
 * 
*/

// Structure to represent Geodetic coordinate -->  latitude, longitude, and altitude (Degree,Degree,metres)
struct Geodetic_Coordinate {
    double Latitude;
    double Longitude;
    double Altitude;
    double bearing;

    Geodetic_Coordinate (double lat, double lon , double altitude, double angle ): Latitude(lat) , Longitude(lon), Altitude(altitude), bearing(angle){}
};

//Structure to represent local Coordinate ---> x_loc , y_loc , z_loc ( metres, metres, metres)
struct Local_Coordinate {

    double x_loc;
    double y_loc;
    double z_loc;
    double qf;
  

    Local_Coordinate(double x, double y, double z , double quality_factor): x_loc(x) , y_loc(y),z_loc(z),qf(quality_factor){}

};

//global variables 

int tag_cfg_flag =0;
int clientSocket_send = socket(AF_INET, SOCK_STREAM, 0);
int clientSocket_recv = socket(AF_INET, SOCK_STREAM, 0);

/**
 * @brief   Creates a NMEA - GNGGA format message 
 * @param   Geodetic_Coordinate type structure
 * @result  NMEA String 
*/


std::string createNMEA_Message(Geodetic_Coordinate SendPosition) {

    //Systemtime in hhmmss.ss format

   std::time_t now = std::time(nullptr);
    struct tm* timeinfo = std::localtime(&now);
    int hours = timeinfo->tm_hour;
    int minutes = timeinfo->tm_min;
    int seconds = timeinfo->tm_sec;
    int tenths = (std::clock() * 10 / CLOCKS_PER_SEC) % 10;
    std::string formatted_time = 
        (hours < 10 ? "0" : "") + std::to_string(hours) +
        (minutes < 10 ? "0" : "") + std::to_string(minutes) +
        (seconds < 10 ? "0" : "") + std::to_string(seconds) + "."+
        (tenths< 10 ? "0" : "") + std::to_string(tenths);
    std::string time = formatted_time;

    //Variable required for NMEA

    double latitude = SendPosition.Latitude;
    double longitude = SendPosition.Longitude;
    char latitudeDirection = 'N';
    char longitudeDirection = 'E';
    int fixQuality = 2;
    int numSatellites = 11;
    double hdop = 1.40;
    double altitude = 83.2;
    char altitudeUnit = 'M';
    double geoidSeparation = 34.4;
    char geoidUnit = 'M';
    double ageOfDGPS = 0.7;
    int referenceStationID = 0;

    char rmc_status = 'A';
    double rmc_speed = 0.500;
    double rmc_course = 0;
    int rmc_date = 131023;
    double rmc_magnetic_variation = 0;
    char rmc_mode = 'V';

    // Create NMEA string

    std::ostringstream ggaString;
    ggaString << "$GNGGA," << time << ",";
    ggaString << std::fixed << std::setprecision(5) << latitude << "," << latitudeDirection << ",";
    ggaString << std::fixed << std::setprecision(5) << "0"<<longitude << "," << longitudeDirection << ",";
    ggaString << fixQuality << "," << numSatellites << "," << std::fixed << std::setprecision(2) << hdop << ",";
    ggaString << std::fixed << std::setprecision(1) << altitude << "," << altitudeUnit << ",";
    ggaString << std::fixed << std::setprecision(1) << geoidSeparation << "," << geoidUnit << ",";
    ggaString << std::fixed << std::setprecision(1) << ageOfDGPS << "," << std::setw(4) << std::setfill('0') << referenceStationID;

    // Calculate the NMEA ggaChecksum ignoring the $ in the start

    std::ostringstream rmcString;
    rmcString << "$GNRMC," << time << ","<<rmc_status<<",";
    rmcString << std::fixed << std::setprecision(5) << latitude << "," << latitudeDirection << ",";
    rmcString << std::fixed << std::setprecision(5) << "0"<<longitude << "," << longitudeDirection << ","<<std::setprecision(3);
    rmcString <<rmc_speed<<",,"<<rmc_date<<",,,"<<"D,"<<rmc_mode;

   

    std::string ggaMessage = ggaString.str();
    std::string rmcMessage = rmcString.str();


//Generate Checksums to be added at the end of the strings

int ggaChecksum = 0;
int rmcChecksum = 0;

bool skipChecksumCalculation = true; // Flag to skip ggaChecksum calculation until '$' is encountered

for (char c : ggaMessage) {
    if (c == '$') {
        skipChecksumCalculation = false;
        continue;
    }
    
    if (!skipChecksumCalculation) {
        ggaChecksum ^= c;
    }
}

skipChecksumCalculation = true;

for (char c : rmcMessage) {
    if (c == '$') {
        skipChecksumCalculation = false;
        continue;
    }
    
    if (!skipChecksumCalculation) {
        rmcChecksum ^= c;
    }
}

    // Add the ggaChecksum and terminate the NMEA message

    ggaString << "*" << std::hex << std::setw(2) << std::setfill('0') << ggaChecksum;
    ggaString << "\r\n";

    rmcString << "*" << std::hex << std::setw(2) << std::setfill('0') << rmcChecksum;
    rmcString << "\r\n";

    rmcString<<ggaString.str();

    std::string return_message = rmcString.str();

    return return_message;
}


/**
 * @brief   writes the input parameter in the next line of a word file
 * @param   String data refernecer
 * @result  The input string is written in the woed file at the predefined path
*/

void writeToTextFile( const std::string& data) {
    const std::string& fileName = "/home/rafal/Documents/gnss_feed.txt";
    std::ofstream file(fileName, std::ios_base::app); // Open the file in append mode
    if (!file) {
        std::cerr << "Error opening the file!" << std::endl;
        return;
    }

    file << data; // Writing with a newline character
    file.close();
}




/** !
* @brief                 Converts local coordinates to global geodetic coordinates 
* @param                 Geodetic_Coordinate Structure, Local_Coordinate structure 
* @result                The new global Geodetic_coordinate representing the new position of the Mobile robot

*/

Geodetic_Coordinate local2geodetic(Geodetic_Coordinate anchor_origin, Local_Coordinate UV_local ) {

    //Mathmetical constants 
    const double DegreesToRadians = M_PI / 180;
    const double RadiansToDegrees = 180/ M_PI;
    const double EarthRadius = 6378160;  // Earth's radius in meters
    const double TwoPi = 2.0 * M_PI;

  
    double total_bearing;

    //Finding the total_bearing and the displacement of the mobile robot 
    double local_bearing = atan2(UV_local.y_loc,UV_local.x_loc);
    double displacement_from_origin = sqrt((UV_local.x_loc * UV_local.x_loc)+(UV_local.y_loc * UV_local.y_loc));
    total_bearing = ((local_bearing)*RadiansToDegrees + (anchor_origin.bearing));

   // std::cout<<total_bearing<<"  "<<local_bearing*RadiansToDegrees <<'\n';

    //Degrees to Radians 
    double latA = anchor_origin.Latitude * DegreesToRadians;
    double lonA = anchor_origin.Longitude * DegreesToRadians;
    double trueCourse = total_bearing * DegreesToRadians;

    double angularDistance = displacement_from_origin / EarthRadius;

    /*   ----- Haversine ( Inverse ) formula  --------
    
        ->    y = arcsin((sin(y0) * cos(d/r)) + (cos(y0) * sin(d/r) * cos(theta)))
        ->    x = x0 + arctan2((sin(theta) * sin(d/r) * cos(y0)), (cos(d/r) - (sin(y0) * sin(y))))
        
        Where `x0` and `y0` are the initial coordinates, `d` is the distance traveled, `r` is the radius of the  Earth), 
        theta is the total_bearing, and `x` and `y` are the new coordinates 
        
     */


    double lat = asin(sin(latA) * cos(angularDistance) + cos(latA) * sin(angularDistance) * cos(trueCourse));

    double dlon = atan2(sin(trueCourse) * sin(angularDistance) * cos(latA), cos(angularDistance) - sin(latA) * sin(lat));

    double lon = fmod(lonA + dlon + M_PI, TwoPi) - M_PI;

    return Geodetic_Coordinate(lat * RadiansToDegrees, lon * RadiansToDegrees, anchor_origin.Altitude,total_bearing);
}





int tcp_init() {

    //Establish tcp connection 
    
   
    if (clientSocket_send == -1) {
        perror("Error creating socket");
        return 1;
    }

    struct sockaddr_in serverAddress_send;
    serverAddress_send.sin_family = AF_INET;
    serverAddress_send.sin_port = htons(6100);
    serverAddress_send.sin_addr.s_addr = inet_addr("192.168.14.24");

    if (connect(clientSocket_send, (struct sockaddr*)&serverAddress_send, sizeof(serverAddress_send)) == -1) {
        perror("Error connecting to the server");
        close(clientSocket_send);
        return 1;
    }

    struct sockaddr_in serverAddress_recv;
    serverAddress_recv.sin_family = AF_INET;
    serverAddress_recv.sin_port = htons(6200);
    serverAddress_recv.sin_addr.s_addr = inet_addr("192.168.14.24");

    if (connect(clientSocket_recv, (struct sockaddr*)&serverAddress_recv, sizeof(serverAddress_recv)) == -1) {
        perror("Error connecting to the server");
        close(clientSocket_recv);
        return 1;
    }
   
   
    return 1;
   
    
  
}


int tcp_send(const char* message){

      if (send(clientSocket_send, message, strlen(message), 0) == -1) {
        perror("Error sending data to the server");
        close(clientSocket_send);
        return 1;
    }
    std::cout<<"Sent\n";

    return 1;
}

std::string tcp_receive(){

    char buffer[1024];
    int bytesRead = recv(clientSocket_recv, buffer, sizeof(buffer), 0);
    if (bytesRead == -1) {
        std::cerr << "Error receiving data." << std::endl;
        return NULL;
    }

    // Null-terminate the received data to make it a C-string
    buffer[bytesRead] = '\0';

    // Display the received data
    return buffer;

}


/**
 * @brief  Converts the coordinate format from dd.dddddd  ->  ddmm.mmmmm
 * @param  Input coordinate 
 * @result Converted coordinate 
*/

double type_converter(double input ) {
    // Extract digits before and after the decimal point
    int digits_before_decimal = static_cast<int>(input);
    double digits_after_decimal = input - digits_before_decimal;
    
    // Multiply digits before the decimal by 100 and add digits after the decimal
    double result = (digits_before_decimal * 100) + (digits_after_decimal * 60);

    return result;
}


int tag_cfg_init(void) {

    //Init. Tag
    int wait_period = 1000;
    dwm_cfg_tag_t cfg_tag;
    dwm_cfg_t cfg_node;

    HAL_Print("dwm_init(): dev%d\n", HAL_DevNum());

    dwm_init();

    HAL_Print("Setting to tag: dev%d.\n", HAL_DevNum());
    cfg_tag.low_power_en = 0;
    cfg_tag.meas_mode = DWM_MEAS_MODE_TWR;
    cfg_tag.loc_engine_en = 1;
    cfg_tag.common.led_en = 1;
    cfg_tag.common.ble_en = 1;
    cfg_tag.common.uwb_mode = DWM_UWB_MODE_ACTIVE;
    cfg_tag.common.fw_update_en = 0;
    HAL_Print("dwm_cfg_tag_set(&cfg_tag): dev%d.\n", HAL_DevNum());
    dwm_cfg_tag_set(&cfg_tag);

    HAL_Print("Wait 2s for node to reset.\n");

    HAL_Delay(2000);
     //Set the update rate for the quorvo modules 
    dwm_upd_rate_set(1, 5);

    dwm_cfg_get(&cfg_node);

    HAL_Print("Comparing set vs. get: dev%d.\n", HAL_DevNum());
    if ((cfg_tag.low_power_en != cfg_node.low_power_en)
        || (cfg_tag.meas_mode != cfg_node.meas_mode)
        || (cfg_tag.loc_engine_en != cfg_node.loc_engine_en)
        || (cfg_tag.common.led_en != cfg_node.common.led_en)
        || (cfg_tag.common.ble_en != cfg_node.common.ble_en)
        || (cfg_tag.common.uwb_mode != cfg_node.common.uwb_mode)
        || (cfg_tag.common.fw_update_en != cfg_node.common.fw_update_en)) {
        HAL_Print("low_power_en        cfg_tag=%d : cfg_node=%d\n", cfg_tag.low_power_en, cfg_node.low_power_en);
        HAL_Print("meas_mode           cfg_tag=%d : cfg_node=%d\n", cfg_tag.meas_mode, cfg_node.meas_mode);
        HAL_Print("loc_engine_en       cfg_tag=%d : cfg_node=%d\n", cfg_tag.loc_engine_en, cfg_node.loc_engine_en);
        HAL_Print("common.led_en       cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.led_en, cfg_node.common.led_en);
        HAL_Print("common.ble_en       cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.ble_en, cfg_node.common.ble_en);
        HAL_Print("common.uwb_mode     cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.uwb_mode, cfg_node.common.uwb_mode);
        HAL_Print("common.fw_update_en cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.fw_update_en, cfg_node.common.fw_update_en);
        HAL_Print("\nConfiguration failed.\n\n");
        return 0 ;

    } else {
        tag_cfg_flag=1;
        std::cout<<tag_cfg_flag;
        HAL_Print("\nConfiguration succeeded.\n\n");
        return 0 ;
    }
}

int main_program(double bearing, double lat,double lon){

    //Initialize the tag

    tag_cfg_init();

    //Initialize tcp sockets

    tcp_init();

    //Initialize Coordinate stucture 
     Geodetic_Coordinate anchor_origin(52.194042,20.920792,0,40); 

    //continuously fetch and send location ibformation 

    while (1){

        if (tag_cfg_flag==1){

            //Successively receive Location in regular intervals ( 1s )
            dwm_loc_data_t loc;
            dwm_pos_t pos;
            loc.p_pos = &pos;

            Local_Coordinate UV_local(0,0,0,0);

            if (dwm_loc_get(&loc) == RV_OK) {
                 UV_local.x_loc = (loc.p_pos->x)/1000.0000;
                 UV_local.y_loc = (loc.p_pos->y)/1000.0000;
                 UV_local.z_loc=(loc.p_pos->z)/1000.0000;
                 UV_local.qf = loc.p_pos->qf;


                std::cout<<"Quality Factor "<< UV_local.qf <<" \n";
            
                std::cout<<UV_local.x_loc<<"  "<<UV_local.y_loc<<"   "<<UV_local.z_loc<<"\n";
                

                //Calulate the new coordinates to NMEA format 
                Geodetic_Coordinate destination = local2geodetic(anchor_origin, UV_local);

                //Convert Coordinates to NMEA format (ddmm.mmmmm)
                destination.Latitude = type_converter(destination.Latitude);
                destination.Longitude = type_converter(destination.Longitude);
            
                //Generate NMEA string 
                std::string send_NMEA_String = createNMEA_Message(destination);

                //Type conversion for sending via TCP
                char charArray[send_NMEA_String.length() + 1]; 
                strcpy(charArray, send_NMEA_String.c_str());
                
                //Write NMEA string to the text file 
                writeToTextFile(send_NMEA_String);

                std::string incoming_gps = tcp_receive();

              

                std::string incoming_gps = tcp_receive();

              
                //Send NMEA string to the server via TCP

                if(UV_local.qf>10){
                    tcp_send(charArray);
                    std::cout<<"LOCAL -- LOCAL -- LOCAL \n";
                  
                    }
                    
                    else{

                     char gps[incoming_gps.length() + 1]; 
                      strcpy(gps, incoming_gps.c_str());
                      tcp_send(gps);
                      std::cout<<"GPS -- GPS -- GPS \n ";
                 }

            }

        }

    }

}

//Program for providing terminal interface with three input parametes 

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    // Setup the options description for command-line arguments
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display the options")
        ("bearing,b", po::value<double>(), "Bearing value")
        ("lat,l", po::value<double>(), "Latitude value")
        ("lon,o", po::value<double>(), "Longitude value");

    po::variables_map vm;

    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (const po::error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    if (vm.count("help")) {
        std::cout << "Usage: main_program [bearing , lat , lon]\n";
        std::cout << desc << std::endl;
        return 0;
    }

    if (vm.count("bearing") && vm.count("lat") && vm.count("lon")) {
        double bearing = vm["bearing"].as<double>();
        double lat = vm["lat"].as<double>();
        double lon = vm["lon"].as<double>();
        
     //main_program call if above conditions are satisified 
        
        main_program(bearing, lat, lon);
    } else {
        std::cerr << "Error: Missing required input parameters. Use --help for usage information.\n";
        return 1;
    }

    return 0;
}

    
