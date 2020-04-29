# wracksweeper
In which we outfit a  UUV running MOOS-iVP with Emergency Beacon, Sidescan Sonar, and other goodies.

Our AUV is a Riptide Systems MP1 Mk.II, managed by a Beaglebone Black running MOOS-IvP (http://www.moos-ivp.org).

The project goal is to have full side-scan sonar capabilities, an emergency asset recovery tracking beacon, 
rechargeable power supply, and long-range wireless communications allowing us to check on the AUV's status,
give it new mission parameters, or toggle sonar functions without recovery and relaunch. Added features are
modular and removable to restore original state to the AUV.

Our sonar system is an Imagenex OEM side-scan module, and our wireless communications are done over LoRa at 
915 MHz using the Adafruit Feather M0 with HopeRF RFM95 transceiver modules. 

This project has been conducted at Portland State University by a team consisting of:

Drew Wendebrn, Matthew Moore, Patrick Hawn - Mechanical Engineering.  

Roman Minko, Adel Alkharraz - Sonar Interface Software.  

Colton Bruce - Beacon and RF link characterization.  

Jeff Roman - Communications Software.  

Jens Evens, Grace Semerdjian - Power System.  

http://www.wracksweeper.com
