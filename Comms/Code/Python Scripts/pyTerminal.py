import serial
import serial.tools.list_ports as port_list
import time

# list com ports for unit test of serial connection
ports = list(port_list.comports())
for p in ports:
    print (p)


thePort = input("Enter the port:  ")
#ser = serial.Serial('COM13',9600)
print (thePort)
ser = serial.Serial()
#ser.port = "/dev/ttyO2"
#ser.port = "COM13"
ser.port = thePort
ser.baudrate = 9600
ser.bytesize = serial.EIGHTBITS #number of bits per bytes
ser.parity = serial.PARITY_NONE #set parity check: no parity
ser.stopbits = serial.STOPBITS_ONE #number of stop bits
#ser.timeout = None          #block read
ser.timeout = 1            #non-block read
#ser.timeout = 2              #timeout block read
ser.xonxoff = False     #disable software flow control
ser.rtscts = False     #disable hardware (RTS/CTS) flow control
ser.dsrdtr = False       #disable hardware (DSR/DTR) flow control
ser.writeTimeout = 2     #timeout for write

try: 
    ser.open()
    print(thePort + " is now open.")
except Exception as e:
    print ("error open serial port: " + str(e))
    exit()

if ser.isOpen():

    try:
        ser.flushInput() #flush input buffer, discarding all its contents
        ser.flushOutput()#flush output buffer, aborting current output 
                 #and discard all that is in buffer
        stringy = input("$ ")
        #write data
        ser.write(stringy.encode())

        time.sleep(0.2)  #give the serial port sometime to receive the data

        numOfLines = 0

        while True:
           response = ser.readline()
           print(response)
           numOfLines = numOfLines + 1
           if (numOfLines >= 5):
             break
        ser.close()
    except Exception as e1:
        print ("error communicating...: " + str(e1))

else:
    print ("cannot open serial port.")


