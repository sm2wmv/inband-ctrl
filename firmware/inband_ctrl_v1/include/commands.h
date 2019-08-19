#ifndef _COMMANDS_H_
#define _COMMANDS_H_

//Main commands are always three characters
#define CMD_MAIN_SET_ROTATOR_HEADING        "SRH"
#define CMD_MAIN_GET_ROTATOR_HEADING        "GRH"
#define CMD_MAIN_SET_ROTATOR_STOP	    	"SRS"
#define CMD_MAIN_SET_ROTATOR_CALIBRATION    "SRC"
#define CMD_MAIN_GET_BATTERY_VOLTAGE        "GBV"
#define CMD_MAIN_GET_BATTERY_CURRENT        "GBC"
#define CMD_MAIN_SET_OUTPUT                 "SDO" //Set digital output
#define CMD_MAIN_GET_OUTPUT                 "GDO" //Get digital output
#define CMD_MAIN_GET_STATUS                 "GCS" //Get the current status
#define CMD_MAIN_SET_ERRORS                 "SEC" //Clear the errors
#define CMD_MAIN_SET_CHARGING               "SCG" //Enables the charging (switches from slow charge)
#define CMD_MAIN_GET_CHARGE_COUNT           "GCC" //Get how many times the charge button has been pressed
#define CMD_MAIN_INFORMATION                "INF" //Send a text information message
#define CMD_MAIN_SET_CHARGE_PUSH_INTERVAL	  "SCI" //Set the charge interval we push the charge button in minutes
#define CMD_MAIN_GET_BOX_TEMP               "GTB" //Get the box temperature
#define CMD_MAIN_GET_OUTSIDE_TEMP           "GTO" //Get the outside temperature

//Sub commands are always two characters

//Set the rotator calibration for counter clock-wise direction
//Example, "SRC CC 520"
#define CMD_SUB_ROTATOR_CALIBRATION_CCW     "CC"
//Set the rotator calibration for clock-wise direction
//Example, "SRC CW -120"
#define CMD_SUB_ROTATOR_CALIBRATION_CW      "CW"
//Saves the rotator calibration settings to the EEPROM
#define CMD_SUB_ROTATOR_CALIBRATION_SAVE    "SE"

#define CMD_SUB_OUTPUT_SET_HIGH             "HI"  //Set outputs (set to 1)
#define CMD_SUB_OUTPUT_SET_LOW              "LO"  //Clear output (set to 0)

#endif
