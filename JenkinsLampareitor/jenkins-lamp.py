#!/usr/bin/env python

import serial
import io
from jenkinsapi.api import *
import time
import logging


class Lamp():


    def __init__(self):
        self.ser = serial.Serial('/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_74937303936351F00121-if00', 9600, timeout=1)

    def setColor(self, r, g, b):
        self._sendCommand('#'+chr(r)+chr(g)+chr(b)+";")

    def blink(self):
        self._sendCommand("blink;")

    def off(self):
        self._sendCommand("off;")

    def _sendCommand(self, command):
        try:
            ready=done=0
            attemptsWrite=0
            while ( (not done) and (attemptsWrite < 10) ):
                attemptsWrite+=1

                line = self.ser.readline()
                ready = (line.strip() == "READY")

                if ready:
                    print "SEND("+ str(attemptsWrite) +"): "+command
                    self.ser.flush()
                    self.ser.write(command)

                    attemptsRead=0;
                    while (not done) and (attemptsRead<10):
                        line=self.ser.readline()
                        if(line==""):
                            attemptsRead+=1
                        print "READING("+ str(attemptsRead)+"): "+line
                        done = (line.strip()=="DONE")

                else:
                    self.ser.write(";")  # request for command
                    self.ser.flush()

                if(not done):
                    time.sleep(1)
        except Exception as e:
            print "ERROR: "+e.message
        finally:
            time.sleep(5)



class Jenkins():
    STATUS_RUNNING = 1
    STATUS_OK = 2
    STATUS_ERR = 3

    TIME_LEDS_ON = 10 * 60;

    lamp = Lamp()

    lastChange = -1;

    def _statusRunning(self):
        print "set lamp to running"
        self.lamp.setColor(0x033,0x33,0x33)
        self.lamp.blink()

    def _statusOk(self):
        print "set lamp to ok"
        self.lamp.setColor(0x01,0xFE,0x33)

    def _statusAborted(self):
        print "set lamp to aborted"
        self.lamp.setColor(0xAA,0x99,0x33)

    def _statusErr(self):
        print "set lamp to err"
        self.lamp.setColor(0xFE,0x01,0x01)

    def _statusUnstable(self):
        print "set lamp to unstable"
        self.lamp.setColor(0x33,0x01,0x01)

    def _statusNotBuilt(self):
        print "set lamp to not built"
        self.lamp.setColor(0x01,0x01,0xFE)

    def CheckJob(self, server, port, jobname):
        jenkinsurl = "http://"+server+":"+port+"/"

        prevStatus = -1

        while(1):
            try:
                result = get_latest_build(jenkinsurl, jobname)

                if(result.is_running()):
                    status="RUNNING"
                else:
                    status = result.get_status()

                print "Jenkins -> " + status

                if status != prevStatus:
                    prevStatus=status
                    self._setStatus(status)

                remain = self.TIME_LEDS_ON - (time.time() - self.lastChange)
                print "Remain "+str(remain)
                
                if remain < 0:
                    self.lamp.off()
                    print "OFF";

                time.sleep(1)
            except Exception as err:
                print err;

    def _setStatus(self, status):
	self.lastChange = time.time();

        if(status == "SUCCESS"):
            self._statusOk();
        else:
            if(status == "NOT_BUILT"):
                self._statusNotBuilt();
            else:
                if(status=="ERROR" or status=="FAILURE"):
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
    logger = logging.getLogger("jenkinsapi.jenkinsbase")
    logger.setLevel(logging.DEBUG)
    #create console handler and set level to debug
    ch = logging.StreamHandler()
    ch.setLevel(logging.DEBUG)
    #create formatter
    formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
    #add formatter to ch
    ch.setFormatter(formatter)
    #add ch to logger
    logger.addHandler(ch)
    # logger.debug("debug message")
    

    j = Jenkins()
    l = Lamp()
    #l.setColor(0xff,0x10,0xff)
    #l.setColor(0xff,0xff,0x10)
    #l.setColor(0x10,0x10,0xff)


    j.CheckJob("192.168.4.20", "8080", "webapp-trunk")

    # h.CheckJob("jenkins.fch", "8080", "webapp-trunk")

