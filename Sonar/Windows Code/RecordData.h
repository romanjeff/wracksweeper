
#if !defined(AFX_RECORDDATA_H__7B6CB2DA_8C12_4586_8F9C_01D20788D3B8__INCLUDED_)
#define AFX_RECORDDATA_H__7B6CB2DA_8C12_4586_8F9C_01D20788D3B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "RecordDataConfig.h"

//*********************************************************
//used for real time data acquisition
	struct SonarSwitches{
	BYTE Start1;		//Byte0, always 0xFE  
	BYTE Start2;		//Byte1, always 0x44
	BYTE Head_ID;		//Byte2, 0x10
	BYTE Range;			//Byte3, 
	BYTE RangeOffset;	//Byte4, 
	BYTE HOLD;			//Byte5, 
	BYTE MA_SL;			//Byte6, 
	BYTE Freq;			//Byte7,
	BYTE StartGain;		//Byte8, 
	BYTE LOGF;			//Byte9, 
	BYTE Absorption;	//Byte10, 
	BYTE PulseLength;	//Byte11, 
	BYTE Delay;			//Byte12, 
	BYTE Resvd0;		//Byte13, 
	BYTE PortGain;		//Byte14, 
	BYTE StbdGain;		//Byte15, 
	BYTE Resvd1;		//Byte16, Researved, always 0
	BYTE Resvd2;		//Byte17, 
	BYTE Pack_Number;	//Byte18, packet number?
	BYTE Resvd4;		//Byte19, 
	BYTE Resvd5;		//Byte20, 
	BYTE Resvd6;		//Byte21, 
	BYTE Resvd7;		//Byte22, 
	BYTE Resvd8;		//Byte23, Researved
	BYTE Resvd9;		//Byte24, 
	BYTE Resvd10;		//Byte25, Researved
	BYTE Term;			//Byte26, 0xFD
};

	SonarSwitches sonarSwitches;

	CFile m_wFile;
	BYTE m_ReceiveBuf[1024], m_DataBuf[2046], m_HeaderBuf[12];
	int ping_number,m_nRepRate,m_nLastTime;
	SYSTEMTIME m_UnivSysTime;
	CString m_strDate, m_strTime;
	int m_nRange, m_nRangeIndex;//match YellowFin style
	double m_dAUVSpeed;//in knots

	void WriteOnePingData();
	int GetRangeIndex(int nRange);

#endif // !defined(AFX_RECORDDATA_H__7B6CB2DA_8C12_4586_8F9C_01D20788D3B8__INCLUDED_)
