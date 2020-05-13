//typedef unsigned char BYTE;

#include <sys/socket.h>
#include <string>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <fstream>
#include <ctime>
#include <sys/time.h>
#include <stdint.h>
#include <string>
#include <cstring>

#define BYTE u_char

//*********************************************************
//used for real time data acquisition
	struct SonarSwitches{
		BYTE Start1;		//Byte0, always 0xFE  
		BYTE Start2;		//Byte1, always 0x44
		BYTE Head_ID;		//Byte2, 0x0
		BYTE Range;			//Byte3, Operating Range 
		BYTE RangeOffset;	//Byte4, 
		BYTE HOLD;			//Byte5, 
		BYTE MA_SL;			//Byte6, 
		BYTE Freq;			//Byte7, [0=low, 1=med, 2=high]
		BYTE StartGain;		//Byte8, 0=40dB
		BYTE LOGF;			//Byte9, Reserved
		BYTE Absorption;	//Byte10, Reserved
		BYTE PulseLength;	//Byte11, Reserved
		BYTE Delay;			//Byte12, Reserved
		BYTE Resvd0;		//Byte13, Reserved
		BYTE PortGain;		//Byte14, Reserved
		BYTE StbdGain;		//Byte15, Reserved
		BYTE Resvd1;		//Byte16, Reserved, always 0
		BYTE Resvd2;		//Byte17, Reserved
		BYTE Pack_Number;	//Byte18, packet number? {0x00=Port, 0x02=Starboard}
		BYTE Resvd4;		//Byte19, Reserved
		BYTE Resvd5;		//Byte20, Reserved
		BYTE Resvd6;		//Byte21, Reserved
		BYTE Resvd7;		//Byte22, Reserved
		BYTE Resvd8;		//Byte23, Reserved
		BYTE Resvd9;		//Byte24, Reserved
		BYTE Resvd10;		//Byte25, Reserved
		BYTE Term;			//Byte26, 0xFD (253 dec) ALWAYS!
};

SonarSwitches sonarSwitches;

BYTE receiveBuf[1024], dataBuf[2046], headerBuf[12];
int ping_number,m_nRepRate,m_nLastTime;
std::string m_strDate, m_strTime;
int m_nRange, m_nRangeIndex;//match YellowFin style
double m_dAUVSpeed;//in knots

//void WriteOnePingData(fstream& output);
int GetRangeIndex(int nRange);
