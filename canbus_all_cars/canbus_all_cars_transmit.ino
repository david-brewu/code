#include "driver/gpio.h"
#include "driver/can.h"



unsigned long previousMillis = 0;
const long interval = 3000;

void app_main()
{
    //Initialize configuration structures using macro initializers
    can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, CAN_MODE_NORMAL);
    can_timing_config_t t_config = CAN_TIMING_CONFIG_500KBITS();
    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    //Install CAN driver
    if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        printf("Driver installed\n");
    } else {
        printf("Failed to install driver\n");
        return;
    }
    //Start CAN driver
    if (can_start() == ESP_OK) {
        printf("Driver started\n");
    } else {
        printf("Failed to start driver\n");
        return;
    } 
}

void rpm_request(){
   can_message_t message;
   message.identifier = 0x7df;
   message.data_length_code = 8;
   message.data[0] = 0x02;
   message.data[1] = 0x01;
   message.data[0] = 0x0c;
   message.data[1] = 0x00;
   message.data[0] = 0x00;
   message.data[1] = 0x00;
   message.data[0] = 0x00;
   message.data[1] = 0x00;

   if (can_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
    printf("RPM request queued for transmission\n");
} else {
    printf("Failed to queue RPM request for transmission\n");
}
}



void receiver_filter()
{
   can_message_t message;
if (can_receive(&message, pdMS_TO_TICKS(10000)) == ESP_OK) {
    printf("Message received\n");
} else {
    printf("Failed to receive message\n");
    return;
}

if (message.flags & CAN_MSG_FLAG_EXTD) {
    printf("Message is in Extended Format\n");
} else {
    printf("Message is in Standard Format\n");
}

printf("identifier %x\n",message.identifier);
 if (!(message.flags & CAN_MSG_FLAG_RTR)) {
for (int i = 0; i < message.data_length_code; i++) {
printf("Data byte %d = %x\n", i, message.data[i]);
}
}
//}else{printf("message not rpm");}

}
  

 
void setup(){
  app_main();
}


void loop(){
//  unsigned long currentMillis = millis();
//
//  if (currentMillis - previousMillis >= interval) {
//    // save the last time you blinked the LED
//    previousMillis = currentMillis;
//  rpm_request();
//}
// receiver_filter();
transmit();
}




void transmit(){
  can_message_t message;
    message.identifier = 0xAAAA;
    message.flags = CAN_MSG_FLAG_EXTD;
    message.data_length_code = 4;
    for (int i = 0; i < 4; i++) {
       message.data[i] = i;
    }

//Queue message for transmission
if (can_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
    printf("Message queued for transmission\n");
} else {
    printf("Failed to queue message for transmission\n");
}
delay(5000);
}
