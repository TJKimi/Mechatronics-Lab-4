#include "msp.h"
#include "driverlib.h"
#include "stdlib.h"
#include "uart.h"
#include <stdint.h>
#include <stdbool.h>

volatile char receiveBuffer[500]="";
int array = 0;
int edge = 0;

const eUSCI_UART_Config uartConfig =
{
    EUSCI_A_UART_CLOCKSOURCE_SMCLK, // SMCLK Clock Source
    19,                                     // BRDIV = 19
    8,                                       // UCxBRF = 8
    85,                                       // UCxBRS = 85
    EUSCI_A_UART_NO_PARITY, // No Parity
    EUSCI_A_UART_LSB_FIRST, // LSB First
    EUSCI_A_UART_ONE_STOP_BIT, // One stop bit
    EUSCI_A_UART_MODE, // UART mode
    EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION // Oversampling
    /*clock prescalar: 19, firstModReg: 8, secondModReg: 85, oversampling: 1*/
};

int main(void)
{
    MAP_WDT_A_holdTimer();//stop watchdog timer
    unsigned int dcoFrequency = 3E+6; // configure clock
    MAP_CS_setDCOFrequency(dcoFrequency);
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_3);
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);//P1.1,P1.2,P1.3

    /*initialize UART A0*/
    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);//configure UART
    MAP_UART_enableModule(EUSCI_A0_BASE); //enable UART Module

    /*enable interrupts*/
    MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
    MAP_Interrupt_enableMaster();

    while(1)
        {
            if (edge == 1) {
                int i;
                for (i = 0; i < array; i = i + 1) {
                    MAP_UART_transmitData(EUSCI_A0_BASE, receiveBuffer[i]);
                }
                array = 0;
                edge = 0;
                MAP_UART_transmitData(EUSCI_A0_BASE,13);
                MAP_UART_transmitData(EUSCI_A0_BASE,10);
            }

        }

}//end of main

/* EUSCI A0 UART ISR - Echoes data back to PC host */
void EUSCIA0_IRQHandler(void)
{
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_BASE);
    MAP_UART_clearInterruptFlag(EUSCI_A0_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        int ASCII = MAP_UART_receiveData(EUSCI_A0_BASE);

        if (ASCII != 13){
            receiveBuffer[array] = MAP_UART_receiveData(EUSCI_A0_BASE);
            array++;
        } else {
            edge = 1;
        }

    }

}

/*
void uart_isr(void)
{
    receivedBuffer[200] = EUSCI_A_UART_receiveData(EUSCI_A0_BASE);

    if(receivedBuffer[0] & 0x02 == 0)
    {
        MAP_UART_transmitData(EUSCI_A0_BASE, MAP_UART_receiveData(EUSCI_A0_BASE));

    }
}*/
