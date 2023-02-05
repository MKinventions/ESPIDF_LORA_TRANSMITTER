#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "string.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "cJSON.h"
#include "lora.h"

#define JOYX ADC1_CHANNEL_6//GPIO34(ADC1_6)34
#define JOYY ADC1_CHANNEL_7//GPIO35(ADC1_7)35
#define POT1 ADC1_CHANNEL_4//GPIO34(ADC1_4)32
#define POT2 ADC1_CHANNEL_5//GPIO34(ADC1_5)33
#define SW1 15
#define SW2 13



int lastSwitch1 = 0;
int lastButton2 = 0;
int switch1_state = 0;
int button2_state = 0;


void task_tx(void *p)
{

	adc1_config_width(ADC_WIDTH_BIT_12);//(0 - 4095)
//    adc1_config_channel_atten(POTENTIOMETER1, ADC_ATTEN_DB_11);   //~150mv - 2450mv(11dB)
  adc1_config_channel_atten(JOYX, ADC_ATTEN_DB_11);   //~150mv - 2450mv(11dB)
  adc1_config_channel_atten(JOYY, ADC_ATTEN_DB_11);   //~150mv - 2450mv(11dB)
  adc1_config_channel_atten(POT1, ADC_ATTEN_DB_11);   //~150mv - 2450mv(11dB)
  adc1_config_channel_atten(POT2, ADC_ATTEN_DB_11);   //~150mv - 2450mv(11dB)

  gpio_pad_select_gpio(SW1);
  gpio_pad_select_gpio(SW2);
  gpio_pullup_en(SW1);
  gpio_pullup_en(SW2);
  gpio_set_direction(SW1, GPIO_MODE_INPUT);
  gpio_set_direction(SW2, GPIO_MODE_INPUT);


   while(1) {
		int JOY_X = adc1_get_raw(JOYX);
		int JOY_Y = adc1_get_raw(JOYY);
		int POT_1 = adc1_get_raw(POT1);
		int POT_2 = adc1_get_raw(POT2);
//		int SWITCH1 = gpio_get_level(SW1);
        int SWITCH2 = gpio_get_level(SW2);


		 /***************button start***********************/
		          int currentSwitch1 = gpio_get_level(SW1);
		          if(currentSwitch1 != lastSwitch1){
		          	lastSwitch1 = currentSwitch1;

		          	if(lastSwitch1 == 1){
//		                  switch1_state = (switch1_state == 1)? 0 : 1;
		                  switch1_state = !switch1_state;
		          	   }
		          	vTaskDelay(250 / portTICK_PERIOD_MS);
		          }

		  /***************button end***********************/


	ESP_LOGI("TX", "Serialize.....");
	cJSON *root;
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "version", "1.1.2v");
	cJSON_AddStringToObject(root, "macid", "12345ABC256");
//	cJSON_AddNumberToObject(root, "joy_x", JOY_X);
//	cJSON_AddNumberToObject(root, "joy_y", JOY_Y);
//	cJSON_AddNumberToObject(root, "pot_1", POT_1);
//	cJSON_AddNumberToObject(root, "pot_2", POT_2);
//	cJSON_AddNumberToObject(root, "sw_1", switch1_state);
//	cJSON_AddNumberToObject(root, "sw_2", SWITCH2);

	// Add JOYSTICK simple element
		cJSON *array1;
		array1 = cJSON_AddArrayToObject(root, "JOYSTICK");
		//array = cJSON_CreateArray();
		cJSON *element1;
		element1 = cJSON_CreateNumber(JOY_X);
		cJSON_AddItemToArray(array1, element1);
		element1 = cJSON_CreateNumber(JOY_Y);
		cJSON_AddItemToArray(array1, element1);


	// Add POTENTIOMETER simple element
		cJSON *array2;
		array2 = cJSON_AddArrayToObject(root, "POTENTIOMETER");
		//array = cJSON_CreateArray();
		cJSON *element2;
		element2 = cJSON_CreateNumber(POT_1);
		cJSON_AddItemToArray(array2, element2);
		element2 = cJSON_CreateNumber(POT_2);
		cJSON_AddItemToArray(array2, element2);

	// Add SWITCH simple element
		cJSON *array3;
		array3 = cJSON_AddArrayToObject(root, "SWITCH");
		//array = cJSON_CreateArray();
		cJSON *element3;
		element3 = cJSON_CreateNumber(switch1_state);
		cJSON_AddItemToArray(array3, element3);
		element3 = cJSON_CreateNumber(SWITCH2);
		cJSON_AddItemToArray(array3, element3);


	char *my_json_string = cJSON_Print(root);
//	ESP_LOGI("TX", "my_json_string\n%s",my_json_string);
	cJSON_Delete(root);


      lora_send_packet((uint8_t*)my_json_string, strlen(my_json_string));
      printf("Sending Json Data : %s\n", my_json_string);
      printf("packet sent...\n");

	vTaskDelay(1/portTICK_PERIOD_MS);
   }
}


void app_main()
{
   lora_init();
   lora_set_frequency(433e6);
   lora_enable_crc();
   xTaskCreate(&task_tx, "task_tx", 2048, NULL, 5, NULL);
}
