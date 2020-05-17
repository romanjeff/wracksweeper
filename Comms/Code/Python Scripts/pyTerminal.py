import serial
import time
import os
import subprocess
from systemd.daemon import notify, Notification



ser = serial.Serial()
thePort = "/dev/ttyO2"
#thePort = "COM7"
cwd = os.getcwd()
outLog = cwd + "/loraIO.txt"
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
                                ser.write(line.encode())
                                time.sleep(0.01)
                                ack = getAck()
		                while(ack<1):
			            time.sleep(0.01)
			            ack = getAck()
		    os.remove(outLog)
                numBytes = ser.in_waiting
                if(numBytes>0):
                    chunk = ser.read(numBytes)
                    key = chunk[0].encode('ascii')
                    if (key =="-"):
			print('not a system call.')
                        pass
                    else:
			os.system('touch ' + outLog)
			chunk = chunk.replace('\n','').replace('\r','')
			with open(outLog,'w+') as outFile:
                            subprocess.call([chunk],shell=True,stdout=outFile,stderr=outFile)
                time.sleep(0.01)
		notify(Notification.WATCHDOG)
            ser.close()
        except Exception as e1:
            print ("error communicating...: " + str(e1))
    
    else:
        print ("cannot open serial port.")
    
if (__name__ == '__main__'):
    main()
