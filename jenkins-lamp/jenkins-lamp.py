#!/usr/bin/env python

import serial
import io
import jenkinsapi
import time


class Lamp():
    def setColor(self, r, g, b):
        ser = serial.Serial('/dev/tty.usbmodem641', 9600, timeout=1)

        ready=0
        attempts=0
        while not ready:
            print attempts
            attempts+=1
            ser.write('.');
            ser.flush();
            line = ser.readline()
            ready = (line.strip() == "READY")

            if(attempts>100):
                return

            if(not ready):
                time.sleep(1);

        if ready:
            print "change color"
            command='#'+chr(r)+chr(g)+chr(b)+"\n"
            print command
            ser.write(command)

            line = ''
            while(line.strip() != "DONE"):
                line=ser.readline()
                print "> "+line


            time.sleep(3);


        ser.close()



    def blink(self):
        print 'blink'
        ser = serial.Serial('/dev/tty.usbmodem641', 9600, timeout=0)


        ready=0
        attempts=0
        while not ready:
            print attempts
            attempts+=1

            ser.write('.');
            ser.flush();
            line = ser.readline()

            ready = (line.strip() == "READY")

            if(attempts>100):
                return

            if(not ready):
                time.sleep(3);

        if ready:
            print 'READY'
            ser.write('*')
            ser.flush()
            time.sleep(1);

        ser.close()

class Jenkins():
    def CheckJob(self, server, port, jobname, waitForCompletion = False):
        # args = ["-s", "http://%s:%s/hudson/" % (server, port), "build", jobname]
        lamp = Lamp()
        lamp.setColor(0xFF,0x00,0x00);
        lamp.setColor(0xFF,0x00,0xFF);
        lamp.setColor(0xFF,0xFF,0x00);
        lamp.setColor(0X00,0xFF,0xFF); # verde

        #lamp.blink()



if __name__ == "__main__":
    h = Jenkins()
    h.CheckJob("localhost", "8080", "ZgzBus", False)