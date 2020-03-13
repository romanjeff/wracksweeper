#!/usr/bin/env python3

""" This program asks a client for data and waits for the response, then sends an ACK. """

# Copyright 2018 Rui Silva.
#
# This file is part of rpsreal/pySX127x, fork of mayeranalytics/pySX127x.
#
# pySX127x is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
# License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# pySX127x is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
# details.
#
# You can be released from the requirements of the license by obtaining a commercial license. Such a license is
# mandatory as soon as you develop commercial activities involving pySX127x without disclosing the source code of your
# own applications, or shipping pySX127x with a closed source product.
#
# You should have received a copy of the GNU General Public License along with pySX127.  If not, see
# <http://www.gnu.org/licenses/>.

import os
import pandas as pd
import sys
import csv
import time
from SX127x.LoRa import *
from SX127x.board_config import BOARD
from os.path import join

sensorName=str(sys.argv[1])
argLen = len(sys.argv)
#print(argLen)
BOARD.setup()
BOARD.reset()


class mylora(LoRa):
    def __init__(self, verbose=False):
        super(mylora, self).__init__(verbose)
        self.set_mode(MODE.SLEEP)
        self.set_dio_mapping([0] * 6)
        self.var = 0

    def on_rx_done(self):
        BOARD.led_on()
        wrkdir = '/home/pi/Desktop/'
        # print("\nRxDone")
        self.clear_irq_flags(RxDone=1)
        payload = self.read_payload(nocheck=True)
        print("Receive: ")
        dataRaw = bytes(payload).decode("utf-8", 'ignore')
        dataRec = dataRaw[2:-1].split(",")
        print(dataRec)  # Receive DATA
       # print(argLen)
        if argLen is 2:
            filename = sensorName + ".csv"
            filename = join('/home/pi/Desktop/',filename)
            with open(filename, "a+") as csvfile:
                wr = csv.writer(csvfile, quoting = csv.QUOTE_ALL)
                wr.writerow(dataRec)
            csvfile.closed
        BOARD.led_off()
        time.sleep(2)  # Wait for the client be ready
        print("Send: ACK")
        self.write_payload([255, 255, 0, 0, 65, 67, 75, 0])  # Send ACK
        self.set_mode(MODE.TX)
        self.var = 1
        time.sleep(.5)  # a tad to complete
        print("Data Sent")
        sys.stdout.flush()
        lora.set_mode(MODE.SLEEP)
        BOARD.teardown()
        sys.exit()

    def start(self):
        while True:
            self.tOut = 0
            while (self.var == 0 and self.tOut < 3):
                print("Send: "+ sensorName)
                breakName = list(sensorName)
                breakName = [ord(ch) for ch in breakName]
                #print(breakName)
                sendName = [255,255,0,0]+breakName+[0]
                #print(sendName)
                self.write_payload(sendName)  # Send SM01
                self.set_mode(MODE.TX)
                # there must be a better solution but sleep() works
                time.sleep(2)
                self.reset_ptr_rx()
                self.set_mode(MODE.RXCONT)  # Receiver mode

                start_time = time.time()
                while (time.time() - start_time < 10):  # wait until receive data or 10s
                    pass
                self.tOut += 1
            if self.tOut is 3:
                print("Can't connect to " + sensorName)
                sys.stdout.flush()
                lora.set_mode(MODE.SLEEP)
                #BOARD.teardown()
                sys.exit()
            self.var = 0
            self.reset_ptr_rx()
            self.set_mode(MODE.RXCONT)  # Receiver mode
            time.sleep(10)

lora = mylora(verbose=True)

#     Slow+long range  Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on. 13 dBm
lora.set_pa_config(pa_select=1)
lora.set_freq(915)
lora.set_bw(BW.BW125)
lora.set_coding_rate(CODING_RATE.CR4_8)
lora.set_spreading_factor(12)
lora.set_rx_crc(True)
lora.set_lna_gain(GAIN.G1)
lora.set_implicit_header_mode(False)
lora.set_low_data_rate_optim(True)


try:
    print("START")
    lora.start()
except KeyboardInterrupt:
    sys.stdout.flush()
    print("Exit")
    sys.stderr.write("KeyboardInterrupt\n")
finally:
    sys.stdout.flush()
    print("Exit")
    lora.set_mode(MODE.SLEEP)
    BOARD.teardown()
