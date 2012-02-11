#!/usr/bin/env python

import serial
import io
from jenkinsapi.api import *
import time


class Lamp():
    def setColor(self, r, g, b):
        self._sendCommand('#'+chr(r)+chr(g)+chr(b))

    def blink(self):
        self._sendCommand('*')

    def _sendCommand(self, command):
        ser = serial.Serial('/dev/tty.usbmodem641', 9600, timeout=1)

        ready=done=0
        attempts=0
        while not done:
            print attempts
            attempts+=1
            ser.write(".");  # request for command
            ser.flush();
            line = ser.readline()
            ready = (line.strip() == "READY")

            if ready:
                print "< " + command
                ser.flush();
                ser.write(command)

                while not done:
                    print "> "+line
                    line=ser.readline()
                    done = (line.strip()=="DONE")

            if(attempts>100):
                return

            if(not done):
                time.sleep(1);

        ser.close()


class Jenkins():
    STATUS_RUNNING = 1
    STATUS_OK = 2
    STATUS_ERR = 3

    lamp = Lamp()

    def _statusRunning(self):
        print "set lamp to running"
        self.lamp.setColor(0xFF,0x01,0xFF);
        self.lamp.blink()

    def _statusOk(self):
        print "set lamp to ok"
        self.lamp.setColor(0xFF,0x01,0xFF);

    def _statusErr(self):
        print "set lamp to err"
        self.lamp.setColor(0xFF,0xFF,0x01); # rojo

    def _statusNotBuilt(self):
        print "set lamp to not built"
        self.lamp.setColor(0x33,0x00,0x01); # rojo


    def CheckJob(self, server, port, jobname, waitForCompletion = False):
        # self.lamp.setColor(0xFF,0x00,0x00); # rojo

        # args = ["-s", "http://%s:%s/hudson/" % (server, port), "build", jobname]

        jenkinsurl = "http://"+server+":"+port+"/"
        artifact_ids = [  ] # I need a build that contains all of these



        prevStatus = 0

        while(1):
            result = get_latest_build(jenkinsurl, jobname)


            if(result.is_running()):
                status="RUNNING"
            else:
                status = result.get_status()

            print "Jenkins -> " + status

            if status != prevStatus:
                try:
                    {
                        "SUCCESS": self._statusOk(),
                        "NOT_BUILT": self._statusNotBuilt(),
                        "ERROR": self._statusErr(),
                        "RUNNING": self._statusRunning()
                    }[status]()
                except:
                    self.lamp.setColor(0x00,0x00,0x00);
                    print "Jenkins -> Estado desconocido"

            prevStatus=status

            time.sleep(10);


        #
        #lamp.setColor(0xFF,0x00,0xFF);
        #lamp.setColor(0xFF,0xFF,0x00);
        #

        #lamp.blink()

if __name__ == "__main__":
    h = Jenkins()
    h.CheckJob("localhost", "8080", "ZgzBus", False)