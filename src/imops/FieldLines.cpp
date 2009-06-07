
// This file is part of Man, a robotic perception, locomotion, and
// team strategy application created by the Northern Bites RoboCup
// team of Bowdoin College in Brunswick, Maine, for the Aldebaran
// Nao robot.
//
// Man is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Man is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser Public License for more details.
//
// You should have received a copy of the GNU General Public License
// and the GNU Lesser Public License along with Man.  If not, see
// <http://www.gnu.org/licenses/>.


/* Field Lines class */

#include <algorithm>    // for sort() and merge()
#include <boost/shared_ptr.hpp>

#include "FieldLines.h"
using namespace std;


#ifndef OFFLINE
#define haveFound(edgeY) edgeY != NO_EDGE

#define isEdgeClose(edgeLoc, newLoc) (abs(edgeLoc - newLoc) < ADJACENT_SAME_EDGE_SEPARATION)
#define isMoreSuitableTopEdge(topEdgeY,newY,imageColumn) (topEdgeY == NO_EDGE || isEdgeClose(topEdgeY, newY))
#define isMoreSuitableRightEdge(rightEdgeX,newX,y) (rightEdgeX == NO_EDGE || isEdgeClose(rightEdgeX, newX))
#define downhillEdgeWasTooFar(imageColumn,imageRow,dir)
#define secondDownhillButInvalid(imageColumn,imageRow,dir)
#define foundDownhillNoUphill(imageColumn,imageRow,dir)
#define couldNotFindCorrespondingBottom(imageColumn,imageRow)
#define isUphillEdge(new_y_value,old_y_value,dir) (dir == VERTICAL ? new_y_value - old_y_value >= VERTICAL_TRANSITION_VALUE : new_y_value - old_y_value >= HORIZONTAL_TRANSITION_VALUE)
#define isDownhillEdge(new_y_value,old_y_value,dir) (dir == VERTICAL ? old_y_value - new_y_value >= VERTICAL_TRANSITION_VALUE : old_y_value - new_y_value >= HORIZONTAL_TRANSITION_VALUE)

/* Check to see if we are at the top of the image. The top could be capped
 * by the horizon as well.
 */

#define isAtTopOfImage(y, stopValue) (y-stopValue == 1) || (y == 1)
#define isAtRightOfImage(x, endX) (x == endX - 1)
#define isWaitingForAnotherTopEdge(topEdgeY, currentY) (topEdgeY - currentY <= 3)
#define isWaitingForAnotherRightEdge(rightEdgeX, currentX) (currentX - rightEdgeX <= 3)
#define isFirstUphillEdge(uphillEdgeLoc,x,y,dir) (!haveFound(uphillEdgeLoc))
#define resetLineCounters(numWhite,numUndefined,numNonWhite) (numWhite=numUndefined=numNonWhite=0)
#define countersHitSanityChecks(numWhite,numUndefined,numNonWhite,print) (numNonWhite > NUM_NON_WHITE_SANITY_CHECK || numUndefined > NUM_UNDEFINED_SANITY_CHECK)
#endif

using boost::shared_ptr;

const int FieldLines::FIELD_COLORS[NUM_GREEN_COLORS] =
{ GREEN, BLUEGREEN };
const int FieldLines::LINE_COLORS[NUM_WHITE_COLORS] =
{ WHITE, YELLOWWHITE };

const char * FieldLines::linePointInfoFile = "linepoints.xls";

//const float FieldLines::MAX_PERCENT_ERROR_IN_JOIN_LINES = .024f;


FieldLines::FieldLines(Vision *visPtr, shared_ptr<NaoPose> posePtr) {
    vision = visPtr;
    pose = posePtr;

    timeInFindVerticalLinePoints = 0;
    timeInFindHorizontalLinePoints = 0;
    timeInSort = 0;
    timeInMerge = 0;
    timeInCreateLines = 0;
    timeInExtendLines = 0;
    timeInIntersectLines = 0;
    timeInCenterCircleScan = 0;
    timeInIdentifyCorners = 0;
    timeInJoinLines = 0;
    timeInFitUnusedPoints = 0;

    timeInPercentColorBetween = timeInGetAngle = timeInVisualLineCreation = 0;

    numFrames = 0;

    // Initialize the array of VisualFieldObject which we use for distance
    // based identification of corners
    allFieldObjects[0] = vision->bgrp;
    allFieldObjects[1] = vision->bglp;
    allFieldObjects[2] = vision->ygrp;
    allFieldObjects[3] = vision->yglp;

    // When online, these variables are always false and can never be changed,
    // but offline they can be toggled		
#ifdef OFFLINE
    debugVertEdgeDetect = false;
    debugHorEdgeDetect = false;
    debugSecondVertEdgeDetect = false;
    debugCreateLines = false;
    debugJoinLines = false;
    debugIntersectLines = false;
    debugProcessCorners = false;
    debugIdentifyCorners = false;
    debugCornerAndObjectDistances = false;
    debugCcScan = false;

    debugBallCheck = false;

    standardView = true;//false;
#else
    //if we're working offline (in cortex) set isGoalie manually
    //otherwise is goalie is true if player number is 1
    isGoalie = ((vision->getPlayerNumber()) == 1);
#endif

    // Makes setprecision dictate number of decimal places
    cout.setf(ios::fixed);
}

// Main Line Loop. Calls all of the smaller line functions.  Order matters.
void FieldLines::lineLoop() {
    //long startTime;

    //startTime = vision->getMillisFromStartup();
    vector<linePoint> vertLinePoints;
    findVerticalLinePoints(vertLinePoints);
    //timeInFindVerticalLinePoints += (vision->getMillisFromStartup() - startTime);


    //startTime = vision->getMillisFromStartup();
    vector<linePoint> horLinePoints;
    findHorizontalLinePoints(horLinePoints);
    //timeInFindHorizontalLinePoints +=
    //(vision->getMillisFromStartup() - startTime);


    //startTime = vision->getMillisFromStartup();
    sort(horLinePoints.begin(), horLinePoints.end());
    //timeInSort += (vision->getMillisFromStartup() - startTime);


    //startTime = vision->getMillisFromStartup();
    // Must allocate enough space to fit both hor and vert points into this list
    list<linePoint> linePoints(vertLinePoints.size() + horLinePoints.size());
    merge(vertLinePoints.begin(), vertLinePoints.end(),
          horLinePoints.begin(), horLinePoints.end(),
          linePoints.begin());
    //timeInMerge += (vision->getMillisFromStartup() - startTime);

    //startTime = vision->getMillisFromStartup();
    // Creating a copy of the list, maybe not the best way but .. it'll work for now
    linesList = createLines(linePoints); // Lines is a global member of FieldLines
    // Only those linePoints which were not used in any line remain within the
    // linePoints list
    //timeInCreateLines += (vision->getMillisFromStartup() - startTime);


    //startTime = vision->getMillisFromStartup();
    joinLines(linesList);
    //timeInJoinLines += (vision->getMillisFromStartup() - startTime);


    // unusedPoints is a global member of FieldLines; is used by vision to draw
    // points on the screen
    // TODO:  eliminate copying?
    unusedPointsList = linePoints;

    //  startTime = vision->getMillisFromStartup();
    
     if (isGoalie) {
     extendLines(linesList);
    }
    

    // Corners is a global member of FieldLines
    //startTime = vision->getMillisFromStartup();
    cornersList = intersectLines(linesList);
    //timeInIntersectLines += (vision->getMillisFromStartup() - startTime);

    ++numFrames;

#ifdef LINES_PROFILING
    int numFramesToProfile = 30;
    if (numFrames % numFramesToProfile == 0) {
        cout << endl << endl;
        cout << "Average time in Find vertical points: "
             << static_cast<float>(timeInFindVerticalLinePoints)
            / numFramesToProfile << endl;

        cout << "Average time in Find Horizontal points: "
             << static_cast<float>(timeInFindHorizontalLinePoints)
            / numFramesToProfile << endl;

        cout << "Average time in sort: "
             << static_cast<float>(timeInSort)
            / numFramesToProfile << endl;

        cout << "Average time in merge: "
             << static_cast<float>(timeInMerge)
            / numFramesToProfile << endl;

        cout << "Average time in create lines: "
             << static_cast<float>(timeInCreateLines)
            / numFramesToProfile << endl;

        cout << "Average time in join lines: "
             << static_cast<float>(timeInJoinLines)
            / numFramesToProfile << endl;

        cout << "Average time in fit unused points: "
             << static_cast<float>(timeInFitUnusedPoints)
            / numFramesToProfile << endl;

        cout << "Average time in IntersectLines: "
             << static_cast<float>(timeInIntersectLines)
            / numFramesToProfile << endl;

        cout << "Average time in centerCircleScan: "
             << static_cast<float>(timeInCenterCircleScan)
            / numFramesToProfile << endl;

        cout << "Average time in identifyCorners: "
             << static_cast<float>(timeInIdentifyCorners)
            / numFramesToProfile << endl;


        timeInFindVerticalLinePoints = 0;
        timeInFindHorizontalLinePoints = 0;
        timeInSort = 0;
        timeInMerge = 0;
        timeInCreateLines = 0;
        timeInExtendLines = 0;
        timeInIntersectLines = 0;
        timeInCenterCircleScan = 0;
        timeInIdentifyCorners = 0;
        timeInJoinLines = 0;
        timeInFitUnusedPoints = 0;
    }
#endif
}



// While lineLoop is called before object recognition so that ObjectFragments
// can make use of VisualLines and VisualCorners, the methods called from
// here use VisualFieldObject and as such must be performed after the ObjectFragments
// loop is completed.
void FieldLines::afterObjectFragments() {
    // Remove those corners that lead to bad localization problems, like
    // L's on the edge of screen which might be Ts
    if (cornersList.size() > 0) {
        removeRiskyCorners(cornersList);
    }

    identifyCorners(cornersList);
}


// This method populates the points vector with line points it finds in
// the image.  A line point ideally occurs in the middle of a line on the
// screen.  We detect lines via a simple edge detection scheme -
// a transition from green to white involves a big positive jump in Y channel,
// while a transition from white to green involves a big negative jump in Y
// channel.
//
// The vertical in this method name refers to the fact that we start at the
// bottom of the image and scan up for points.
//
// Note:  Whenever we are referring to the Y channel in the YUV color space,
// we will name the variable with underscores (e.g. last_y_value).  Any camel
// case named Y constants (e.g. greenWhiteY) refers to a coordinate on the
// image
// Because this method uses no continues or breaks, we update counters for
// sanity checks at one place only in the entire method.
//
// @param vertLinePoints - the vector to fill with all points found in
// the scan
void FieldLines::findVerticalLinePoints(vector <linePoint> &points) {
    FILE * lp = NULL;
    if (printLinePointInfo) {
        lp = fopen(linePointInfoFile, "a");
        fprintf(lp, "Vertical\n");
    }

    if (debugVertEdgeDetect) {
        printf("\nvertLineEdgeDetect():\n");
    }


    // We ensure that we scan the last column of the image; often very valuable
    // information there
    for (int x = 0; x < IMAGE_WIDTH + COL_SKIP - 1; x += COL_SKIP) {
        if (x > IMAGE_WIDTH - 1) {
            x = IMAGE_WIDTH - 1;
        }

        int greenWhiteY = NO_EDGE;
        int whiteGreenY = NO_EDGE;

        int numWhite = 0;
        int numUndefined = 0;
        int numNonWhite = 0;

        if (debugVertEdgeDetect)
            printf("\tColumn #%d:\n", x);

        int stopY = vision->thresh->getVisionHorizon();

        // We start at the second to last pixel because we will be examining
        // transitions between adjacent pixels, so the first step would be to
        // compare lastY = IMAGE_HEIGHT-1 with currentY = IMAGE_HEIGHT-2
        for (int y = IMAGE_HEIGHT - 2, last_y_value =
                 vision->thresh->getY(x,IMAGE_HEIGHT-1);
             y >= 0 && y > stopY; --y) {


            int current_y_value = vision->thresh->getY(x,y);
            int thresholdedColor = vision->thresh->thresholded[y][x];

            bool isAtAnUphillEdge = isUphillEdge(current_y_value, last_y_value,
                                                 VERTICAL);


            // Do some checks before we actively search for edges

            if (haveFound(greenWhiteY)) {
                updateLineCounters(thresholdedColor, numWhite, numUndefined,
                                   numNonWhite);
                if (haveFound(whiteGreenY)) {

                    // We're beyond the point where we could possibly find another
                    // legitimate top to the line, go ahead and add the line point
                    if (isGreenColor(thresholdedColor) || isAtAnUphillEdge ||
                        !isWaitingForAnotherTopEdge(whiteGreenY, y)
                        // About to exit this iteration of loop due to y position
                        || isAtTopOfImage(y, stopY)) {

                        int linePointY = (whiteGreenY + greenWhiteY) / 2;
                        // greenWhiteY is at bottom, hence higher in our coordinate system
                        int width = greenWhiteY - whiteGreenY;
                        float distance = static_cast<float>(pose->pixEstimate(x, linePointY, 0).dist);

                        if (isReasonableVerticalWidth(x, linePointY,
													  distance, width)) {

                            // assign x, y, lineWidth
                            linePoint point(x, linePointY,
											static_cast<float>(width),
											distance, VERTICAL);
                            if (debugVertEdgeDetect) {
                                cout << "\t\t\tPoint " << point << " passed all checks!"
                                     << endl;
                            }

                            if (printLinePointInfo) {
                                fprintf(lp, "%f\t%d\t(%d,%d)\n",distance,width,x,linePointY);
                            }

                            points.push_back(point);
                            vision->thresh->drawPoint(point.x, point.y,
                                                      UNUSED_VERT_POINT_COLOR);
                        }

                        resetLineCounters(numWhite, numUndefined, numNonWhite);
                        greenWhiteY = NO_EDGE;
                        whiteGreenY = NO_EDGE;
                    }
                }
            } // end  if (haveFound(greenWhiteY))

            if (countersHitSanityChecks(numWhite, numUndefined, numNonWhite,
                                        debugVertEdgeDetect)) {
                resetLineCounters(numWhite, numUndefined, numNonWhite);
                greenWhiteY = NO_EDGE;
                whiteGreenY = NO_EDGE;
            }

            // Possible bottom edge to line
            if (isAtAnUphillEdge) {
                if (isGreenWhiteEdge(x, y, VERTICAL)) {
                    if (isFirstUphillEdge(greenWhiteY, x, y, VERTICAL))
                        // mark this pixel as the bottom
                        greenWhiteY = y;

                    else if (isSecondCloseUphillEdge(x, greenWhiteY, x, y, VERTICAL)) {
                        // Do nothing. Keep going.
                    }

                    // It's the second uphill edge but far away from first.
                    // We missed the last line. Start anew
                    else if (isSecondFarUphillEdge(x, greenWhiteY, x, y, VERTICAL)) {
                        if (debugVertEdgeDetect) {
                            vision->thresh->drawPoint(x, y, YELLOW);
                        }
                        resetLineCounters(numWhite, numUndefined, numNonWhite);
                        greenWhiteY = y;
                    }
                    else {
                        cout << " Should not be reaching this statement at (" << x << ", "
                             << y << ")" << endl;
                    }
                } // end isGreenWhiteEdge

                else if (isSecondUphillButInvalid(x, greenWhiteY, y, x, VERTICAL)) {
                    resetLineCounters(numWhite, numUndefined, numNonWhite);
                    greenWhiteY = NO_EDGE;
                    whiteGreenY = NO_EDGE;
                }
            } // End isUphillEdge()

            // Possible top edge to line
            else if (isDownhillEdge(current_y_value, last_y_value, VERTICAL)) {

                // We've found a bottom edge so this might be the top to the line
                if (haveFound(greenWhiteY)) {
                    // point position would be midway between top and bottom edge.
                    int potentialMidpointY = (greenWhiteY + y) / 2;

                    // Legitimate top line edge
                    if (isWhiteGreenEdge(x, y, potentialMidpointY, VERTICAL)) {
                        // Even if we had already found a top edge, this one might be
                        // more suitable.
                        if (isMoreSuitableTopEdge(whiteGreenY, y, x)) {
                            whiteGreenY = y;
                        }
                        // top edge was too far away
                        else downhillEdgeWasTooFar(x, y, VERTICAL);
                    } // end isWhiteGreenEdge

                    else secondDownhillButInvalid(x, y, VERTICAL);
                } // end have found

                // We haven't found a bottom edge.
                // It is a conscious decision not to allow these line points; they will
                // only appear legitimately at the bottom of the screen and we cannot
                // verify their width.
                else {
                    foundDownhillNoUphill(x, y, VERTICAL);
                }
            } // end else if (isDownhillEdge)
            last_y_value = current_y_value;
        } // end y loop
    } // end x loop

    if (printLinePointInfo) {
        fclose(lp);
    }

} // end method


// This method populates the points vector with line points it finds in
// the image.  A line point ideally occurs in the middle of a line on the
// screen.  We detect lines via a simple edge detection scheme -
// a transition from green to white involves a big positive jump in Y channel,
// while a transition from white to green involves a big negative jump in Y
// channel.
//
// The horizontal in the method name denotes that we start at the left of
// the image and scan to the right to find these points
//
// Note:  Whenever we are referring to the Y channel in the YUV color space,
// we will name the variable with underscores (e.g. last_y_value).  Any camel
// case named Y constants (e.g. greenWhiteY) refers to a coordinate on the
// image
// Because this method uses no continues or breaks, we update counters for
// sanity checks at one place only in the entire method.
//
// @param horLinePoints - the vector to fill with all points found in
// the scan
void FieldLines::findHorizontalLinePoints(vector <linePoint> &points) {

    if (debugHorEdgeDetect) {
        printf("\nhorizontalLineEdgeDetect():\n");
    }
    FILE * lp = NULL;
    if (printLinePointInfo) {
        lp = fopen(linePointInfoFile, "a");
        fprintf(lp, "Horizontal\n");
    }

    // Slope is in image coordinates, meaning a negative slope rises to the right
    float horizonSlope = pose->getHorizonSlope();
    bool startAtHorizon = false;
    if (horizonSlope < 0) {
        startAtHorizon = true;
    }
    // scan bottom to top
    int poseHorizon = min(pose->getHorizonY(0),
                          pose->getHorizonY(IMAGE_WIDTH - 1));
    int visualHorizon = vision->thresh->getVisionHorizon();
    int highestPoint = max(0, min(poseHorizon, visualHorizon));
    for (int y = IMAGE_HEIGHT - 1; y > highestPoint; y -= ROW_SKIP) {

        int greenWhiteX = NO_EDGE;
        int whiteGreenX = NO_EDGE;
        int numWhite = 0;
        int numUndefined = 0;
        int numNonWhite = 0;

        // Start by calculating the y value one pixel to left of where we're
        // starting
        int last_y_value = vision->thresh->getY(0, y);

        if (debugHorEdgeDetect)
            cout <<"\tRow #" << y << endl;

        // Start 1 pixel from left so we can test the pixel before to determine
        // starting edge value
        for (int x = 1; x < IMAGE_WIDTH - 1; x++) {
            int current_y_value = vision->thresh->getY(x,y);
            int thresholdedColor = vision->thresh->thresholded[y][x];

            bool isAtAnUphillEdge = isUphillEdge(current_y_value, last_y_value,
                                                 HORIZONTAL);
            // Do some checks before we actively search for edges
            if (haveFound(greenWhiteX)) {

                // We have found a whiteGreen (right) edge, we might accept it right
                // now or we might wait a little longer to ensure we get the full width
                // of the line
                if (haveFound(whiteGreenX)) {

                    // We're beyond the point where we could possibly find another
                    // legitimate right to the line, go ahead and add the line point
                    if (isGreenColor(thresholdedColor) || isAtAnUphillEdge ||
                        !isWaitingForAnotherRightEdge(whiteGreenX, x)
                        // About to exit this iteration of loop due to x position
                        || isAtRightOfImage(x, IMAGE_WIDTH - 1)) {

                        // SANITY CHECK: lines have a much bigger width when they are
                        // up close
                        int linePointX = (whiteGreenX + greenWhiteX) / 2;
                        int width = whiteGreenX - greenWhiteX;
                        float distance = static_cast<float>(pose->pixEstimate(linePointX, y, 0).dist);
                        if (isReasonableHorizontalWidth(linePointX, y, distance, width)) {
                            // assign x, y, lineWidth
                            linePoint point(linePointX, y,
                                            static_cast<float>(width)
											, distance, HORIZONTAL);
                            if (debugHorEdgeDetect) {
                                cout << "\t\tPoint " << point << " accepted." << endl;
                            }

                            if (printLinePointInfo) {
                                fprintf(lp, "%f\t%d\t(%d,%d)\n",distance,width,linePointX,y);
                            }

                            vision->thresh->drawPoint(point.x, point.y,
                                                      UNUSED_HOR_POINT_COLOR);
                            points.push_back(point);
                        }
                        else {
                            if (debugHorEdgeDetect)
                                cout << "Width was invalid, ignoring line point" << endl;
                        }
                        resetLineCounters(numWhite, numUndefined, numNonWhite);
                        greenWhiteX = NO_EDGE;
                        whiteGreenX = NO_EDGE;
                    }
                }
                else {
                    // We are currently "in" a line so keep track of the pixel value at
                    // this point so we know whether the line has too much undefined or
                    // nonwhite, or is too thick
                    updateLineCounters(thresholdedColor, numWhite, numUndefined,
                                       numNonWhite);
                }
            }
            // end if (haveFound(whiteGreenX))

            if (countersHitSanityChecks(numWhite, numUndefined, numNonWhite,
                                        debugHorEdgeDetect)) {
                resetLineCounters(numWhite, numUndefined, numNonWhite);
                greenWhiteX = NO_EDGE;
                whiteGreenX = NO_EDGE;
            }

            // Possible left edge to line
            if (isAtAnUphillEdge) {
                if (isGreenWhiteEdge(x, y, HORIZONTAL)) {

                    if (isFirstUphillEdge(greenWhiteX, x, y, HORIZONTAL)) {
                        // Mark this pixel as left
                        greenWhiteX = x;
                    }
                    else if (isSecondCloseUphillEdge(greenWhiteX, y, x, y, HORIZONTAL)) {
                        // Do nothing.  Keep going
                    }
                    // It's the second uphill edge but far away from first.
                    // We missed the last line. Start anew from this point
                    else if (isSecondFarUphillEdge(greenWhiteX, y, x, y, HORIZONTAL)) {
                        vision->thresh->drawPoint(x, y, YELLOW);
                        resetLineCounters(numWhite, numUndefined, numNonWhite);
                        greenWhiteX = x;
                    }
                    else {
                        cout << " Should not be reaching this statement at (" << x << ", "
                             << y << ")" << endl;
                    }
                } // end isGreenWhiteEdge

                else if (isSecondUphillButInvalid(greenWhiteX, y, x, y, HORIZONTAL)) {
                    resetLineCounters(numWhite, numUndefined, numNonWhite);
                    greenWhiteX = NO_EDGE;
                    whiteGreenX = NO_EDGE;
                }
                else {
                    if (debugHorEdgeDetect) {
                        cout << "\t\tHit an uphill edge but it was not green white or "
                             << "a second close uphill edge, or second far uphill edge "
                             << "at (" << x << ", " << y<< ")" << endl;
                    }
                }

            } // End isUphillEdge()

            // Possible right edge to line
            else if (isDownhillEdge(current_y_value, last_y_value, HORIZONTAL)) {

                if (haveFound(greenWhiteX)) {
                    // point position would be midway between left and right edge
                    int potentialMidpointX = (greenWhiteX + x) / 2;
                    if (isWhiteGreenEdge(x, y, potentialMidpointX, HORIZONTAL)) {
                        // Even if we had already found a right edge, this one might be
                        // more suitable.
                        if (isMoreSuitableRightEdge(whiteGreenX, x, y)) {
                            whiteGreenX = x;
                        }
                        // right edge was too far away
                        else downhillEdgeWasTooFar(x, y, HORIZONTAL);
                    } // end isWhiteGreenEdge

                    else secondDownhillButInvalid(x, y, HORIZONTAL);
                } // end haveFound

                // We haven't found a left edge.
                else {
                    foundDownhillNoUphill(x,y, HORIZONTAL);
                }
            } // end else if (isDownhillEdge)
            last_y_value = current_y_value;
        } // end x loop
    } // end y loop

    if (printLinePointInfo) {
        fprintf(lp, "\n");
        fclose(lp);
    }
} // end method



// ------------------  Section for verbose helper methods ------------------
// These methods are all really simple and their names are meant to be self
// explanatory comments. This really helps keep down the length of
// findVerticalLinePoints down to an acceptable minimum. They also take care
// of printing debugging info and as such have to accept diverse parameters.
//



// TODO: Make all inlined?

// Check to see if we are at the top of the image. The top could be capped
// by the horizon as well.
//
#ifdef OFFLINE
const bool FieldLines::isAtTopOfImage(const int y, const int stopValue) {
    return (y - stopValue == 1) || (y == 1);
}


const bool FieldLines::isAtRightOfImage(const int x, const int endX) {
    return x == endX - 1;
}

// In images it often happens that lines have double uphill and downhill
// edges. We are thus forced to wait a few pixels before we say a top edge
// is legitimate. We would like to capture the last one, so that the line's
// thickness matches what people would estimate it to be.
const bool FieldLines::isWaitingForAnotherTopEdge(const int topEdgeY,
                                                  const int currentY) {
    return topEdgeY - currentY <= 3;
}

const bool FieldLines::isWaitingForAnotherRightEdge(const int rightEdgeX,
                                                    const int currentX) {
    return currentX - rightEdgeX <= 3;
}


// Check if the current uphill edge is the first one we've seen.
const bool FieldLines::isFirstUphillEdge(const int uphillEdgeLoc,
                                         const int x,
                                         const int y,
                                         const ScanDirection dir) const {
    const bool print = (dir == VERTICAL && debugVertEdgeDetect) ||
        (dir == HORIZONTAL && debugHorEdgeDetect);
    if (!haveFound(uphillEdgeLoc)) {
        // mark this pixel as the bottom
        if (print) {
            const point <const int> newEdgePoint(x, y);
            cout << "\t\tFound the first uphill edge of a line at "
                 << newEdgePoint << endl;
        }
        return true;
    }
    return false;
}
#endif

// We have already found a bottom/left edge that is very close spatially
// to this one; do not replace that edge but just mark info down
// about the thresholded value at this point.
const bool FieldLines::isSecondCloseUphillEdge(const int oldEdgeX,
                                               const int oldEdgeY,
                                               const int newEdgeX,
                                               const int newEdgeY,
                                               const ScanDirection direction) const {
    int oldEdgeLocation, newEdgeLocation;
    bool print;
    if (direction == HORIZONTAL) {
        oldEdgeLocation = oldEdgeX;
        newEdgeLocation = newEdgeX;
        print = debugHorEdgeDetect;
    }
    else {
        oldEdgeLocation = oldEdgeY;
        newEdgeLocation = newEdgeY;
        print = debugVertEdgeDetect;
    }
    if (isEdgeClose(oldEdgeLocation, newEdgeLocation)) {
        if (print) {
            const point <const int> newEdgePoint(newEdgeX, newEdgeY);
            const point <const int> oldEdgePoint(oldEdgeX, oldEdgeY);
            cout << "\t\tFound uphill edge at " << newEdgePoint
                 << " that was close to previously marked edge at "
                 << oldEdgePoint << endl;
        }
        return true;
    }
    return false;
}

// Check to see if two "bottom" edges were found too far apart.
// It used to be that we checked whether we had found a top edge, but that
// is no longer necessary. Always returns true for now.
const bool FieldLines::isSecondFarUphillEdge(const int oldEdgeX,
                                             const int oldEdgeY,
                                             const int newX,
                                             const int newY,
                                             const ScanDirection direction) const {
    bool print = (direction == VERTICAL && debugVertEdgeDetect) ||
        (direction == HORIZONTAL && debugHorEdgeDetect);
    if (print) {
        const point <const int> oldEdgePoint(oldEdgeX, oldEdgeY);
        const point <const int> newEdgePoint(newX, newY);
        cout << "\t\tFound uphill edge at " << newEdgePoint
             << " that was too far away from previously marked edge at "
             << oldEdgePoint << endl;
    }
    return true;
}

// Check if the invalid green white transition we just found is the second
// uphill edge we've found
const bool FieldLines::isSecondUphillButInvalid(const int oldEdgeX,
                                                const int oldEdgeY,
                                                const int newEdgeX,
                                                const int newEdgeY,
                                                const ScanDirection dir) const {

    int oldEdgeLoc, newEdgeLoc;
    bool print;
    if (dir == HORIZONTAL) {
        oldEdgeLoc = oldEdgeX;
        newEdgeLoc = newEdgeX;
        print = debugHorEdgeDetect;
    }
    else {
        oldEdgeLoc = oldEdgeY;
        newEdgeLoc = newEdgeY;
        print = debugVertEdgeDetect;
    }

    if (oldEdgeLoc != NO_EDGE &&
        !isEdgeClose(oldEdgeLoc, newEdgeLoc)) {
        if (print) {
            const point <const int> edgePoint(newEdgeX, newEdgeY);
            cout << "\t\tFound uphill edge at " << edgePoint
                 << " but it was not a green white vertical edge.  Resetting."
                 << endl;
        }
        return true;
    }
    return false;
}


#ifdef OFFLINE

// Either we have not found a top edge yet, or the new edge is
// very close to the old top edge.
// We will replace the current top edge (whiteGreenY) with our
// newly found white green edge; sometimes we get tops of lines that
// have adjacent points which each count as edges and we want
// to get the top most edge in order to get the true width of the
// line.
const bool FieldLines::isMoreSuitableTopEdge(const int topEdgeY,
                                             const int newY,
                                             const int imageColumn) const {
    if (topEdgeY == NO_EDGE || isEdgeClose(topEdgeY, newY)) {
        if (debugVertEdgeDetect) {
            const point <const int> newEdgePoint(imageColumn, newY);
            const point <const int> oldEdgePoint(imageColumn, topEdgeY);
            if (topEdgeY == NO_EDGE) {
                cout << "\t\tFound the first downhill edge at "
                     << newEdgePoint << endl;
            }
            else {
                cout << "\t\tFound a new downhill edge at "
                     << newEdgePoint << ". Replacing the old one at "
                     << oldEdgePoint << endl;
            }
        }
        return true;
    }
    return false;
}

// Either we have not found a right edge yet, or the new edge is
// very close to the old right edge.
// We will replace the current right edge (whiteGreenX) with our
// newly found white green edge; sometimes we get right sides of lines that
// have adjacent points which each count as edges and we want
// to get the right most edge in order to get the true width of the
// line.
const bool FieldLines::isMoreSuitableRightEdge(const int rightEdgeX,
                                               const int newX,
                                               const int y) const {
    if (rightEdgeX == NO_EDGE || isEdgeClose(rightEdgeX, newX)) {
        if (debugHorEdgeDetect) {
            const point <const int> newEdgePoint(newX, y);
            const point <const int> oldEdgePoint(rightEdgeX, y);
            if (rightEdgeX == NO_EDGE) {
                cout << "\t\tFound the first downhill edge at "
                     << newEdgePoint << endl;
            }
            else {
                cout << "\t\tFound a new downhill edge at "
                     << newEdgePoint << ". Replacing the old one at "
                     << oldEdgePoint << endl;
            }
        }
        return true;
    }
    return false;
}


/// Notify that the downhill edge we found was too far from the last potential
// downhill edge we currently have on record.
///

void FieldLines::downhillEdgeWasTooFar(const int imageColumn,
                                       const int imageRow,
                                       const ScanDirection dir) const {
    const bool print = (dir == VERTICAL && debugVertEdgeDetect) ||
        (dir == HORIZONTAL && debugHorEdgeDetect);
    if (print) {
        const point <const int> downhillEdgePoint(imageColumn, imageRow);
        cout << "\t\tFound a downhill edge at " << downhillEdgePoint
             << ", but it was too far away. Ignoring." << endl;
    }
}

/// Notify that we found a second downhill edge but it failed our sanity checks
// for a valid top edge.
///

void FieldLines::secondDownhillButInvalid(const int imageColumn,
                                          const int imageRow,
                                          const ScanDirection dir) const {
    const bool print = (dir == VERTICAL && debugVertEdgeDetect) ||
        (dir == HORIZONTAL && debugHorEdgeDetect);
    if (print) {
        const point <const int> topEdgePoint(imageColumn, imageRow);
        cout << "\t\tFound a downhill edge at " << topEdgePoint
             << ", but it wasn't a whiteGreenVertical edge. Ignoring" << endl;
    }
}

/// Notify that we found a downhill edge without having previously found an
// uphill edge. This will definitely happen when lines are at the very bottom
// of the screen.
///

void FieldLines::foundDownhillNoUphill(const int imageColumn,
                                       const int imageRow,
                                       const ScanDirection dir) const {
    const bool print = (dir == VERTICAL && debugVertEdgeDetect) ||
        (dir == HORIZONTAL && debugHorEdgeDetect);
    if (print) {
        const point <const int> edgePoint(imageColumn, imageRow);
        cout <<"\t\tFound a downhill edge at " << edgePoint << endl;
    }
}

/// This is used for the case where we find a top edge without having found an
// uphill edge beforehand. Notify that we were unable to find any uphill edge
// corresponding to the downhill edge we've found.
///

void FieldLines::couldNotFindCorrespondingBottom(const int imageColumn,
                                                 const int imageRow) const {
    if (debugVertEdgeDetect) {
        cout <<"\t\tFailed checks." << endl;
        vision->thresh->drawPoint(imageColumn,imageRow,CYAN);
    }
}


const bool FieldLines::isEdgeClose(const int edgeLoc,
                                   const int newLoc) {
    return abs(edgeLoc - newLoc) < ADJACENT_SAME_EDGE_SEPARATION;
}
#endif


/// This method checks if based on pose-estimated distance, the line point
// we just found appears in a line of reasonable width.
///
const bool FieldLines::isReasonableVerticalWidth(const int x, const int y,
                                                 const float distance,
                                                 const int width) const {
    if (width < 0) {
        if (debugVertEdgeDetect) {
            cout << "isReasonableVerticalWidth() failed: Negative width invalid."
                 << endl;
        }
        return false;
    }
    if (distance <= 0) {
        if (debugVertEdgeDetect) {
            point<int> badP(x,y);
            cout << "isReasonableVerticalWidth() failed: Distance to point "
                 << badP << " was invalid (" << distance
                 << "cm)" << endl;
        }
        return false;
    }

    //return true;

    // These are based on 640x480 images
    // See https://robocup.bowdoin.edu/files/nao/NaoLineWidthData.xls
    if (distance < 100)
        return width < 65;
    else if (distance < 150)
        return width < 40;
    else if (distance < 200)
        return width < 22;
    else if (distance < 250)
        return width < 16;
    else if (distance < 300)
        return width < 11;
    else if (distance < 400)
        return width < 7;
    else
        return width < 5;
}

// TODO: Gross unnamed constants that are probably wrong
const bool FieldLines::isReasonableHorizontalWidth(const int x, const int y,
                                                   const float distance,
                                                   const int width) const {

    if (width < 0) {
        return false;
    }
    if (distance <= 0) {
        if (debugHorEdgeDetect) {
            point<int> badP(x,y);
            cout << "Distance to point " << badP << " was invalid (" << distance
                 << "cm)" << endl;
        }
        return false;
    }

    // These are based on 640x480 images
    // See https://robocup.bowdoin.edu/files/nao/NaoLineWidthData.xls
    //const int ERROR_ALLOWED = 8;
    //return true;
    return width < 6111.8f * std::pow(distance, -1.0701f);
}


#ifdef OFFLINE
/* 'Uphill' in this context means that we're going from a low Y value
 * to a higher Y value (as in YUV). This gets used to find lower edges
 * in the case of vertical edge detection and left edges in the case of
 * horizontal edge detection.
 */

const bool FieldLines::isUphillEdge(const int new_y_value,
                                    const int old_y_value,
                                    const ScanDirection dir) {
    if (dir == VERTICAL)
        return new_y_value - old_y_value >= VERTICAL_TRANSITION_VALUE;
    else
        return new_y_value - old_y_value >= HORIZONTAL_TRANSITION_VALUE;
}


/* 'Uphill' in this context means that we're going from a low Y value
 * to a higher Y value (as in YUV). This gets used to find lower edges
 * in the case of vertical edge detection and left edges in the case of
 * horizontal edge detection.
 */

const bool FieldLines::isDownhillEdge(const int new_y_value,
                                      const int old_y_value,
                                      const ScanDirection dir) {
    if (dir == VERTICAL)
        return old_y_value - new_y_value >= VERTICAL_TRANSITION_VALUE;
    else
        return old_y_value - new_y_value >= HORIZONTAL_TRANSITION_VALUE;
}
#endif



// Attempts to create lines out of a list of linePoints.  In order for points
// to be fit onto a line, they must pass a battery of sanity checks
vector <VisualLine> FieldLines::createLines(list <linePoint> &linePoints) {
    vector <VisualLine> lines;


    if (debugCreateLines)
        cout << "Grouping lines now with " << linePoints.size()
             << " line points" << endl << endl;

    /////////////// MASTER LOOP   /////////////////////////////////
    // For each point, attempt to create a line starting from that point and
    // use as many points as possible until either running out or hitting
    // a sanity check that precludes all further points from passing.
    // If the number of points that fit a line are greater than some threshold,
    // the line is considered legitimate, and all points on that line are
    // removed from consideration for all subsequent lines.
    //////////////////////////////////////////////////////////////
    int counter = 0;

    for (linePointNode firstPoint = linePoints.begin();
         firstPoint != linePoints.end(); counter++) {

        // debug print
        if (debugCreateLines) {
            cout << "MAIN LOOP: Scanning for potential line #" << lines.size() <<
                " with Point #" << counter << " (" << firstPoint->x
                 << ", " << firstPoint->y << ")" << endl;
        }

        list<linePointNode> legitimateLinePoints;
        // we begin a line from this point so we consider it legitimate.
        legitimateLinePoints.push_back(firstPoint);

        //////////// SECOND LOOP ///////////
        // 'ADDING NEXT POINT' LOOP
        // scan through all remaining line points

        int lineStartpointX = firstPoint->x;
        int lineStartpointY = firstPoint->y;

        // Keeps track of the last point we've added to our list
        linePointNode back = firstPoint;
        linePointNode currentPoint = back;
        // We start by considering the remaining points in the list, starting with
        // the one immediately following firstPoint
        currentPoint++;

        for (; currentPoint != linePoints.end(); currentPoint++) {

            // Grab the right endpoint of the potential line and store it.
            int lineEndpointX = back->x;
            int lineEndpointY = back->y;
            // Grab the current point's (x,y)
            int pointX = currentPoint->x;
            int pointY = currentPoint->y;

            // debug print
            if (debugCreateLines)
                cout << "\tSecond loop: " << " --- lineEndpointX: " << lineEndpointX
                     << " lineEndpointY: " << lineEndpointY << " NEW POINT: x: "
                     << pointX << " y: " << pointY << endl;

            ///// SECOND LOOP SANITY CHECKS //////
            // SANITY CHECK: X OFFSET
            // checks if transition point is > an offset # of pixels away
            // if it is, call off scan
            if (abs(pointX-lineEndpointX) > GROUP_MAX_X_OFFSET) {
                if (debugCreateLines)
                    cout << "\tSecond loop: Sanity check 'X Offset' failed, x1 "
                         << lineEndpointX << ", x2: " << pointX << endl;
                break;
            }


            // SANITY CHECK: The points we're considering need to belong to a
            // line of similar width.
            // We compare the lineWidth of the last point the potentialLine has in
            // its list of points and the lineWidth of the currentPoint
            // In addition, if the new point is closer to us than the last point,
            // the lineWidth should have increased or at most decrease by some small
            // number of pixels.



            float lastLineWidth = back->lineWidth;

            if (true || back->foundWithScan == currentPoint->foundWithScan) {
                float lastPointDistance = static_cast<float>(pose->pixEstimate(back->x, back->y, 0).dist);
                float curPointDistance = static_cast<float>(pose->pixEstimate(pointX, pointY, 0).dist);

                float distanceDifference = curPointDistance - lastPointDistance;
                float lineWidthDifference = currentPoint->lineWidth - lastLineWidth;

                if ((distanceDifference < 0 && // line is going toward us
                    // TODO named constant
                    (lineWidthDifference < -2 || // The line shouldn't shrink more than 2 pix
                     lineWidthDifference > 5) ) ||

                    (distanceDifference >= 0 && // line is going away from us
                    (lineWidthDifference > 2 ||
                     lineWidthDifference < -5)) ) {
                    if (debugCreateLines)
                        cout << "\tSecond loop: Sanity check 'Points of similar line widths "
                             << "failed', line width 1: " << lastLineWidth
                             << ", line width 2:" << currentPoint->lineWidth << endl;
                    continue;
                }

            }
            //long startTime;
            //startTime = vision->getMillisFromStartup();
            // TODO: move to bottom
            // SANITY CHECK: Check to see if the segment formed by the old line
            // endpoint and the current point, has green in between.

            // Do not do the check if the line point is thin and far away because
            // by nature of the integer truncation, the line points can show up
            // towards the bottom of the line, where there will be green between the
            // two points.  Furthermore, only skip this check if it's horizontally
            // oriented
            float percentGreen;
            if (currentPoint->lineWidth < MIN_PIXEL_WIDTH_FOR_GREEN_CHECK &&
                lastLineWidth < MIN_PIXEL_WIDTH_FOR_GREEN_CHECK &&
                abs(pointX - lineEndpointX) > abs(pointY - lineEndpointY) &&
                Utility::getLength(static_cast<float>(lineEndpointX),
								   static_cast<float>(lineEndpointY),
								   static_cast<float>(pointX),
								   static_cast<float>(pointY) )
                < MIN_SEPARATION_TO_NOT_CHECK) {
                percentGreen = 0;
            }
            else {
                percentGreen = percentColorBetween(lineEndpointX, lineEndpointY,
                                                   pointX, pointY,
                                                   GREEN);
            }
            if (percentGreen > MAX_GREEN_PERCENT_ALLOWED_IN_LINE) {
                if (debugCreateLines)
                    cout << "\tSecond loop: sanity check 'Found too much green between "
                         << "line points.'  Found " << percentGreen << "%, max of "
                         << MAX_GREEN_PERCENT_ALLOWED_IN_LINE << "% allowed." << endl;
                continue;
            }
            //timeInPercentColorBetween += vision->getMillisFromStartup() - startTime;


            //startTime = vision->getMillisFromStartup();
            // ANGLE CHECK:  Check to see if the horizontal angle of the line as
            // it stands now is significantly different from the horizontal angle
            // of the new segment we're adding.  See Utility::getAngle(x1,y1,x2,y2).
            // Note:  This check makes no sense for the first point we do not perform
            // it in this case
            // Also, we only want to check the angle between the points if they are
            // not extremely close to each other - if they are, the angle error will
            // be too high.
            if (legitimateLinePoints.size() != 1 &&
                Utility::getLength(static_cast<float>(lineEndpointX),
								   static_cast<float>(lineEndpointY),
								   static_cast<float>(pointX),
                                   static_cast<float>(pointY) ) >
				MIN_PIXEL_DIST_TO_CHECK_ANGLE) {
                float curLineAngle = Utility::getAngle(lineStartpointX,
                                                       lineStartpointY,
                                                       lineEndpointX,
                                                       lineEndpointY);
                float segmentAngle = Utility::getAngle(lineEndpointX,
                                                       lineEndpointY,
                                                       pointX,
                                                       pointY);
                float difference = min(fabs(curLineAngle - segmentAngle),
                                       180 - (fabs(curLineAngle - segmentAngle)));
                if (difference > MAX_ANGLE_LINE_SEGMENT) {
                    if (debugCreateLines)
                        cout <<"\tSecond loop: sanity check 'Angle between line and line "
                             << "segment' failed. Difference was " << difference
                             << ", max allowed is " << MAX_ANGLE_LINE_SEGMENT << endl;
                    continue;
                }
            }
            //timeInGetAngle += vision->getMillisFromStartup() - startTime;

            ////// SECOND LOOP MEAT /////

            // POINT HAS PASSED ALL SANITY CHECKS, IS NOW PART OF A LINE!
            if (debugCreateLines) {
                cout << "\tSecond loop: Point "<< counter<< " passed all sanity checks:"
                    "x2: " << pointX << " y2:" << pointY << " added to line " <<
                    lines.size() << endl;
            }

            legitimateLinePoints.push_back(currentPoint);
            back = currentPoint;
        } // END SECOND LOOP

        // We are finished adding all points to the line starting from "firstPoint".
        // We can attempt to form a line. An exception is thrown if something
        // failed. If all goes well, the points we used in the line are removed
        // from further consideration.
        //startTime = vision->getMillisFromStartup();

        // Too few points
        if (legitimateLinePoints.size() < VisualLine::NUM_POINTS_TO_BE_VALID_LINE)
            firstPoint++;
        else {
            VisualLine aLine(legitimateLinePoints);
            if (debugCreateLines) {
                cout << "\tSecond loop: adding line " << lines.size()
                     << " with " << legitimateLinePoints.size() << " line points.\n";
            }

            drawLinePoints(legitimateLinePoints);
            aLine.setColor(static_cast<int>(lines.size()) + BLUEGREEN);
            aLine.setColorString(Utility::getColorString(aLine.color));
            lines.push_back(aLine);

            // Now we need to delete the linePoints that went into the newly found
            // line from the list of all linePoints.
            // :TRICKY: Modification of this code will likely lead to segfaults
            for (list<linePointNode>::reverse_iterator
                     i = legitimateLinePoints.rbegin();
                 i != legitimateLinePoints.rend(); ++i) {
                firstPoint = linePoints.erase(*i);
            }

            if (debugCreateLines) {
                // Draw a bounding box around the line points we put into the line
                drawSurroundingBox(aLine, CYAN);
                drawFieldLine(aLine, aLine.color);
            }
        }
        //timeInVisualLineCreation += vision->getMillisFromStartup() - startTime;
    }

    if (debugCreateLines) {
        cout << linePoints.size() << " points remain after forming "
             << lines.size() << " lines" << endl;
    }
#ifdef LINES_PROFILING
    if (numFrames % 30 == 0) {
        cout << endl;
        cout << "\ttime in percent color between: " << static_cast<float>(timeInPercentColorBetween)/numFrames << endl;
        cout << "\ttime in get angle: " << static_cast<float>(timeInGetAngle)/30 << endl;
        cout << "\ttime in creating the visual lines: "
             << static_cast<float>(timeInVisualLineCreation)/30 << endl;
        timeInPercentColorBetween = timeInGetAngle = timeInVisualLineCreation = 0;
    }
#endif

    return lines;
}

void FieldLines::drawLinePoints(const list<linePointNode> &toDraw) const {
    for (list<linePointNode>::const_iterator i = toDraw.begin();
         i != toDraw.end(); ++i) {
        if ((*i)->foundWithScan == VERTICAL)
            drawLinePoint(**i, BLACK);
        else
            drawLinePoint(**i, RED);
    }
}

void FieldLines::drawLinePoints(const list<linePoint> &toDraw) const {
    for (list<linePoint>::const_iterator i = toDraw.begin();
         i != toDraw.end(); ++i) {
        if (i->foundWithScan == VERTICAL)
            drawLinePoint(*i, BLACK);
        else
            drawLinePoint(*i, RED);
    }
}

void FieldLines::drawCorners(const list<VisualCorner> &toDraw, int color) {
    for (list<VisualCorner>::const_iterator i = toDraw.begin();
         i != toDraw.end(); ++i) {
        vision->thresh->drawPoint(i->getX(), i->getY(), color);
    }
}


// Attempts to fit the left over points that were not used within the
// createLines function to the lines that were output from said function
void FieldLines::fitUnusedPoints(vector<VisualLine> &lines,
                                 list<linePoint> &remainingPoints) {

    // Sort lines by length because we figure that the shortest line can most
    // benefit from adding more points to it and we want the algorithm to be
    // greedy for speed and simplicity's sake
    sort(lines.begin(), lines.end());

    int numPointsRemainining = static_cast<int>(remainingPoints.size());

    if (debugFitUnusedPoints)
        cout << "Beginning fitUnusedPoints with " << lines.size() << " lines and "
             << numPointsRemainining << " unused points." << endl;

    for (vector<VisualLine>::iterator i = lines.begin(); i != lines.end(); ++i) {
        bool foundAdditionalPoints = false;
        list <linePoint> additionalPoints;
        // We will manually increment the j counter so that we can delete points
        // from the list
        for (linePointNode j = remainingPoints.begin();
             j != remainingPoints.end(); ) {

            bool sanityChecksPass = true;

            // Calculate how far away from the line the point is
            const float jDistFromI = Utility::getPointDeviation(*i, j->x, j->y);
            const float maxDist = 2.0f;

            if (jDistFromI > maxDist) {
                sanityChecksPass = false;
                if (debugFitUnusedPoints)
                    cout << "Point " << (*j) << " was too far away from the "
                         << i->colorStr << " line to be "
                         << "included.  Was " << jDistFromI << " pixels away, needed to "
                         << "be at most " << maxDist << " away." << endl;
            }


            // We will not add any horizontal points to a line which is composed
            // entirely of vertical points and vice versa

            // Ensure that the line width of the point vertically found is not too
            // different from that of the line
            else if (j->foundWithScan == VERTICAL) {
                float widthDiff = fabs(j->lineWidth - i->avgVerticalWidth);
                if (widthDiff > MAX_VERT_FIT_UNUSED_WIDTH_DIFFERENCE) {
                    if (debugFitUnusedPoints) {
                        cout << "Point " << (*j) << " had a vertical width that differed "
                             << "too much from the line width of the " << i->colorStr
                             << " line; average width of line was " << i->avgVerticalWidth
                             << ", line point width was " << j->lineWidth
                             << ". Max width difference is "
                             << MAX_VERT_FIT_UNUSED_WIDTH_DIFFERENCE << endl;
                    }
                    sanityChecksPass = false;
                }
            }

            // Ensure that the line width of the point horizontally found is not too
            // different from that of the line.  Since horizontally found lines
            // can differ in width so much more dramatically than vertically found
            // lines, doing a simple difference in width check is a bit dangerous.
            else if (j->foundWithScan == HORIZONTAL) {

                float dist = j->distance;
                float thinnestDist = i->thinnestHorPoint.distance;
                float thickestDist = i->thickestHorPoint.distance;

                float width = j->lineWidth;
                float thickestWidth = i->thickestHorPoint.lineWidth;
                float thinnestWidth = i->thinnestHorPoint.lineWidth;

                // There are no horizontal points in the line
                if (i->avgHorizontalWidth == 0) {
                    if (debugFitUnusedPoints) {
                        cout << "Point " << (*j) << " rejected because there were no "
                             << "horizontal points in the line." << endl;
                    }
                    sanityChecksPass = false;
                }
                // The line point is farther away so it should be thinner
                else if (dist > thinnestDist) {
                    if (width > thinnestWidth) {
                        if (debugFitUnusedPoints) {
                            cout << setprecision(2)
                                 << "Point " << (*j) << " had a horizontal width that "
                                 << "was wider than that of the thinnest part of the line "
                                 << "despite being farther away.  Width: "
                                 << width << "; thinnest width: "
                                 << thinnestWidth << endl;
                        }
                        sanityChecksPass = false;
                    }
                }
                // Line point is closer, so it should be thicker
                else if (dist < thickestDist) {
                    if (width < thickestWidth) {
                        if (debugFitUnusedPoints) {
                            cout << setprecision(2)
                                 << "Point " << (*j) << " had a horizontal width that "
                                 << "was thinner than that of the thickest part of the line "
                                 << "despite being closer.  Width: "
                                 << width << "; thickest width: "
                                 << thickestWidth << endl;
                        }
                        sanityChecksPass = false;
                    }
                }
                // Between the thickest and thinnest, make sure it's between the two
                // line widths
                else {
                    if (width < thinnestWidth || width > thickestWidth) {
                        if (debugFitUnusedPoints) {
                            cout << setprecision(2)
                                 << "Point " << (*j) << " had a horizontal width that "
                                 << "was not between the line widths of thickest and "
                                 << "thinnest portions of the " << i->colorStr << " line; "
                                 << "width was " << width << "; expected to be between "
                                 << thinnestWidth << " and " << thickestWidth << endl;
                        }
                        sanityChecksPass = false;
                    }
                }
            }

            // Check that distance between the two points isn't insane


            // Check for green between the two points

            // If the line point being added isn't between the endpoints of the
            // line, we need to check for green between the endpoint and this
            // point
            if (sanityChecksPass && !(Utility::between(*i, *j))) {
                const point<int> closerEndpoint =
                    Utility::getCloserEndpoint(*i, j->x, j->y);
                const float percentGreenBetween =
                    percentColorBetween(j->x, j->y,
                                        closerEndpoint.x, closerEndpoint.y, GREEN);
                const float MAX_GREEN_BETWEEN = 5.0f;
                if (percentGreenBetween > MAX_GREEN_BETWEEN) {
                    if (debugFitUnusedPoints) {
                        cout << setprecision(2)
                             << "Point " << (*j) << " had too much green between it and "
                             << "the endpoint " << closerEndpoint << " of the "
                             << i->colorStr << " line; found " << percentGreenBetween
                             << ", expected at most " << MAX_GREEN_BETWEEN << endl;
                    }
                    sanityChecksPass = false;
                }
            }


            if (sanityChecksPass) {
                foundAdditionalPoints = true;
                additionalPoints.push_back(*j);
                if (j->foundWithScan == VERTICAL) {
                    // Standard users do not care about how the point was fit to the line
                    if (standardView) {
                        vision->thresh->drawPoint(j->x, j->y, USED_VERT_POINT_COLOR);
                    }
                    else {
                        vision->thresh->drawPoint(j->x, j->y, FIT_VERT_POINT_COLOR);
                    }
                }
                else {
                    if (standardView) {
                        vision->thresh->drawPoint(j->x, j->y, USED_HOR_POINT_COLOR);
                    }
                    else {
                        vision->thresh->drawPoint(j->x, j->y, FIT_HOR_POINT_COLOR);
                    }
                }
                j = remainingPoints.erase(j);
            }
            else {
                ++j;
            }

        }
        // Redo the least squares regression, find left/top/right/bottom over
        // again
        if (foundAdditionalPoints)
            i->addPoints(additionalPoints);
    }


    if (debugFitUnusedPoints) {
        for (vector<VisualLine>::iterator i = lines.begin();
             i != lines.end(); ++i) {
            drawSurroundingBox(*i, FIT_UNUSED_POINTS_BOX_COLOR);
            drawFieldLine(*i, i->color);

        }
        int numPointsAdded = numPointsRemainining -
            static_cast<int>(remainingPoints.size());
        cout << "Successfully added " << numPointsAdded << " points to the lines. "
             << remainingPoints.size() << " points remain unfit. " << endl;
    }

}

// Attempts to join together line segments that are logically part of one
// longer line but for some reason were not grouped within the groupPoints
// method.  This can often happen when there is an obstruction that obscures
// part of the line; due to x offset sanity checks, points that are too far
// apart are not allowed to be within the same line in createLines.
void FieldLines::joinLines(vector <VisualLine> &lines) {
    int numberOfJoinedLines = 0;

    // Compare every pair of lines and merge pairs of lines if they are
    // close enough
    for (vector <VisualLine>::iterator i = lines.begin(); i != lines.end(); ++i) {
        for (vector <VisualLine>::iterator j = i + 1; j != lines.end(); ++j) {
            string iColor = Utility::getColorString(i->color);
            string jColor = Utility::getColorString(j->color);

            if (debugJoinLines) {
                cout <<"Attempting to join the " << iColor << " line "
                     << "and the " << jColor << " line." << endl;
            }

            // ANGLE sanity check
            // Vertical lines get an angle of 90 but near vertical lines often have an
            // angle of maybe -88 or something.  This is a workaround
            const float angleBetween = min(fabs(i->angle - j->angle),
                                           fabs(180-(fabs(i->angle-j->angle))));
            if (angleBetween > MAX_ANGLE_TO_JOIN_LINES) {
                if (debugJoinLines) {
                    cout << "\tAngle sanity check failed: lines had an angle of "
                         << setprecision(2) << angleBetween << " between them; "
                         << "expected a maximum difference of "
                         << MAX_ANGLE_TO_JOIN_LINES   << endl;
                }
                continue;
            }

            // DISTANCE sanity check: even if two lines have a very small angle
            // between them, they might not be legitimately part of the same line;
            // it could be the case that we have two parallel lines that lie apart
            // on the screen

            // Evaluate the value of the line i at j's start point to determine
            // how far away the line deviates
            const float jDistFromI = Utility::getPointDeviation(*i, j->start.x,
                                                                j->start.y);

            // Evaluate the value of the line j at i's start point to determine
            // how far away the line deviates
            float iDistFromJ = Utility::getPointDeviation(*j, i->start.x,
                                                          i->start.y);;

            double avg = (jDistFromI + iDistFromJ) / 2.0;
            if (avg > MAX_DIST_BETWEEN_TO_JOIN_LINES) {
                if (debugJoinLines) {
                    cout << "\tDistance from line sanity check failed: lines "
                         << "had a difference of " << avg << " between them; "
                         << "the max difference allowed is "
                         << MAX_DIST_BETWEEN_TO_JOIN_LINES << endl;
                }
                continue;
            }

            // Passed all sanity checks
            if (debugJoinLines) {
                cout << "\tPASSED all sanity checks; joining " << iColor
                     << " and " << jColor << " lines now." << endl;
            }
            // Replace the information in i with the union of i and j and recalculate
            // the variables in the line struct
            int oldColor = i->color;
            //*i = mergeLines(*i,*j);
            i->addPoints(j->points);
            i->setColor(oldColor);
            ++numberOfJoinedLines;
            if (debugJoinLines)
                drawSurroundingBox(*i, JOIN_LINES_BOX_COLOR );
            // erase the redundant line and position our j pointer correctly
            // tricky: do not change
            j = lines.erase(j) - 1;
        }
    }

    // Output how many lines were joined and draw a bounding box around them
    if (debugJoinLines) {
        cout << "Successfully joined " << numberOfJoinedLines << " pairs of lines." << endl;
    }
}


// Copies the data from line1 and 2 into a new single line.
const VisualLine FieldLines::mergeLines(const VisualLine &line1,
                                        const VisualLine &line2) {
    list<linePoint> linePoints(line1.points.size() + line2.points.size());
    //cout << "Merging in mergeLines" << endl;
    merge(line1.points.begin(), line1.points.end(),
          line2.points.begin(), line2.points.end(),
          linePoints.begin());
    return VisualLine(linePoints);
}

// For each line we have identified on the screen, attempts to extend the
// line farther to both the right and the left in the case of mostly
// horizontal lines, or to the top and bottom in the case of mostly
// vertical lines
void FieldLines::extendLines(vector <VisualLine> &lines) {
    if (debugExtendLines) {
        cout << "In extendLines with " << lines.size() << " lines. " << endl;
    }
    for (vector <VisualLine>::iterator i = lines.begin(); i != lines.end(); ++i) {
        // The line is more vertically oriented on the screen than horizontal,
        // scan above and below
        if (VisualLine::isVerticallyOriented(*i)) {
            extendLineVertically(*i);
        }
        // Scan to the left and right
        else {
            //extendLineHorizontally(*i);
        }
    }
}


// Given a VisualLine, attempts to find points along the equation of the line
// above the topmost point and below the bottommost point that may have been
// missed in the horizontal scans due to lack of green on one side of the line.
// As long as we have a downhill edge from white to green on at least one side
// of the line, we can add that point to the line.  This method helps the goalie
// classify corners as T's instead of outer L's when he pans his head.
void FieldLines::extendLineVertically(VisualLine &line) {
    if (debugExtendLines) {
        cout << "In extendLineVertically with the " << line.colorStr << " line."
             << endl;
        cout << "Stats: " << line << endl;
    }

    list <linePoint> foundLinePoints;
    float slope = line.a;
    float yIntercept = line.b;

    // Keep track of coords so that we can check between the last point and
    // current point for green
    int topX = 0;
    int topY = line.top;
    int bottomX = 0;
    int bottomY = line.bottom;

    // start point matches the top point
    if (line.top == line.start.y) {
        topX = line.start.x;
        bottomX = line.end.x;
    }
    // start point matches the bottom point
    else {
        topX = line.end.x;
        bottomX = line.start.x;
    }

    if (debugExtendLines) {
        cout << "left: " << line.left << " right: " << line.right << " top: " << line.top << " bottom: " << line.bottom << endl;
        point<int> topP(topX, topY);
        point<int> bottomP(bottomX, bottomY);
        cout<< "Top point:" << topP << " bottom point: " << bottomP << endl;
    }

    // TODO: Figure a way to reduce this duplication of code

    if (debugExtendLines) { cout << "Extending to the top" << endl; }
    // Extend towards the top of the image
    for (int y = topY - ROW_SKIP; y > vision->thresh->getVisionHorizon();
         y -= ROW_SKIP) {
        // if VisualLine is perfectly vertical, do not attempt to use slope
        int startX = (VisualLine::isPerfectlyVertical(line)) ?
            line.left :
            Utility::getLineX(y, yIntercept, slope);

        // lastX, lastY, curX, curY
        if (shouldStopExtendingLine(topX, topY, startX, y)) {
            break;
        }
        else if (!isLineColor(vision->thresh->thresholded[y][startX])) {
            continue;
        }
        // Since we are scanning top to bottom, we are looking for HORIZONTAL
        // points
        linePoint newPoint = findLinePointFromMiddleOfLine(startX, y, HORIZONTAL);
        if (newPoint != VisualLine::DUMMY_LINEPOINT) {

            if (abs(line.avgHorizontalWidth - newPoint.lineWidth) >
                MAX_EXTEND_LINES_WIDTH_DIFFERENCE ) {
                if (debugExtendLines) {
                    cout << "Line point " << newPoint << " was too different in width "
                         << "from the line " << line << endl;
                }
                continue;
            }

            if (debugExtendLines) {
                cout << "\tAdding point " << newPoint << endl;
            }
            if (standardView) {
                vision->drawPoint(newPoint.x, newPoint.y, USED_HOR_POINT_COLOR);
            }
            else {
                vision->drawPoint(newPoint.x, newPoint.y, YELLOW);
            }

            foundLinePoints.push_back(newPoint);
        }
    }


    if (debugExtendLines) { cout << "Extending to the bottom" << endl; }
    // Extend to the bottom of screen
    for (int y = bottomY + ROW_SKIP; y < IMAGE_HEIGHT; y += ROW_SKIP) {
        // This is the supposed midpoint in the line, will do further checks
        // before saying that it's part of a valid line point
        // if VisualLine is perfectly vertical, do not attempt to use slope
        int startX = (VisualLine::isPerfectlyVertical(line)) ?
            line.left :
            Utility::getLineX(y, yIntercept, slope);


        // lastX, lastY, curX, curY
        if (shouldStopExtendingLine(bottomX, bottomY, startX, y)) {
            break;
        }
        else if (!isLineColor(vision->thresh->thresholded[y][startX])) {
            continue;
        }
        // Since we are scanning top to bottom, we are looking for HORIZONTAL
        // points
        linePoint newPoint = findLinePointFromMiddleOfLine(startX, y, HORIZONTAL);
        if (newPoint != VisualLine::DUMMY_LINEPOINT) {

            if (abs(line.avgHorizontalWidth - newPoint.lineWidth) >
                MAX_EXTEND_LINES_WIDTH_DIFFERENCE ) {
                if (debugExtendLines) {
                    cout << "Line point " << newPoint << " was too different in width "
                         << "from the line " << line << endl;
                }
                continue;
            }


            if (debugExtendLines) {
                cout << "\tAdding point " << newPoint << endl;
                cout << "\t Width of point to be added: " << newPoint.lineWidth << endl;
                cout << "\taverage width of line: " << line.avgHorizontalWidth << endl;
            }
            if (standardView) {
                vision->drawPoint(newPoint.x, newPoint.y, USED_HOR_POINT_COLOR);
            }
            else {
                vision->drawPoint(newPoint.x, newPoint.y, PINK);
            }
            foundLinePoints.push_back(newPoint);
        }
    }

    if (!foundLinePoints.empty()) {
        line.addPoints(foundLinePoints);

        if (debugExtendLines) {
            drawSurroundingBox(line, LAWN_GREEN);
            drawFieldLine(line, line.color);
        }

    }
}

// Currently not ready for prime time, I have some more work to do here.
void FieldLines::extendLineHorizontally(VisualLine &line) {
    if (debugExtendLines) {
        cout << endl;
        cout << "In extendLineHorizontally with the " << line.colorStr << " line."
             << endl;
        cout << "Stats: " << line << endl;
    }

    list <linePoint> foundLinePoints;
    float slope = line.a;
    float yIntercept = line.b;

    // Keep track of coords so that we can check between the last point and
    // current point for green
    int leftX = line.left;
    int leftY = 0;
    int rightX = line.right;
    int rightY = 0;

    // start point matches the left point
    if (line.left == line.start.x) {
        leftY = line.start.y;
        rightY = line.end.y;
    }
    // start point matches the right
    else {
        leftY = line.end.y;
        rightY = line.start.y;
    }

    if (debugExtendLines) {
        point<int> leftP(leftX, leftY);
        point<int> rightP(rightX, rightY);
        cout<< "Left point:" << leftP << " right point: " << rightP << endl;
    }

    if (debugExtendLines) {
        cout << "\tExtending left." << endl;
    }
    // Extend to the left
    for (int x = leftX - COL_SKIP; x > 0; x -= COL_SKIP) {
        int startY = Utility::getLineY(x, yIntercept, slope);
        // lastX, lastY, curX, curY
        if (shouldStopExtendingLine(leftX, leftY, x, startY)) {
            break;
        }
        else if (!isLineColor(vision->thresh->thresholded[startY][x])) {
            continue;
        }

        // Since we are scanning left to right, we are looking for VERTICAL line
        // points (the lines are near horizontal in the screen)
        linePoint newPoint = findLinePointFromMiddleOfLine(x, startY, VERTICAL);
        if (newPoint != VisualLine::DUMMY_LINEPOINT) {
            // TODO: Make sure the line width isn't too different from the avg
            // of the line (or the closest point on the line)
            if (debugExtendLines) {
                cout << "\tAdding point " << newPoint << endl;
            }
            if (standardView) {
                vision->drawPoint(newPoint.x, newPoint.y, USED_VERT_POINT_COLOR);
            }
            else {
                vision->drawPoint(newPoint.x, newPoint.y, RED);
            }
            foundLinePoints.push_back(newPoint);
        }
        /*
        // Update our variables to reflect that we're past this point
        leftX = x;
        leftY = startY;
        */
    }
    if (debugExtendLines) {
        cout << "\tExtending right." << endl;
    }
    // Extend to the right
    for (int x = rightX + COL_SKIP; x < IMAGE_WIDTH; x += COL_SKIP) {
        int startY = Utility::getLineY(x, yIntercept, slope);
        // lastX, lastY, curX, curY
        if (shouldStopExtendingLine(rightX, rightY, x, startY)) {
            break;
        }
        else if (!isLineColor(vision->thresh->thresholded[startY][x])) {
            if (debugExtendLines) {

                point<int>badP(x,startY);
                vision->thresh->drawPoint(x,startY,RED);
                cout << "\t" << badP << " was not a valid line point because of the color "
                     << Utility::getColorString(vision->thresh->thresholded[startY][x])
                     << endl;
            }
            continue;
        }

        // Since we are scanning left to right, we are looking for VERTICAL line
        // points (the lines are near horizontal in the screen)
        linePoint newPoint = findLinePointFromMiddleOfLine(x, startY, VERTICAL);
        if (newPoint != VisualLine::DUMMY_LINEPOINT) {
            // TODO: Make sure the line width isn't too different from the avg
            // of the line (or the closest point on the line)
            if (debugExtendLines) {
                cout << "\tAdding point " << newPoint << endl;
            }
            if (standardView) {
                vision->drawPoint(newPoint.x, newPoint.y, USED_VERT_POINT_COLOR);
            }
            else {
                vision->drawPoint(newPoint.x, newPoint.y, PURPLE);
            }
            foundLinePoints.push_back(newPoint);
        }
    }
    if (!foundLinePoints.empty()) {
        line.addPoints(foundLinePoints);

        if (debugExtendLines) {
            drawSurroundingBox(line, RED);
            drawFieldLine(line, line.color);
        }
    }
}




// Returns true if the new point trying to be added to the line is offscreen or
// there is too much green in between the old and new point.  Any further
// searching in this direction would be foolish.
const bool FieldLines::shouldStopExtendingLine(const int oldX, const int oldY,
                                               const int newX, const int newY)
    const {

    // A point off screen means that any extending we continue to do in that
    // direction will also yield points that are off screen
    if (!Utility::isPointOnScreen(newX, newY)) {
        if (debugExtendLines) {
            point<int> currentPoint(newX, newY);
            cout << currentPoint << " could not possibly be a point on "
                 << "the line because it is off screen."
                 << endl;
        }
        return true;
    }

    // Too much green between the last and most recent point means we've hit the
    // logical end of the line; any further continuance is extremely dangerous
    const float MAX_GREEN_PERCENT_BETWEEN = 10.0f;
    const float percentGreenBetween = percentColorBetween(oldX, oldY,
                                                          newX, newY, GREEN);
    if (percentGreenBetween > MAX_GREEN_PERCENT_BETWEEN) {
        if (debugExtendLines) {
            point<int> oldPoint(oldX, oldY);
            point<int> currentPoint(newX, newY);
            cout << " Found too much green between " << oldPoint << " and "
                 << currentPoint << "; found " << percentGreenBetween
                 << "%, expected at most " << MAX_GREEN_PERCENT_BETWEEN
                 << "%.  Breaking out of loop" << endl;
        }
        return true;
    }
    return false;
}

// Helper method that just returns whether the thresholded color is a
// line color
const bool FieldLines::isLineColor(const int color) {
    return Utility::isElementInArray(color, LINE_COLORS, NUM_LINE_COLORS);
}

// Helper method that just returns whether the thresholded color is a
// green color
const bool FieldLines::isGreenColor(int threshColor) {
    return Utility::isElementInArray(threshColor, FIELD_COLORS,
                                     NUM_GREEN_COLORS);
}



// Given an (x, y) location and a direction (horizontal or vertical) in which
// to look, attempts to find edges on either side of the (x,y) location.  If
// there are no edges, or if another sanity check fails, returns
// VisualLine::DUMMY_LINEPOINT.  Otherwise it returns the linepoint with
// the correct (x,y) location and width and scan.
linePoint FieldLines::findLinePointFromMiddleOfLine(int x, int y,
                                                    ScanDirection dir) {
    if (dir == HORIZONTAL) {
        // TODO: figure out a limit to search
        int leftX = findEdgeFromMiddleOfLine(x, y, 100, TEST_LEFT);
        int rightX = findEdgeFromMiddleOfLine(x, y, 100, TEST_RIGHT);

        // We can't use this point
        if (leftX == NO_EDGE && rightX == NO_EDGE) {
            return VisualLine::DUMMY_LINEPOINT;
        }
        int lineWidth = 0;
        int linePointX = 0;
        if (leftX == NO_EDGE && rightX != NO_EDGE) {
            linePointX = x;
            lineWidth = 2 * (rightX - x);
        }
        else if (rightX == NO_EDGE && leftX != NO_EDGE) {
            linePointX = x;
            lineWidth = 2 * (x - leftX);
        }
        // Both hit green
        else {
            linePointX = (leftX + rightX) / 2;
            lineWidth = rightX - leftX;
        }
        linePoint newPoint(linePointX, y,
						   static_cast<float>(lineWidth),
						   HORIZONTAL);
        return newPoint;
    }
    // Vertical
    else {
        // TODO: Named constant
        int topY = findEdgeFromMiddleOfLine(x, y, 100, TEST_UP);
        int bottomY = findEdgeFromMiddleOfLine(x, y, 100, TEST_DOWN);

        // We can't use this point
        if (topY == NO_EDGE && bottomY == NO_EDGE) {
            if (debugExtendLines) {
                point<int>badP(x,y);
                cout << "\t\tPoint " << badP << " had no edges above or below; "
                     << "discarding." << endl;
            }
            return VisualLine::DUMMY_LINEPOINT;
        }
        int lineWidth = 0;
        int linePointY = 0;
        if (topY == NO_EDGE && bottomY != NO_EDGE) {
            if (debugExtendLines) {
                point<int>goodP(x,y);
                cout << "\t\tPoint " << goodP << " had no edge above but found one "
                     << "below.  Accepting. " << endl;
            }

            linePointY = y;
            lineWidth = 2 * (bottomY - y);
        }
        else if (bottomY == NO_EDGE && topY != NO_EDGE) {
            if (debugExtendLines) {
                point<int>goodP(x,y);
                cout << "\t\tPoint " << goodP << " had no edge below but found one "
                     << "above.  Accepting. " << endl;
            }

            linePointY = y;
            lineWidth = 2 * (y - topY);
        }
        // Both hit green
        else {
            // Ensure that the point was relatively in the middle; if not, something
            // is wrong
            // Named constant
            if (abs( (bottomY - y) - (y - topY) ) > 5) {
                if (debugExtendLines) {
                    cout << "The line point was not near enough to the center, discarding"
                         << endl;
                }
                return VisualLine::DUMMY_LINEPOINT;
            }
            if (debugExtendLines) {
                point<int>goodP(x,y);
                cout << "\t\tPoint " << goodP << " had an edge above and one "
                     << "below.  Accepting. " << endl;
            }
            linePointY = (topY + bottomY) / 2;
            lineWidth = bottomY - topY;
        }
        linePoint newPoint(x, linePointY,
						   static_cast<float>(lineWidth), VERTICAL);
        return newPoint;
    }
}

// Unlike our normal method for finding line points, this searches from the
// middle of a line outward for an edge, in a given direction, up to a max
// of maxPixelsToSearch.  If no edge is found, returns NO_EDGE.
const int FieldLines::findEdgeFromMiddleOfLine(int x, int y,
                                               int maxPixelsToSearch,
                                               TestDirection dir) const {
    int oldYChannel = vision->thresh->getY(x,y);
    int sign = 1;
    int stopValue = 0;
    // Vertical test
    if (dir == TEST_UP || dir == TEST_DOWN) {
        if (dir == TEST_UP) {
            // Stop at the top of the screen or maxPixelsToSearch higher than start
            // point, whichever is lower on the image (fewer pixels to search)
            stopValue = max(0, y - maxPixelsToSearch);
            sign = -1;
        }
        else {
            // Stop at bottom of the screen or maxPixelsToSearch below, whichever is
            // higher on the image
            stopValue = min(IMAGE_HEIGHT - 1, y + maxPixelsToSearch);
            sign = 1;
        }
        // Iterate over the y axis until we either find the edge or we hit
        // the stopping point
        for (int j = y; j != stopValue; j += sign) {
            int newYChannel = vision->thresh->getY(x, j);
            if (isDownhillEdge(newYChannel, oldYChannel, VERTICAL) &&
                percentColor(x, j, dir, FIELD_COLORS, NUM_GREEN_COLORS,
                             NUM_TEST_PIXELS) >= GREEN_PERCENT_CLEARANCE) {
                // We found the edge we were looking for.
                return j;
            }
            // We're in the field but we didn't see an edge.  No good.
            else if (!isLineColor(vision->thresh->thresholded[j][x])) {
                //      else if (vision->thresh->thresholded[j][x] == GREEN) {
                return NO_EDGE;
            }
            oldYChannel = newYChannel;
        }
        return NO_EDGE;
    } // end dir == TEST_UP || dir == TEST_DOWN

    // Horizontal test
    else {
        if (dir == TEST_LEFT) {
            // Stop at left edge of screen or maxPixelsToSearch to the left, whichever
            // is farther to the right (fewer pixels)
            stopValue = max(0, x - maxPixelsToSearch);
            sign = -1;
        }
        else {
            // stop at right edge of screen or maxPixelsToSearch to the right,
            // whichever is farther to the left (fewer pixels)
            stopValue = min(IMAGE_WIDTH - 1, x + maxPixelsToSearch);
            sign = 1;
        }

        // Iterate over the x axis until we either find the edge or hit the
        // stopping point
        for (int i = x; i != stopValue; i += sign) {
            int newYChannel = vision->thresh->getY(i, y);
            if (isDownhillEdge(newYChannel, oldYChannel, HORIZONTAL) &&
                percentColor(i, y, dir, FIELD_COLORS, NUM_GREEN_COLORS,
                             NUM_TEST_PIXELS) >= GREEN_PERCENT_CLEARANCE) {
                // We found the edge we were looking for.
                return i;
            }
            // We're in the field but we didn't see an edge.  No good.
            else if (vision->thresh->thresholded[y][i] == GREEN) {
                return NO_EDGE;
            }
            oldYChannel = newYChannel;
        }
        return NO_EDGE;
    } // end horizontal test
} // end method








// Pairwise tests each line on the screen against each other, calculates
// where the intersection occurs, and then subjects the intersection
// to a battery of sanity checks before determining that the intersection
// is a legitimate corner on the field.
// @param lines - the vector of visual lines that have been found after
// createLines, join lines, and fit unused points.
// @return a vector of VisualCorners created from the intersection points that
// successfully pass all sanity checks.
//
list <VisualCorner> FieldLines::intersectLines(vector <VisualLine> &lines) {
    list <VisualCorner> corners;

    if (debugIntersectLines) {
        cout <<"Beginning intersectLines() with " << lines.size() << " lines.."
             << endl;
    }
    // Keep track of the number of duplicate intersections (where two intersection
    // points occur very close to each other on the screen).  In places not near
    // the center circle, there should be very few duplicates, if any.
    int numDupes = 0;

    if (standardView) {
        for (vector <VisualLine>::iterator i = lines.begin(); i != lines.end(); ++i) {
            drawSurroundingBox(*i, BLACK);
            drawFieldLine(*i, i->color);
        }
    }

    // Compare every pair of lines
    for (vector <VisualLine>::iterator i = lines.begin(); i != lines.end(); ++i) {

        for (vector <VisualLine>::iterator j = i+1; j != lines.end(); ++j) {
            string iColor = i->colorStr;
            string jColor = j->colorStr;
            int numChecksPassed = 0;

            // get intersection
            point<int> intersection = Utility::getIntersection(*i,*j);
            int intersectX = intersection.x;
            int intersectY = intersection.y;

            if (debugIntersectLines) {
                cout << endl << "Trying to form an intersection between the "
                     << iColor << " and "  << jColor << " lines.." << endl;
            }

            if (intersectX == NO_INTERSECTION) {
                if (debugIntersectLines)
                    cout << "\t" << numChecksPassed
                         <<"- Lines are parallel. No intersection" << endl;
                continue;
            }
            else {
                if (debugIntersectLines) {
                    cout << "\tIntersection occurs at " << intersection << endl;
                }
            }
            vision->thresh->drawPoint(intersectX, intersectY,
                                      TENTATIVE_INTERSECTION_POINT_COLOR);


            // Duplicate corner check:  ensure that corner is not too close to any
            // previously found corners
            if (dupeCorner(corners, intersectX, intersectY, numChecksPassed)) {
                ++numDupes;
                if (numDupes > MAX_NUM_DUPES) {
                    if (debugIntersectLines || debugCcScan) {
                        cout <<"\t" << numChecksPassed
                             << "-'Num duplicates' sanity check failed; found "
                             << numDupes << " duplicate intersections; max allowed is "
                             << MAX_NUM_DUPES << endl;
                    }
                    vision->drawPoint(intersectX, intersectY,
                                      INVALIDATED_INTERSECTION_POINT_COLOR);
                    // at this point, we are by a center and so we discard all
                    // corners after recoloring them so we can tell they are illegitimate
                    drawCorners(corners, INVALIDATED_INTERSECTION_POINT_COLOR);
                    corners.clear();
                    return corners;
                }
                continue;
            }
            ++numChecksPassed;



            // Angle check: only intersect those lines that have a minimum angle
            // between them.  This catches many bad intersections at the center circle
            // where the lines form very shallow angles
            float angleOnScreen = min(fabs(i->angle - j->angle),
                                      fabs(180-(fabs(i->angle-j->angle))));

            /*cout << i->colorStr << " has angle of " << i->angle << " and "
              << j->colorStr << " has angle of " << j->angle << endl;
            */

            if (angleOnScreen < MIN_ANGLE_BETWEEN_INTERSECTING_LINES) {
                if (debugIntersectLines) {
                    cout << "\t" << numChecksPassed
                         << "- Failed due to an insufficient angle between the lines. "
                         << "Found angle of " << setprecision(2) << angleOnScreen
                         << "; need at least "
                         << MIN_ANGLE_BETWEEN_INTERSECTING_LINES << endl;
                }
                continue;
            }
            ++numChecksPassed;


            // Off screen sanity check: only allow corners that are within image
            // frame
            if (!Utility::isPointOnScreen(intersection)) {
                if (debugIntersectLines)
                    cout << "\t" << numChecksPassed
                         << "-Intersection point was off screen." << endl;
                continue;
            }
            ++numChecksPassed;

            // Nao's pix estimates seem slightly better than the aibo, resulting in
            // better angles returned.
#if ROBOT(NAO)
            float angleOnField = getEstimatedAngle(*i, *j, intersectX, intersectY);
            //cout << "angle on field: " << angleOnField << endl;
            // BAD_ANGLE signifies the angle could not be computed
            if (angleOnField != BAD_ANGLE &&
                (angleOnField < MIN_ANGLE_ON_FIELD ||
                 angleOnField > MAX_ANGLE_ON_FIELD)) {
                if (debugIntersectLines) {
                    cout << "\t" << numChecksPassed
                         << "- Failed due to an angle on the field that is too "
                         << "far from 90 degrees. Found angle of "
                         << setprecision(2) << angleOnField
                         << ".  Expected angle to be between " << MIN_ANGLE_ON_FIELD
                         << " and " << MAX_ANGLE_ON_FIELD << endl;
                }
                continue;
            }
            ++numChecksPassed;
#endif

            // Too much green check:  Discard potential corners where the intersection
            // point is actually in the field green rather than in a line
            float percentGreen = percentSurrounding(intersection,
                                                    FIELD_COLORS,
                                                    NUM_GREEN_COLORS,
                                                    CORNER_TEST_RADIUS);
            if (percentGreen > MAX_GREEN_PERCENT_ALLOWED_AT_CORNER) {
                if (debugIntersectLines)
                    cout << "\t" << numChecksPassed
                         << "-'Corner surrounded by green' sanity check failed: "
                         << "Found " << setprecision(2) << percentGreen
                         << "% green around the point; max allowed is "
                         << MAX_GREEN_PERCENT_ALLOWED_AT_CORNER << "%"
                         << endl;
                continue;
            }
            ++numChecksPassed;

            // Too small check: ensure that at least one of the line segments is
            // long enough (if both are small it indicates we might be at the
            // center circle)
            int MIN_LENGTH_LINE_TO_INTERSECT = 30; // cm
            if (i->length < TWO_CORNER_LINES_MIN_LENGTH &&
                j->length < TWO_CORNER_LINES_MIN_LENGTH &&
                getEstimatedLength(*i) < MIN_LENGTH_LINE_TO_INTERSECT &&
                getEstimatedLength(*j) < MIN_LENGTH_LINE_TO_INTERSECT) {

                if (debugIntersectLines)
                    cout << "\t" << numChecksPassed
                         << "-'Lines too small' sanity check failed. "
                         << "The " << iColor << " line was " << setprecision(2)
                         << i->length << " pixels long, the " << jColor << " line was "
                         << j->length << " pixels long.  Require at least "
                         << TWO_CORNER_LINES_MIN_LENGTH << endl;
                continue;
            }
            ++numChecksPassed;


            // Cross corner check: ensure that the lines do not both continue through
            // the point of intersection to form a cross; this indicates we have
            // malformed lines at the center circle

            // We parameterize the line such that x and y are functions of one
            // variable t. Then we figure out what t gives us the corner's
            // x coordinate. When t = 0, x = x1; when t = lineLength, x = x2;
            float t_I = Utility::
                findLinePointDistanceFromStart(intersection, *i);
            float t_J = Utility::
                findLinePointDistanceFromStart(intersection, *j);


            if (Utility::tValueInMiddleOfLine(t_I, i->length,
                                              static_cast<float>(MIN_CROSS_EXTEND)) &&
                Utility::tValueInMiddleOfLine(t_J, j->length,
                                              static_cast<float>(MIN_CROSS_EXTEND))) {
                if (debugIntersectLines || debugCcScan)
                    cout <<"\t" << numChecksPassed
                         << "-'Cross corner' sanity check failed, intersection "
                         << "point was within the endpoints of both lines.  Discarding "
                         << "all corners" << endl;
                // at this point, we are by a center and so we discard all corners after
                // recoloring them so we can tell they are illegitimate
                drawCorners(corners, INVALIDATED_INTERSECTION_POINT_COLOR);
                corners.clear();
                return corners;
            }
            ++numChecksPassed;


            // Distance sanity check:  We cannot trust any corners more than around
            // 100 cm away.  Due to deficiencies in pose, extremely far distances
            // actually show up as negative numbers (we have a polynomial function
            // that maxes out at around 110 and then decreases towards negative
            // infinity)
            estimate pixEstimate = pose->pixEstimate(intersectX, intersectY,
                                                     LINE_HEIGHT);
            float distance = static_cast<float>(pixEstimate.dist);
            float bearing = static_cast<float>(pixEstimate.bearing);
            // Ridiculously far away points have -distance due to pose problems.
            if (distance > MAX_CORNER_DISTANCE || distance < MIN_CORNER_DISTANCE) {
                if (debugIntersectLines)
                    cout <<"\t" << numChecksPassed
                         << "-'Distance' sanity check failed: estimated distance: "
                         << distance << "cm, require distance between "
                         << MIN_CORNER_DISTANCE << "cm and " << MAX_CORNER_DISTANCE
                         << "cm" << endl;
                continue;
            }
            ++numChecksPassed;

            // Lines on the screen are not perfect.  As a result, we allow our corners
            // to be made even if the lines do not both actually reach the corner
            // in question.  We create a box around each line that extends parallel
            // and orthogonal to the line and then test whether both boxes contain
            // the intersection, rather than testing whether the lines themselves
            // both contain the intersection
            BoundingBox box1 = Utility::
                getBoundingBox(*j,
                               INTERSECT_MAX_ORTHOGONAL_EXTENSION,
                               INTERSECT_MAX_PARALLEL_EXTENSION);
            BoundingBox box2 = Utility::
                getBoundingBox(*i,
                               INTERSECT_MAX_ORTHOGONAL_EXTENSION,
                               INTERSECT_MAX_PARALLEL_EXTENSION);
            if (debugIntersectLines) {
                drawBox(box1, RED);
                drawBox(box2, BLUE);
            }

            bool box1Contains = Utility::
                boxContainsPoint(box1, intersectX, intersectY);
            bool box2Contains = Utility::
                boxContainsPoint(box2, intersectX, intersectY);
            if (!(box1Contains && box2Contains)) {
                if (debugIntersectLines) {
                    cout <<"\t" << numChecksPassed
                         << "-Point was not contained in both bounding boxes." << endl;
                    cout << "\tDistance was " << distance << endl;
                }
                continue;
            }
            ++numChecksPassed;

            // Find which end of each line is closer to the potential corner
            point<int> line1Closer;
            point<int> line2Closer;

            if (Utility::getLength(static_cast<float>(intersectX),
								   static_cast<float>(intersectY),
                                   static_cast<float>(i->start.x),
								   static_cast<float>(i->start.y) ) <
                Utility::getLength(static_cast<float>(intersectX),
								   static_cast<float>(intersectY),
                                   static_cast<float>(i->end.x),
								   static_cast<float>(i->end.y)) ) {
                line1Closer = i->start;
            }
            else
                line1Closer = i->end;

            if (Utility::getLength(static_cast<float>(intersectX),
								   static_cast<float>(intersectY),
                                   static_cast<float>(j->start.x),
								   static_cast<float>(j->start.y) ) <
                Utility::getLength(static_cast<float>(intersectX),
								   static_cast<float>(intersectY),
                                   static_cast<float>(j->end.x),
								   static_cast<float>(j->end.y) )) {
                line2Closer = j->start;
            }
            else
                line2Closer = j->end;

            if (debugIntersectLines)
                cout << "Corner is close to line endpoint " << line1Closer
                     << " and " << line2Closer << endl;

            float percent1 = percentSurrounding((line1Closer.x + intersectX)/2,
                                               (line1Closer.y + intersectY)/2,
                                               FIELD_COLORS,
                                               NUM_FIELD_COLORS,
                                               1);
            float percent2 = percentSurrounding((line2Closer.x + intersectX)/2,
                                               (line2Closer.y + intersectY)/2,
                                               FIELD_COLORS,
                                               NUM_FIELD_COLORS,
                                               1);
            if (debugIntersectLines)
                cout << "Percent green in between line 1 and corner: "
                     << percent1 << ". between line 2 --- : " << percent2
                     << endl;

            const float MAX_PERCENT_GREEN_BETWEEN_CORNER_LINE = 66.0f;
            if (percent1 > MAX_PERCENT_GREEN_BETWEEN_CORNER_LINE ||
                percent2 > MAX_PERCENT_GREEN_BETWEEN_CORNER_LINE) {
                if (debugIntersectLines) {
                    cout <<"\t" << numChecksPassed
                         << "-There was too much green between the corner and"
                        " the endpoints of the two lines." << endl;
                    cout << "\t " << "b/w corner and line1: " << percent1
                         << "\t " << " and line 2: " << percent2 << endl;
                }
                continue;
            }
            ++numChecksPassed;


            // Make sure the intersection point stands out, even against balls
            vision->thresh->drawPoint(intersectX - 1, intersectY, BLACK);
            vision->thresh->drawPoint(intersectX + 1, intersectY, BLACK);
            vision->thresh->drawPoint(intersectX, intersectY + 1, BLACK);
            vision->thresh->drawPoint(intersectX, intersectY - 1, BLACK);
            vision->thresh->drawPoint(intersectX, intersectY,
                                      LEGIT_INTERSECTION_POINT_COLOR);
            // assign x, y, dist, bearing, line i, line j, t value for line i,
            // t value for line 2
            VisualCorner c(intersectX, intersectY, distance, bearing,
                           *i, *j, t_I, t_J);
            corners.push_back(c);

            // TODO:  Should I be adding the intersection point to both lines?
            //i->addPoint

            if (debugIntersectLines)
                cout <<"\tPassed all " << numChecksPassed << " checks with corner "
                     << c << endl;
        }
    }

    if (debugIntersectLines) {
        cout << "Found " << corners.size() << " legitimate corners." << endl;
    }

    return corners;

}




// Given a list of VisualCorners, attempts to assign ConcreteCorners (ideally
// one, but sometimes multiple) that correspond with where the corner could
// possibly be on the field.  For instance, if we have a T corner and it is
// right next to the blue goal left post, then it is the blue goal right T.
// Modifies the corners passed in by calling the setPossibleCorners method;
// in certain cases the shape of a corner might be switched too (if an L
// corner is determined to be a T instead, its shape is changed accordingly).
void FieldLines::identifyCorners(list <VisualCorner> &corners) {

    if (debugIdentifyCorners)
        cout << "Beginning identifyCorners() with " << corners.size() << " corners"
             << endl;

    vector <const VisualFieldObject*> visibleObjects = getVisibleFieldObjects();

    // No explicit movement of iterator; will do it manually
    for (list <VisualCorner>::iterator i = corners.begin(); i != corners.end();) {

        if (debugIdentifyCorners) {
            cout << endl << "Before identification: Corner: " << *i << endl;
        }

        // Get all the possible corners given the shape of the corner
        const list <const ConcreteCorner*> possibleCorners =
            ConcreteCorner::getPossibleCorners(i->getShape());

        list <const ConcreteCorner*> possibleClassifications =
            getPossibleClassifications(*i, visibleObjects, possibleCorners);

        // T's can sometimes be misclassified as L's, check them if nothing
        // matched (and the geometric tests failed)

        if (!possibleClassifications.empty()) {
            // Do a final sanity check
            eliminateImpossibleIDs(*i, visibleObjects, possibleClassifications);
        }

        // Keep it completely abstract
        if (possibleClassifications.empty()) {
            i->setPossibleCorners(ConcreteCorner::getPossibleCorners(i->getShape()));
            if (debugIdentifyCorners) {
                cout << "No matches were found for this corner; going to keep it "
                     << "completely abstract." << endl;
                printPossibilities(i->getPossibleCorners());
            }
            ++i;
        }

        // It is unique, append to the front of the list
        // In python we want the positively identified corners to come first so that
        // they can inform the localization system and help identify abstract
        // corners that might be in the frame
        else if (possibleClassifications.size() == 1) {
            if (debugIdentifyCorners) {
                cout << "Only one possibility; appending to the front of the list "
                     << endl;
                printPossibilities(possibleClassifications);
            }

            VisualCorner copy = *i;
            copy.setPossibleCorners(possibleClassifications);
            // This has the effect of incrementing our iterator and deleting the
            // corner from our list.
            i = corners.erase(i);
            corners.push_front(copy);
        }
        // More than 1 possibility for the corner
        else {
            i->setPossibleCorners(possibleClassifications);
            if (debugIdentifyCorners) {
                printPossibilities(possibleClassifications);
            }
            ++i;
        }
    }
}

// Determines if the given L corner does not geometrically make sense for its
// shape given the objects on the screen.
const bool FieldLines::LCornerShouldBeTCorner(const VisualCorner &L) const {
    const float CLOSE_DIST = FIELD_HEIGHT / 2.0f;

    if (debugIdentifyCorners) {
        cout << "Testing to see if corner should be classified as a T."
             << endl;
    }

    const int NUM_POSTS = 4;
    const VisualFieldObject * posts[NUM_POSTS] = {vision->bglp, vision->bgrp,
                                                  vision->yglp, vision->ygrp};


    for (int i = 0; i < NUM_POSTS; ++i) {
        if (posts[i]->getDistance() > 0 && posts[i]->getDistance() < CLOSE_DIST &&
            // Make sure the corner is approximately where a T should be
            nearGoalTCornerLocation(L, posts[i])) {

            if (!LWorksWithPost(L, posts[i])) {
                if (debugIdentifyCorners) {
                    cout << "\tCorner should be classified as a T due to bad geometrical "
                         << "configuration with the " << posts[i]->toString() << endl;
                }
                return true;
            }
        }
    }

    if (debugIdentifyCorners) {
        cout << "\tCorner does not seem to be a T" << endl;
    }
    return false;
}

// Returns true if corner is in the general vicinity of where a T corner could
// be
// Precondition: post is either vision->yglp, vision->ygrp, vision->bglp,
// or vision->bgrp
const bool FieldLines::nearGoalTCornerLocation(const VisualCorner& corner,
                                               const VisualFieldObject * post) const {
    const float ALLOWABLE_ERROR = getAllowedDistanceError(post);
    if (post == vision->bglp || post == vision->bgrp) {
        for (list <const ConcreteCorner*>::const_iterator i =
                 ConcreteCorner::blueGoalTCorners.begin();
             i != ConcreteCorner::blueGoalTCorners.end(); ++i) {
            if (fabs(getEstimatedDistance(&corner, post) -
                     getRealDistance(*i, post)) < ALLOWABLE_ERROR) {
                return true;
            }
        }
    }
    else if (post == vision->yglp || post == vision->ygrp) {
        for (list <const ConcreteCorner*>::const_iterator i =
                 ConcreteCorner::yellowGoalTCorners.begin();
             i != ConcreteCorner::yellowGoalTCorners.end(); ++i) {
            if (fabs(getEstimatedDistance(&corner, post) -
                     getRealDistance(*i, post)) < ALLOWABLE_ERROR) {
                return true;
            }
        }
    }
    return false;
}

// In some Nao frames, robots obscure part of the goal and the bottom is not
// visible.  We can only use pix estimates of goals whose bottoms are visible
const bool FieldLines::goalSuitableForPixEstimate(const VisualFieldObject * goal) const {
    // When we're the goalie, we don't expect there to be any robots blocking the
    // goal.  Furthermore, we're seeing it from a much different angle where
    // the green will not be at the bottom of the post
    if (isGoalie) { return true; }
    const int MAX_PIXEL_DIFF = 10;
    int greenHorizon = vision->thresh->getVisionHorizon();
    int midBottomY = (goal->getLeftBottomY() + goal->getRightBottomY())/2;
    // higher values are lower in the image
    return greenHorizon - midBottomY < MAX_PIXEL_DIFF;
}

// If it's a legitimate L, the post should be INSIDE of the two lines
// See the wiki for an illustration
const bool FieldLines::LWorksWithPost(const VisualCorner& c,
                                      const VisualFieldObject * post) const {
    point<int> cornerLoc(c.getX(), c.getY());

    point <int> leftPostLoc(post->getLeftBottomX(), post->getLeftBottomY());
    point <int> rightPostLoc(post->getRightBottomX(), post->getRightBottomY());

    VisualLine line1 = c.getLine1();
    VisualLine line2 = c.getLine2();
    point<int> fartherPoint1 =
        Utility::getPointFartherFromCorner(line1, c.getX(), c.getY());
    point<int> fartherPoint2 =
        Utility::getPointFartherFromCorner(line2, c.getX(), c.getY());

    // Orient the line from the farther point to the corner.
    // If the post is INSIDE of the two lines, then it will be on the left of one
    // and on the right of the other.  Otherwise if it's on the left of both or
    // on the right of both, it's OUTSIDE and illegitimate.
    // Test both bottom portions of the goal for good measure.
    return (Utility::left(fartherPoint1, cornerLoc, leftPostLoc) ^
            Utility::left(fartherPoint2, cornerLoc, leftPostLoc)) &&
        (Utility::left(fartherPoint1, cornerLoc, rightPostLoc) ^
         Utility::left(fartherPoint2, cornerLoc, rightPostLoc));
}
/*
  void FieldLines::goalieSpecificCheck(list <VisualCorner>& corners) {
  // If we see an inner L and an outer L in the same frame, chances are
  // the outer L is a T

  }*/

// Uses special information about goalies and where they are on the field
// to change corner IDs etc
void FieldLines::identifyGoalieCorners(list <VisualCorner> &corners) {
    // If we see an inner L and an outer L in the same frame, chances are
    // the outer L is a T


}

void FieldLines::printPossibilities(const list <const ConcreteCorner*> &_list) const {
    cout << "Possibilities: " << endl;
    for (list<const ConcreteCorner*>::const_iterator i = _list.begin();
         i != _list.end(); ++i) {
        cout << (*i)->toString() << endl;
    }
}

void FieldLines::printFieldObjectsInformation() {
    vector<const VisualFieldObject*> objs = getVisibleFieldObjects();
    for (vector<const VisualFieldObject*>::const_iterator i = objs.begin();
         i != objs.end(); ++i) {
        cout << *i << endl;
    }
}

// Last sanity checks before localization gets the IDs.  Uses the information
// about what is visible on the screen to throw out corners that could not
// be visible.
void FieldLines::
eliminateImpossibleIDs(VisualCorner &c,
                       vector <const VisualFieldObject*> &visibleObjects,
                       list <const ConcreteCorner*> &possibleClassifications)
{
    if (debugIdentifyCorners) {
        cout << "Attempting to eliminate impossible corner IDs for " << c << endl;
    }

    unsigned int originalSize = possibleClassifications.size();

    if (debugIdentifyCorners) {
        cout << "Removed " << (originalSize - possibleClassifications.size())
             << " impossible corner possibilities" << endl;
    }
}

// Returns true if there is a yellow post on the screen whose certainty is
// not sure
const bool FieldLines::unsureYellowPostOnScreen() const{
    VisualFieldObject * leftPost = vision->yglp;
    VisualFieldObject * rightPost = vision->ygrp;
    return (leftPost->getDistance() > 0 && leftPost->getIDCertainty() != _SURE) ||
        (rightPost->getDistance() > 0 && rightPost->getIDCertainty() != _SURE);
}

// Returns true if there is a blue post on the screen whose certainty is
// not sure
const bool FieldLines::unsureBluePostOnScreen() const{
    VisualFieldObject * leftPost = vision->bglp;
    VisualFieldObject * rightPost = vision->bgrp;
    return (leftPost->getDistance() > 0 && leftPost->getIDCertainty() != _SURE) ||
        (rightPost->getDistance() > 0 && rightPost->getIDCertainty() != _SURE);
}

// Returns whether there is a yellow post close to this corner
const bool FieldLines::yellowPostCloseToCorner(VisualCorner& c) {
    static const int CLOSE = 75;
    if (vision->yglp->getDistance() > 0 &&
        vision->yglp->getIDCertainty() == _SURE) {
        return getEstimatedDistance(&c, vision->yglp) < CLOSE;
    }
    if (vision->ygrp->getDistance() > 0 &&
        vision->ygrp->getIDCertainty() == _SURE) {
        return getEstimatedDistance(&c, vision->ygrp) < CLOSE;
    }
    return false;
}
// Returns whether there is a blue post close to this corner
const bool FieldLines::bluePostCloseToCorner(VisualCorner& c) {
    static const int CLOSE = 75;
    if (vision->bglp->getDistance() > 0 &&
        vision->bglp->getIDCertainty() == _SURE) {
        return getEstimatedDistance(&c, vision->bglp) < CLOSE;
    }
    if (vision->bgrp->getDistance() > 0 &&
        vision->bgrp->getIDCertainty() == _SURE) {
        return getEstimatedDistance(&c, vision->bgrp) < CLOSE;
    }
    return false;
}

// Returns true if there is a goal post visible on the screen whose
// certainty is sure
const bool FieldLines::postOnScreen() const {
    return
        (vision->bglp->getDistance() > 0 && vision->bglp->getIDCertainty() == _SURE) ||
        (vision->bgrp->getDistance() > 0 && vision->bgrp->getIDCertainty() == _SURE) ||
        (vision->yglp->getDistance() > 0 && vision->yglp->getIDCertainty() == _SURE) ||
        (vision->ygrp->getDistance() > 0 && vision->ygrp->getIDCertainty() == _SURE);
}

// Given a list of concrete corners that the visual corner could possibly be,
// weeds out the bad ones based on distances to visible objects and returns
// those that are still in the running.
// TODO: rename method.
list <const ConcreteCorner*> FieldLines::
getPossibleClassifications(const VisualCorner &corner,
                           const vector <const VisualFieldObject*> &visibleObjects,
                           const list <const ConcreteCorner*> &concreteCorners) const {

    list <const ConcreteCorner*> possibleClassifications;
    if (debugIdentifyCorners) {
        cout << endl
             << "Beginning to get possible classifications for corner given "
             << visibleObjects.size() << " visible objects and "
             << concreteCorners.size() << " corner possibilities" << endl << endl;
    }
    // For each concrete corner that's possible, calculate its distance to the
    // visible field objects on the screen as compared to the estimated
    // distance of our visual corner
    for (list <const ConcreteCorner*>::const_iterator j =
             concreteCorners.begin(); j != concreteCorners.end(); ++j) {

        bool badPossibleCorner = false;
        int numCorroboratingObjects = 0;

        for (vector <const VisualFieldObject*>::const_iterator k =
                 visibleObjects.begin(); k != visibleObjects.end(); ++k) {
            float realDistance = getRealDistance(*j, *k);
            // Todo:  We are recalculating this over and over.  How to remember?
            float estimatedDistance = getEstimatedDistance(&corner, *k);
            float absoluteError = fabs(realDistance - estimatedDistance);
            float relativeError = absoluteError / realDistance * 100.0f;

            // If we have already one good distance between this corner and a field
            // object, we only require that the next objects have a small relative
            // error.
            const float MAX_RELATIVE_ERROR = 15.0f;
            // For each corroborating object we find, we allow this much more
            // percent error for subsequent objects
            const float ERROR_ALLOWANCE_INCREASE = 5.0f;
            float relErrorRelaxation =
                ERROR_ALLOWANCE_INCREASE *
				static_cast<float>(numCorroboratingObjects);

            if (numCorroboratingObjects > 0 &&
                relativeError < MAX_RELATIVE_ERROR + relErrorRelaxation) {
                if (debugIdentifyCorners) {
                    cout << "\tDistance between " << (*j)->toString() << " and "
                         << (*k)->toString() << " was fine! Relative error of "
                         << relativeError
                         << " corner pos: (" << (*j)->getFieldX() << ","
                         << (*j)->getFieldY()
                         << " goal pos: (" << (*k)->getFieldX() << ","
                         << (*k)->getFieldY() << endl;
                }
                numCorroboratingObjects++;
            }

            else if (absoluteError > getAllowedDistanceError(*k)) {
                if (debugIdentifyCorners) {
                    cout << "\tDistance between " << (*j)->toString() << " and "
                         << (*k)->toString() << " was too different." << endl
                         << "\tReal: " << realDistance
                         << "\tEstimated: " << estimatedDistance
                         << "\tAbsolute error: " << absoluteError
                         << "\tRelative error: " << relativeError << "%" << endl;
                }
                badPossibleCorner = true;
                break;
            }
            else {
                if (debugIdentifyCorners) {
                    cout << "\tDistance between " << (*j)->toString() << " and "
                         << (*k)->toString() << " was fine! Absolute error of "
                         << absoluteError
                         << " corner pos: (" << (*j)->getFieldX() << ","
                         << (*j)->getFieldY()
                         << " goal pos: (" << (*k)->getFieldX() << ","
                         << (*k)->getFieldY() << endl;
                }
                numCorroboratingObjects++;
            }
        }
        if (!badPossibleCorner) {
            possibleClassifications.push_back(*j);
            if (debugIdentifyCorners) {
                cout << "Corner is possibly a " << (*j)->toString() << endl;
            }
        }
    }
    return possibleClassifications;
}


float FieldLines::getAllowedDistanceError(VisualFieldObject const *obj) const {
    switch (obj->getID()) {
    case BLUE_GOAL_POST:
    case BLUE_GOAL_LEFT_POST:
    case BLUE_GOAL_RIGHT_POST:
    case YELLOW_GOAL_POST:
    case YELLOW_GOAL_LEFT_POST:
    case YELLOW_GOAL_RIGHT_POST:
        return 30;
    default:
        return 0;
    }

}


/**
 * A green-white transition would have green below (or to the left).  We test
 * for white above in a different check
 **/
const bool FieldLines::isGreenWhiteEdge(int x, int y, ScanDirection direction) const {

    TestDirection searchDirection =
        (direction == HORIZONTAL ? TEST_LEFT : TEST_DOWN);
    bool print =  (direction == VERTICAL && debugVertEdgeDetect) ||
        (direction == HORIZONTAL && debugHorEdgeDetect);
    float greenPercent = percentColor(x, y, searchDirection, FIELD_COLORS,
                                      NUM_GREEN_COLORS, NUM_TEST_PIXELS);
    bool enoughGreen = greenPercent >= GREEN_PERCENT_CLEARANCE;

    if (print && !enoughGreen)
        cout << "\t\tisGreenWhiteEdge(): Green before failed. Found "
             << greenPercent << "%, expected " << GREEN_PERCENT_CLEARANCE
             << "%" << endl;
    return enoughGreen;
}

const bool FieldLines::isWhiteGreenEdge(int x, int y,
                                        int potentialMidPoint,
                                        ScanDirection direction) const {
    bool print = (direction == VERTICAL && debugVertEdgeDetect) ||
        (direction == HORIZONTAL && debugHorEdgeDetect);

    int MIN_GREEN_PIXELS_TO_TEST = 3;
    int MAX_GREEN_PIXELS_TO_TEST = NUM_TEST_PIXELS;

    // Idea:  For lines at the edge of the field that are far away there is
    // VERY little green above them.  Rather than test a constant number
    // of pixels above for green, we start small and work up only if we
    // haven't already found enough green.
    bool greenAbove = false;
    float greenPercent = 0.0f;
    TestDirection searchDirection =
        (direction == HORIZONTAL) ? TEST_RIGHT : TEST_UP;

    for (int numPixels = MIN_GREEN_PIXELS_TO_TEST;
         numPixels <= MAX_GREEN_PIXELS_TO_TEST; numPixels++) {
        greenPercent = percentColor(x, y, searchDirection,
                                    FIELD_COLORS, NUM_GREEN_COLORS,
                                    numPixels);
        /*
          if (debugVertEdgeDetect)
          printf("\t\t\tisWhiteGreenVerticalEdge(): greenPercent at (%d, %d): %f%%\n",
          x, y, greenPercent);
        */
        if (greenPercent >= GREEN_PERCENT_CLEARANCE) {
            greenAbove = true;
            break;
        }
    }

    if (!greenAbove) {
        if (print) {
            printf("\t\t\tisWhiteGreenEdge():: (2) Green above failed at "
                   "(%d, %d); found %f%% green; needed %d%%\n", x, y,
                   greenPercent, GREEN_PERCENT_CLEARANCE);

        }
        return false;
    }


    // we have a good amount of green above so continue to check the line for
    // white
    float whitePercent, yellowPercent;
    if (searchDirection == TEST_UP || searchDirection == TEST_DOWN) {
        whitePercent = percentSurrounding(x, potentialMidPoint, WHITE, 1);
        // Check that the line isn't yellow (within the yellow field arcs)
        yellowPercent = percentSurrounding(x, potentialMidPoint, YELLOW, 1);
    }
    else {
        whitePercent = percentSurrounding(potentialMidPoint, y, WHITE, 1);
        yellowPercent = percentSurrounding(potentialMidPoint, y, YELLOW, 1);
    }

    // Require at least 2 out of 9 pixels to be white..
    // TODO: named constant
    bool whiteBelow = whitePercent >= 200.0/9.0;

    // Require at most 3 out of 9 pixels to be yellow
    // TODO: named constant
    float MAX_YELLOW_IN_LINE = 300.0f/9.0f;
    bool notMuchYellow = yellowPercent <= MAX_YELLOW_IN_LINE;


    // A simple, unadulterated whiteGreen transition
    if (greenAbove && whiteBelow && notMuchYellow) {
        return true;
    }

    if (debugVertEdgeDetect) {
        if (!whiteBelow) {
            printf("\t\t\tisWhiteGreenVerticalEdge():: (3) White below failed at "
                   "(%d, %d); found %f%% white; needed %f%%\n", x, y,
                   whitePercent, 200.0/9.0);
        }
        if (!notMuchYellow) {
            printf("\t\t\tisWhiteGreenVerticalEdge():: (4) Yellow below failed at "
                   "(%d, %d); found %f%% yellow; needed at most %f%%\n", x, y,
                   yellowPercent, MAX_YELLOW_IN_LINE);
        }
    }
    return false;
}





// AIBOSPECIFIC:
// Determines from the lines and corners found on the screen whether the
// robot is probably at the center circle.  Uses the fact that limitations
// in the robot's camera prevent us from seeing more than a certain number
// of lines when legitimately seeing lines and intersections in order to
// screen out cases where we see too many (like at the center circle).
const bool FieldLines::probablyAtCenterCircle(vector<VisualLine> &lines,
                                              list<VisualCorner> &corners) {
    if (debugCcScan)
        cout << "Beginning center circle check with " << lines.size()
             << " lines and " << corners.size() << " corners" << endl;

    // Sort by increasing length
    sort(lines.begin(), lines.end());


    const unsigned int MAX_NUM_UNUSED_POINTS = 8;



    if (unusedPointsList.size() > MAX_NUM_UNUSED_POINTS) {
        if (debugCcScan) {
            cout << "\t6. Too many unused points in the image.  Found "
                 << unusedPointsList.size() << " expected at most "
                 << MAX_NUM_UNUSED_POINTS << endl;
        }
        return true;
    }


    // Number of lines sanity check
    if (lines.size() > MAX_NUM_LINES) {
        if (debugCcScan)
            cout << "\t1.  Too many lines on screen.  Found "
                 << lines.size() << " lines, expected a maximum of "
                 << MAX_NUM_LINES << endl;
        return true;
    }
    // Number of corners sanity check
    if (corners.size() > MAX_NUM_CORNERS) {
        if (debugCcScan)
            cout << "\t2.  Too many corners on screen.  Found "
                 << corners.size() << " corners, expected a maximum of "
                 << MAX_NUM_CORNERS << endl;
        return true;
    }

    // Count how many lines on the screen are below a certain length
    unsigned int numSmallLines = 0;
    for (vector<VisualLine>::const_iterator i = lines.begin(); i != lines.end();
         ++i) {
        if (i->length < TWO_CORNER_LINES_MIN_LENGTH)
            numSmallLines++;
    }

    // Number of small lines sanity check
    if (numSmallLines > MAX_NUM_SMALL_LINES) {
        if (debugCcScan)
            cout << "\t3.  Too many small lines on screen.  Found "
                 << numSmallLines << ", expected a maximum of " << MAX_NUM_SMALL_LINES
                 << endl;
        return true;
    }

    // Many lines on screen, one long one, the rest absolutely short
    if ((lines.size() == MAX_NUM_LINES) &&
        // Longest one is very long
        (lines[lines.size() - 1].length > LONG_LINE_LENGTH) &&
        // next longest is not very long
        (lines[lines.size() - 2].length < MEDIUM_LINE_LENGTH)) {

        if (debugCcScan)
            cout << "\t4.  One long line with many smaller ones." << endl;
        return true;
    }

    // Look for outer L's that have an extremely obtuse angle; these don't
    // happen at goal boxes
    for (list<VisualCorner>::const_iterator i = corners.begin();
         i != corners.end(); ++i) {
        if (i->getShape() == OUTER_L &&
            i->getAngleBetweenLines() > MAX_OUTER_L_ANGLE) {
            if (debugCcScan)
                cout << "\t5.  Corner " << *i << " had an angle of "
                     << i->getAngleBetweenLines()
                     << "; max angle allowed is " << MAX_OUTER_L_ANGLE
                     << endl;
            return true;
        }
    }




    // TODO: Use unused points and check if they're above the T? Desktop/CC/42.FRM

    // If there are only two lines, then we can't possibly use these tests
    if (lines.size() > 2) {
        // We want to use T corners to screen out the center circle; in a legitimate
        // T corner we never see a line on the opposite side of the bar from the stem.
        // In the center circle's case, we see this frequently.
        // See the Wiki for an illustration of what is going on here
        list<VisualCorner>::const_iterator firstT = find_if(corners.begin(),
                                                            corners.end(),
                                                            cornerShapeEquals(T));
        if (firstT != corners.end()) {
            // TODO: store this information somewhere earlier
            int TCornerXCoord = firstT->getX();
            int TCornerYCoord = firstT->getY();

            // Figure out which point on the stem is farther away from the corner, the
            // start or end variable.  Allows us to determine which side of the bar
            // the stem is on
            point<int> fartherPoint =
                Utility::getPointFartherFromCorner(firstT->getTStem(), firstT->getX(),
                                                   firstT->getY());
            // Is the stem to the left of the bar?
            bool left = Utility::left(firstT->getTBar().start,
                                      firstT->getTBar().end,
                                      fartherPoint);
            // After excluding the lines that make up the T, there will be exactly 2
            // fewer elements
            vector <VisualLine> remainingLines(lines.size() - 2);
            // Copy only those lines into the remaining lines vector which are NOT
            // part of the corner; check them each to see if one lies on the wrong
            // side
            remove_copy_if(lines.begin(), lines.end(),
                           remainingLines.begin(), lineInCorner(*firstT));

            for(vector<VisualLine>::iterator k = remainingLines.begin();
                k != remainingLines.end(); ++k) {
                point<int> fartherPointOnRemainingLine =
                    Utility::getPointFartherFromCorner(*k, TCornerXCoord, TCornerYCoord);

                // The line lies on the opposite side of the bar than the stem..
                if (Utility::left(firstT->getTBar().start,
                                  firstT->getTBar().end,
                                  fartherPointOnRemainingLine) != left) {

                    if (debugCcScan) {
                        cout << "\t7 Found a line that lay on the wrong side of "
                             << "the TBar of a T corner." << endl
                             << "\tBar: " << firstT->getTBar() << endl
                             << "\tLine: " << *k << endl;
                    }
                    return true;
                }
            }
        }
    }

    /*
    // Midline with shallow inner L corner above
    // see CC/23.FRM, 32.FRM, 39.FRM, 42.FRM
    */

    // Check the number of unused points and how close they lie to each other?

    if (debugCcScan)
        cout << "\t Passed non-field objects sanity checks." << endl;

    return false;
}


// Iterates over the corners and removes those that are too risky to
// use for localization data
void FieldLines::removeRiskyCorners(//vector<VisualLine> &lines,
    list<VisualCorner> &corners) {



    // It's very risky for us to allow any L corners at the edges of the
    // screen if there are no field objects on the screen to corroborate
    // what they are.
    // In some cases, the L is actually a T that got cut off by the
    // edge of the screen.  When we identify corners, the L will be kept
    // completely abstract and localization will try to fit the L to
    // whatever it can, which will be very wrong if the L is in fact a T.
    // If we do see field objects on the screen, we can use distance to
    // determine that the L corner is actually a T.

    int oldNumCorners = corners.size();


    const int NUM_PIXELS_CLOSE_TO_EDGE = 15;
    // No field objects on screen..
    if (getVisibleFieldObjects().empty()) {
        int numLByEdge =
            count_if(corners.begin(), corners.end(),
                     LCornerNearEdgeOfScreen(SCREEN, NUM_PIXELS_CLOSE_TO_EDGE));
        list<VisualCorner>::iterator riskyCorners =
            remove_if(corners.begin(), corners.end(),
                      LCornerNearEdgeOfScreen(SCREEN, NUM_PIXELS_CLOSE_TO_EDGE));

        // We found at least one risky corner
        if (riskyCorners != corners.end()) {
            if (debugRiskyCorners || debugIdentifyCorners) {
                cout << "Removing " << numLByEdge << " L corners that are within "
                     << NUM_PIXELS_CLOSE_TO_EDGE << " pixels from the edge of the "
                     << "screen; likely a T that is cut off." << endl;
            }
            corners.erase(riskyCorners, corners.end());
        }
    }

    if (debugRiskyCorners || debugIdentifyCorners) {
        cout << "\tRemoved " << oldNumCorners - corners.size()
             << " risky corner(s). "  << endl;
    }
}




// This one is in addition to the first one but runs after
// ObjectFragments because it needs field objects to be detected.
// TODO: rename
const bool FieldLines::probablyAtCenterCircle2(vector<VisualLine> &lines,
                                               list<VisualCorner> &corners) {

    if (debugCcScan) {
        cout << "Beginning the second set of center circle checks after "
             << "object recognition with " << lines.size() << " lines "
             << "and " << corners.size() << " corners." << endl;
    }


    // Get the fictitious "corner" at the center circle (directly in the center)
    // and calculate its distances from various field objects that are
    // visible in field.  Attempt to determine if the visual corner we have
    // matches up with this prototypical center circle corner in terms of distance
    ConcreteCorner const * center_circle_corner = &ConcreteCorner::center_circle;
    vector <const VisualFieldObject*> visibleObjects = getVisibleFieldObjects();

    // If there's a close goal in the frame, we couldn't be near the center
    // circle.
    static const int CLOSE_DIST = 150;
    if ((vision->yglp->getDistance() > 0 && vision->yglp->getDistance() < CLOSE_DIST) ||
        (vision->ygrp->getDistance() > 0 && vision->ygrp->getDistance() < CLOSE_DIST) ||
        (vision->bglp->getDistance() > 0 && vision->bglp->getDistance() < CLOSE_DIST) ||
        (vision->bgrp->getDistance() > 0 && vision->bgrp->getDistance() < CLOSE_DIST)) {
        if (debugCcScan) {
            cout << "Found a goal that was close in the image; could not be near "
                 << "the center circle." << endl;
        }
        return false;
    }

    // Loop over all the inner L's in the frame; if we have an inner L with
    // a shallow angle AND another line on the screen, it's probably the
    // center circle.  Although this check doesn't use the field objects
    // explicitly, we put it in this method rather than in probablyAtCenterCircle
    // itself because we want to ensure we are not near any field objects
    // (in some frames, you can see two inner L's by the goal posts, because a
    // T corner is cut off and we only see an L)
    if (lines.size() > 2) {
        for (list<VisualCorner>::iterator i = corners.begin();
             i != corners.end(); ++i) {
            /*
              if (i->getShape() == INNER_L && debugCcScan) {
              cout << " angle lines for " << *i << " is " << i->getAngleBetweenLines() << endl;
              }*/

            if (i->getShape() == INNER_L && i->getAngleBetweenLines() > MAX_INNER_L_ANGLE) {
                if (debugCcScan) {
                    cout << "\t8 Found an inner L corner with a very shallow angle "
                         << "on the screen (" << i->getAngleBetweenLines()
                         << ") when there were more than two lines on the "
                         << "screen; shallowest angle allowed is " << MAX_INNER_L_ANGLE
                         << endl;
                }
                return true;
            }
        }
    }


    // Related to above, it might be the case that the we miss a beacon but
    // still see those goal posts.  Check to see if there are just two lines
    // in the image and that both of them are longer than the diameter of the
    // center circle
    if (lines.size() == 2 &&
        getEstimatedLength(lines[0]) > 2 * CENTER_CIRCLE_RADIUS &&
        getEstimatedLength(lines[1]) > 2 * CENTER_CIRCLE_RADIUS) {
        if (debugCcScan) {
            cout << setprecision(2)
                 << "There were only two lines in the image and both were longer than"
                 << " is possible at the center circle.  The " << lines[0].colorStr
                 << " line is " << getEstimatedLength(lines[0]) << "cm long, the "
                 << lines[1].colorStr << " line is " << getEstimatedLength(lines[1])
                 << endl;
        }
        return false;
    }


    // Look for distances that corroborate being at center circle
    for (list<VisualCorner>::const_iterator i = corners.begin();
         i != corners.end(); ++i) {

        for (vector<const VisualFieldObject*>::const_iterator j = visibleObjects.begin();
             j != visibleObjects.end(); ++j) {
            float realDistance = getRealDistance(center_circle_corner, *j);
            // Dereference the iterator i to get the VisualCorner and then get
            // the reference since getEstimatedDistance works with pointers.
            float estimatedDistance = getEstimatedDistance(&(*i), *j);
            float absoluteError = fabs(realDistance - estimatedDistance);
            //float relativeError = absoluteError / realDistance * 100.0f;

            if (absoluteError < MAX_DIST_ERROR_FOR_CENTER_CIRCLE_DETERMINATION) {
                if (debugCcScan) {
                    cout << setprecision(2)
                         << "\t7.  Corner " << *i << " was estimated to be close to "
                         << "the distance between the center circle and "
                         << (*j)->toString() << endl
                         << "\tReal distance: " << realDistance << "\tEstimate distance: "
                         << estimatedDistance << endl;
                }
                return true;
            }

        }
    }


    if (debugCcScan)
        cout << "\t Passed field objects sanity checks." << endl;

    return false;
}


int FieldLines::numPixelsToHitColor(const int x, const int y,
                                    const int colors[], const int numColors,
                                    const TestDirection testDir) const {
    int sign = 1;
    int count = 0;

    if (testDir == TEST_UP || testDir == TEST_DOWN) {
        if (testDir == TEST_UP) { sign = -1; }
        else { sign = 1; }

        while (Utility::isPointOnScreen(x, y + (sign * count))) {
            for (int j = 0; j < numColors; ++j) {
                if (vision->thresh->thresholded[y + sign * count][x] == colors[j]) {
                    // We found it
                    return count;
                }
            }
            // We didn't find the right color there, increase the count
            count++;
        }
        return count;

    }
    else if (testDir == TEST_LEFT || testDir == TEST_RIGHT) {
        if (testDir == TEST_LEFT) { sign = -1; }
        else { sign = 1; }

        while (Utility::isPointOnScreen(x + (sign * count), y)) {
            for (int j = 0; j < numColors; ++j) {
                if (vision->thresh->thresholded[y][x + sign * count] == colors[j]) {
                    // We found it
                    return count;
                }
            }// We didn't find the right color there, increase the count
            count++;
        }
        return count;
    }
    // TestDirection wrong
    else {
        cerr << "testDir is invalid: " << testDir << endl;
        return -1;
    }

}

int FieldLines::numPixelsToHitColor(const int x, const int y, const int color,
                                    const TestDirection testDir) const {
    const int colors[] = {color};
    return numPixelsToHitColor(x,y,colors,1,testDir);
}


// Draws the the lines of the BoundingBox.  Note that we construct the Bounding
// box in such a way that coord 0 connects with coord 1, and so on and so forth.
// If you pass in a box that was not created by getBoundingBox(), this will
// not draw correctly.
void FieldLines::drawBox(BoundingBox box, int color) const {
    for (int i = 0; i < 4; ++i) {
        vision->thresh->drawLine(box.corners[i].x, box.corners[i].y,
                                 box.corners[(i+1)%4].x, box.corners[(i+1)%4].y,
                                 color);
    }
}

// Draws a small box around the line in the given color
void FieldLines::drawSurroundingBox(const VisualLine &line, int color) const {
    drawBox(Utility::getBoundingBox(line,
                                    DEBUG_GROUP_LINES_BOX_WIDTH,
                                    DEBUG_GROUP_LINES_BOX_WIDTH),
            color);
}



// This method should only be called with a VisualFieldObject object that has a
// concrete location on the field (YGLP, YGRP, BGLP, BGRP, BY, and YB).
// Calculates the length of the straight line between the two objects on the
// field
float FieldLines::getRealDistance(const ConcreteCorner *c,
                                  const VisualFieldObject *obj) const {
    return Utility::getLength(c->getFieldX(), c->getFieldY(),
                              obj->getFieldX(), obj->getFieldY());
}

#ifdef OFFLINE
/*
 * This method prints out every pixel in the image, the Y channel value at
 * each pixel, and the thresholded color for that pixel.
 * By importing the resulting file into excel and applying conditional
 * formatting, we can see exactly where edges are found and how we can use
 * the edge information to improve the algorithm.
 */
void FieldLines::printThresholdedImage() {
    const char * file = "debug.xls";
    FILE * stream = fopen(file, "w");
    if (stream == NULL) {
        fprintf(stderr, "Error: Could not open file to print thresholded image.\n");
        return;
    }

    // Print the column headers
    fprintf(stream, "\t");
    for (int x = 0; x < IMAGE_WIDTH; x++) {
        fprintf(stream, "%d\t", x);
    }
    fprintf(stream, "\n");

    for(int y = 0; y < IMAGE_HEIGHT; y++) {

        fprintf(stream, "%d\t", y);

        for(int x = 0; x < IMAGE_WIDTH; x++) {
            fprintf(stream, "%03d%s\t", vision->thresh->getY(x,y),
                    Threshold::getShortColor(vision->thresh->thresholded[y][x]));
            // we're done this row, skip down
            if (x >= IMAGE_WIDTH - 1) { fprintf(stream, "\n"); }
        }
    }
    fprintf(stream, "\nThresh value:\t%d\n", VERTICAL_TRANSITION_VALUE);

    printf("Successfully printed thresholded image to %s.\n", file);

    fflush(stream);
    fclose(stream);
}
#endif


// Creates a vector of all those field objects that are visible in the frame
// and have SURE certainty
vector <const VisualFieldObject*> FieldLines::getVisibleFieldObjects() const {
    vector <const VisualFieldObject*> visibleObjects;
    for (int i = 0; i < NUM_FIELD_OBJECTS_WITH_DIST_INFO; ++i) {
        if (allFieldObjects[i]->getDistance() > 0 &&
            // We don't want to identify corners based on posts that aren't sure,
            // for instance
            allFieldObjects[i]->getIDCertainty() == _SURE) {
            // With the Nao we need to make sure that the goal posts are near the
            // green of the field in order to use them for distance
            if (ConcreteFieldObject::isGoal(allFieldObjects[i]->getID()) &&
                !(goalSuitableForPixEstimate(allFieldObjects[i]))) {
                // We can't use it.
            }
            else {
                visibleObjects.push_back(allFieldObjects[i]);
            }
        }
    }
    return visibleObjects;
}


// Calculates the angle (in degrees) on the field between line1 and line2,
// given that they meet at intersectX, intersectY.
// See the Wiki for more information
float FieldLines::getEstimatedAngle(const VisualLine &line1,
                                    const VisualLine &line2,
                                    const int intersectX,
                                    const int intersectY) const {

    const bool debugEstimatedAngle = false;
    if (debugEstimatedAngle) {
        cout << "Attempting to find the angle between the " << line1.colorStr
             << "line and the " << line2.colorStr << " line on the field, which "
             << "have an intersection at (" << intersectX << ", " << intersectY
             << ")" << endl;
    }


    estimate cornerEst = vision->pose->pixEstimate(intersectX,
                                                   intersectY,
                                                   0);

    if (debugEstimatedAngle) {
        cout << "Corner estimate: " << cornerEst << endl;
    }

    // Endpoint in the sense that a vector starting at the corner and going along
    // the line segment line 1 would end at this point
    point <int> line1EndPoint = Utility::getPointFartherFromCorner(line1,
                                                                   intersectX,
                                                                   intersectY);
    // Ditto
    point <int> line2EndPoint = Utility::getPointFartherFromCorner(line2,
                                                                   intersectX,
                                                                   intersectY);

    if (debugEstimatedAngle) {
        cout << "Line 1 endpoint: " << line1EndPoint << endl;
        cout << "Line 2 endpoint: " << line2EndPoint << endl;
    }

    // Convert to field coordinates
    estimate line1EndEst = vision->pose->pixEstimate(line1EndPoint.x,
                                                     line1EndPoint.y,
                                                     0);

    estimate line2EndEst = vision->pose->pixEstimate(line2EndPoint.x,
                                                     line2EndPoint.y,
                                                     0);
    if (debugEstimatedAngle) {
        cout << "Line 1 estimate: " << line1EndEst << endl;
        cout << "Line 2 estimate: " << line2EndEst << endl;
    }


    if (line1EndEst.dist <= 0 || line2EndEst.dist < 0) {
        if (debugIntersectLines) {
            cout << "An end point had negative distance; cannot accurately "
                 << "calculate angle." << endl;
        }
        return BAD_ANGLE;
    }

    // Create a vector directed from the corner to the point on line farther
    // from the corner
    pair <float, float> line1Vector(line1EndEst.x - cornerEst.x,
                                    line1EndEst.y - cornerEst.y);

    pair <float, float> line2Vector(line2EndEst.x - cornerEst.x,
                                    line2EndEst.y - cornerEst.y);

    // By this point, we now have the actual vector representations of the lines
    // and can calculate theta
    float dotProduct =
        line1Vector.first * line2Vector.first +
        line1Vector.second * line2Vector.second;

    // sqrt of dot product with itself
    float lengthOfVector1 =
        Utility::getLength(line1EndEst.x, line1EndEst.y,
                           cornerEst.x, cornerEst.y);

    float lengthOfVector2 =
        Utility::getLength(line2EndEst.x, line2EndEst.y,
                           cornerEst.x, cornerEst.y);

    // v dot w = ||v|| ||w|| cos theta -> v dot w / (||v|| ||w||) = cos theta
    // -> ...
    float theta = TO_DEG * acos(dotProduct/
                                      (lengthOfVector1 * lengthOfVector2));
    return theta;
}


// Determines the vector emanating from the corner to the endpoint of both
// lines, then calculates the dot product between the two vectors.  Returns
// the angle between the lines, in degrees
float FieldLines::getEstimatedAngle(const VisualCorner &corner) const {
    return getEstimatedAngle(corner.getLine1(), corner.getLine2(),
                             corner.getX(), corner.getY());
}


// Estimates how long the line is on the field
float FieldLines::getEstimatedLength(const VisualLine &line) const {
    return getEstimatedDistance(line.start, line.end);
}

// Given two points on the screen, estimates the straight line distance
// between them, on the field
float FieldLines::getEstimatedDistance(const point<int> &point1,
                                       const point<int> &point2) const{

    estimate point1Est = pose->pixEstimate(point1.x, point1.y, 0);
    estimate point2Est = pose->pixEstimate(point2.x, point2.y, 0);

    float point1Dist = point1Est.dist;
    float point2Dist = point2Est.dist;
    // We cannot accurately calculate the distance because the pix estimate
    // is too bad
    if (point1Dist <= 0 || point2Dist <= 0) { return BAD_DISTANCE; }

    return Utility::getLength(point1Est.x, point1Est.y,
                              point2Est.x, point2Est.y);
}

// Uses vector subtraction in order to determine distance between corner and
// field object.
// If the field object is very close (<100 cm away) we can use pose to estimate
// bearing and distance to the field object rather than the distance and bearing
// provided by the VisualFieldObject obj itself.
float FieldLines::getEstimatedDistance(const VisualCorner *c,
                                       const VisualFieldObject *obj) const {
    float objDist, objBearing;
    string typeOfEstimate;

    estimate cornerEst = vision->pose->pixEstimate(c->getX(), c->getY(), 0);

    //fieldObjectID id = obj->getID();

    int midX = (obj->getLeftBottomX() + obj->getRightBottomX()) / 2;
    int midBottomY = (obj->getLeftBottomY() + obj->getRightBottomY()) / 2;

    bool isGoal = ConcreteFieldObject::isGoal(obj->getID());
    bool pixEstimate = false;

    // With new regression, cannot trust pix estimates that are not right on
    // the ground
    if (isGoal && (obj->getDistance() < MAXIMUM_DIST_TO_USE_PIX_ESTIMATE ||
                   // The corner is extremely close in the picture to the object, use
                   // pix estimate
                   // TODO: named constant
                   Utility::getLength(static_cast<float>(midX),
									  static_cast<float>(midBottomY),
									  static_cast<float>(c->getX()),
									  static_cast<float>(c->getY()) ) < 15)) {

        typeOfEstimate = "Pose Pix Estimate";
        pixEstimate = true;
        // We pass in an x, y, and z (height from the ground) which we change
        // based on the type of object
        estimate objBottomEst =
            pose->pixEstimate(midX, midBottomY,
                              ConcreteFieldObject::getHeightFromGround(obj->getID()));
        objDist = objBottomEst.dist;
        // Keep the bearing positive in this case as it matches the coordinate
        // system of the corner bearing
        objBearing = objBottomEst.bearing;// * RAD_OVER_DEG;
    }

    else {
        typeOfEstimate = "FieldObjects getDistance()";
        objDist = obj->getDistance();
        // NOTE: since our corner and object coordinate systems are reversed, take
        // the negative of the corner bearing
        objBearing = -obj->getBearing();// * RAD_OVER_DEG;
    }

    float cornerDist = c->getDistance();
    // Convert degrees to radians for the sin/cos formulas
    float cornerBearing = c->getBearing();// * RAD_OVER_DEG;

    // We want the objY to always be Positive because it is away from your body
    float cornerX = cornerDist * sin(cornerBearing);
    float cornerY = cornerDist * cos(cornerBearing);

    // We want the objY to always be Positive because it is away from your body
    float objX = objDist * sin(objBearing);
    float objY = objDist * cos(objBearing);

    float diffX = objX - cornerX;
    float diffY = objY - cornerY;

    float dist = sqrt(diffX * diffX + diffY * diffY);

    if (debugCornerAndObjectDistances) {
        cout << endl
             << "////////////////////////////////////////////////////////////"
             << endl;
        cout << "Using " << typeOfEstimate << " for distance estimate." << endl;
        cout << "Corner: " << *c << endl;
        cout << "CornerX: " << cornerX << "\tCornerY: " << cornerY << endl;
        cout << obj->toString() << " dist: " << objDist << "\tbearing: "
             << objBearing * DEG_OVER_RAD << endl;
        cout << obj->toString() << ".x = "<< objX << ", .y =" << objY << endl;
        cout << "DiffX: " << diffX << "\tDiffY" << diffY << endl;
        cout << "Distance: " << dist << endl;

        cout << endl
             << "////////////////////////////////////////////////////////////"
             << endl;
    }
    // Temporary hack - see Goalie/Leo/103.FRM
    if (pixEstimate) {
        point<int> objP(midX, midBottomY);
        point<int> cornerP(c->getX(), c->getY());
        /*cout << " normal dist for corner " << cornerP << ":" << dist << endl;
          cout << " new dist: " << getEstimatedDistance(objP, cornerP) << endl;*/
        return min(dist, getEstimatedDistance(objP, cornerP));
    }
    return dist;
}

// Returns true if the line segment directed from first to second intersects
// any VisualLines identified on the screen.
const bool FieldLines::intersectsFieldLines(const point<int>& first,
                                            const point<int>& second) const {

    const vector <VisualLine> * lines = getLines();

    for (vector <VisualLine>::const_iterator i = lines->begin();
         i != lines->end(); ++i) {
        // Returns true if the line segments (first, second) (i->start, i->end)
        // intersect
        if (Utility::intersectProp(first, second, i->start, i->end)) {
            return true;
        }
    }
    return false;
}



/*
  FIELD LINE HELPER/SUB METHODS
*/

// filter out double corners found in the same spot
// normally happens with overlapping vertically/hor found lines
// returns true if corner already exists near spot, false otherwise
const bool FieldLines::dupeCorner(const list<VisualCorner> &corners,
                                  const int x, const int y,
                                  const int testNumber) const {
    for (list<VisualCorner>::const_iterator i = corners.begin();
         i != corners.end(); ++i) {
        if (abs(x - i->getX()) < DUPE_MIN_X_SEPARATION &&
            abs(y - i->getY()) < DUPE_MIN_Y_SEPARATION) {
            if (debugIntersectLines) {
                cout <<"\t" << testNumber
                     << "-Failed due to duplication of existing corner " << *i << endl;
            }
            return true;
        }
    }
    return false;
}


// Tests all the pixels in a square centered at (x,y) which extends numPixels
// in all directions.  Counts how many of those pixels are in the colors array
// and divides by the total number of pixels it was able to test in order to
// determine the percent of the immediate area which match the colors array.
// Note:  The percentage is converted from 0 <= x <= 1 into 0 <= x <= 100.
// Additionally, the test area will not necessarily be a square if the test
// pixel is at the edge of the screen and testing the entire square would
// cause us to test outside of the image.
const float FieldLines::percentSurrounding(const int x, const int y,
                                           const int colors[],
                                           const int numColors,
                                           const int numPixels) const {

    if (!Utility::isPointOnScreen(x, y)) {
        //cout << "("<< x << ", " << y << ")" << endl;
        //cout << "Pixel passed to percentSurrounding was off the screen" << endl;
        return 0;
    }
    else if (numPixels <= 0) {
        //cout << numPixels << endl;
        //cout << "numPixels argument in percentSurrounding must be positive, got "
		//   << numPixels << endl;
    }

    // Ensure that the x values over which we iterate are on the screen
    int startX = max(0, x - numPixels);
    int endX = min(IMAGE_WIDTH - 1, x + numPixels);

    // Ensure that the y values fall within those available on the screen
    int startY = max(0, y - numPixels);
    int endY = min(IMAGE_HEIGHT - 1, y + numPixels);

    int numFound = 0;
    for (int i = startX; i <= endX; ++i) {
        for (int j = startY; j <= endY; ++j) {
            // Search for the color at that pixel within the vector of acceptable
            // colors
            for (int k = 0; k < numColors; ++k) {
                if (colors[k] == vision->thresh->thresholded[j][i]) {
                    ++numFound;
                    break;
                }
            }
        }
    }
    int totalPixels = (endX - startX + 1) * (endY - startY + 1);
    return (static_cast<float> (numFound) /
            static_cast<float> (totalPixels)) * 100.0f;
}

// Alternative form of percent surrounding that uses points.
const float FieldLines::percentSurrounding(const point<int> &p,
                                           const int colors[],
                                           const int numColors,
                                           const int numPixels) const {
    return percentSurrounding(p.x, p.y, colors, numColors, numPixels);
}


// Tests all the pixels in a square centered at (x,y) which extends numPixels
// in all directions.  Counts how many of those pixels are the color passed in
// and divides by the total number of pixels it was able to test in order to
// determine the percent of the immediate area which match the colors array.
// Note:  The percentage is converted from 0 <= x <= 1 into 0 <= x <= 100.
// Additionally, the test area will not necessarily be a square if the test
// pixel is at the edge of the screen and testing the entire square would
// cause us to test outside of the image.
const float FieldLines::percentSurrounding(const int x, const int y,
                                           const int color,
                                           const int numPixels) const {
    const int colors[] = {color};
    return percentSurrounding(x, y, colors, 1, numPixels);
}


// Tests all pixels on the line between (x1, y1) and (x2, y2) for thresholded
// values in the colors[] array.
// Returns the percent of the pixels along the line that are found within
// the colors array; always returns a number between 0 and 100.0.
// Preconditions:  (x1, y1) and (x2, y2) are valid coordinates in our image
const float FieldLines::percentColorBetween(const int x1, const int y1,
                                            const int x2, const int y2,
                                            const int color) const {
    const int colors[] = {color};
    return percentColorBetween(x1,y1,x2,y2,colors,1);
}

// Tests all pixels on the line between (x1, y1) and (x2, y2) for thresholded
// values in the colors[] array.
// Returns the percent of the pixels along the line that are found within
// the colors array; always returns a number between 0 and 100.0.
// Preconditions:  (x1, y1) and (x2, y2) are valid coordinates in our image
const float FieldLines::percentColorBetween(const int x1, const int y1,
                                            const int x2, const int y2,
                                            const int colors[],
                                            const int numColors) const {
    int totalPixels = 0;
    int numFound = 0;

    // Vertical line
    if (x2 == x1) {
        int sign = 1;
        if (y2 < y1)
            sign = -1;
        for (int j = y1; j != y2; j += sign, ++totalPixels) {
            if (Utility::isElementInArray(vision->thresh->thresholded[j][x2],
                                          colors, numColors)) {
                ++numFound;
            }
        }
    }
    else {

        float slope = static_cast<float>(y2 - y1) /
            static_cast<float>(x2 - x1);
        int sign = 1;

        if ((abs(y2 - y1)) > (abs(x2 - x1))) {
            slope = 1.0f / slope;
            if (y1 > y2) sign = -1;
            for (int i = y1; i != y2; i += sign) {
                ++totalPixels;
                int newx = x1 +
					static_cast<int>( (slope * static_cast<float>(i - y1)) );

                if (Utility::isElementInArray(vision->thresh->thresholded[i][newx],
                                              colors, numColors))
                    ++numFound;
            }
        }

        else if (slope != 0) {
            //slope = 1.0 / slope;
            if (x1 > x2) sign = -1;
            for (int i = x1; i != x2; i += sign) {
                ++totalPixels;
                int newy = y1 +
					static_cast<int>( (slope * static_cast<float>(i - x1)) );
                if (Utility::isElementInArray(vision->thresh->thresholded[newy][i],
                                              colors, numColors))
                    ++numFound;
            }
        }
        // horizontal line, slope == 0
        else {
            int startX = min(x1,x2);
            int endX = max(x1,x2);
            for (int i = startX; i <= endX; ++i) {
                ++totalPixels;
                if (Utility::isElementInArray(vision->thresh->thresholded[y2][i],
                                              colors, numColors))
                    ++numFound;
            }
        }
    }

    return ( static_cast<float> (numFound) /
			 static_cast<float> (totalPixels) * 100.0f );
}


const float FieldLines::percentColor(const int x, const int y,
                                     const TestDirection dir,
                                     const int colors[], const int numColors,
                                     const int numPixels) const {
    int numFound = 0;
    int numTotal = 0;
    int sign = 1;

    // up/down directions
    if (dir == TEST_UP || dir == TEST_DOWN) {
        // test up, sign goes negative
        if (dir == TEST_UP) sign = -1;
        // test down, sign goes positive
        else if (dir == TEST_DOWN) sign = 1;
        // loop through starting at y, keeping x constant, stopping whether
        // we've exhausted num or we've gone off screen
        for (int i = y + sign; numTotal < numPixels &&
                 i < IMAGE_HEIGHT && i >= 0; i += sign, ++numTotal) {
            for (int j = 0; j < numColors; ++j) {
                if (colors[j] == vision->thresh->thresholded[i][x]) {
                    ++numFound;
                    break;
                }
            }
        }
    }
    // test left/right directions
    else if (dir == TEST_LEFT || dir == TEST_RIGHT) {
        // test up, sign goes negative
        if (dir == TEST_LEFT) sign = -1;
        // test down, sign goes positive
        if (dir == TEST_RIGHT) sign = 1;
        for (int i = x + sign; numTotal < numPixels &&
                 i < IMAGE_WIDTH && i >= 0; i += sign, ++numTotal) {
            for (int j = 0; j < numColors; ++j) {
                if (colors[j] == vision->thresh->thresholded[y][i]) {
                    ++numFound;
                    break;
                }
            }
        }
    }
    return (static_cast<float>(numFound) /
			static_cast<float>(numTotal) * 100.0f);
}

/* Since this is a specific case where we're just testing for the percent
 * of a single color, we can apply the more general algorithm above to do
 * this for us.
 */
const float FieldLines::percentColor(const int x, const int y,
                                     const TestDirection dir,
                                     const int color,
                                     const int numPixels) const {
    const int colors[] = {color};
    return percentColor(x, y, dir, colors, 1, numPixels);
}


// Draws a line through the start and endpoint of the line, which were created
// from the least squares approximation of the line rather than actual points
// on the line.
void FieldLines::drawFieldLine(const VisualLine &toDraw, const int color) const {
    vision->thresh->drawLine(toDraw.start.x, toDraw.start.y,
                             toDraw.end.x, toDraw.end.y, color);
}


void FieldLines::drawLinePoint(const linePoint &p, const int color) const {
    vision->thresh->drawPoint(p.x, p.y, color);
}

void FieldLines::updateLineCounters(const int threshColor, int &numWhite,
                                    int &numUndefined, int &numNonWhite) {
    if (isLineColor(threshColor)) {
        ++numWhite;
    }
    else if (threshColor == GREY) {
        ++numUndefined;
    }
    else {
        ++numNonWhite;
    }
}

#ifdef OFFLINE
void FieldLines::resetLineCounters(int &numWhite, int &numUndefined,
                                   int &numNonWhite) {
    numWhite = numUndefined = numNonWhite = 0;
}


// For now we are ignoring the numWhite sanity checks because we test for
// valid widths before accepting a line point
bool FieldLines::countersHitSanityChecks(const int numWhite,
                                         const int numUndefined,
                                         const int numNonWhite,
                                         const bool print) const {
    if (numNonWhite > NUM_NON_WHITE_SANITY_CHECK ||
        numUndefined > NUM_UNDEFINED_SANITY_CHECK) {
        if (print) {
            if (numNonWhite > NUM_NON_WHITE_SANITY_CHECK) {
                printf("\t\t\t -numNonWhite hit sanity check\n");
            }
            if (numUndefined > NUM_UNDEFINED_SANITY_CHECK) {
                printf("\t\t\t - numUndefined hit sanity check\n");
            }
        }
        return true;
    }
    return false;
}
#endif
