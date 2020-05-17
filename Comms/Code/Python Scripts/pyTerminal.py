import serial
import time
import os
import subprocess
import systemd.daemon as SD
import Adafruit_BBIO.UART as UART

 # SET THE THINGS UP

UART.setup("UART2")
thePort = "/dev/ttyO2"
cwd = os.getcwd()
outLog = cwd + "/loraIO.txt"

ser = serial.Serial()
ser.port = thePort
ser.baudrate = 9600
ser.bytesize = serial.EIGHTBITS #number of bits per bytes
ser.parity = serial.PARITY_NONE #set parity check: no parity
ser.stopbits = serial.STOPBITS_ONE #number of stop bits
ser.timeout = 1            #non-block read
ser.xonxoff = False     #disable software flow control
ser.rtscts = False     #disable hardware (RTS/CTS) flow control
ser.dsrdtr = False       #disable hardware (DSR/DTR) flow control
ser.writeTimeout = 1     #timeout for write

def initPort(thePort):
    try:
        ser.open()
        print(thePort + " is now open.")
    except Exception as e:
        print ("error open serial port: " + str(e))
        exit()

def getAck():
    numBytes = ser.in_waiting
    if(numBytes==0):
        return 0
    if(numBytes>0):
        chunk = ser.read(numBytes)
        chunk = chunk.replace('\n','').replace('\r','')
        if (str(chunk.split()[0]) != "..."):
            return 0
        else:
            return 1

def main():
    initPort(thePort)
    if ser.is_open:
        try:
            ser.flushInput() #flush input buffer, discarding all its contents
            ser.flushOutput() #flush output buffer, aborting current output
            SD.notify('READY=1')
            try:
                os.remove(outLog)
            except:
                pass
            while True:
                if os.path.exists(outLog):
                    with open(outLog,"r") as f:
                        lines = f.readlines()
                        for line in lines:
                            if line[0:2] == '\n':
                                pass
                            else:
                                SD.notify('WATCHDOG=1')
                                ser.write(line.encode())
                                time.sleep(0.01)
                                ack = getAck()
                                c = 1000    #try each line at most ten seconds
                                while((ack<1) and (c>0)):
                                    time.sleep(0.01)
                                    ack = getAck()
                                    c = c -1
                        os.remove(outLog)
                numBytes = ser.in_waiting
                if(numBytes>0):
                    chunk = ser.read(numBytes)
                    key = chunk[0].encode('ascii')
                    if (key =="-"):
                        pass
                    else:
                        os.system('touch ' + outLog)
                        chunk = chunk.replace('\n','').replace('\r','')
                        with open(outLog,'w+') as outFile:
                                subprocess.call([chunk],
                                                shell=True,
                                                stdout=outFile,
                                                stderr=outFile)
                        time.sleep(0.01)
                SD.notify("WATCHDOG=1")
        except Exception as e1:
            print ("error communicating...: " + str(e1))

    else:
        print ("cannot open serial port.")

if (__name__ == '__main__'):
    main()
