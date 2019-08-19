#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/eeprom.h>

#include "main.h"
#include "board.h"
#include "delay.h"
#include "usart.h"
#include "init.h"
#include "a2d.h"
#include "commands.h"
#include "xbee_interface.h"
#include "helper.h"
#include "errors.h"

volatile static uint16_t ms_counter = 0;
volatile static uint16_t ms_counter_voltage = 0;
volatile static uint16_t ms_counter_current = 0;
volatile static uint16_t ms_counter_charge  = 0;
volatile static uint16_t ms_counter_direction = 0;
volatile static uint16_t ms_counter_temperature = 0;

volatile static uint16_t counter_battery_charge = 0;
volatile static uint16_t counter_battery_charge_count = 0;

volatile static uint8_t counter_stuck_tick = 0;
volatile static int16_t rotator_last_tick_pos = 0;

volatile static uint8_t flag_errors = 0;

volatile static uint8_t sw_curr_state = 0xFF;
volatile static uint8_t sw_prev_state = 0xFF;

static struct_xbee_api_frame new_frame;

static uint8_t flag_xbee_frame_available = 0;

struct_settings settings;
struct_status status;

#define XBEE_TRANSMIT_FRAME() xbee_interface_transmit_frame(0x10, source_addr, rf_data, strlen((char *)rf_data))

int8_t main_rotator_stop(void);

//This filter was written by SM0SVX
static uint16_t median_filter(uint16_t adc) {
  status.adc_val_direction[status.adc_val_direction_pos] = adc;
  if (status.adc_val_direction_pos >= 2) {
    status.adc_val_direction_pos = 0;
  }
  else {
    status.adc_val_direction_pos++;
  }

  uint16_t a = status.adc_val_direction[0];
  uint16_t b = status.adc_val_direction[1];
  uint16_t c = status.adc_val_direction[2];

  if ((a <= b) && (a <= c))
  {
      /* a is the smallest value so the median must be either b or c */
    return (b <= c) ? b : c;
  }
  else if ((b <= a) && (b <= c))
  {
      /* b is the smallest value so the median must be either a or c */
    return (a <= c) ? a : c;
  }

    /* c is the smallest value so the median must be either a or b */
  return (a <= b) ? a : b;
}

void calculate_battery_params(void) {
  status.bat_voltage = (float)(status.adc_val_voltage * ADC_VOLTAGE_SCALING);

  int16_t adc_val = (status.adc_val_current - 511) & 0x3FF;

  //Sign extending
  if ((adc_val & 0x200) != 0) {
    adc_val &= 0x1FF;
    adc_val |= 0xFE00;
  }
  
  status.bat_current = (float)(adc_val * ADC_CURRENT_SCALING);
}

void main_parse_rx_message(uint8_t *rx_data, uint8_t length, uint8_t *source_addr) {
  uint8_t rf_data[50];

  if (length < 3)
    return;

  //Set direction command
  if (strncmp((char *)rx_data, CMD_MAIN_SET_ROTATOR_HEADING, 3) == 0) {
    if (flag_errors & (1<<ERROR_ROTATOR_STUCK)) {
      snprintf((char *)rf_data, 25, "%s ERROR: ROTATOR STUCK", CMD_MAIN_INFORMATION);

      XBEE_TRANSMIT_FRAME();
    }
    else {
      status.rotator_target_heading_deg = helper_convert_atoi(&rx_data[4], length-4);
    
      snprintf((char *)rf_data, 8, "%s %i", CMD_MAIN_SET_ROTATOR_HEADING, status.rotator_target_heading_deg);

      XBEE_TRANSMIT_FRAME();
      
      main_rotate_to(status.rotator_target_heading_deg);
    }
  }
  else if (strncmp((char *)rx_data, CMD_MAIN_GET_BATTERY_VOLTAGE, 3) == 0) { // Get the battery voltage
    calculate_battery_params();
    
    snprintf((char *)rf_data, 10, "%s %2.2f", CMD_MAIN_GET_BATTERY_VOLTAGE, (double)status.bat_voltage);

    XBEE_TRANSMIT_FRAME();
  }
  else if (strncmp((char *)rx_data, CMD_MAIN_GET_BATTERY_CURRENT, 3) == 0) { // Get the current drawn from the battery
    snprintf((char *)rf_data, 10, "%s %2.2f", CMD_MAIN_GET_BATTERY_CURRENT, (double)status.bat_current);
   
   XBEE_TRANSMIT_FRAME();
  }
  else if (strncmp((char *)rx_data, CMD_MAIN_GET_ROTATOR_HEADING, 3) == 0) { // Get the current rotator heading
    snprintf((char *)rf_data, 8, "%s %i", CMD_MAIN_GET_ROTATOR_HEADING, status.rotator_curr_heading_deg);
    XBEE_TRANSMIT_FRAME();
  }
  else if (strncmp((char *)rx_data, CMD_MAIN_GET_STATUS, 3) == 0) { // Get the current status
    calculate_battery_params();

    //Errors, Rotator status, charge status, current heading, target heading, temperature box, temperature outside, 
    //battery voltage, battery current
    sprintf((char *)rf_data, "%s %i %i %i %i %i %.1f %.1f %2.2f %2.2f", CMD_MAIN_GET_STATUS, flag_errors, status.rotator_status, status.charge_status,
            status.rotator_curr_heading_deg, status.rotator_target_heading_deg, (double)status.temp_box, (double)status.temp_outside, (double)status.bat_voltage, (double)status.bat_current);

   XBEE_TRANSMIT_FRAME();
  }
  else if (strncmp((char *)rx_data, CMD_MAIN_SET_ROTATOR_STOP, 3) == 0) { // Stop the rotation
    main_rotator_stop();
    
    snprintf((char *)rf_data, 4, "%s", CMD_MAIN_SET_ROTATOR_STOP);

    XBEE_TRANSMIT_FRAME();
    
    flag_errors = 0;
  }
  else if (strncmp((char *)rx_data, CMD_MAIN_SET_ERRORS, 3) == 0) { // Clear the errors
    snprintf((char *)rf_data, 4, "%s", CMD_MAIN_SET_ERRORS);

    XBEE_TRANSMIT_FRAME();
    flag_errors = 0;
  }
  else if (strncmp((char *)rx_data, CMD_MAIN_SET_CHARGING, 3) == 0) { // Clear the errors
    snprintf((char *)rf_data, 4, "%s", CMD_MAIN_SET_CHARGING);

    XBEE_TRANSMIT_FRAME();    
    main_enable_charging();
  }  
  else if (strncmp((char *)rx_data, CMD_MAIN_GET_CHARGE_COUNT, 3) == 0) { // Get the charge count
    sprintf((char *)rf_data,"%s %i", CMD_MAIN_GET_CHARGE_COUNT, counter_battery_charge_count);

    XBEE_TRANSMIT_FRAME();
  }    
  else if (strncmp((char *)rx_data, CMD_MAIN_GET_BOX_TEMP, 3) == 0) { // Get the box temperature
    sprintf((char *)rf_data,"%s %.1f", CMD_MAIN_GET_BOX_TEMP, (double)status.temp_box);

    XBEE_TRANSMIT_FRAME();
  }    
  else if (strncmp((char *)rx_data, CMD_MAIN_GET_OUTSIDE_TEMP, 3) == 0) { // Get the outside temperature
    sprintf((char *)rf_data,"%s %.1f", CMD_MAIN_GET_OUTSIDE_TEMP, (double)status.temp_outside);

    XBEE_TRANSMIT_FRAME();
  }    
  else if (strncmp((char *)rx_data, CMD_MAIN_SET_CHARGE_PUSH_INTERVAL, 3) == 0) { // Clear the errors
    settings.charge_push_interval = helper_convert_atoi(&rx_data[4], length-4);
	
	sprintf((char *)rf_data,"%s %i", CMD_MAIN_SET_CHARGE_PUSH_INTERVAL, settings.charge_push_interval);

	//Save the settings to the EEPROM
	eeprom_write_block(&settings, EEPROM_SETTINGS_ADDR, sizeof(settings));

    XBEE_TRANSMIT_FRAME();
  }    
  else if (strncmp((char *)rx_data, CMD_MAIN_SET_ROTATOR_CALIBRATION, 3) == 0) { //Set the rotation calibration
    if (strncmp((char *)(rx_data+4), CMD_SUB_ROTATOR_CALIBRATION_CW, 2) == 0) {
      settings.rotator_cw_lim_deg = helper_convert_atoi(&rx_data[7], length-7);
      settings.rotator_adc_val_cw = status.adc_val_direction_filtered;
      snprintf((char *)rf_data, 16, "%s %s %i %i", CMD_MAIN_SET_ROTATOR_CALIBRATION, CMD_SUB_ROTATOR_CALIBRATION_CW, settings.rotator_cw_lim_deg, settings.rotator_adc_val_cw);

      XBEE_TRANSMIT_FRAME();
    }
    else if (strncmp((char *)(rx_data+4), CMD_SUB_ROTATOR_CALIBRATION_CCW, 2) == 0) {
      settings.rotator_ccw_lim_deg = helper_convert_atoi(&rx_data[7], length-7);
      settings.rotator_adc_val_ccw = status.adc_val_direction_filtered;
      snprintf((char *)rf_data, 16, "%s %s %i %i", CMD_MAIN_SET_ROTATOR_CALIBRATION, CMD_SUB_ROTATOR_CALIBRATION_CCW, settings.rotator_ccw_lim_deg, settings.rotator_adc_val_ccw);

      XBEE_TRANSMIT_FRAME();
    }
    else if (strncmp((char *)(rx_data+4), CMD_SUB_ROTATOR_CALIBRATION_SAVE, 2) == 0) {
      settings.rotator_heading_scale = (double)((double)(settings.rotator_adc_val_cw - settings.rotator_adc_val_ccw) / (double)(settings.rotator_cw_lim_deg - settings.rotator_ccw_lim_deg));

      if (settings.rotator_heading_scale < 0)
        settings.rotator_heading_scale *= -1;

      //Save the settings to the EEPROM
      eeprom_write_block(&settings, EEPROM_SETTINGS_ADDR, sizeof(settings));

      snprintf((char *)rf_data, 12, "%s %s", CMD_MAIN_SET_ROTATOR_CALIBRATION, CMD_SUB_ROTATOR_CALIBRATION_SAVE);
      XBEE_TRANSMIT_FRAME();
    }
  }
  else if (strncmp((char *)rx_data, CMD_MAIN_SET_OUTPUT, 3) == 0) { //Set an output high or low
    if (strncmp((char *)(rx_data+4), CMD_SUB_OUTPUT_SET_HIGH, 2) == 0) {
      uint8_t output_index = helper_convert_atoi(&rx_data[7], length-7);

      switch (output_index) {
        case 1:
          //RELAY_K1_SET; //Something wrong with this channel
          break;
        case 2:
          RELAY_K2_SET;
          break;
        case 3:
          RELAY_K3_SET;
          break;
        case 4:
          RELAY_K4_SET;
          break;
        case 5:
          RELAY_K5_SET;
          break;
        case 6:
          RELAY_K6_SET;
          break;
        case 7:
          //RELAY_EXT1_SET; //Reserved for the rotation, disabled for safety reasons
          break;
        case 8:
          //RELAY_EXT2_SET; //Reserved for the rotation, disabled for safety reasons
          break;
        case 9:
          RELAY_EXT3_SET;
          break;
        case 10:
          RELAY_EXT4_SET;
          break;
        case 11:
          RELAY_EXT5_SET;
          break;
        case 12:
          RELAY_EXT6_SET;
          break;
        case 13:
          RELAY_EXT7_SET;
          break;
        case 14:
          RELAY_EXT8_SET;
          break;
        default:
          break;
      }
    }
    else if (strncmp((char *)(rx_data+4), CMD_SUB_OUTPUT_SET_LOW, 2) == 0) {
      uint8_t output_index = helper_convert_atoi(&rx_data[7], length-7);

      switch (output_index) {
        case 1:
          //RELAY_K1_CLR; //Something wrong with this channel
          break;
        case 2:
          RELAY_K2_CLR;
          break;
        case 3:
          RELAY_K3_CLR;
          break;
        case 4:
          RELAY_K4_CLR;
          break;
        case 5:
          RELAY_K5_CLR;
          break;
        case 6:
          RELAY_K6_CLR;
          break;
        case 7:
//          RELAY_EXT1_CLR; //Reserved for the rotation, disabled for safety reasons
          break;
        case 8:
  //        RELAY_EXT2_CLR;	//Reserved for the rotation, disabled for safety reasons
          break;
        case 9:
          RELAY_EXT3_CLR;
          break;
        case 10:
          RELAY_EXT4_CLR;
          break;
        case 11:
          RELAY_EXT5_CLR;
          break;
        case 12:
          RELAY_EXT6_CLR;
          break;
        case 13:
          RELAY_EXT7_CLR;
          break;
        case 14:
          RELAY_EXT8_CLR;
          break;
        default:
          break;
      }
    }
  }
}

int8_t main_rotate_cw(void) {
  printf("ROTATOR >> ROTATING CW\n");
  status.rotator_status = ROTATOR_STATUS_CW;
  
  rotator_last_tick_pos = status.rotator_curr_heading_deg;
  counter_stuck_tick = 0;
  
  ROTATOR_CCW_RELAY_CLR;
  ROTATOR_CW_RELAY_SET;

  return(0);
}

int8_t main_rotate_ccw(void) {
  printf("ROTATOR >> ROTATING CCW\n");
  status.rotator_status = ROTATOR_STATUS_CCW;

  rotator_last_tick_pos = status.rotator_curr_heading_deg;
  counter_stuck_tick = 0;
  
  ROTATOR_CW_RELAY_CLR;
  ROTATOR_CCW_RELAY_SET;

  return(0);
}

int8_t main_rotate_manual_cw(void) {
  printf("ROTATOR >> ROTATING MANUAL CW\n");
  status.rotator_status = ROTATOR_STATUS_MANUAL_CW;
  
  rotator_last_tick_pos = status.rotator_curr_heading_deg;
  counter_stuck_tick = 0;
  
  ROTATOR_CCW_RELAY_CLR;
  ROTATOR_CW_RELAY_SET;

  return(0);
}

int8_t main_rotate_manual_ccw(void) {
  printf("ROTATOR >> ROTATING MANUAL CCW\n");
  status.rotator_status = ROTATOR_STATUS_MANUAL_CCW;

  rotator_last_tick_pos = status.rotator_curr_heading_deg;
  counter_stuck_tick = 0;
  
  ROTATOR_CW_RELAY_CLR;
  ROTATOR_CCW_RELAY_SET;

  return(0);
}

int8_t main_rotator_stop_ccw(void) {
  printf("ROTATOR >> STOPPED, CCW END LIMIT REACHED\n");
  
  main_set_error(ERROR_ROTATOR_END_LIMIT_CCW);
  
  ROTATOR_CCW_RELAY_CLR;

  return(0);
}

int8_t main_rotator_stop_cw(void) {
  printf("ROTATOR >> STOPPED, CW END LIMIT REACHED\n");
  
  main_set_error(ERROR_ROTATOR_END_LIMIT_CW);
  
  ROTATOR_CW_RELAY_CLR;

  return(0);
}

int8_t main_rotator_stop(void) {
  printf("ROTATOR >> STOPPED\n");
  status.rotator_status = ROTATOR_STATUS_STOPPED;

  ROTATOR_CCW_RELAY_CLR;
  ROTATOR_CW_RELAY_CLR;

  return(0);
}

int8_t main_rotate_to(uint16_t target_heading_deg) {
  /* Get the current and target headings and adjust them so that they
      * are within the 0-359 degree range */
//    target_heading_deg = helper_adjust_range(target_heading_deg);
    int16_t unadj_current_heading_deg = helper_adc2deg(status.adc_val_direction_filtered, settings.rotator_heading_scale, settings.rotator_ccw_lim_deg);
    int16_t current_heading_deg = helper_adjust_range(unadj_current_heading_deg);
// 
    if (target_heading_deg == current_heading_deg) {
      return 0;
    }
// 
//     //This part written by SM0SVX
//    /* Find out how far we have to travel in the CW and CCW directions
//     * respectively to reach the target heading */
//    int16_t ccw_diff_deg =
//        helper_adjust_range(current_heading_deg - target_heading_deg);
//    int16_t cw_diff_deg =
//        helper_adjust_range(target_heading_deg - current_heading_deg);
// 
//    /* Find out if we will hit any rotation limits when going in a
//      * certain direction */
//    if (unadj_current_heading_deg - ccw_diff_deg < settings.rotator_ccw_lim_deg) {
//      ccw_diff_deg = INT16_MAX;
//    }
//    if (unadj_current_heading_deg + cw_diff_deg > settings.rotator_cw_lim_deg) {
//      cw_diff_deg = INT16_MAX;
//    }
// 
//    /* Choose which is the best rotation direction */
//    int16_t diff_deg;
//    if (ccw_diff_deg < cw_diff_deg) {
//      diff_deg = -ccw_diff_deg;
//    }
//    else {
//      diff_deg = cw_diff_deg;
//    }
// 
//    /* If we hit the rotation limits no matter in which direction we go,
//     * there is no way to reach the target heading. */
//    if (diff_deg == INT16_MAX) {
//      return -1;
//   }
// 
//   /* Set up the target heading and check if we should go CW or CCW. */
//   target_heading_deg = unadj_current_heading_deg;
//   
  status.rotator_target_heading = helper_deg2adc(target_heading_deg, settings.rotator_heading_scale, settings.rotator_adc_val_cw / settings.rotator_heading_scale - settings.rotator_cw_lim_deg);
  
  printf("TARGET: %i\n",status.rotator_target_heading);
  printf("TARGET deg: %i\n",status.rotator_target_heading_deg);
  
  if (status.rotator_target_heading > status.adc_val_direction_filtered) {
    return main_rotate_cw();
  }
  else if (status.rotator_target_heading < status.adc_val_direction_filtered) {
    return main_rotate_ccw();
  }

    /* We did not need to move */
    //status.rotator_target_heading = INT16_MAX;
    main_rotator_stop();

    return(0);
}

void main_set_error(uint8_t error_type) {
  flag_errors |= (1<<error_type);
  
  printf("ERROR: %i\n",error_type);
}

void main_enable_charging(void) {
  RELAY_K6_SET;
  delay_ms(250);
  delay_ms(250);
  RELAY_K6_CLR;
  
  counter_battery_charge_count++;
}

void main_xbee_frame_rxed(struct_xbee_api_frame frame) {
  new_frame = frame;
  flag_xbee_frame_available = 1;
}

void main_parse_xbee_frame(struct_xbee_api_frame frame) {
  if (frame.frame_type == XBEE_API_FRAME_RX_PACKAGE) {
    //Extract the data command that was sent
    uint8_t rx_data[frame.length-12];
    uint8_t source_addr[8];
    memcpy(source_addr, frame.frame_data, 8);

    for (uint8_t i=11;i<frame.length-1;i++) {
      rx_data[i-11] = frame.frame_data[i];
    }

    main_parse_rx_message((uint8_t *)&rx_data, sizeof(rx_data), (uint8_t *)&source_addr);
  }
}

int main(void) {
  cli();

  init_ports();
  init_timer_0();

  //115.2kbaud - debug interface
  usart0_init(7);
  fdevopen((void*)usart0_transmit, (void*)usart0_receive_loopback);

  // 57600 baud - XBee communication
  usart1_init(15);

  delay_ms(255);

  printf("Initializing ADC\n");
  a2dInit();

  xbee_interface_init(usart1_transmit, main_xbee_frame_rxed);

  status.rotator_status = ROTATOR_STATUS_STOPPED;
  status.charge_status = 0;

  //Read the settings from the EEPROM
  eeprom_read_block(&settings, EEPROM_SETTINGS_ADDR, sizeof(settings));

  sei();

  delay_ms(255);

  printf("Starting software\n");

  printf("Settings\n");
  printf("--------\n");
  printf("HEADING_SCALE: %f\n",settings.rotator_heading_scale);
  printf("LIM CCW: %i deg\n", settings.rotator_ccw_lim_deg);
  printf("LIM CW: %i deg\n", settings.rotator_cw_lim_deg);
  printf("LIM CCW: %i ADC\n", settings.rotator_adc_val_ccw);
  printf("LIM CW: %i ADC\n", settings.rotator_adc_val_cw);
  printf("CHARGE INTERVAL: %i\n", settings.charge_push_interval);
    
  //Read the direction so we are sure that the median filter is full
  status.adc_val_direction_filtered = median_filter(a2dConvert10bit(ADC_CH_DIRECTION));
  status.adc_val_direction_filtered = median_filter(a2dConvert10bit(ADC_CH_DIRECTION));
  status.adc_val_direction_filtered = median_filter(a2dConvert10bit(ADC_CH_DIRECTION));
  
  main_enable_charging();
  
  while(1) {
    //Check if we should push the battery charger, if so we push the button for it to
    //change charge mode
    if ((counter_battery_charge/60) >= settings.charge_push_interval) {
      main_enable_charging();
      
      counter_battery_charge = 0;
    }
  
    if (ms_counter_temperature > POLL_INTERVAL_TEMPERATURE) {
      status.adc_val_temp_box = a2dConvert10bit(ADC_CH_BOX_TMP36);
      status.temp_box = (float)(status.adc_val_temp_box - 102.4f)*0.48828125;
      printf("TEMP BOX: %i %.1fC\n", status.adc_val_temp_box, (double)status.temp_box);
      
      
      status.adc_val_temp_outside = a2dConvert10bit(ADC_CH_OUTSIDE_TMP36);
      status.temp_outside = (float)(status.adc_val_temp_outside - 102.4f)*0.48828125;
      
      printf("TEMP OUT: %i %.1fC\n", status.adc_val_temp_outside, (double)status.temp_outside);
      
      ms_counter_temperature = 0;
    }
  
    if (ms_counter_voltage > POLL_INTERVAL_VOLTAGE) {
      status.adc_val_voltage = a2dConvert10bit(ADC_CH_VOLTAGE);
      ms_counter_voltage = 0;
    }

    if (ms_counter_current > POLL_INTERVAL_CURRENT) {
      status.adc_val_current = a2dConvert10bit(ADC_CH_CURRENT);
      ms_counter_current = 0;
    }

    if (ms_counter_charge > POLL_INTERVAL_CHARGE) {
      if (PINE & (1<<LED_SLOW_CHARGE))
        status.charge_status |= (1<<0);
      else
        status.charge_status &= ~(1<<0);
        
      if (PINE & (1<<LED_CHARGE))
        status.charge_status |= (1<<1);
      else
        status.charge_status &= ~(1<<1);
        
      ms_counter_charge = 0;
    }

    if (ms_counter_direction > POLL_INTERVAL_DIRECTION) {
      status.adc_val_direction_filtered = median_filter(a2dConvert10bit(ADC_CH_DIRECTION));

      status.rotator_curr_heading_deg = helper_adjust_range(helper_adc2deg(status.adc_val_direction_filtered, settings.rotator_heading_scale, settings.rotator_adc_val_cw / settings.rotator_heading_scale - settings.rotator_cw_lim_deg));
      printf("ROTATOR HEADING: %i - %i\n", status.rotator_curr_heading_deg, status.adc_val_direction_filtered);

      if ((status.rotator_status == ROTATOR_STATUS_CW) || (status.rotator_status == ROTATOR_STATUS_CCW)) {
	//End limit reachced CCW
	if (status.adc_val_direction_filtered <= settings.rotator_adc_val_ccw) {
	  //Force the rotator to stop rotation to the CCW direction
	  main_rotator_stop_ccw();
	}  //End limit reached CW
	else if (status.adc_val_direction_filtered >= settings.rotator_adc_val_cw) {
	  //Force the rotator to stop rotation to the CW direction
	  main_rotator_stop_cw();
	}
      }	
	  
      if (status.rotator_status == ROTATOR_STATUS_CW) {
        if (status.adc_val_direction_filtered > status.rotator_target_heading) {
          main_rotator_stop();
          printf("TARGET reached\n");
        }
      }
  
      if (status.rotator_status == ROTATOR_STATUS_CCW) {
        if (status.adc_val_direction_filtered <= status.rotator_target_heading) {
          main_rotator_stop();
          printf("TARGET reached\n");
        }
      }

      sw_curr_state = PING & 0x03;

      if (sw_curr_state != sw_prev_state) {
        if (sw_prev_state == 0x03) {
          if ((sw_curr_state & (1<<0)) == 0) {
            if ((flag_errors & (1<<ERROR_ROTATOR_STUCK)) == 0) {
              main_rotate_manual_ccw();
              printf("Manual CCW\n");
            }
          }
          else if ((sw_curr_state & (1<<1)) == 0) {
            if ((flag_errors & (1<<ERROR_ROTATOR_STUCK)) == 0) {
              main_rotate_manual_cw();
              printf("Manual CW\n");
            }
          }
        }
        else {
          main_rotator_stop();
          printf("MANUAL ROTATION STOPPED\n");
        }
        
        sw_prev_state = sw_curr_state;
      }

      ms_counter_direction = 0;
    }

    if (flag_xbee_frame_available) {
      main_parse_xbee_frame(new_frame);

      flag_xbee_frame_available = 0;
    }
  }
}

ISR(SIG_USART1_RECV) {
  xbee_interface_rx_char(UDR1);
}

ISR(SIG_USART1_DATA) {
}

ISR(SIG_OUTPUT_COMPARE0) {
  ms_counter++;
  ms_counter_current++;
  ms_counter_voltage++;
  ms_counter_direction++;
  ms_counter_charge++;
  ms_counter_temperature++;


  //Blink Green status LED every second
  if ((ms_counter % 500) == 0) {
    PORTA ^= (1<<7);
  }

  //Check every second
  if ((ms_counter % 1000) == 0) {
    counter_battery_charge++;
  
    //Check if the rotator is actually turning
    if ((status.rotator_status == ROTATOR_STATUS_CCW) && (status.rotator_status == ROTATOR_STATUS_CW)) { 
      if (counter_stuck_tick >= ROTATOR_STUCK_TICK_LIMIT) {
        if (abs(rotator_last_tick_pos - status.rotator_curr_heading_deg) < ROTATOR_STUCK_TICK_DEG_LIMIT) {
          main_rotator_stop();
          main_set_error(ERROR_ROTATOR_STUCK);
        }
        
        rotator_last_tick_pos = status.rotator_curr_heading_deg;
        counter_stuck_tick = 0;
      }
      
      counter_stuck_tick++;
    }
  }
  
  xbee_interface_ms_tick();
}
