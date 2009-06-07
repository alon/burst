#ifndef Threshold_h_DEFINED
#define Threshold_h_DEFINED

#include <boost/shared_ptr.hpp>

typedef unsigned char uchar;

class Threshold;  // forward reference

#include "ObjectFragments.h"
#include "NaoPose.h"
#ifndef NO_ZLIB
#include "Zlib.h"
#endif
#include "Profiler.h"
//
// COLOR TABLE CONSTANTS
// remember to change both values when chaning the color tables

//these must be changed everytime we load a new table
#ifdef SMALL_TABLES
#define YSHIFT  3
#define USHIFT  2
#define VSHIFT  2
#define YMAX  32
#define UMAX  64
#define VMAX  64
#else
#define YSHIFT  1
#define USHIFT  1
#define VSHIFT  1
#define YMAX  128
#define UMAX  128
#define VMAX  128
#endif

//
// THRESHOLDING CONSTANTS
// Constants pertaining to object detection and horizon detection
static const int MIN_RUN_SIZE = 25; // Decrease for lower resolution n/2
// we're more demanding of Green because there is so much
static const int MIN_GREEN_SIZE = 10;

/* The following two constants are used in the traversal of the image
   inside thresholdAndRuns. We start at the bottom left of the image which
   is (IMAGE_HEIGHT-1)*IMAGE_ROW_OFFSET. ADDRESS_JUMP means we want to move to
   the next column which we do by adding a 1 in there */
static const unsigned int ADDRESS_START = (IMAGE_HEIGHT)*IMAGE_ROW_OFFSET;
static const unsigned int ADDRESS_JUMP = (ADDRESS_START) + 1;

// open field constants
static const int MIN_X_OPEN = 40;

static const int VISUAL_HORIZON_COLOR = BROWN;

//
// DISTANCE ESTIMATES CONSTANTS
// based on Height and Width
static const float POST_MIN_FOC_DIST = 10.0f; // goal posts
static const float POST_MAX_FOC_DIST = 800.0f;

// forward definitions
class Vision;
class VisualFieldObject;
class VisualRobot;

class Threshold
{
    friend class Vision;
public:
    Threshold(Vision* vis, boost::shared_ptr<NaoPose> posPtr);
    virtual ~Threshold() {}

    // main methods
    void visionLoop();
    inline void threshold();
    inline void runs();
    void thresholdAndRuns();
    void objectRecognition();
    // helper methods
    void initObjects(void);
    void initColors();
    void initTable(std::string filename);
    void initTableFromBuffer(byte* tbfr);
    void initCompressedTable(std::string filename);

    void storeFieldObjects();
    void setFieldObjectInfo(VisualFieldObject *objPtr);
    void setVisualRobotInfo(VisualRobot *objPtr);
    float getGoalPostDistFromHeight(float height);
    float getGoalPostDistFromWidth(float width);
    float getBeaconDistFromHeight(float height);
    int distance(int x1, int x2, int x3, int x4);
    float getEuclidianDist(point <int> coord1, point <int> coord2);
    void findGreenHorizon();
    point <int> findIntersection(int col, int dir, int c);
    int postCheck(bool which, int left, int right);
    point <int> backStopCheck(bool which, int left, int right);
    void setYUV(const uchar* newyuv);
    const uchar* getYUV();
    static const char * getShortColor(int _id);

    void swapUV() { inverted = !inverted; setYUV(yuv); }
    void swapUV(bool _inverted) { inverted = _inverted; setYUV(yuv); }


#ifdef OFFLINE
    void setConstant(int c);
    void setHorizonDebug(bool _bool) { visualHorizonDebug = _bool; }
    bool getHorizonDebug() { return visualHorizonDebug; }
#endif

    void initDebugImage();
    void transposeDebugImage();
    void drawX(int x, int y, int c);
    void drawPoint(int x, int y, int c);
    void drawLine(const point<int> start, const point<int> end,
                  const int color);
    void drawVisualHorizon();
    void drawLine(int x, int y, int x1, int y1, int c);
    void drawBox(int left, int right, int bottom, int top, int c);
    void drawRect(int left, int top, int width, int height, int c);


#if ROBOT(NAO_RL)
    inline uchar getY(int x, int y) {
        return yplane[y*IMAGE_ROW_OFFSET+2*x];
    }
    inline uchar getU(int x, int y) {
        return uplane[y*IMAGE_ROW_OFFSET+4*(x/2)];
    }
    inline uchar getV(int x, int y) {
        return vplane[y*IMAGE_ROW_OFFSET+4*(x/2)];
    }
#elif ROBOT(NAO_SIM)
#  error NAO_SIM robot type not implemented
#else
#  error Undefined robot type
#endif

    int getVisionHorizon() { return horizon; }

    inline static int ROUND(float x) {
		return static_cast<int>( std::floor(x + 0.5f) );
    }


    boost::shared_ptr<ObjectFragments> blue;
    boost::shared_ptr<ObjectFragments> yellow;
    boost::shared_ptr<ObjectFragments> orange;
    boost::shared_ptr<ObjectFragments> green;
    boost::shared_ptr<ObjectFragments> navyblue;
    boost::shared_ptr<ObjectFragments> red;

    // main array
    unsigned char thresholded[IMAGE_HEIGHT][IMAGE_WIDTH];

#ifdef OFFLINE
    //write lines, points, boxes to this array to avoid changing the real image
    unsigned char debugImage[IMAGE_HEIGHT][IMAGE_WIDTH];
#endif

    bool inverted;

private:

    // class pointers
    Vision* vision;
    boost::shared_ptr<NaoPose> pose;

    const uchar* yuv;
    const uchar* yplane, *uplane, *vplane;

    unsigned char bigTable[YMAX][UMAX][VMAX];

    // open field variables
    int openField[IMAGE_WIDTH];
    int closePoint;
    int closePoint1;
    int closePoint2;
    bool stillOpen;

    bool greenBlue[IMAGE_WIDTH];
    bool greenYellow[IMAGE_WIDTH];
    int yellowWhite[IMAGE_WIDTH];
    int blueWhite[IMAGE_WIDTH];
    int navyTops[IMAGE_WIDTH];
    int redTops[IMAGE_WIDTH];
    int navyBottoms[IMAGE_WIDTH];
    int redBottoms[IMAGE_WIDTH];

    // thresholding variables
    int horizon;
    int lastPixel;
    int currentRun;
    int previousRun;
    int newPixel;

#ifdef OFFLINE
    // Visual horizon debugging
    bool visualHorizonDebug;
#endif
};

#endif // RLE_h_DEFINED
