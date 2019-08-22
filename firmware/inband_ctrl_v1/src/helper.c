#include <avr/io.h>
#include <avr/signal.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>

#include "helper.h"
#include "global.h"
#include "avrlibdefs.h"
#include "avrlibtypes.h"

int16_t helper_convert_atoi(uint8_t *data, uint8_t length) {
  uint8_t temp[length+1];

  for (uint8_t i=0;i<length;i++) {
    temp[i] = data[i];
  }

  temp[length] = '\n';

  return(atoi(temp));
}

int16_t helper_adc2deg(int16_t adc, double scale, int16_t offset) {
  return (adc / scale - offset);
}

int16_t helper_deg2adc(int16_t deg, double scale, int16_t offset) {
  return (deg + offset) * scale;
}

int16_t helper_adjust_range(int16_t deg) {
  int16_t retval = deg;
  
  while (retval < 0) {
    retval += 360;
  }
  
  while (retval > 359) {
    retval -= 360;
  }
  
  return (retval);
}
