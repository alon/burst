#ifndef Kinematics_h_DEFINED
#define Kinematics_h_DEFINED

/**
 * This file is strictly specific to the Nao robot.
 * It contains infomation relevant to the physical configuration of the robot
 * including:
 * length of links
 * number of chains, etc.
 * It includes method definitions related to the calculation of forward and
 * inverse kinematics.
 * It also supports the creation of rotation and translation matrices.
 * NOTE: All of the lengths are in millimeters.
 */

#include <string.h>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/lu.hpp>              // for lu_factorize
#include <boost/numeric/ublas/io.hpp>              // for cout

#include <cmath>
#include "manconfig.h" // for USE_ACADEMICS_EDITION
#include "NBMath.h"
#include "NBMatrixMath.h"
#include "CoordFrame.h"

namespace Kinematics {

    //Accuracy constants for dls
    //in mm, how close dls will get to the target
    static const float UNBELIEVABLY_LOW_ERROR = 0.01f; //mm
    static const float REALLY_LOW_ERROR = 0.1f; //mm
    static const float ACCEPTABLE_ERROR = 0.5f; //mm
    static const float COARSE_ERROR     = 1.0f; //mm

    enum ChainID {
        HEAD_CHAIN = 0,
        LARM_CHAIN,
        LLEG_CHAIN,
        RLEG_CHAIN,
        RARM_CHAIN,
        LANKLE_CHAIN,
        RANKLE_CHAIN
    };

	// Interpolation types
	enum InterpolationType {
		INTERPOLATION_SMOOTH = 0,
		INTERPOLATION_LINEAR
	};

    /// Joint Name constants ///
    enum JointNames {
        HEAD_YAW = 0,
        HEAD_PITCH,
        // LARM,
        L_SHOULDER_PITCH,
        L_SHOULDER_ROLL,
        L_ELBOW_YAW,
        L_ELBOW_ROLL,
#ifdef USE_ACADEMICS_EDITION
        L_WRIST_YAW,
        L_HAND,
#endif
        // LLEG,
        L_HIP_YAW_PITCH,
        L_HIP_ROLL,
        L_HIP_PITCH,
        L_KNEE_PITCH,
        L_ANKLE_PITCH,
        L_ANKLE_ROLL,
        // RLEG,
        R_HIP_YAW_PITCH,
        R_HIP_ROLL,
        R_HIP_PITCH,
        R_KNEE_PITCH,
        R_ANKLE_PITCH,
        R_ANKLE_ROLL,
        // RARM,
        R_SHOULDER_PITCH,
        R_SHOULDER_ROLL,
        R_ELBOW_YAW,
        R_ELBOW_ROLL
#ifdef USE_ACADEMICS_EDITION
        ,R_WRIST_YAW,
        R_HAND,
#endif
    };

    enum IKOutcome {
        STUCK = 0,
        SUCCESS = 1
    };

    struct IKLegResult {
        IKOutcome outcome;
        float angles[6];
    };

    /**
     * (Deprecated)
    enum Motion_IntFlag {
        UNINT_INTR_CMD, // Un-interruptable interrupter command
        INT_INTR_CMD,   // Interruptable interupter command
        UNINT_CMD,      // Un-interruptable command
        INT_CMD         // Interruptable command
    };

    enum SupportLeg{
        BOTH_LEGS = 0,
        RIGHT_LEG,
        LEFT_LEG
    };
    **/

    static const unsigned int HEAD_JOINTS = 2;
#ifdef USE_ACADEMICS_EDITION
    static const unsigned int ARM_JOINTS = 6;
#else
    static const unsigned int ARM_JOINTS = 4;
#endif
    static const unsigned int LEG_JOINTS = 6;
    static const unsigned int NUM_CHAINS = 5;
	static const unsigned int NUM_BODY_CHAINS = 4;

    static const unsigned int NUM_JOINTS = HEAD_JOINTS + ARM_JOINTS*2 +
        LEG_JOINTS*2;
    static const unsigned int NUM_BODY_JOINTS = ARM_JOINTS*2 + LEG_JOINTS*2;
    static const unsigned int chain_lengths[NUM_CHAINS] = {
        HEAD_JOINTS, ARM_JOINTS, LEG_JOINTS, LEG_JOINTS, ARM_JOINTS};
	static const unsigned int chain_first_joint[NUM_CHAINS] = {0,
        HEAD_JOINTS,
        HEAD_JOINTS + ARM_JOINTS,
        HEAD_JOINTS + ARM_JOINTS + LEG_JOINTS,
        HEAD_JOINTS + ARM_JOINTS + LEG_JOINTS + LEG_JOINTS};
	static const unsigned int chain_last_joint[NUM_CHAINS] = {
        HEAD_JOINTS - 1,
        HEAD_JOINTS + ARM_JOINTS -1,
        HEAD_JOINTS + ARM_JOINTS + LEG_JOINTS - 1,
        HEAD_JOINTS + ARM_JOINTS + LEG_JOINTS + LEG_JOINTS - 1,
        HEAD_JOINTS + ARM_JOINTS + LEG_JOINTS + LEG_JOINTS + ARM_JOINTS - 1};

    static const std::string CHAIN_STRINGS[NUM_CHAINS] =
    { "Head",
      "LArm",
      "LLeg",
      "RLeg",
      "RArm" };

    //Note: this joint list constains RHipYawPitch, which is not compatible
    //with the DCM convention on naming joints (this joint is technially
    //nonexistant)
    static const std::string JOINT_STRINGS[NUM_JOINTS] =
    { "HeadYaw",
      "HeadPitch",
      "LShoulderPitch",
      "LShoulderRoll",
      "LElbowYaw",
      "LElbowRoll",
#ifdef USE_ACADEMICS_EDITION
      "LWristYaw",
      "LHand",
#endif
      "LHipYawPitch",
      "LHipRoll",
      "LHipPitch",
      "LKneePitch",
      "LAnklePitch",
      "LAnkleRoll",
      "RHipYawPitch",
      "RHipRoll",
      "RHipPitch",
      "RKneePitch",
      "RAnklePitch",
      "RAnkleRoll",
      "RShoulderPitch",
      "RShoulderRoll",
      "RElbowYaw",
      "RElbowRoll"
#ifdef USE_ACADEMICS_EDITION
      ,"RWristYaw",
      "RHand"
#endif
      };

    /**********    Bodily dimensions     ***********/

    static const float SHOULDER_OFFSET_Y = 98.0f;
    static const float UPPER_ARM_LENGTH = 90.0f;
    static const float LOWER_ARM_LENGTH = 145.0f;
    static const float SHOULDER_OFFSET_Z = 100.0f;
    static const float THIGH_LENGTH = 100.0f;
    static const float TIBIA_LENGTH = 100.0f;
    static const float NECK_OFFSET_Z = 126.5f;
    static const float HIP_OFFSET_Y = 50.0f;
    static const float HIP_OFFSET_Z = 85.0f;
    static const float FOOT_HEIGHT = 46.0f;

    // Camera
    static const float CAMERA_OFF_X = 48.80f; // in millimeters
    static const float CAMERA_OFF_Z = 23.81f;  // in millimeters
    static const float CAMERA_PITCH_ANGLE = 40.0f * TO_RAD; // 40 degrees

    /**********       Joint Bounds       ***********/
    static const float HEAD_BOUNDS[2][2] = {{-2.09f,2.09f},{-.785f,.785f}};

    // Order of arm joints: ShoulderPitch, SRoll, ElbowYaw, ERoll
    static const float LEFT_ARM_BOUNDS[][2] = {{-2.09f,2.09f},
                                               {0.0f,1.65f},
                                               {-2.09f,2.09f},
                                               {-1.57f,0.0f}
#ifdef USE_ACADEMICS_EDITION
                                               ,{-1.832f, 1.832f},
                                               {0.0f, 0.0f}
#endif
                                               };
    static const float RIGHT_ARM_BOUNDS[][2] = {{-2.09f,2.09f},
                                                {-1.65f,0.0f},
                                                {-2.09f,2.09f},
                                                {0.0f,1.57f}
#ifdef USE_ACADEMICS_EDITION
                                               ,{-1.832f, 1.832f},
                                               {0.0f, 0.0f}
#endif
                                                };

    // Order of leg joints: HYPitch HipPitch HipRoll KneePitch APitch ARoll
    static const float LEFT_LEG_BOUNDS[][2] = {{-1.57f,0.0f},
                                               {-1.57f,.436f},
                                               {-.349f,.785f},
                                               {0.0f,2.269f},
                                               {-1.309f,.524f},
                                               {-.785f,.349f}};
    static const float RIGHT_LEG_BOUNDS[][2] = {{-1.57f,0.0f},
                                                {-1.57f,.436f},
                                                {-.785f,.349f},
                                                {0.0f,2.269f},
                                                {-1.309f,.524f},
                                                {-.349f,.785f}};

    /**********     joint velocity limits **********/
    //Set hardware values- nominal speed in rad/20ms
    //from $AL_DIR/doc/reddoc
    //M=motor r = reduction ratio

    static const float M1R1_NOMINAL = 0.0658f;
    static const float M1R2_NOMINAL = 0.1012f;
    static const float M2R1_NOMINAL = 0.1227f;
    static const float M2R2_NOMINAL = 0.1065f;

    static const float M1R1_NO_LOAD = 0.08308f;
    static const float M1R2_NO_LOAD = 0.1279f;
    static const float M2R1_NO_LOAD = 0.16528f;
    static const float M2R2_NO_LOAD = 0.1438f;

    static const float M1R1_AVG = (M1R1_NOMINAL + M1R1_NO_LOAD )*0.5f;
    static const float M1R2_AVG = (M1R2_NOMINAL + M1R2_NO_LOAD )*0.5f;
    static const float M2R1_AVG = (M2R1_NOMINAL + M2R1_NO_LOAD )*0.5f;
    static const float M2R2_AVG = (M2R2_NOMINAL + M2R2_NO_LOAD )*0.5f;

    static const float jointsMaxVelNominal[Kinematics::NUM_JOINTS] = {
        //head
        M2R2_NOMINAL, M2R1_NOMINAL,
        //left arm
        M2R1_NOMINAL, M2R2_NOMINAL, M2R1_NOMINAL, M2R2_NOMINAL,
#ifdef USE_ACADEMICS_EDITION
        M1R1_NOMINAL, M1R1_NOMINAL,
#endif
        //left leg
        M1R1_NOMINAL, M1R1_NOMINAL, M1R2_NOMINAL,
        M1R2_NOMINAL, M1R2_NOMINAL, M1R1_NOMINAL,
        //right leg
        M1R1_NOMINAL, M1R1_NOMINAL, M1R2_NOMINAL,
        M1R2_NOMINAL, M1R2_NOMINAL, M1R1_NOMINAL,
        //right arm
        M2R2_NOMINAL, M2R2_NOMINAL, M2R1_NOMINAL, M2R2_NOMINAL
#ifdef USE_ACADEMICS_EDITION
        ,M1R1_NOMINAL, M1R1_NOMINAL
#endif
    };

    static const float jointsMaxVelNoLoad[Kinematics::NUM_JOINTS] = {
        //head
        M2R2_NO_LOAD, M2R1_NO_LOAD,
        //left arm
        M2R1_NO_LOAD, M2R2_NO_LOAD, M2R1_NO_LOAD, M2R2_NO_LOAD,
#ifdef USE_ACADEMICS_EDITION
        M1R1_NO_LOAD, M1R1_NO_LOAD,
#endif
        //left leg
        M1R1_NO_LOAD, M1R1_NO_LOAD, M1R2_NO_LOAD,
        M1R2_NO_LOAD, M1R2_NO_LOAD, M1R1_NO_LOAD,
        //right leg
        M1R1_NO_LOAD, M1R1_NO_LOAD, M1R2_NO_LOAD,
        M1R2_NO_LOAD, M1R2_NO_LOAD, M1R1_NO_LOAD,
        //right arm
        M2R2_NO_LOAD, M2R2_NO_LOAD, M2R1_NO_LOAD, M2R2_NO_LOAD
#ifdef USE_ACADEMICS_EDITION
        ,M1R1_NO_LOAD, M1R1_NO_LOAD
#endif
    };

    static const float jointsMaxVelAvg[Kinematics::NUM_JOINTS] = {
        //head
        M2R2_AVG, M2R1_AVG,
        //left arm
        M2R1_AVG, M2R2_AVG, M2R1_AVG, M2R2_AVG,
#ifdef USE_ACADEMICS_EDITION
        M1R1_AVG, M1R1_AVG,
#endif
        //left leg
        M1R1_AVG, M1R1_AVG, M1R2_AVG,
        M1R2_AVG, M1R2_AVG, M1R1_AVG,
        //right leg
        M1R1_AVG, M1R1_AVG, M1R2_AVG,
        M1R2_AVG, M1R2_AVG, M1R1_AVG,
        //right arm
        M2R2_AVG, M2R2_AVG, M2R1_AVG, M2R2_AVG
#ifdef USE_ACADEMICS_EDITION
        ,M1R1_AVG, M1R1_AVG
#endif
    };


    /**********      mDH parameters      ***********/

    enum mDHNames {
        ALPHA = 0,
        L,
        THETA,
        D
    };

    //                                  (alpha,  a ,  theta ,   d  )
    const float HEAD_MDH_PARAMS[2][4] = {{0.0f , 0.0f,  0.0f , 0.0f},
                                         {-M_PI_FLOAT/2, 0.0f, -M_PI_FLOAT/2 , 0.0f}};

    const float LEFT_ARM_MDH_PARAMS[4][4] = {{-M_PI_FLOAT/2,0.0f,0.0f,0.0f},
                                             { M_PI_FLOAT/2,0.0f,M_PI_FLOAT/2,0.0f},
                                             { M_PI_FLOAT/2,0.0f,0.0f,UPPER_ARM_LENGTH},
                                             {-M_PI_FLOAT/2,0.0f,0.0f,0.0f}};

    const float LEFT_LEG_MDH_PARAMS[6][4] = {{ -3*M_PI_FLOAT/4, 0.0f,  -M_PI_FLOAT/2, 0.0f},
                                             { -M_PI_FLOAT/2,   0.0f,   M_PI_FLOAT/4, 0.0f},
                                             { M_PI_FLOAT/2,    0.0f,     0.0f, 0.0f},
                                             //{ M_PI_FLOAT/2,-THIGH_LENGTH,0.0f, 0.0f},
                                             {   0.0f,-THIGH_LENGTH,0.0f, 0.0f},
                                             {   0.0f,-TIBIA_LENGTH,0.0f, 0.0f},
                                             {-M_PI_FLOAT/2,    0.0f,     0.0f, 0.0f}};

    const float RIGHT_LEG_MDH_PARAMS[6][4]= {{ -M_PI_FLOAT/4,  0.0f,   -M_PI_FLOAT/2, 0.0f},
                                             { -M_PI_FLOAT/2,   0.0f,  -M_PI_FLOAT/4, 0.0f},
                                             {  M_PI_FLOAT/2,    0.0f,    0.0f, 0.0f},
                                             //{  M_PI_FLOAT/2,-THIGH_LENGTH,0.0f,0.0f},
                                             { 0.0f,-THIGH_LENGTH,0.0f, 0.0f},
                                             {0.0f,-TIBIA_LENGTH,0.0f,0.0f},
                                             {-M_PI_FLOAT/2,0.0f,0.0f,0.0f}};

    const float RIGHT_ARM_MDH_PARAMS[4][4] = {{-M_PI_FLOAT/2, 0.0f,0.0f,0.0f},
                                              { M_PI_FLOAT/2, 0.0f,M_PI_FLOAT/2,0.0f},
                                              { M_PI_FLOAT/2, 0.0f,0.0f,UPPER_ARM_LENGTH},
                                              {-M_PI_FLOAT/2, 0.0f,0.0f,0.0f}};

    static const float* MDH_PARAMS[NUM_CHAINS] = {&HEAD_MDH_PARAMS[0][0],
                                                  &LEFT_ARM_MDH_PARAMS[0][0],
                                                  &LEFT_LEG_MDH_PARAMS[0][0],
                                                  &RIGHT_LEG_MDH_PARAMS[0][0],
                                                  &RIGHT_ARM_MDH_PARAMS[0][0]};

    //Base transforms to get from body center to beg. of chain
    static const boost::numeric::ublas::matrix <float> HEAD_BASE_TRANSFORMS[1]
    = { CoordFrame4D::translation4D( 0.0f,
                       0.0f,
                       NECK_OFFSET_Z ) };

    static const boost::numeric::ublas::matrix <float> LEFT_ARM_BASE_TRANSFORMS[1]
    = { CoordFrame4D::translation4D( 0.0f,
                       SHOULDER_OFFSET_Y,
                       SHOULDER_OFFSET_Z ) };

    static const boost::numeric::ublas::matrix <float> LEFT_LEG_BASE_TRANSFORMS[1]
    ={ CoordFrame4D::translation4D( 0.0f,
                      HIP_OFFSET_Y,
                      -HIP_OFFSET_Z ) };

    static const boost::numeric::ublas::matrix <float> RIGHT_LEG_BASE_TRANSFORMS[1]
    ={ CoordFrame4D::translation4D( 0.0f,
                      -HIP_OFFSET_Y,
                      -HIP_OFFSET_Z ) };

    static const boost::numeric::ublas::matrix <float> RIGHT_ARM_BASE_TRANSFORMS[1]
    ={ CoordFrame4D::translation4D( 0.0f,
                      -SHOULDER_OFFSET_Y,
                      SHOULDER_OFFSET_Z ) };

    static const boost::numeric::ublas::matrix <float> * BASE_TRANSFORMS[NUM_CHAINS] =
    { &HEAD_BASE_TRANSFORMS[0],
      &LEFT_ARM_BASE_TRANSFORMS[0],
      &LEFT_LEG_BASE_TRANSFORMS[0],
      &RIGHT_LEG_BASE_TRANSFORMS[0],
      &RIGHT_ARM_BASE_TRANSFORMS[0] };

    //Base transforms to get from body center to beg. of chain
    static const boost::numeric::ublas::matrix <float> HEAD_END_TRANSFORMS[4]
    = { CoordFrame4D::rotation4D(CoordFrame4D::X_AXIS, M_PI_FLOAT/2),
        CoordFrame4D::rotation4D(CoordFrame4D::Y_AXIS,M_PI_FLOAT/2),
        CoordFrame4D::translation4D(CAMERA_OFF_X, 0, CAMERA_OFF_Z),
        CoordFrame4D::rotation4D(CoordFrame4D::Y_AXIS, CAMERA_PITCH_ANGLE) };


    static const boost::numeric::ublas::matrix <float> LEFT_ARM_END_TRANSFORMS[2]
    = { CoordFrame4D::rotation4D(CoordFrame4D::Z_AXIS, -M_PI_FLOAT/2),
        CoordFrame4D::translation4D(LOWER_ARM_LENGTH,0.0f,0.0f) };

    static const boost::numeric::ublas::matrix <float> LEFT_LEG_END_TRANSFORMS[3]
    = { CoordFrame4D::rotation4D(CoordFrame4D::Z_AXIS, M_PI_FLOAT),
        CoordFrame4D::rotation4D(CoordFrame4D::Y_AXIS, -M_PI_FLOAT/2),
        CoordFrame4D::translation4D(0.0f,
                      0.0f,
                      -FOOT_HEIGHT) };

    static const boost::numeric::ublas::matrix <float> RIGHT_LEG_END_TRANSFORMS[3] =
        { CoordFrame4D::rotation4D(CoordFrame4D::Z_AXIS, M_PI_FLOAT),
        CoordFrame4D::rotation4D(CoordFrame4D::Y_AXIS, -M_PI_FLOAT/2),
        CoordFrame4D::translation4D(0.0f,
                      0.0f,
                      -FOOT_HEIGHT) };

    static const boost::numeric::ublas::matrix <float> RIGHT_ARM_END_TRANSFORMS[2] =
        { CoordFrame4D::rotation4D(CoordFrame4D::Z_AXIS, -M_PI_FLOAT/2),
        CoordFrame4D::translation4D(LOWER_ARM_LENGTH,0.0f,0.0f) };


    static const boost::numeric::ublas::matrix <float> * END_TRANSFORMS[NUM_CHAINS] =
    { &HEAD_END_TRANSFORMS[0],
      &LEFT_ARM_END_TRANSFORMS[0],
      &LEFT_LEG_END_TRANSFORMS[0],
      &RIGHT_LEG_END_TRANSFORMS[0],
      &RIGHT_ARM_END_TRANSFORMS[0] };
    static const int NUM_BASE_TRANSFORMS[NUM_CHAINS] = {1,1,1,1,1};
    static const int NUM_END_TRANSFORMS[NUM_CHAINS] = {4,2,3,3,2};
    static const int NUM_JOINTS_CHAIN[NUM_CHAINS] = {2,4,6,6,4};


    /*
     * Declarations for constants and methods concerning forward and inverse
     * kinematics.
     */
    static const float dampFactor = 0.4f;
    static const float maxDeltaTheta = 0.5f;
    static const int maxAnkleIterations = 60;
    static const int maxHeelIterations = 20;

    const void clipChainAngles(const ChainID id,
                               float angles[]);
    const float getMinValue(const ChainID id, const int jointNumber);
    const float getMaxValue(const ChainID id, const int jointNumber);
    const NBMath::ufvector3 forwardKinematics(const ChainID id,
                                              const float angles[]);
    const NBMath::ufmatrix3 buildHeelJacobian(const ChainID chainID,
                                              const float angles[]);
    const NBMath::ufmatrix3 buildLegJacobian(const ChainID chainID,
                                             const float angles[]);

    // Both adjustment methods return whether the search was successful.
    // The correct angles required to fulfill the goal are returned through
    // startAngles by reference.
    const bool adjustAnkle(const ChainID chainID,
                           const NBMath::ufvector3 &goal,
                           float startAngles[],
                           const float maxError);
    const bool adjustHeel(const ChainID chainID,
                          const NBMath::ufvector3 &goal,
                          float startAngles[],
                          const float maxError);
    const IKLegResult dls(const ChainID chainID,
                          const NBMath::ufvector3 &goal,
                          const float startAngles[],
                          const float maxError = ACCEPTABLE_ERROR,
                          const float maxHeelError = UNBELIEVABLY_LOW_ERROR);


    void hackJointOrder(float angles[]);

};

#endif
