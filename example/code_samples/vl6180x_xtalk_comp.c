/*******************************************************************************
Copyright © 2014, STMicroelectronics International N.V.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of STMicroelectronics nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
NON-INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS ARE DISCLAIMED. 
IN NO EVENT SHALL STMICROELECTRONICS INTERNATIONAL N.V. BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************/
/*
 * $Date: 2015-01-08 14:50:32 +0100 (Thu, 08 Jan 2015) $
 * $Revision: 2044 $
 */
 
/**
 * @file vl6180x_xtalk_comp.c
 *
 * @brief Cross talk compensation calibration and simple used
 */

#include <vl6180x_api.h>
#include <vl6180x_sample_plat.h> /* contain all device/platform specific code */

#define N_MEASURE_AVG   20


/**
 * All in one device init
 * @param myDev  The device
 * @return     0 on success may @a #CALIBRATION_WARNING <0 on errir
 */
int Sample_Init(VL6180xDev_t myDev){
    int status, init_status;
    MyDev_Init(myDev);           // your code
    MyDev_SetChipEnable(myDev);  // your code
    MyDev_uSleep(2000);          // your code sleep at least 1msec prior to do i2c to device
    init_status = VL6180x_InitData(myDev);
    if(init_status == 0 || init_status == CALIBRATION_WARNING ){
        status = VL6180x_Prepare(myDev);
        if( !status )
            status=init_status; // if prepare is successfull return potential init warning
    }
    return status;
}

/**
 * Implement Xtalk calibration as described in VL6180x Datasheet (DocID026171 Rev 6)
 *
 * Device must be initialized
 *
 * @note device scaling wrap filter and xtalk setting are scraped !
 *  It is safer to reset and re init device when done
 * @warning  follow strictly procedure described in the device manual
 * @param myDev  The device
 * @return The cross talk (9.7 fix point as expected in register)
 */
int Sample_XTalkRunCalibration(VL6180xDev_t myDev)
{
    VL6180x_RangeData_t Range[N_MEASURE_AVG];
    int32_t   RangeSum;
    int32_t   RateSum;
    int status;
    int i;
    int XTalkInt;
    int RealTargetDistance=100;

    VL6180x_UpscaleSetScaling(myDev, 1); // set scaling x1
    // we do not check status as this can fail only in config that do not have configurable upscale
#if VL6180x_WRAP_AROUND_FILTER_SUPPORT
    VL6180x_FilterSetState(myDev, 0); // turn off wrap around filter
#endif
    status = VL6180x_ClearAllInterrupt(myDev);
    if( status ){
        HandleError("VL6180x_ClearAllInterrupt  fail");
    }

    /* Ask user to place a black target at know RealTargetDistance  (100mm) */
    MyDev_UserWait(myDev, "Calibrating : place black target at  100mm from glass");
    status=VL6180x_WrWord(myDev, SYSRANGE_CROSSTALK_COMPENSATION_RATE, 0);

    for( i=0; i<N_MEASURE_AVG; i++){
        status = VL6180x_RangePollMeasurement(myDev, &Range[i]);
        if( status ){
            HandleError("VL6180x_RangePollMeasurement  fail");
        }
        if( Range[i].errorStatus != 0 ){
            HandleError("No target detect");
        }
    }
    RangeSum=0;
    RateSum=0;
    for( i=0; i<N_MEASURE_AVG; i++){
        RangeSum+= Range[i].range_mm;
        RateSum+= Range[i].signalRate_mcps;
    }
    /* Rate sum is 9.7 fixpoint so does xtlak computed below
     * The order of operation is important to have decent final precision without use of  floating point
     * using a higher real distance and number of point may lead to 32 bit integer overflow in formula below */
    XTalkInt = RateSum*(N_MEASURE_AVG*RealTargetDistance-RangeSum)/N_MEASURE_AVG /(RealTargetDistance*N_MEASURE_AVG);
    MyDev_Printf(myDev, "avg range %d rate %d comp=%d\n", RangeSum/N_MEASURE_AVG, RateSum/N_MEASURE_AVG, XTalkInt);
    return XTalkInt;
}


void Sample_XTalkCalibrate(void) {
    VL6180xDev_t myDev;
    VL6180x_RangeData_t Range;

    int XTalkRate;
    int i, status;

    /* init device */
    MyDev_Init(myDev);
    status = Sample_Init(myDev);
    if( status <0 ){
        HandleError("Sample_Init fail");
    }
    /* run calibration */
    XTalkRate = Sample_XTalkRunCalibration(myDev);

    /*TODO when possible reset re-init device else set back required scaling/filter*/
    VL6180x_FilterSetState(myDev, 1);  // turn on wrap around filter again
    VL6180x_UpscaleSetScaling(myDev, 3);  // set scaling
    /* apply cross talk */
    status = VL6180x_SetXTalkCompensationRate(myDev, XTalkRate);
    if( status<0 ){ /* ignore warning but not error */
        HandleError("VL6180x_WrWord fail");
    }

    for( i=0; i< 10; i++){
        status = VL6180x_RangePollMeasurement(myDev, &Range);
        MyDev_ShowRange(myDev, Range.range_mm);
    }
}
