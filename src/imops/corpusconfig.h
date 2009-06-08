/* This is an auto generated file. Please do not edit it.*/


#ifndef _corpusconfig_h
#define _corpusconfig_h



// :::::::::::::::::::::::::::::::::::::::::::::: Options variables :::..


// Compile Python sensors and _leds module
// as a shared library for dynamic loading
#define PYTHON_SHARED_CORPUS_OFF
#ifdef  PYTHON_SHARED_CORPUS_ON
#  define PYTHON_SHARED_SENSORS
#  define PYTHON_SHARED_LEDS
#else
#  undef  PYTHON_SHARED_SENSORS
#  undef  PYTHON_SHARED_LEDS
#endif

// Insert a 'fake' Sensors object into the Python module
#define USE_PYSENSORS_FAKE_BACKEND_OFF
#ifdef  USE_PYSENSORS_FAKE_BACKEND_ON
#  define USE_PYSENSORS_FAKE_BACKEND
#else
#  undef  USE_PYSENSORS_FAKE_BACKEND
#endif

// Turn on/off the actual backend proxy calls to the ALLeds module
#define USE_PYLEDS_CXX_BACKEND_ON
#ifdef  USE_PYLEDS_CXX_BACKEND_ON
#  define USE_PYLEDS_CXX_BACKEND
#else
#  undef  USE_PYLEDS_CXX_BACKEND
#endif

//Turn on/off debugging information for the Thread class.
#define DEBUG_THREAD_ON
#ifdef  DEBUG_THREAD_ON
#  define DEBUG_THREAD
#else
#  undef  DEBUG_THREAD
#endif


#endif // !_corpusconfig_h

