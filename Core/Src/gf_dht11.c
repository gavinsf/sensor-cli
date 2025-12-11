#include "gf_dht11.h"

void init_dht11(dht11_t* dht, TIM_HandleTypeDef* htim, GPIO_TypeDef* port, uint16_t pin)
{
	dht->htim = htim;
	dht->port = port;
	dht->pin = pin;
}

// !! Using GPIO mode Open Drain with 5k pull-up resistor
dht_status_t read_dht11(dht11_t* dht)
{
	uint16_t tim = 0;
	uint8_t bit;
	uint8_t bitmap[5] = {0};


	HAL_GPIO_WritePin(dht->port, dht->pin, GPIO_PIN_RESET);
	HAL_Delay(18); // Wait 18ms in low
	__disable_irq(); // Disabling interrupts for accurate timer
	HAL_TIM_Base_Start(dht->htim); // start timer
	HAL_GPIO_WritePin(dht->port, dht->pin, GPIO_PIN_SET);

	__HAL_TIM_SET_COUNTER(dht->htim, 0); // set timer counter to 0
	while (HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_SET) {
		if ((uint16_t)__HAL_TIM_GET_COUNTER(dht->htim) > 500) {
			__enable_irq();
			return DHT_TIMEOUT_START;
		}
	}


	__HAL_TIM_SET_COUNTER(dht->htim, 0);
	while (HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_RESET) {
		if ((uint16_t)__HAL_TIM_GET_COUNTER(dht->htim) > 500) {
			__enable_irq();
			return DHT_TIMEOUT_START;
		}
	}
	tim = (uint16_t)__HAL_TIM_GET_COUNTER(dht->htim);

	// Expecting tim_1 = 80 (microseconds)
	if (tim < 75 || tim > 85) {
		__enable_irq();
		return DHT_ERROR_START_1;
	}

	__HAL_TIM_SET_COUNTER(dht->htim, 0);
	while (HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_SET) {
		if ((uint16_t)__HAL_TIM_GET_COUNTER(dht->htim) > 500) {
			__enable_irq();
			return DHT_TIMEOUT_START;
		}
	}
	tim = (uint16_t)__HAL_TIM_GET_COUNTER(dht->htim);

	// Expecting tim_2 = 80 (microseconds)
	if (tim < 70 || tim > 90) {
		__enable_irq();
		return DHT_ERROR_START_2;
	}

	/*
	 Complete data transmission is 40 bits
	 Expected format in order:
	 - 8 bit integral RH data
	 - 8 bit decimal RH data
	 - 8 bit integral T data
	 - 8 bit decimal T data
	 - 8 bit checksum:
	 	 - Last 8 bits of data summed
	*/
	for (int b = 0; b < 40; b++) {
		__HAL_TIM_SET_COUNTER(dht->htim, 0);
		while (HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_RESET) {
			if ((uint16_t)__HAL_TIM_GET_COUNTER(dht->htim) > 500) {
				__enable_irq();
				return DHT_TIMEOUT_BIT_START;
			}
		}
		tim = (uint16_t)__HAL_TIM_GET_COUNTER(dht->htim);

		// Expecting 50 (microseconds)
		if (tim < 40 || tim > 60) {
			__enable_irq();
			return DHT_ERROR_BIT_START;
		}

		__HAL_TIM_SET_COUNTER(dht->htim, 0);
		while (HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_SET) {
			if ((uint16_t)__HAL_TIM_GET_COUNTER(dht->htim) > 500) {
				__enable_irq();
				return DHT_TIMEOUT_BIT_START;
			}
		}
		tim = (uint16_t)__HAL_TIM_GET_COUNTER(dht->htim);

		if (tim > 15 && tim < 35) {
			bit = 0;
		} else if (tim > 60 && tim < 80) {
			bit = 1;
		} else {
			__enable_irq();
			return DHT_TIMEOUT_BIT_TRANS;
		}

		// b >> 3 means get byte in bitmap. Either 0, 1, 2, 3, or 4
		// b & 7 means get order in byte which is 0 to 7. 7 is first 3 bits.
		bitmap[b >> 3] |= bit << (7 - (b & 7));
	}
	HAL_TIM_Base_Stop(dht->htim);
	__enable_irq();

	uint8_t RH_integral = bitmap[0];
	uint8_t RH_decimal = bitmap[1];
	uint8_t T_integral = bitmap[2];
	uint8_t T_decimal = bitmap[3];
	uint8_t checksum = bitmap[4];

	uint8_t last = RH_integral + RH_decimal + T_integral + T_decimal;

	if (last != checksum) {
		return DHT_ERROR_CHECKSUM;
	}

	// 1% resolution
	dht->hum = RH_integral + (RH_decimal / 10.0f);

	// 1 degree resolution
	dht->temp = T_integral;

	return DHT_OK;
}
