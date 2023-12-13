#include <FlexCAN_T4.h>
#include "arduino_freertos.h"
#include "avr/pgmspace.h"
#include <semphr.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
CAN_message_t msg;
CAN_message_t receivedMsg;
char buf[40];
String checks;
int8_t  dataTherm[10];
int value1, adcValue;
unsigned long value3;
SemaphoreHandle_t adcMutex;
bool adcValueReady = false;
QueueHandle_t canQueue;


 void canint(const CAN_message_t &receivedMsg) {
//  can1.events();
            value3 = receivedMsg.id;
          if(receivedMsg.id == 0x1838F380){
            ::Serial.print("CAN ID:  ");
            ::Serial.print(receivedMsg.id,arduino:: HEX );
            ::Serial.print(" ");
//            dataTherm = recievedMs
            for(int k=0;k<8;k++){
             dataTherm[k] =  (int8_t)receivedMsg.buf[k];
              ::Serial.print(dataTherm[k]);
               ::Serial.print(" ");
              }
              ::Serial.println();
//            ::vTaskDelay(pdMS_TO_TICKS(30));
//        }
          }
}

////    if (xHigherPriorityTaskWoken != pdFALSE) {
////        portYIELD_FROM_ISR();
////    }
//}
