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
    if(numBytes<3):
        return 0
    else:
        chunk = ser.read(numBytes)
        chunk = str(chunk).split()
        return chunk[0]

def main():
    escFlag = False
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
                            if escFlag == True:
                                ser.write(line.encode())
                                break
                            if line[0:2] == '\n':
                                pass
                            else:
                                ser.write(line.encode())
                                ack = getAck()
                                if (ack == 'esc'):
                                    escFlag = True
                                t = time.time()    #try each line at most five seconds
                                while((ack != '...') and (t > (time.time()-5))):
                                    ack = getAck
                                    if (ack == 'esc'):
                                        escFlag = True
                                        break
                        os.remove(outLog)
                        escFlag = False
                numBytes = ser.in_waiting
                if(numBytes>0):
                    chunk = ser.read(numBytes)
                    chunk = str(chunk).split()
                    key = chunk[0]
        	    if (key =="-"):
                     	pass
        	    elif (key == '...'):
        		pass
                    else:
                        os.system('touch ' + outLog)
                        with open(outLog,'w+') as outFile:
                                try:
				    subprocess.call(chunk,
                                                    shell=False,
                                                    stdout=outFile,
                                                    stderr=outFile)
				except Exception as e3:
				    ser.write(("Couldn't execute. " + str(e3) + "\n").encode())
                        time.sleep(0.005)
        except Exception as e1:
            print ("error communicating...: " + str(e1))

    else:
        print ("cannot open serial port.")

if (__name__ == '__main__'):
    main()
