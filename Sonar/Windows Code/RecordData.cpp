// RecordData.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "pch.h"
#include "framework.h"
#include "afxsock.h"

#include "RecordData.h"

#include <conio.h>
#include <ctype.h>

#ifdef _DEBUG
#define new DEBUG_NEW

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define m_numDataPoints 1000
//#define m_nDataPointsPerChannel 500


#define m_numDataPoints 2000
#define m_nDataPointsPerChannel 1000
#define m_nFileRecordSize 4096
#define m_nGPSDataOffset 3000

const int IndexToRange[] = {1,2,3,4,5,10,20,30,40,50,60,80,100,125,150,200};
/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

CSocket socketClient;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	} else	{
		// Initialize WinSocket
		if (!AfxSocketInit())
		{
			cerr << _T("Socket initialization failed") << endl;
			return 1;
		}

		CString strPath, filename;
		char input[256];
		int index=0, i=0;

		strPath=argv[0];
		index=strPath.ReverseFind('\\');
		strPath=strPath.Left(index+1);

		if(argc==1)
		{
			for(i=0;i<256;i++)input[i]=0;
			input[0]='\0';
			while(input[0]=='\0')
			{
				cout << "Input a file name:" ;
				cin >> input ;
			}
			filename=input;
		}
		else if(argc==2)
			filename=argv[1];

		filename=strPath+filename+".872";	

		m_wFile.m_hFile=NULL;

		if (!m_wFile.Open(filename, CFile::modeCreate |CFile::modeWrite|CFile::shareDenyNone, NULL))
		{
			cerr << _T("File creation failed") << endl;
			m_wFile.m_hFile=NULL;
			return 1;

		}
	//******************************************************************
	m_dAUVSpeed =0.;//if there is the speed info, feed in

	m_nRange=50;
	m_nRangeIndex = 9;
	sonarSwitches.Start1=0xFE;		//Byte0, always 0xFE  
	sonarSwitches.Start2=0x44;		//Byte1, always 0x44
	sonarSwitches.Head_ID=0;		//Byte2, 0x00
	sonarSwitches.Range = m_nRange;	//Byte3,  
	sonarSwitches.RangeOffset =0;	//Byte4, 0
	sonarSwitches.HOLD =0;			//Byte5, not used
	sonarSwitches.MA_SL =0x00;		//Byte6, 
	sonarSwitches.Freq =0;			//Byte7, 0: low, 1: medium 2:high
	sonarSwitches.StartGain = 20;	//Byte8, 
	sonarSwitches.LOGF = 0;			//Byte9, 
	sonarSwitches.Absorption=0;		//Byte10, 
	sonarSwitches.PulseLength=0;	//Byte11, 
	sonarSwitches.Delay=0;			//Byte12, 
	sonarSwitches.Resvd0 =0x00;		//Byte13,
	sonarSwitches.PortGain =0x00;	//Byte14,
	sonarSwitches.StbdGain =0x00;	//Byte15,
	sonarSwitches.Resvd1 =0;		//Byte16, Researved, always 0
	sonarSwitches.Resvd2 =0;		//Byte17, Researved, always 0
	sonarSwitches.Pack_Number =0;	//Byte18, packet number?
	sonarSwitches.Resvd4 =0;		//Byte19, Researved, always 0
	sonarSwitches.Resvd5 =0;		//Byte20, 
	sonarSwitches.Resvd6 =0;		//Byte21, 
	sonarSwitches.Resvd7 =0;		//Byte22, 
	sonarSwitches.Resvd8 =0;		//Byte23, Researved
	sonarSwitches.Resvd9 =0;		//Byte24, Researved
	sonarSwitches.Resvd10 =0;		//Byte25, 
	sonarSwitches.Term =0xFD;		//Byte26, 0xFD
//******************************************************************

	CString CfgFilename=strPath+"settings.cfg";	
	GetPrivateProfileString("Settings", "Range","50", input, sizeof(input), CfgFilename);
	sonarSwitches.Range=atoi(input);

	if(sonarSwitches.Range<10)sonarSwitches.Range=10;
	if(sonarSwitches.Range>200)sonarSwitches.Range=200;

	GetPrivateProfileString("Settings", "Frequency","0", input, sizeof(input), CfgFilename);
	sonarSwitches.Freq =atoi(input);
	if(sonarSwitches.Freq<0)sonarSwitches.Freq=0;
	if(sonarSwitches.Freq>2)sonarSwitches.Freq=2;
	if(sonarSwitches.Freq==2 && sonarSwitches.Range>50)//high freq limit range to 50m
		sonarSwitches.Range=50;

	GetPrivateProfileString("Settings", "Start Gain","20", input, sizeof(input), CfgFilename);
	sonarSwitches.StartGain =atoi(input);
	if(sonarSwitches.StartGain<0)sonarSwitches.StartGain=0;
	if(sonarSwitches.StartGain>40)sonarSwitches.StartGain=40;//limit 0-40dB

	m_nRange = sonarSwitches.Range;
	m_nRangeIndex = GetRangeIndex(m_nRange);

	//******************************************************************
	ping_number=0;

	index=0;
	index=socketClient.Create();
	if(!socketClient.Connect("192.168.0.5",4040))
	{
		index=GetLastError();
		AfxMessageBox("Connection Failed!");
	}
	else
	{
		index=socketClient.Send((void *)&(sonarSwitches), 27, 0);
		while(1)
		{
			//Sleep(300);

			//******************************************************************
			//******************************************************************
			index=socketClient.Receive((void *)&(m_ReceiveBuf), 1013, 0);

			if(index>=1013)
			{

				//changes made (Oct-16-2007) need to have two packets to get 2000 bytes
				//1000 bytes/channel
				if(sonarSwitches.Pack_Number ==0)
				{//side scan

					for (i = 0; i< 1000 ; i++)//flip the port side data 1000 bytes
						m_DataBuf[999-i]=m_ReceiveBuf[i+12];
				}
				if(sonarSwitches.Pack_Number==2 )
				{
					//get the starboard side data
					for (i = 0; i< 12; i++)		
						m_HeaderBuf[i]= m_ReceiveBuf[i];
					for (i = 0; i< 1001 ; i++)
						m_DataBuf[i+1000]=m_ReceiveBuf[i+12];

					//write one ping data to file
					//*********************************
					WriteOnePingData();
					
				}

				//******************************************************************
				//need change packet number here before send command
				if(sonarSwitches.Pack_Number ==0)sonarSwitches.Pack_Number =2;
				else if(sonarSwitches.Pack_Number ==2)sonarSwitches.Pack_Number =0;
				index=socketClient.Send((void *)&(sonarSwitches), 27, 0);
			}
		}
	}//connected OK

////////////////////////////////////////////////
	if(m_wFile.m_hFile)m_wFile.Close();

	}//with MFC OK

	return nRetCode;
}

void WriteOnePingData()
{
	BYTE Buf[1001];
	int nBytes, index, i;
	index=0;
	for(i=0;i<1001;i++)Buf[i]=0;//init to all zeros
	
	// first 1000 bytes: ping header
	Buf[index++] = '8';
	Buf[index++] = '7';
	Buf[index++] = '2';
				
	Buf[index++] = 0;//version: 0-current version 1.xx, 1-later version
				
	ping_number++;  //index=4;
	Buf[index++] = (BYTE)((ping_number&0xFF000000)>>24);
	Buf[index++] = (BYTE)((ping_number&0x00FF0000)>>16);
	Buf[index++] = (BYTE)((ping_number&0x0000FF00)>>8);
	Buf[index++] = (BYTE)((ping_number&0x000000FF)); 	/* PingNumber */


	nBytes = m_nFileRecordSize; //total bytes for this record, index=8,9
	Buf[index++] = (BYTE)(((nBytes)&0xFF00)>>8);
	Buf[index++] = (BYTE)((nBytes)&0x00FF);

	nBytes = m_nDataPointsPerChannel; //data points per channel, index=10,11
	Buf[index++] = (BYTE)(((nBytes)&0xFF00)>>8);
	Buf[index++] = (BYTE)((nBytes)&0x00FF);

	Buf[index++] = 1;//Bytes per data point 12
	Buf[index++] = 8;//data point bit depth 13
	Buf[index++] = 0;//(BYTE)(((gps_type&0x03)<<4)|0x04);//GPS strings type 14

	nBytes = m_nGPSDataOffset; //GPS string data start address offset 15-16
	Buf[index++] = (BYTE)(((nBytes)&0xFF00)>>8);
	Buf[index++] = (BYTE)((nBytes)&0x00FF);

	//Event
	nBytes=0; //Event and annotation 17-18
	Buf[index++] = (BYTE)(((nBytes)&0xFF00)>>8);
	Buf[index++] = (BYTE)((nBytes)&0x00FF);
	//get date and time
	GetLocalTime(&m_UnivSysTime);
			
	int tempmonth = m_UnivSysTime.wMonth;
	CString strMonth;
	switch(tempmonth) {
		case (1)  : strMonth = "JAN"; break;
		case (2)  : strMonth = "FEB"; break;
		case (3)  : strMonth = "MAR"; break;
		case (4)  : strMonth = "APR"; break;
		case (5)  : strMonth = "MAY"; break;
		case (6)  : strMonth = "JUN"; break;
		case (7)  : strMonth = "JUL"; break;
		case (8)  : strMonth = "AUG"; break;
		case (9)  : strMonth = "SEP"; break;
		case (10) : strMonth = "OCT"; break;
		case (11) : strMonth = "NOV"; break;
		case (12) : strMonth = "DEC"; break;
	}

	//CString strDate, strTime;
	m_strTime.Format ("%02d:%02d:%02d",m_UnivSysTime.wHour,m_UnivSysTime.wMinute,m_UnivSysTime.wSecond);
	m_strDate.Format ("%02d-%s-%04d",m_UnivSysTime.wDay,strMonth,m_UnivSysTime.wYear);


	for(i=0;i<11;i++) Buf[index++] = m_strDate.GetAt(i);//m_disk_date[i];Date DD-MMM-YYYY
	Buf[index++] = '\0';
	for(i=0;i<8;i++) Buf[index++] = m_strTime.GetAt(i);//m_disk_time[i];time HH:MM:SS
	Buf[index++] = '\0';
				
	UINT temp = (UINT)(m_UnivSysTime.wMilliseconds);
	sprintf((char *)&Buf[index],".%03d",temp);

	index=45;
	Buf[index++] = sonarSwitches.Freq;
	Buf[index++] = m_nRangeIndex;
	Buf[index++] = sonarSwitches.StartGain;
	Buf[index++] = 0;//in order to match with YellowFin

	Buf[index++] = (BYTE)(((m_nRepRate)&0xFF00)>>8);
	Buf[index++] = (BYTE)((m_nRepRate)&0x00FF);
	nBytes = 1500*10; //sound velocity
	Buf[index++] = (BYTE)(((nBytes)&0xFF00)>>8);
	Buf[index++] = (BYTE)((nBytes)&0x00FF);

	for (i = 0; i< 12; i++)		//start at index=53
		Buf[index+i]=m_HeaderBuf[i];//add sonar header 12 bytes

	index=70;
	Buf[index++] = 1;//AUV board
	Buf[index++] = m_nRange;

	temp = (UINT)(m_dAUVSpeed*100);
	Buf[index++] = (BYTE)((temp&0xFF00)>>8);
	Buf[index++] = (BYTE)(temp&0x00FF);


	if(m_wFile.m_hFile!=NULL)
	{
		m_wFile.Write(Buf,1000);
		m_wFile.Flush();
	}

	// write out the two channel data 1000 bytes and zero fill 1000 bytes
	if(m_wFile.m_hFile!=NULL)
		m_wFile.Write(m_DataBuf,m_numDataPoints);//now 2000 bytes (Oct-16-2007)

	//zero fill for GPS
	for(i=0;i<1001;i++)Buf[i]=0;//init to all zeros
	if(m_wFile.m_hFile!=NULL)
	{
		m_wFile.Write(Buf,1000);
		m_wFile.Flush();
	}

	//************** Zero fill ********************
	//	for(i=0;i<1001;i++)Buf[i]=0;//init to all zeros
	//4094-4095
	//pointer to a previous ping bytes of this ping + last ping
	nBytes = 4096+4096; 
	Buf[94] = (BYTE)(((nBytes)&0xFF00)>>8);
	Buf[95] = (BYTE)((nBytes)&0x00FF);

	if(m_wFile.m_hFile!=NULL)
	{
		m_wFile.Write(Buf,96);
		m_wFile.Flush();
	}

	m_nRepRate=abs((int) GetTickCount()-m_nLastTime);
	m_nLastTime=GetTickCount();

}
int GetRangeIndex(int nRange)
{
	int index=5;
	switch (nRange)
	{
	case 10: index=5; break;
	case 20: index=6;break;
	case 30: index=7;break;
	case 40: index=8;break;
	case 50: index=9;break;
	case 60: index=10;break;
	case 80: index=11;break;
	case 100: index=12;break;
	case 125: index=13;break;
	case 150: index=14;break;
	case 200: index=15;break;
	default: index=15;
	}

	return index;
}