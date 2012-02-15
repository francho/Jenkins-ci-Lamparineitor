#!/usr/bin/env python

import serial
import io
from jenkinsapi.api import *
import time


class Lamp():
    def setColor(self, r, g, b):
        self._sendCommand('#'+chr(r)+chr(g)+chr(b)+";")

    def blink(self):
        #self._sendCommand("blink;")
        a=1

    def _sendCommand(self, command):
        try:
            ser = serial.Serial('/dev/tty.usbmodem641', 9600, timeout=1)


            ready=done=0
            attemptsWrite=0
            while ( (not done) and (attemptsWrite < 10) ):
                attemptsWrite+=1

                line = ser.readline()
                ready = (line.strip() == "READY")

                if ready:
                    print "SEND("+ str(attemptsWrite) +"): "+command
                    ser.flush();
                    ser.write(command)

                    attemptsRead=0;
                    while (not done) and (attemptsRead<10):
                        line=ser.readline()
                        if(line==""):
                            attemptsRead+=1;
                        print "READING("+ str(attemptsRead)+"): "+line
                        done = (line.strip()=="DONE")

                else:
                    ser.write(";");  # request for command
                    ser.flush();

                if(not done):
                    time.sleep(1);
        except Exception as e:
            print "ERROR: "+e.message
        finally:
            time.sleep(5);
            ser.close()


class Jenkins():
    STATUS_RUNNING = 1
    STATUS_OK = 2
    STATUS_ERR = 3

    lamp = Lamp()

    def _statusRunning(self):
        print "set lamp to running"
        self.lamp.setColor(0x01,0x33,0xFE);
        self.lamp.blink()

    def _statusOk(self):
        print "set lamp to ok"
        self.lamp.setColor(0x01,0xFE,0x01);

    def _statusAborted(self):
        print "set lamp to aborted"
        self.lamp.setColor(0xAA,0x99,0x33);

    def _statusErr(self):
        print "set lamp to err"
        self.lamp.setColor(0xFE,0x01,0x01); # rojo

    def _statusUnstable(self):
        print "set lamp to unstable"
        self.lamp.setColor(0x33,0x01,0x01); # rojo

    def _statusNotBuilt(self):
        print "set lamp to not built"
        self.lamp.setColor(0x01,0x01,0xFE); # rojo

    def CheckJob(self, server, port, jobname, waitForCompletion = False):
        jenkinsurl = "http://"+server+":"+port+"/"

        prevStatus = 0

        while(1):
            result = get_latest_build(jenkinsurl, jobname)


            if(result.is_running()):
                status="RUNNING"
            else:
                status = result.get_status()

            print "Jenkins -> " + status

            if status != prevStatus:
                prevStatus=status

            self._setStatus(status);


            time.sleep(10);

    def _setStatus(self, status):
        if(status == "SUCCESS"):
            self._statusOk();
        else:
            if(status == "NOT_BUILT"):
                self._statusNotBuilt();
            else:
                if(status=="ERROR"):
                    self._statusErr();
                else:
                    if(status=="RUNNING"):
                        self._statusRunning()
                    else:
                        if(status=="ABORTED"):
                            self._statusAborted()
                        else:
                            if(status=="UNSTABLE"):
                                self._statusUnstable()
                            else:
                                print "Jenkins -> Estado desconocido"


if __name__ == "__main__":
    h = Jenkins()

    h.CheckJob("localhost", "8080", "ZgzBus", False)