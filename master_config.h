/*
 * config.h
 *
 * Created: 29.09.2018 17:06:07
 *  Author: Kostya
 */ 


#ifndef MASTER_CONFIG_H_
#define MASTER_CONFIG_H_

#define HTML_PROGMEM          //Использовать веб страницы из flash памяти

#ifdef HTML_PROGMEM
#include "Page.h"
#endif

#define MIN_CHG 500			//ADC = (Vin * 1024)/Vref  Vref = 1V  Vin = 0.49v  3.5v-4.3v


#endif /* MASTER_CONFIG_H_ */