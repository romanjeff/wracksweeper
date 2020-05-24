// RecordData.cpp : Defines the entry point for the console application.
//

#include "RecordData.h"

#define PORT 4040

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define m_numDataPoints 2000
#define m_nDataPointsPerChannel 1000
#define m_nFileRecordSize 4096
#define m_nGPSDataOffset 3000

#define SIZE 50

#ifdef UNICODE
    typedef std::string CString;
#else
    typedef std::wstring CString;
#endif

const int IndexToRange[] = {1,2,3,4,5,10,20,30,40,50,60,80,100,125,150,200};
/////////////////////////////////////////////////////////////////////////////
// The one and only application object

using namespace std;

void WriteOnePingData(fstream& outfile);

int main(int argc, char * argv[], char * envp[])
{
    int nRetCode = 0;
    struct sockaddr_in serv_add;

        std::string strPath, filename;
        char input[256];
        int index=0, i=0;


        strPath=argv[0];
        index=strPath.rfind('\\');
        strPath=strPath.substr(index+1);

        if(argc==1)
        {
            for(i=0;i<256;i++)input[i]=0;
            input[0]='\0';
            while(input[0]=='\0')
            {
                cout << "Input a file name: " ;
                cin >> input ;
            }
            filename=input;
        }
        else if(argc==2)
            filename=argv[1];

        filename=filename+".872";

        fstream outfile;
        outfile.open(filename, ios::out/* | ios::binary*/);
	
	if(!outfile)
            cout << "Error, file not created" << endl;

    //******************************************************************
    m_dAUVSpeed =0.;//if there is the speed info, feed in

    m_nRange=50;
    m_nRangeIndex = 9;
    sonarSwitches.Start1 = 0xFE;        //Byte0, always 0xFE
    sonarSwitches.Start2 = 0x44;        //Byte1, always 0x44
    sonarSwitches.Head_ID = 0;        //Byte2, 0x00
    sonarSwitches.Range = m_nRange;    //Byte3,
    sonarSwitches.RangeOffset = 0;    //Byte4, 0
    sonarSwitches.HOLD = 0;            //Byte5, not used
    sonarSwitches.MA_SL = 0x00;        //Byte6,
    sonarSwitches.Freq = 0;            //Byte7, 0: low, 1: medium 2:high
    sonarSwitches.StartGain = 20;    //Byte8,
    sonarSwitches.LOGF = 0;            //Byte9,
    sonarSwitches.Absorption = 0;        //Byte10,
    sonarSwitches.PulseLength = 0;    //Byte11,
    sonarSwitches.Delay = 0;            //Byte12,
    sonarSwitches.Resvd0 = 0x00;        //Byte13,
    sonarSwitches.PortGain = 0x00;    //Byte14,
    sonarSwitches.StbdGain = 0x00;    //Byte15,
    sonarSwitches.Resvd1 = 0;        //Byte16, Researved, always 0
    sonarSwitches.Resvd2 = 0;        //Byte17, Researved, always 0
    sonarSwitches.Pack_Number = 0;    //Byte18, packet number?
    sonarSwitches.Resvd4 = 0;        //Byte19, Researved, always 0
    sonarSwitches.Resvd5 = 0;        //Byte20,
    sonarSwitches.Resvd6 = 0;        //Byte21,
    sonarSwitches.Resvd7 = 0;        //Byte22,
    sonarSwitches.Resvd8 = 0;        //Byte23, Researved
    sonarSwitches.Resvd9 = 0;        //Byte24, Researved
    sonarSwitches.Resvd10 = 0;        //Byte25,
    sonarSwitches.Term =0xFD;        //Byte26, 0xFD
//******************************************************************
 
// Testing purposes
/*	sonarSwitches.Range = 50;
	sonarSwitches.Freq = 1;
	sonarSwitches.StartGain = 20;*/

    cout << "\nEnter a range: ";
    cin >> sonarSwitches.Range;

    std::string CfgFilename=strPath+"settings.cfg";

    if(sonarSwitches.Range<10) sonarSwitches.Range=10;
    if(sonarSwitches.Range>200) sonarSwitches.Range=200;

    cout << "\nEnter a frequency: ";
    cin >> sonarSwitches.Freq;

    if(sonarSwitches.Freq<0)sonarSwitches.Freq=0;
    if(sonarSwitches.Freq>2)sonarSwitches.Freq=2;
    if(sonarSwitches.Freq==2 && sonarSwitches.Range>50)//high freq limit range to 50m
        sonarSwitches.Range=50;

    cout << "\nEnter a gain: ";
    cin >> sonarSwitches.StartGain;

    if(sonarSwitches.StartGain<0)sonarSwitches.StartGain=0;
    if(sonarSwitches.StartGain>40)sonarSwitches.StartGain=40;//limit 0-40dB
    
    m_nRange = sonarSwitches.Range;
    m_nRangeIndex = GetRangeIndex(m_nRange);

    int client;

    const char ip[] = "192.168.0.5";

    struct sockaddr_in server_addr;

    client = socket(AF_INET, SOCK_STREAM, 0);
    if(client < 0)
    {
        cout << "Error" << endl;
        exit (1);
    }

    cout << "Socket created" << endl;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("192.168.0.5");
    inet_pton(AF_INET, ip, &server_addr.sin_addr);


    if(connect(client,(struct sockaddr *)&server_addr, sizeof(server_addr)) == 0)
        cout << "Connected" << endl;

    ping_number=0;

    index=0;

    index = send(client, (void *)&(sonarSwitches), 27, 0);

    while(1)
    {
	    index = recv(client, (void *)&(receiveBuf), 1013, 0);

	    if (index >= 1013)
	    {
		    if (sonarSwitches.Pack_Number == 0)
		    {
			    for (i = 0; i < 1000; i++)
				    dataBuf[999 - i] = receiveBuf[i + 12];
		    }

		    if (sonarSwitches.Pack_Number == 2)
		    {
			    for (i = 0; i < 12; i++)
				    headerBuf[i] = receiveBuf[i];
			    for (i = 0; i< 1001; i++)
				    dataBuf[i + 1000] = receiveBuf[i + 12];

			    WriteOnePingData(outfile);
		    }

		    if (sonarSwitches.Pack_Number == 0)
			    sonarSwitches.Pack_Number = 2;
		    else if (sonarSwitches.Pack_Number == 2)
			    sonarSwitches.Pack_Number == 0;

		    index = send(client, (void *)&(sonarSwitches), 27, 0);
	    }
    }

    if(outfile)
	    outfile.close();

    return nRetCode;
}

void WriteOnePingData(fstream& outfile)
{
    // Write a formatted .872 file using data received from the YellowFin Side Scan Sonar
    
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
    Buf[index++] = (BYTE)((ping_number&0x000000FF));      //PingNumber


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
    
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char MY_TIME[SIZE];
    char MY_DATE[SIZE];

    time(&now);
    
    ltm = localtime(&now);

    strftime(MY_DATE, sizeof(MY_DATE), "%d-%b-%Y", ltm);
    printf("Formatted date : %s\n", MY_DATE );
    strftime(MY_TIME, sizeof(MY_TIME), "%X", ltm);
    printf("Formatted date : %s\n", MY_TIME );

    for(i=0;i<11;i++) Buf[index++] = MY_DATE[i];//m_disk_date[i];Date DD-MMM-YYYY
    Buf[index++] = '\0';
    for(i=0;i<8;i++) Buf[index++] = MY_TIME[i];//m_disk_time[i];time HH:MM:SS
    Buf[index++] = '\0';

    timeval curTime;
    gettimeofday(&curTime, NULL);
    unsigned int milli = curTime.tv_usec / 1000;
    printf("\nMilli = %03d\n", milli);
    sprintf((char *)&Buf[index],".%03d",milli);

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

    for (i = 0; i< 12; i++)        //start at index=53
        Buf[index+i]=headerBuf[i];//add sonar header 12 bytes

    index=70;
    Buf[index++] = 1;//AUV board
    Buf[index++] = m_nRange;

    unsigned int temp;
    temp = (unsigned int)(m_dAUVSpeed*100);
    Buf[index++] = (BYTE)((temp&0xFF00)>>8);
    Buf[index++] = (BYTE)(temp&0x00FF);

      if(outfile)
      {
          outfile.write((char*)Buf, 1000);
          outfile.flush();
      }

	if (outfile)
	{
		outfile.write((char*)dataBuf, m_numDataPoints);
	}

    //zero fill for GPS
    for(i=0;i<1001;i++)Buf[i]=0;//init to all zeros
      if(outfile)
      {
          outfile.write((char*)Buf,1000);
          outfile.flush();
      }

    //4094-4095
    //pointer to a previous ping bytes of this ping + last ping
    nBytes = 4096+4096;
    Buf[94] = (BYTE)(((nBytes)&0xFF00)>>8);
    Buf[95] = (BYTE)((nBytes)&0x00FF);

      if(outfile)
      {
          outfile.write((char*)Buf,96);
          outfile.flush();
      }

//      outfile.close();

    //m_nRepRate=abs(GetTickCount()-m_nLastTime);
    //m_nLastTime=GetTickCount();

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
