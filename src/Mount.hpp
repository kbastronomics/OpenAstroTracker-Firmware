#ifndef _MOUNT_HPP_
#define _MOUNT_HPP_

#include "Declination.hpp"
#include "Latitude.hpp"
#include "Longitude.hpp"

#include "hal/hal.h"

#if (AZ_DRIVER_TYPE == DRIVER_TYPE_TMC2209_UART) || (ALT_DRIVER_TYPE == DRIVER_TYPE_TMC2209_UART)
 #include <TMCStepper.h>
 // If you get an error here, download the TMCstepper library from "Tools > Manage Libraries"
#endif

// Forward declarations
class AccelStepper;
class LcdMenu;
class TMC2209Stepper;

#define NORTH                      B00000001
#define EAST                       B00000010
#define SOUTH                      B00000100
#define WEST                       B00001000
#define ALL_DIRECTIONS             B00001111
#define TRACKING                   B00010000

#define LCDMENU_STRING      B0001
#define MEADE_STRING        B0010
#define PRINT_STRING        B0011
#define LCD_STRING          B0100
#define COMPACT_STRING      B0101
#define FORMAT_STRING_MASK  B0111

#define TARGET_STRING      B01000
#define CURRENT_STRING     B10000

#define RA_STEPS  1
#define DEC_STEPS 2
#define AZIMUTH_STEPS 5
#define ALTITUDE_STEPS 6

struct LocalDate {
  int year;
  int month;
  int day;
};

//////////////////////////////////////////////////////////////////
//
// Class that represent the OpenAstroTracker mount, with all its parameters, motors, etc.
//
//////////////////////////////////////////////////////////////////
class Mount {
public:
  Mount(RaAxis &raAxis, DecAxis &decAxis);

// Configure the AZ/ALT stepper motors.
#if AZIMUTH_ALTITUDE_MOTORS == 1
  #if AZ_DRIVER_TYPE == DRIVER_TYPE_ULN2003
    void configureAZStepper(byte pin1, byte pin2, byte pin3, byte pin4, int maxSpeed, int maxAcceleration);
  #elif AZ_DRIVER_TYPE == DRIVER_TYPE_A4988_GENERIC || AZ_DRIVER_TYPE == DRIVER_TYPE_TMC2209_STANDALONE || AZ_DRIVER_TYPE == DRIVER_TYPE_TMC2209_UART
    void configureAZStepper(byte pin1, byte pin2, int maxSpeed, int maxAcceleration);
  #endif
  #if ALT_DRIVER_TYPE == DRIVER_TYPE_ULN2003
    void configureALTStepper(byte pin1, byte pin2, byte pin3, byte pin4, int maxSpeed, int maxAcceleration);
  #elif ALT_DRIVER_TYPE == DRIVER_TYPE_A4988_GENERIC || ALT_DRIVER_TYPE == DRIVER_TYPE_TMC2209_STANDALONE || ALT_DRIVER_TYPE == DRIVER_TYPE_TMC2209_UART
    void configureALTStepper(byte pin1, byte pin2, int maxSpeed, int maxAcceleration);
  #endif
#endif

// Configure the AZ/ALT drivers.
#if AZIMUTH_ALTITUDE_MOTORS == 1
  #if AZ_DRIVER_TYPE == DRIVER_TYPE_TMC2209_UART
    #if SW_SERIAL_UART == 0
      void configureAZdriver(Stream *serial, float rsense, byte driveraddress, int rmscurrent, int stallvalue);
    #elif SW_SERIAL_UART == 1
      void configureAZdriver(uint16_t AZ_SW_RX, uint16_t AZ_SW_TX, float rsense, byte driveraddress, int rmscurrent, int stallvalue);
    #endif
  #endif
  #if ALT_DRIVER_TYPE == DRIVER_TYPE_TMC2209_UART
    #if SW_SERIAL_UART == 0
      void configureALTdriver(Stream *serial, float rsense, byte driveraddress, int rmscurrent, int stallvalue);
    #elif SW_SERIAL_UART == 1
      void configureALTdriver(uint16_t AlT_SW_RX, uint16_t ALT_SW_TX, float rsense, byte driveraddress, int rmscurrent, int stallvalue);
    #endif
  #endif
#endif
  
  // Get the current RA tracking speed factor
  float getSpeedCalibration();

  // Set the current RA tracking speed factor
  void setSpeedCalibration(float val, bool saveToStorage);

#if USE_GYRO_LEVEL == 1
  // Get the current pitch angle calibraton
  float getPitchCalibrationAngle();

  // Set the current pitch angle calibration
  void setPitchCalibrationAngle(float angle);

  // Get the current roll angle calibration
  float getRollCalibrationAngle();

  // Set the current pitch angle calibration
  void setRollCalibrationAngle(float angle);
#endif

  // Returns the number of slew microsteps the given motor turns to move one degree
  float getStepsPerDegree(int which);

  // Function to set the number of slew microsteps the given motor turns to move one 
  // degree for each axis. This function stores the value in persistent storage
  void setStepsPerDegree(int which, float steps);

  // Sets the slew rate of the mount. rate is between 1 (slowest) and 4 (fastest)
  void setSlewRate(int rate);

  // Set the HA time (HA is derived from LST, the setter calculates and sets LST)
  void setHA(const DayTime& haTime);
  const DayTime HA() const;

  // Set the LST time (HA is derived from LST)
  void setLST(const DayTime& haTime);
  const DayTime& LST() const;

  void setLatitude(Latitude lat);
  void setLongitude(Longitude lon);
  const Latitude latitude() const;
  const Longitude longitude() const;

  // Get a reference to the target RA value.
  DayTime& targetRA();

  // Get a reference to the target DEC value.
  Declination& targetDEC();

  // Get current RA value.
  const DayTime currentRA() const;

  // Get current DEC value.
  const Declination currentDEC() const;

  // Set the current RA and DEC position to be the given coordinates
  void syncPosition(DayTime ra, Declination dec);

  // Calculates movement parameters and program steppers to move
  // there. Must call loop() frequently to actually move.
  void startSlewingToTarget();

  // Various status query functions
  bool isSlewingDEC() const;
  bool isSlewingRA() const;
  bool isSlewingRAorDEC() const;
  bool isSlewingIdle() const;
  bool isSlewingTRK() const;
  bool isParked() const;
  bool isParking() const;
  bool isGuiding() const;
  bool isFindingHome() const;
  #if AZIMUTH_ALTITUDE_MOTORS == 1
  bool isRunningAZ() const;
  bool isRunningALT() const;
  void getAltAzPositions(long * altSteps, long* azSteps, float* altDelta, float*  azDelta);
  #endif

  // Starts manual slewing in one of eight directions or tracking
  void startSlewing(int direction);

  // Stop manual slewing in one of two directions or tracking. NS is the same. EW is the same
  void stopSlewing(int direction);

  // Block until the motors specified (NORTH, EAST, TRACKING, etc.) are stopped
  void waitUntilStopped(byte direction);

  // Same as Arduino delay() but keeps the tracker going.
  void delay(int ms);

  // Gets the position in one of eight directions or tracking
  long getCurrentStepperPosition(int direction);

  // Process any stepper movement. 
  void loop();

  // Low-leve process any stepper movement on interrupt callback.
  void interruptLoop();

  // Set RA and DEC to the home position
  void setTargetToHome();

  // Asynchronously slews the mount to the home position 
  void goHome();

  // Set the current stepper positions to be home.
  void setHome(bool clearZeroPos);
  
  // Set the current stepper positions to be parking position.
  void setParkingPosition();

  // Set the DEC limit position to the current stepper position. If upper is true, sets the upper limit, else the lower limit.
  void setDecLimitPosition(bool upper); 
  
  // Clear the DEC limit position. If upper is true, clears upper limit, else the lower limit.
  void clearDecLimitPosition(bool upper);

  // Get the DEC limit positions
  void getDecLimitPositions(long & lowerLimit, long & upperLimit);

  // Auto Home with TMC2209 UART
  #if (RA_DRIVER_TYPE == DRIVER_TYPE_TMC2209_UART) || (DEC_DRIVER_TYPE == DRIVER_TYPE_TMC2209_UART)
    void startFindingHomeRA();
    void startFindingHomeDEC();
    void finishFindingHomeRA();
    void finishFindingHomeDEC();
  #endif

  // Asynchronously parks the mount. Moves to the home position and stops all motors. 
  void park();

  // Runs the RA motor at twice the speed (or stops it), or the DEC motor at tracking speed for the given duration in ms.
  void guidePulse(byte direction, int duration);

  // Stops any guide operation in progress.
  void stopGuiding();

  // Stops given guide operations in progress.
  void stopGuiding(bool ra, bool dec);

  // Return a string of DEC in the given format. For LCDSTRING, active determines where the cursor is
  String DECString(byte type, byte active = 0);

  // Return a string of DEC in the given format. For LCDSTRING, active determines where the cursor is
  String RAString(byte type, byte active = 0);

  // Returns a comma-delimited string with all the mounts' information
  String getStatusString();

  // Get the current speed of the stepper. NORTH, WEST, TRACKING
  float getSpeed(int direction);

  // Displays the current location of the mount every n ms, where n is defined in Globals.h as DISPLAY_UPDATE_TIME
  void displayStepperPositionThrottled();

  // Runs a phase of the drift alignment procedure
  void runDriftAlignmentPhase(int direction, int durationSecs);

  // Toggle the state where we run the motors at a constant speed
  void setManualSlewMode(bool state);

  // Set the speed of the given motor
  void setSpeed(int which, float speedDegsPerSec);

#if AZIMUTH_ALTITUDE_MOTORS == 1
  // Support for moving the mount in azimuth and altitude (requires extra hardware)
  void moveBy(int direction, float arcMinutes);
  void disableAzAltMotors();
  void enableAzAltMotors();
#endif

  // Set the number of steps to use for backlash correction
  void setBacklashCorrection(int steps);

  // Get the number of steps to use for backlash correction
  int getBacklashCorrection();
  
  // Read the saved configuration from persistent storage
  void readConfiguration();

  // Clear all saved configuration data from persistent storage
  void clearConfiguration();

  // Get Mount configuration data
  String getMountHardwareInfo();

  // Returns a flag indicating whether the mount is fully booted.
  bool isBootComplete();

  // Let the mount know that the system has finished booting
  void bootComplete();

  DayTime getUtcTime();
  DayTime getLocalTime();
  LocalDate getLocalDate();

  const int getLocalUtcOffset() const;

  void setLocalStartDate( int year, int month, int day );
  void setLocalStartTime( DayTime localTime );
  void setLocalUtcOffset( int offset );

  DayTime calculateLst();
  DayTime calculateHa();
private:

  // Reads values from EEPROM that configure the mount (if previously stored)
  void readPersistentData();

  // Writes a 16-bit value to persistent (EEPROM) storage
  void writePersistentData(int which, long val);

  void calculateRAandDECSteppers(DayTime const& ra, Declination const& dec, long& targetRASteps, long& targetDECSteps) const;
  void displayStepperPosition();
  void moveSteppersTo(float targetRA, float targetDEC);

  // Returns NOT_SLEWING, SLEWING_DEC, SLEWING_RA, or SLEWING_BOTH. SLEWING_TRACKING is an overlaid bit.
  byte slewStatus() const;

  // What is the state of the mount. 
  // Returns some combination of these flags: STATUS_PARKED, STATUS_SLEWING, STATUS_SLEWING_TO_TARGET, STATUS_SLEWING_FREE, STATUS_TRACKING, STATUS_PARKING
  byte mountStatus();

  #if DEBUG_LEVEL&(DEBUG_MOUNT|DEBUG_MOUNT_VERBOSE)
    String mountStatusString();
  #endif

  void autoCalcHa();

private:
  LcdMenu* _lcdMenu;
  float _stepsPerRADegree;    // u-steps/degree when slewing (see RA_STEPS_PER_DEGREE)
  float _stepsPerDECDegree;   // u-steps/degree when slewing (see RA_STEPS_PER_DEGREE)
  int _maxRASpeed;
  int _maxDECSpeed;
  int _maxAZSpeed;
  int _maxALTSpeed;
  int _maxRAAcceleration;
  int _maxDECAcceleration;
  int _maxAZAcceleration;
  int _maxALTAcceleration;
  int _backlashCorrectionSteps;
  int _moveRate;
  long _raParkingPos;     // Parking position in slewing steps
  long _decParkingPos;    // Parking position in slewing steps
  long _decLowerLimit;    // Movement limit in slewing steps
  long _decUpperLimit;    // Movement limit in slewing steps

#if USE_GYRO_LEVEL == 1
  float _pitchCalibrationAngle;
  float _rollCalibrationAngle;
#endif

  long _lastHASet;
  DayTime _LST;
  DayTime _zeroPosRA;

  DayTime _targetRA;
  long _currentRAStepperPosition;

  Declination _targetDEC;
  long _currentDECStepperPosition;

  float _totalDECMove;
  float _totalRAMove;
  Latitude _latitude;
  Longitude _longitude;

  // Stepper control for RA, DEC and TRK.
  AccelStepper* _stepperRA;
  AccelStepper* _stepperDEC;
  AccelStepper* _stepperTRK;
  #if RA_DRIVER_TYPE == DRIVER_TYPE_TMC2209_UART
    TMC2209Stepper* _driverRA;
  #endif  
  #if DEC_DRIVER_TYPE == DRIVER_TYPE_TMC2209_UART
    TMC2209Stepper* _driverDEC;
  #endif  

  #if AZIMUTH_ALTITUDE_MOTORS == 1
    AccelStepper* _stepperAZ;
    AccelStepper* _stepperALT;
    const long _stepsPerAZDegree;    // u-steps/degree (from CTOR)
    const long _stepsPerALTDegree;   // u-steps/degree (from CTOR)
    bool _azAltWasRunning;
    #if AZ_DRIVER_TYPE == DRIVER_TYPE_TMC2209_UART
      TMC2209Stepper* _driverAZ;
    #endif 
    #if ALT_DRIVER_TYPE == DRIVER_TYPE_TMC2209_UART
      TMC2209Stepper* _driverALT;
    #endif 
  #endif

  unsigned long _guideRaEndTime;
  unsigned long _guideDecEndTime;
  unsigned long _lastMountPrint = 0;
  unsigned long _lastTrackingPrint = 0;
  float _trackingSpeed;                 // RA u-steps/sec when in tracking mode
  float _trackingSpeedCalibration;      // Dimensionless, very close to 1.0
  unsigned long _lastDisplayUpdate;
  unsigned long _trackerStoppedAt;
  bool _compensateForTrackerOff;
  volatile int _mountStatus;
  char scratchBuffer[24];
  bool _stepperWasRunning;
  bool _correctForBacklash;
  bool _slewingToHome;
  bool _slewingToPark;
  bool _bootComplete;

  int _localUtcOffset;
  LocalDate _localStartDate;
  DayTime _localStartTime;
  long _localStartTimeSetMillis;

};

#endif
