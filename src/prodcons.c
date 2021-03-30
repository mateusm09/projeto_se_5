#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS
#include "stdbool.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"

#define BUFFER_SIZE 8

osThreadId_t ledThread;
osSemaphoreId_t ledSignal;

const int debounceTicks = 250;
int lastTick = 0;

void ledThreadHandler(void *arg){
  uint8_t count = 0;
  
  while(1){
    osSemaphoreAcquire(ledSignal, osWaitForever); // há dado disponível?
    
    if(count >= 16) count = 0;
    count++;
    
    LEDWrite(LED4 | LED3 | LED2 | LED1, count & 0xF); // apresenta informação consumida
  } 
}

void buttonIntHandler() {
 
  int currentTick = osKernelGetTickCount();
  
  if(currentTick - lastTick >= debounceTicks) {
    osSemaphoreRelease(ledSignal);
    lastTick = currentTick;
  }
  
  GPIOIntClear(GPIO_PORTJ_BASE, GPIO_INT_PIN_0); 
}

void configButton() {
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); 
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)); // Aguarda final da habilita??o
  
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
  GPIOPadConfigSet(GPIO_PORTJ_BASE ,GPIO_PIN_0,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
  GPIOIntDisable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);
  GPIOIntTypeSet(GPIO_PORTJ_BASE,GPIO_PIN_0,GPIO_FALLING_EDGE);
  GPIOIntRegister(GPIO_PORTJ_BASE, buttonIntHandler);
  GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);
  int i;
  for(i = 0; i < 25000; i++);
}

void main(void){
  SystemInit();
  LEDInit(LED4 | LED3 | LED2 | LED1);
  configButton();
  
  osKernelInitialize();

  ledThread = osThreadNew(ledThreadHandler, NULL, NULL);
  ledSignal = osSemaphoreNew(BUFFER_SIZE, 0, NULL); // espaços ocupados = 0
  
  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main
