#pragma once

#include "Accessories.h"

enum RateCompensation {RC_NONE, RC_REFR_RA, RC_REFR_BOTH, RC_FULL_RA, RC_FULL_BOTH};
enum MountTypes {MT_UNKNOWN, MT_GEM, MT_FORK, MT_FORKALT, MT_ALTAZM};
enum Errors {
  ERR_NONE, ERR_MOTOR_FAULT, ERR_ALT_MIN, ERR_LIMIT_SENSE, ERR_DEC, ERR_AZM, 
  ERR_UNDER_POLE, ERR_MERIDIAN, ERR_SYNC, ERR_PARK, ERR_GOTO_SYNC, ERR_UNSPECIFIED,
  ERR_ALT_MAX, ERR_WEATHER_INIT, ERR_RTC_INIT};

#define PierSideNone     0
#define PierSideEast     1
#define PierSideWest     2
#define PierSideBest     3
#define PierSideFlipWE1  10
#define PierSideFlipWE2  11
#define PierSideFlipWE3  12
#define PierSideFlipEW1  20
#define PierSideFlipEW2  21
#define PierSideFlipEW3  22

class MountStatus {
  public:
    bool update(bool all=false) {

      char s[20] = "";
      if (!_valid) {
        if (!command(":GVP#",s) || s[0] == 0 || !strstr(s,"On-Step")) { _valid=false; return false; }
        if (!command(":GVN#",s) || s[0] == 0 ) { _valid=false; return false; }
        strcpy(_id,"OnStep");
        strcpy(_ver,s);
      }

      if (!command(":GU#",s) || s[0] == 0) { _valid=false; return false; }

      _tracking=false; _slewing=false;
      if (!strstr(s,"N")) _slewing=true; else _tracking=(!strstr(s,"n"));

      _parked      = strstr(s,"P");
      if (strstr(s,"p")) _parked=false;
      _parking     = strstr(s,"I");
      _parkFail    = strstr(s,"F");

      _pecRecorded = strstr(s,"R");
      _pecIgnore   = strstr(s,"/");
      _pecReadyPlay= strstr(s,",");
      _pecPlaying  = strstr(s,"~");
      _pecReadyRec = strstr(s,";");
      _pecRecording= strstr(s,"^");
    
      _atHome      = strstr(s,"H");
      _ppsSync     = strstr(s,"S");
      _guiding     = strstr(s,"G");
      _axisFault   = strstr(s,"f");
      
      if (strstr(s,"r")) { if (strstr(s,"s")) _rateCompensation=RC_REFR_RA; else _rateCompensation=RC_REFR_BOTH; } else
      if (strstr(s,"t")) { if (strstr(s,"s")) _rateCompensation=RC_FULL_RA; else _rateCompensation=RC_FULL_BOTH; } else _rateCompensation=RC_NONE;

      _waitingHome   = strstr(s,"w");
      _pauseAtHome   = strstr(s,"u");
      _buzzerEnabled = strstr(s,"z");

      if (strstr(s,"E")) _mountType=MT_GEM; else
      if (strstr(s,"K")) _mountType=MT_FORK; else
      if (strstr(s,"k")) _mountType=MT_FORKALT; else
      if (strstr(s,"A")) _mountType=MT_ALTAZM; else _mountType=MT_UNKNOWN;

      if (_mountType==MT_GEM) _autoMeridianFlips = strstr(s,"a"); else _autoMeridianFlips=false;

      _lastError=(Errors)(s[strlen(s)-1]-'0');

      if (all) {
        if (!command(":GX94#",s) || s[0]==0) { _valid=false; return false; }
        _meridianFlips=!strstr(s, "N");
        _pierSide=strtol(&s[0],NULL,10);

        static int driverStatusTries = 0;
        _validStepperDriverStatus = false;
        _stst1 = false; _olb1 = false; _ola1 = false; _s2ga1 = false; _s2gb1 = false; _ot1 = false; _otpw1 = false;
        _stst2 = false; _olb2 = false; _ola2 = false; _s2ga2 = false; _s2gb2 = false; _ot2 = false; _otpw2 = false;
        if (!command(":GXU1#",s) || (s[0] != 0 && s[0] != '0' && driverStatusTries<3)) {
          driverStatusTries=0;
          if (strstr(s,"ST")) _stst1=true;
          if (strstr(s,"OA")) _ola1=true;
          if (strstr(s,"OB")) _olb1=true;
          if (strstr(s,"GA")) _s2ga1=true;
          if (strstr(s,"GB")) _s2gb1=true;
          if (strstr(s,"OT")) _ot1=true;
          if (strstr(s,"PW")) _otpw1=true;

          if (!command(":GXU2#",s) || (s[0] != 0 && s[0] != '0')) {
            _validStepperDriverStatus = true;
            if (strstr(s,"ST")) _stst2=true;
            if (strstr(s,"OA")) _ola2=true;
            if (strstr(s,"OB")) _olb2=true;
            if (strstr(s,"GA")) _s2ga2=true;
            if (strstr(s,"GB")) _s2gb2=true;
            if (strstr(s,"OT")) _ot2=true;
            if (strstr(s,"PW")) _otpw2=true;
          }
        } else { driverStatusTries++; if (driverStatusTries>3) driverStatusTries=3; }

        if (_alignMaxStars==-1) {
          _alignMaxStars=3;
          if (!command(":A?#",s) || s[0] != 0) { if (s[0] > '0' && s[0] <= '9') _alignMaxStars=s[0]-'0'; }
        }
      }
      
      _valid=true;
      return true;
    }
    bool getId(char id[]) { if (!_valid) return false; else { strcpy(id,_id); return true; } }
    bool getVer(char ver[]) { if (!_valid) return false; else { strcpy(ver,_ver); return true; } }
    bool valid() { return _valid; }
    bool aligning() { char s[20]=""; if (command(":A?#",s) && strlen(s) == 3 && s[1] <= s[2] && s[1] != '0') return true; else return false; }
    bool tracking() { return _tracking; }
    bool slewing() { return _slewing; }
    bool parked() { return _parked; }
    bool parking() { return _parking; }
    bool parkFail() { return _parkFail; }
    bool pecIgnore() { return _pecIgnore; }
    bool pecReadyPlay() { return _pecReadyPlay; }
    bool pecPlaying() { return _pecPlaying; }
    bool pecReadyRec() { return _pecReadyRec; }
    bool pecRecorded() { return _pecRecorded; }
    bool pecRecording() { return _pecRecording; }
    bool atHome() { return _atHome; }
    bool ppsSync() { return _ppsSync; }
    bool guiding() { return _guiding; }

    bool axisFault() { return _axisFault; }
    bool axisStatusValid() { return _validStepperDriverStatus; }
    bool axis1StSt() { return _stst1; }
    bool axis1OLa() { return _ola1; }
    bool axis1OLb() { return _olb1; }
    bool axis1S2Ga() { return _s2ga1; }
    bool axis1S2Gb() { return _s2gb1; }
    bool axis1OT() { return _ot1; }
    bool axis1OTPW() { return _otpw1; }
    bool axis2StSt() { return _stst2; }
    bool axis2OLa() { return _ola2; }
    bool axis2OLb() { return _olb2; }
    bool axis2S2Ga() { return _s2ga2; }
    bool axis2S2Gb() { return _s2gb2; }
    bool axis2OT() { return _ot2; }
    bool axis2OTPW() { return _otpw2; }

    bool waitingHome() { return _waitingHome; }
    bool pauseAtHome() { return _pauseAtHome; }
    bool buzzerEnabled() { return _buzzerEnabled; }
    MountTypes mountType() { return _mountType; }
    RateCompensation rateCompensation() { return _rateCompensation; }
    bool meridianFlips() { return _meridianFlips; }
    bool autoMeridianFlips() { return _autoMeridianFlips; }
    byte pierSide() { return _pierSide; }
    int alignMaxStars() { return _alignMaxStars; }
    Errors lastError() { return _lastError; }
    bool getLastErrorMessage(char message[]) {
      strcpy(message,"");
      if (_lastError==ERR_NONE) strcpy(message,"None"); else
      if (_lastError==ERR_MOTOR_FAULT) strcpy(message,"Motor/driver fault"); else
      if (_lastError==ERR_ALT_MIN) strcpy(message,"Below horizon limit"); else
      if (_lastError==ERR_LIMIT_SENSE) strcpy(message,"Limit sense"); else
      if (_lastError==ERR_DEC) strcpy(message,"Dec limit exceeded"); else
      if (_lastError==ERR_AZM) strcpy(message,"Azm limit exceeded"); else
      if (_lastError==ERR_UNDER_POLE) strcpy(message,"Under pole limit exceeded"); else
      if (_lastError==ERR_MERIDIAN) strcpy(message,"Meridian limit (W) exceeded"); else
      if (_lastError==ERR_SYNC) strcpy(message,"Sync safety limit exceeded"); else
      if (_lastError==ERR_PARK) strcpy(message,"Park failed"); else
      if (_lastError==ERR_GOTO_SYNC) strcpy(message,"Goto sync failed"); else
      if (_lastError==ERR_UNSPECIFIED) strcpy(message,"Unknown error"); else
      if (_lastError==ERR_ALT_MAX) strcpy(message,"Above overhead limit"); else
      if (_lastError==ERR_WEATHER_INIT) strcpy(message,"Weather sensor init failed"); else
      if (_lastError==ERR_RTC_INIT) strcpy(message,"RTC init failed"); else
        sprintf(message,"Unknown Error, code %d",(int)_lastError);
      return message[0];
    }
  private:
    char _id[10]="";
    char _ver[10]="";
    bool _valid=false;
    bool _tracking=false;
    bool _slewing=false;
    bool _parked=false;
    bool _parking=false;
    bool _parkFail=false;
    bool _pecIgnore=false;
    bool _pecReadyPlay=false;
    bool _pecPlaying=false;
    bool _pecReadyRec=false;
    bool _pecRecorded=false;
    bool _pecRecording=false;
    bool _atHome=false;
    bool _ppsSync=false;
    bool _guiding=false;
    bool _axisFault=false;
    bool _waitingHome=false;
    bool _pauseAtHome=false;
    bool _buzzerEnabled=false;
    MountTypes _mountType=MT_UNKNOWN;
    RateCompensation _rateCompensation=RC_NONE;
    bool _meridianFlips=true;
    bool _autoMeridianFlips=false;
    byte _pierSide=PierSideNone;
    int _alignMaxStars = -1;

    Errors _lastError=ERR_NONE;
    bool _validStepperDriverStatus = false;
    bool _stst1 = false;
    bool _olb1 = false;
    bool _ola1 = false;
    bool _s2ga1 = false;
    bool _s2gb1 = false;
    bool _ot1 = false;
    bool _otpw1 = false;
    bool _valid2 = false;
    bool _stst2 = false;
    bool _olb2 = false;
    bool _ola2 = false;
    bool _s2ga2 = false;
    bool _s2gb2 = false;
    bool _ot2 = false;
    bool _otpw2 = false;

};

MountStatus mountStatus;

enum CommandErrors {
  CE_NONE, CE_0, CE_CMD_UNKNOWN, CE_REPLY_UNKNOWN, CE_PARAM_RANGE, CE_PARAM_FORM,
  CE_ALIGN_FAIL, CE_ALIGN_NOT_ACTIVE, CE_NOT_PARKED_OR_AT_HOME, CE_PARKED,
  CE_PARK_FAILED, CE_NOT_PARKED, CE_NO_PARK_POSITION_SET, CE_GOTO_FAIL, CE_LIBRARY_FULL,
  CE_GOTO_ERR_BELOW_HORIZON, CE_GOTO_ERR_ABOVE_OVERHEAD, CE_SLEW_ERR_IN_STANDBY, 
  CE_SLEW_ERR_IN_PARK, CE_GOTO_ERR_GOTO, CE_GOTO_ERR_OUTSIDE_LIMITS, CE_SLEW_ERR_HARDWARE_FAULT,
  CE_MOUNT_IN_MOTION, CE_GOTO_ERR_UNSPECIFIED, CE_NULL};

char* commandErrorToStr(int e) {
  static char reply[40];
  strcpy(reply,"Error, ");
  
  switch (e) {
    case CE_NONE: strcpy(reply,"No Errors"); break;
    case CE_0: strcpy(reply,"Reply 0"); break;
    case CE_CMD_UNKNOWN: strcat(reply,"command unknown"); break;
    case CE_REPLY_UNKNOWN: strcat(reply,"invalid reply"); break;
    case CE_PARAM_RANGE: strcat(reply,"parameter out of range"); break;
    case CE_PARAM_FORM: strcat(reply,"bad parameter format"); break;
    case CE_ALIGN_FAIL: strcat(reply,"align failed"); break;
    case CE_ALIGN_NOT_ACTIVE: strcat(reply,"align not active"); break;
    case CE_NOT_PARKED_OR_AT_HOME: strcat(reply,"not parked or at home"); break;
    case CE_PARKED: strcat(reply,"already parked"); break;
    case CE_PARK_FAILED: strcat(reply,"park failed"); break;
    case CE_NOT_PARKED: strcat(reply,"not parked"); break;
    case CE_NO_PARK_POSITION_SET: strcat(reply,"no park position set"); break;
    case CE_GOTO_FAIL: strcat(reply,"goto failed"); break;
    case CE_LIBRARY_FULL: strcat(reply,"library full"); break;
    case CE_GOTO_ERR_BELOW_HORIZON: strcat(reply,"goto below horizon"); break;
    case CE_GOTO_ERR_ABOVE_OVERHEAD: strcat(reply,"goto above overhead"); break;
    case CE_SLEW_ERR_IN_STANDBY: strcat(reply,"slew in standby"); break;
    case CE_SLEW_ERR_IN_PARK: strcat(reply,"slew in park"); break;
    case CE_GOTO_ERR_GOTO: strcat(reply,"already in goto"); break;
    case CE_GOTO_ERR_OUTSIDE_LIMITS: strcat(reply,"goto outside limits"); break;
    case CE_SLEW_ERR_HARDWARE_FAULT: strcat(reply,"hardware fault"); break;
    case CE_MOUNT_IN_MOTION: strcat(reply,"mount in motion"); break;
    case CE_GOTO_ERR_UNSPECIFIED: strcat(reply,"other"); break;
    case CE_NULL: strcpy(reply,""); break;
    default: strcat(reply,"unknown");
  }

  return reply;
}
