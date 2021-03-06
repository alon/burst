#ifndef RoboCupGameControlData_H
#define RoboCupGameControlData_H

//---------------------------------------------------------------------------------------
//  File:         RoboCupGameControlData.h
//  Date:         April 30, 2008
//  Description:  This is a subset version of the RoboCupGameControlData.h used by
//                standard RoboCup Game Controller.
//                This simpified version is used for the Robotstadium simulation.
//                The RoboCupGameControlData struct contains global game information that
//                is sent to all players at 500 ms intervals.
//  Project:      Robotstadium, the online robot soccer competition
//  Author:       Yvan Bourquin, Cyberbotics Ltd.
//---------------------------------------------------------------------------------------

// data structure header
#define GAMECONTROLLER_STRUCT_HEADER    "RGme"

// data structure version number
#define GAMECONTROLLER_STRUCT_VERSION   6

// the maximum number of players per team
// this is different from original GameController2006 !
#define MAX_NUM_PLAYERS             4

// team numbers
#define TEAM_BLUE                   0
#define TEAM_RED                    1

// game states
#define STATE_INITIAL               0
#define STATE_READY                 1
#define STATE_SET                   2
#define STATE_PLAYING               3
#define STATE_FINISHED              4

// secondary game states
#define STATE2_NORMAL               0
#define STATE2_PENALTYSHOOT         1

// penalties
#define PENALTY_NONE                0
#define PENALTY_ILLEGAL_DEFENDER    4

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned short uint16;

// information that describes a robot
struct RobotInfo {
  uint16 penalty;               // penalty state of the player
  uint16 secsTillUnpenalised;   // estimate of time till unpenalised
};

// information that describes a team
struct TeamInfo {
  uint8 teamNumber;                           // (not supported in Robotstadium) unique team number
  uint8 teamColour;                           // TEAM_BLUE or TEAM_RED
  uint16 score;                               // team's current score
  struct RobotInfo players[MAX_NUM_PLAYERS];  // team's robots
};

// warning: this structure must be aligned on 32 bit words: modify with care !
struct RoboCupGameControlData {
  // subset of the original RoboCupGameControlData fields:
  char header[4];             // header to identify the structure
  uint32 version;             // version of the data structure
  uint8 playersPerTeam;       // The number of players on a team
  uint8 state;                // state of the game (STATE_READY, STATE_PLAYING, etc)
  uint8 firstHalf;            // 1 = game in first half, 0 otherwise
  uint8 kickOffTeam;          // the next team to kick off
  uint8 secondaryState;       // Extra state information (STATE2_NORMAL, STATE2_PENALTYSHOOT, etc)
  uint8 dropInTeam;           // (not supported in Robotstadium) team that caused last drop in
  uint16 dropInTime;          // (not supported in Robotstadium) number of seconds passed since the last drop in
  uint32 secsRemaining;       // estimate of number of seconds remaining in the half
  struct TeamInfo teams[2];   // blue and red team info

  // Webots extra: global ball position
  // FOR TRAINING ONLY: THIS WILL BE DISABLED IN THE CONTEST MATCHES !!!
  float ballXPos;
  float ballZPos;
};

#endif
