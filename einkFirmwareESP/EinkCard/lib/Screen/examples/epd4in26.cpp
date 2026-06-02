/**
 *  @filename   :   epd4in26-demo.ino
 *  @brief      :   4.26inch e-paper display demo
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     23-12-20
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <Arduino.h>
#include <SPI.h>
#include "epd4in26.h"

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
    Epd epd;
    Serial.print("e-Paper init \r\n ");
    if (epd.Init() != 0) {
        Serial.print("e-Paper init failed\r\n ");
        return;
    }

    Serial.print("clear 1\r\n ");
    epd.Clear();
    
    Serial.print("Displayed\r\n ");
    // epd.Displaypart(IMAGE_DATA,250, 200,240,103);
    epd.SendCommand(0x24);
    for (int i = 0;i < (480*50);i++) {
      epd.SendData(0xF0);
    }
    epd.TurnOnDisplay();

    Serial.print("finish\r\n ");    
    epd.Sleep();
}

void loop() {
  // put your main code here, to run repeatedly:

}
