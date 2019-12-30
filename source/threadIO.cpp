/* 
 * Copyright (c) 2019 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "threadIO.h"
#include "Adafruit_ADS1015.h"
#include "MCP23017.h"
#include "TextLCD.h"
#include "globalVars.h"
#include "Rotor.h"

#define STACKSIZE   (2 * 1024)
#define THREADNAME  "ThreadIO"

ThreadIO::ThreadIO(uint32_t cycleTime_ms) :
    _thread(osPriorityNormal, STACKSIZE, nullptr, THREADNAME)
{
    _cycleTime = cycleTime_ms;
}

/*
    start() : starts the thread
*/
void ThreadIO::start()
{
    _running = true;
    _thread.start( callback(this, &ThreadIO::myThreadFn) );
}


/*
    start() : starts the thread
*/
void ThreadIO::myThreadFn()
{
    // thread local objects
    // take care of thread stacksize !
    DigitalOut led1(LED1);
    I2C i2c(I2C_SDA, I2C_SCL);
    Adafruit_ADS1115 ads(&i2c);

    while(_running) {
        uint64_t nextTime = get_ms_count() + _cycleTime;

        rotor1.process();

        led1 = !led1;
        
        for (uint i = 0; i < sizeof(globalVars.adcValues)/sizeof(globalVars.adcValues[0]); i++) {
            globalVars.adcValues[i] = ads.readADC_SingleEnded_V(i) * 1000.0f; // read channel 0 [mV]
        }

        globalVars.rotorPosActual += 0.3f;
        if (globalVars.rotorPosActual >= 360.0f)
            globalVars.rotorPosActual = 0.0f;
        


        ThisThread::sleep_until(nextTime);
    }
}
