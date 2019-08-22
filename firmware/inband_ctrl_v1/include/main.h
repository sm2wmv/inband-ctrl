/*! \file general_io/main.h
 *  \defgroup general_io_group General I/O card
 *  \brief Main file of the General I/O card.
 *  \author Mikael Larsmark, SM2WMV
 *  \date 2010-05-18
 *  \code #include "general_io/main.h" \endcode
 */
//    Copyright (C) 2009  Mikael Larsmark, SM2WMV
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef _MAIN_H_
#define _MAIN_H_

#define EEPROM_SETTINGS_ADDR  100

// Current poll interval, default = 500 ms
#define POLL_INTERVAL_CURRENT       500
// Voltage poll interval, default = 500 ms
#define POLL_INTERVAL_VOLTAGE       500
// Rotator direction poll interval, default = 250 ms
#define POLL_INTERVAL_DIRECTION     250
// Poll interval of the charging status
#define POLL_INTERVAL_CHARGE        1000
// The time between the Charge button on the charger is pushed, in minutes
#define CHARGE_PUSH_INTERVAL		1
// The temperature poll interval
#define POLL_INTERVAL_TEMPERATURE	1000

#define ADC_CH_CURRENT    ADC_CH_ADC4
#define ADC_CH_VOLTAGE    ADC_CH_ADC6
#define ADC_CH_DIRECTION  ADC_CH_ADC1
#define ADC_CH_OUTSIDE_TMP36  ADC_CH_ADC5
#define ADC_CH_BOX_TMP36  ADC_CH_ADC7

// VREF / 1024, 5.5982 = resistor divider for the voltage, (2.56/1024.0)*5.5982
#define ADC_VOLTAGE_SCALING 0.0139955f

//  (2.56/1024.0)/(0.003*50)
#define ADC_CURRENT_SCALING 0.0166667f

#define ROTATOR_STATUS_STOPPED        0
#define ROTATOR_STATUS_CCW            1
#define ROTATOR_STATUS_CW             2
#define ROTATOR_STATUS_MANUAL_CCW     3
#define ROTATOR_STATUS_MANUAL_CW      4


//Rotator stuck tick counts (each tick = 1 second), default = 5 seconds
#define ROTATOR_STUCK_TICK_LIMIT  3
//Rotator stuck tick limit in degrees, the rotator has to have turned at least this amount of degrees
//In a ROTATOR_STUCK_TICK_LIMIT period, default = 5
#define ROTATOR_STUCK_TICK_DEG_LIMIT 1

typedef struct {
  uint16_t adc_val_current;
  uint16_t adc_val_voltage;
  float bat_voltage;
  float bat_current;

  uint16_t adc_val_direction[3];
  uint16_t adc_val_direction_pos;
  uint16_t adc_val_direction_filtered;

  uint16_t rotator_curr_heading_deg;
  uint16_t rotator_target_heading_deg;
  uint16_t rotator_target_heading;

  uint8_t rotator_status;
  uint8_t charge_status;
  
  uint16_t adc_val_temp_box;
  uint16_t adc_val_temp_outside;
  float temp_box;
  float temp_outside;
} struct_status;

typedef struct {
  int16_t rotator_adc_val_ccw;
  int16_t rotator_adc_val_cw;
  int16_t rotator_ccw_lim_deg;
  int16_t rotator_cw_lim_deg;
  double rotator_heading_scale;
  int16_t charge_push_interval;
} struct_settings;

void main_set_error(uint8_t error_type);
int8_t main_rotate_to(uint16_t target_heading_deg);
void main_enable_charging(void);

#define ROTATOR_CCW_RELAY_SET   RELAY_EXT1_SET
#define ROTATOR_CCW_RELAY_CLR   RELAY_EXT1_CLR
#define ROTATOR_CW_RELAY_SET    RELAY_EXT2_SET
#define ROTATOR_CW_RELAY_CLR    RELAY_EXT2_CLR

#endif
