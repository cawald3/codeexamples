/*
 * Code for Diabot Transmitting Side (Groundstation to robot)
 */

/*
 */
/* Application header files */
#include <ti_radio_config.h>

/* Board Header files */
#include "ti_drivers_config.h"

/* Standard C Libraries */
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>

/* TI Drivers */
#include <ti/drivers/ADC.h>
#include <ti/drivers/SPI.h>
#include <ti/display/Display.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/PWM.h>
#include <ti/drivers/rf/RF.h>
#include <ti/devices/DeviceFamily.h>

/* EasyLink API Header files */
#include "easylink/EasyLink.h"

/* Undefine to not use async mode */
#define RFEASYLINKTX_ASYNC

#define RFEASYLINKTX_BURST_SIZE         10
#define RFEASYLINKTXPAYLOAD_LENGTH      30

/* Pin driver handle */
static PIN_Handle pinHandle;
static PIN_State pinState;

/* Misc Handlers */
static Display_Handle display;

uint16_t adcValue0;
uint32_t adcValue0MicroVolt;
uint8_t MOTOR_MASK = 0b00000000;
uint8_t CAMERA_MASK = 0b00000001;

/*
 * Application LED pin configuration table:
 *   - All LEDs board LEDs are off.
 */
PIN_Config pinTable[] = {
    CONFIG_PIN_GLED | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    //CONFIG_PIN_RLED | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

static uint16_t seqNumber;

static volatile bool txDoneFlag;
static volatile uint8_t txSleepPeriodsElapsed;

/**
 * Converts a reading from 0 - maxinput to 0 - maxOut
 */
uint16_t convToRange(float reading, float maxInput, uint16_t maxOut){
    return (int)((reading/maxInput)*(float)maxOut) ;
}

void txDoneCb(EasyLink_Status status)
{
    if (status == EasyLink_Status_Success)
    {
        /* Toggle GLED to indicate TX */
        PIN_setOutputValue(pinHandle, CONFIG_PIN_GLED,!PIN_getOutputValue(CONFIG_PIN_GLED));
    }
    else if(status == EasyLink_Status_Aborted)
    {
        /* Toggle RLED to indicate command aborted */
        PIN_setOutputValue(pinHandle, CONFIG_PIN_RLED,!PIN_getOutputValue(CONFIG_PIN_RLED));
    }
    else
    {
        /* Toggle GLED and RLED to indicate error */
        PIN_setOutputValue(pinHandle, CONFIG_PIN_GLED,!PIN_getOutputValue(CONFIG_PIN_GLED));
        PIN_setOutputValue(pinHandle, CONFIG_PIN_RLED,!PIN_getOutputValue(CONFIG_PIN_RLED));
    }
    txDoneFlag = true;
    txSleepPeriodsElapsed = 0;
}

void *mainThread(void *arg0)
{
    //PWM Settings
    uint16_t   pwmPeriod = 300;
    uint16_t   duty = 0;

    PWM_Handle pwm1 = NULL;
    PWM_Params params;

    //These two calls allow us to use the ADC and Display Drivers
    PWM_init();
    Display_init();

    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL) {
        /* Failed to open display driver */
        while (1);
    }
    Display_printf(display, 0, 0, "Display Successfully Opened!\n");

    uint32_t absTime;

    ADC_Handle adc1;
    ADC_Handle   adc;
    ADC_Params   adcParams;
    int_fast16_t res;

    //Open up the ADC for use
    ADC_init();
    ADC_Params_init(&adcParams);
    adc = ADC_open(CONFIG_ADC_0, &adcParams);
    adc1 = ADC_open(CONFIG_ADC_1, &adcParams);

    //Configure PWM
    PWM_Params_init(&params);
    params.dutyUnits = PWM_DUTY_US;
    params.dutyValue = 0;
    params.periodUnits = PWM_PERIOD_US;
    params.periodValue = pwmPeriod;
    pwm1 = PWM_open(CONFIG_PWM_0, &params);
    PWM_start(pwm1);

    /* Open LED pins */
    pinHandle = PIN_open(&pinState, pinTable);
    if (pinHandle == NULL)
    {
        while(1);
    }

    /* Clear LED pins */
    PIN_setOutputValue(pinHandle, CONFIG_PIN_GLED, 0);
    PIN_setOutputValue(pinHandle, CONFIG_PIN_RLED, 0);


    /* Reset the sleep period counter */
    txSleepPeriodsElapsed = 0;
    /* Set the transmission flag to its default state */
    txDoneFlag = false;


    // Initialize the EasyLink parameters to their default values
    EasyLink_Params easyLink_params;
    EasyLink_Params_init(&easyLink_params);

    /*
     * Initialize EasyLink with the settings found in ti_easylink_config.h
     * Modify EASYLINK_PARAM_CONFIG in ti_easylink_config.h to change the default
     * PHY
     */
    if (EasyLink_init(&easyLink_params) != EasyLink_Status_Success){
        while(1);
    }


    /*
     * If you wish to use a frequency other than the default, use
     * the following API:
     * EasyLink_setFrequency(868000000);
     */
    uint16_t dutyCycleNew;
    float adcValuef;

    while(1) {
        EasyLink_TxPacket txPacket =  { {0}, 0, 0, {0} };
        res = ADC_convert(adc, &adcValue0);  //Get the result of the adc conversion
        adcValue0MicroVolt = ADC_convertToMicroVolts(adc, adcValue0);         //This will give us a value in microvolts
        adcValuef = adcValue0MicroVolt;
        dutyCycleNew = convToRange(adcValuef, 3300000, 255);

        //PWM_setDuty(pwm1, dutyCycleNew);
        
        Display_printf(display, 0, 0, "CONFIG_ADC_0 convert result: %d uV\n",
                    adcValue0MicroVolt);
        Display_printf(display, 0, 0, "Duty Cycle New Value: %d uV\n",
                    dutyCycleNew);
        txPacket.payload[0] = (uint8_t)(MOTOR_MASK); //Used to send motor commands
        txPacket.payload[1] = (uint8_t)(0); //Unused
        txPacket.payload[2] = (uint8_t)(dutyCycleNew); //Send the x reading

        res = ADC_convert(adc1, &adcValue0);  //Get the result of the adc conversion
        adcValue0MicroVolt = ADC_convertToMicroVolts(adc1, adcValue0);         //This will give us a value in microvolts
        adcValuef = adcValue0MicroVolt;
        dutyCycleNew = convToRange(adcValuef, 3300000, 255);
        PWM_setDuty(pwm1, dutyCycleNew);
        txPacket.payload[3] = (uint8_t)(dutyCycleNew);
        txPacket.len = 4; //How many bytes of data we are sending

        /*
         * Address filtering is enabled by default on the Rx device with the
         * an address of 0xAA. This device must set the dstAddr accordingly.
         */
        txPacket.dstAddr[0] = 0xaa;

        /*
         * Set the Transmit done flag to false, callback will set it to true
         * Also set the sleep counter to 0
         */
        txDoneFlag = false;
        txSleepPeriodsElapsed = 0;

        /* Transmit the packet */
        EasyLink_transmitAsync(&txPacket, txDoneCb);

        while(!txDoneFlag){
            /*
             * Set the device to sleep for 108ms. The packet transmission is
             * set 100 ms in the future but takes about 7ms to complete and
             * for the execution to hit the callback. A 1ms buffer is added to
             * the sleep time to ensure the callback always execute prior to
             * the end of usleep().
             */
            usleep(10000);

            /* check to see if the transmit flag was set during sleep */
            if(!txDoneFlag){
                txSleepPeriodsElapsed++;
                if(txSleepPeriodsElapsed == 100){
                    /* 324 ms have passed. We need to abort the transmission */
                    if(EasyLink_abort() == EasyLink_Status_Success)
                    {
                        /*
                         * Abort will cause the txDoneCb to be called and the
                         * txDoneFlag to be set
                         */
                        while(!txDoneFlag){};
                    }
                    break;
                }
            }
        }

    }
}

/*
 * Copyright (c) 2015-2018, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
