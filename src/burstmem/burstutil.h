#ifndef __BURST_UTIL_H__
#define __BURST_UTIL_H__

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>

inline void readVariablesFile(const char* file_name, std::vector<std::string>& out)
{
    // read the variable names we will be writing to a csv file
    std::ifstream varfile (file_name);

    if (!varfile.is_open ()) {
        std::cout <<
            "burstutil: ERROR: cannot find the variable files - no recording done. where is "
            << file_name << "?" << std::endl;
        return;
    }

    out.resize(0, "");

    while (!varfile.eof ()) {
        std::string line;
        getline (varfile, line);
        if (line.size () <= 0) {
            continue;
        }
        out.push_back (line);    // TODO - algorithms, copy?
    }
    varfile.close ();
}

inline std::string get_date ()
{
    time_t t = time (0);
    struct tm *lt = localtime (&t);
    char time_str[15];
    sprintf (time_str, "%04d%02d%02d_%02d%02d%02d", lt->tm_year + 1900,
             lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min,
             lt->tm_sec);
    return std::string (time_str);
}

// --------------------------------------------------------------------------------
// Small class for easy profiling - just put around whatever you want
// static Counter mycounter
// mycounter.one("mymodule: mymessage: "); // this will print every 100 times
// ... do something ...
// mycounter.two();
struct Counter {
    std::string msg;
    timeval t1, t2;
    float total;
    int counter;
    Counter(std::string _msg) : msg(_msg), total(0.0), counter(0) {
    }
    inline void one() {
        if (counter >= 100) {
            std::cout << msg << (total / counter) << std::endl;
            counter = 0;
            total = 0.0;
        }
        counter ++;
        gettimeofday(&t1, NULL);
    }
    inline void two() {
        gettimeofday(&t2, NULL);
        total += (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec);
    }
};
// --------------------------------------------------------------------------------

// Convenient place to put this, but they are unrelated: definitions to
// hide all API changes between 1.2.0 and 1.3.8 versions of Naoqi

#ifdef NAOQI_138
#define VIDEO_MODULE "ALVideoDevice"
#define SONAR_MODULE "ALSonar"
#define VIDEO_MODULE_SUBSCRIBE "subscribe"
#define VIDEO_MODULE_UNSUBSCRIBE "unsubscribe"
#define SONAR_EXTRACTOR_VARIABLE "extractors/alsonar/distances"
#define SONAR_PRECISION 1.0
#else
#ifdef NAOQI_120
#define VIDEO_MODULE "NaoCam"
#define SONAR_MODULE "ALUltraSound"
#define VIDEO_MODULE_SUBSCRIBE "register"
#define VIDEO_MODULE_UNSUBSCRIBE "unregister"
#define SONAR_EXTRACTOR_VARIABLE "extractors/alultrasound/distances"
namespace AL {
class ALVisionImage;
typedef ALVisionImage ALImage;
};
#else
#error "You must have NAOQI_138 or NAOQI_120 defined"
#endif // NAOQI_120
#endif // NAOQI_138
#endif // __BURST_UTIL_H__

