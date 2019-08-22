
#ifndef _HELPER_H_
#define _HELPER_H_

//#define PRINTF_XBEE printf
#define PRINTF_XBEE(...)

int16_t helper_convert_atoi(uint8_t *data, uint8_t length);

int16_t helper_adc2deg(int16_t adc, double scale, int16_t offset);
int16_t helper_deg2adc(int16_t deg, double scale, int16_t offset);

//Converts a signed heading to an unsigned heading (0-359 deg)
int16_t helper_adjust_range(int16_t deg);

#endif
