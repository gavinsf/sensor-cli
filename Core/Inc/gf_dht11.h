

#ifndef GF_DHT11_H_
#define GF_DHT11_H_

#include "stm32f4xx_it.h"
#include "main.h"

struct _dht11_t {
	GPIO_TypeDef* port;
	uint16_t pin;
	TIM_HandleTypeDef* htim;
	float temp;
	float hum;
};
typedef struct _dht11_t dht11_t;

typedef enum {
	DHT_OK = 0,
	DHT_ERROR_START_1,
	DHT_ERROR_START_2,
	DHT_TIMEOUT_START,
	DHT_TIMEOUT_BIT_START,
	DHT_TIMEOUT_BIT_TRANS,
	DHT_ERROR_BIT_START,
	DHT_ERROR_BIT_TRANS,
	DHT_ERROR_CHECKSUM
} dht_status_t;

void init_dht11(dht11_t* dht, TIM_HandleTypeDef* htim, GPIO_TypeDef* port, uint16_t pin);
dht_status_t read_dht11(dht11_t* dht);


#endif
