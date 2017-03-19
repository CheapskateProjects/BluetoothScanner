/*
  Bluetooth scanner

  Scanner which lists nearby bluetooth devices and sends the JSON formatted list over serial connection.

  This scanner should be used with AWS gateway ( https://github.com/CheapskateProjects/ESP8266AWSIoTGateway ) to automatically send results to IoT cloud. 

  Serial data example: 
  Bluetooth1;{"state":{"reported":{"devices":["2845:98:680782","1D:D8:96578A"]}}}
  
  created   Mar 2017
  by CheapskateProjects

  ---------------------------
  The MIT License (MIT)

  Copyright (c) 2017 CheapskateProjects

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <SoftwareSerial.h>
SoftwareSerial BTserial(2, 3); // RX | TX

char c = ' ';

void delayAndRead()
{
  delay(50);
  while(BTserial.available())
  {
    c = BTserial.read();
  }
  delay(800);
}

void initHC05ToInq()
{
    BTserial.println("AT+CMODE=1");// Enable connect to any device
    delayAndRead();
    BTserial.println("AT+ROLE=1");// Set to master in order to enable scanning
    delayAndRead();
    BTserial.println("AT+INQM=1,10,24");//RSSI, Max 10 devices, ~30s
    delayAndRead();
    BTserial.println("AT+CLASS=0");// Disable COD filter
    delayAndRead();
    BTserial.println("AT+INIT");// Init.
    delayAndRead();
}

void setup() 
{
    Serial.begin(115200);
 
    // HC-05 default serial speed for AT mode is 38400
    BTserial.begin(38400);  

    // Wait for hardware to initialize
    delay(1000);

    // Set correct states for inq
    initHC05ToInq();

    // Start inq
    initMessage();
    BTserial.println("AT+INQ");
}

char lineBuffer[100];
char subBuffer[30];
int index = 0;
int index2 = 0;
int total = 0;
bool capture = false;
String send = "";

void initMessage()
{
  send = "Bluetooth1;{\"state\":{\"reported\":{\"devices\":[";
}

void loop()
{   
    // Keep reading from HC-05 and send to Arduino Serial Monitor
    if (BTserial.available())
    {  
        // Read character and append into buffer
        c = BTserial.read();
        lineBuffer[index] = c;
        index++;

        // When line ends
        if(c=='\n')
        {
          // Remove line end characters from buffer
          lineBuffer[index-1]=0;// \r\n

          // Reset buffer index for next line
          index = 0;

          if(lineBuffer[0] == 'O' && lineBuffer[1] == 'K')
          {
            // Finish message
            send += "]}}}";
            // DEBUG / TODO actually send this message
            if(total > 0)
            {
              Serial.println(send);
            }
            // Restart INQ
            BTserial.println("AT+INQ");
            total = 0;
            initMessage();
          }
          else
          {
            capture = false;
            index2 = 0;
            for(index = 0; index < 30; index++)
            {
              if(!capture)
              {
                if(lineBuffer[index]==':')
                {
                  capture = true;
                }
              }
              else
              {
                subBuffer[index2] = lineBuffer[index];
                if(lineBuffer[index] == ',')
                {
                  subBuffer[index2] = 0;
                  break;
                }
                index2++;
              }
            }
            index = 0;

            // Add this line buffer
            String str((char*)subBuffer);

            if(send.indexOf(str) <= 0)
            {
              // If not first then add comma
              if(total > 0)
              {
                send += ",";
              } 
            
              send += "\"";
              send += str;
              send += "\"";
              // Keep count
              total++;
            }
          }
        }
    }
}
