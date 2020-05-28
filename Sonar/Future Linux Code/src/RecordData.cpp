// RecordData.cpp : Defines the entry point for the console application.

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

// Function prototype for ping function
void WriteOnePingData(fstream& outfile);

int main(int argc, char * argv[], char * envp[])
{
	int nRetCode = 0;
	struct sockaddr_in serv_add;

	std::string filename;
	char input[256];
	int index = 0, i = 0;

	// If there was no argument to specify filename, prompt user
	if(argc == 1)
	{
		for(i = 0; i < 256; i++)
			input[i] = 0;
		input[0] = '\0';
		while(input[0] == '\0')
		{
			cout << "Input a file name: " ;
			cin >> input ;
		}
		filename = input;
	}
	// If there was, use it
	else if(argc == 2)
		filename = argv[1];

	// Location of file (path, name, extension)
	filename = "/media/Data/missions/" + filename + ".872";

	// Open and create the file
	fstream outfile;
	outfile.open(filename, ios::out);

	// If file not created, error
	if(!outfile)
		cout << "Error, file not created" << endl;

	// Sonar switch variables
	//******************************************************************
	m_dAUVSpeed = 0.;		    // If there is the speed info, feed in

	m_nRange = 50;
	m_nRangeIndex = 9;
	sonarSwitches.Start1 = 0xFE;	    // Byte0, always 0xFE
	sonarSwitches.Start2 = 0x44;        // Byte1, always 0x44
	sonarSwitches.Head_ID = 0;     	    // Byte2, 0x00
	sonarSwitches.Range = m_nRange;     // Byte3,
	sonarSwitches.RangeOffset = 0;      // Byte4, 0
	sonarSwitches.HOLD = 0;             // Byte5, not used
	sonarSwitches.MA_SL = 0x00;         // Byte6,
	sonarSwitches.Freq = 0;             // Byte7, 0: low, 1: medium 2:high
	sonarSwitches.StartGain = 20;       // Byte8,
	sonarSwitches.LOGF = 0;             // Byte9,
	sonarSwitches.Absorption = 0;       // Byte10,
	sonarSwitches.PulseLength = 0;      // Byte11,
	sonarSwitches.Delay = 0;            // Byte12,
	sonarSwitches.Resvd0 = 0x00;        // Byte13,
	sonarSwitches.PortGain = 0x00;      // Byte14,
	sonarSwitches.StbdGain = 0x00;      // Byte15,
	sonarSwitches.Resvd1 = 0;           // Byte16, Researved, always 0
	sonarSwitches.Resvd2 = 0;           // Byte17, Researved, always 0
	sonarSwitches.Pack_Number = 0;      // Byte18, packet number?
	sonarSwitches.Resvd4 = 0;           // Byte19, Researved, always 0
	sonarSwitches.Resvd5 = 0;           // Byte20,
	sonarSwitches.Resvd6 = 0;           // Byte21,
	sonarSwitches.Resvd7 = 0;           // Byte22,
	sonarSwitches.Resvd8 = 0;           // Byte23, Researved
	sonarSwitches.Resvd9 = 0;           // Byte24, Researved
	sonarSwitches.Resvd10 = 0;          // Byte25,
	sonarSwitches.Term = 0xFD;          // Byte26, 0xFD
	//******************************************************************

	// Prompt user for range
	int temp; 
	cout << "Enter a range: ";
	cin >> temp;
	cin.ignore(256, '\n');
	sonarSwitches.Range = temp;

	// Set minimum and maximum values for range
	if(sonarSwitches.Range < 10)
		sonarSwitches.Range = 10;
	if(sonarSwitches.Range > 200)
		sonarSwitches.Range = 200;

	// Prompt user for frequency
	cout << "Enter a frequency: ";
	cin >> temp;
	cin.ignore(256, '\n');
	sonarSwitches.Freq = temp;

	// Set minimum and maximum values for frequency
	if(sonarSwitches.Freq < 0)
		sonarSwitches.Freq = 0;
	if(sonarSwitches.Freq > 2)
		sonarSwitches.Freq = 2;
	if(sonarSwitches.Freq == 2 && sonarSwitches.Range > 50) // High freq limit range to 50m
		sonarSwitches.Range = 50;

	// Prompt user for gain
	cout << "Enter a gain: ";
	cin >> temp;
	cin.ignore(256, '\n');
	sonarSwitches.StartGain = temp;

	// Set minimum and maximum values for gain
	if(sonarSwitches.StartGain < 0)
		sonarSwitches.StartGain = 0;
	if(sonarSwitches.StartGain > 40)
		sonarSwitches.StartGain = 40; // Limit 0 - 40dB

	m_nRange = sonarSwitches.Range;
	m_nRangeIndex = GetRangeIndex(m_nRange);

	// Client socket setup
	int client;
	const char ip[] = "192.168.0.5";
	struct sockaddr_in server_addr;
	client = socket(AF_INET, SOCK_STREAM, 0);
	if(client < 0)
	{
		cout << "Error: Unable to create client socket" << endl;
		exit (1);
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr("192.168.0.5");
	inet_pton(AF_INET, ip, &server_addr.sin_addr);

	// Check if socket is connected
	if(connect(client,(struct sockaddr *)&server_addr, sizeof(server_addr)) == 0)
		cout << "Connected" << endl;

	char ch;
	ping_number = 0;
	index = 0;

	// Enable ncurses
	initscr();

	// Suppress the automatic echoing of typed characters
	noecho();

	// Client send
	index = send(client, (void *)&(sonarSwitches), 27, 0);

	cout << "Began mission..." << endl;

	// Allows user input while recording data and doesn't wait for input
	nodelay(stdscr, TRUE);
	
	// Infinite loop that captures sonar data
	while(1)
	{
		// Client receive
		index = recv(client, (void *)&(receiveBuf), 1013, 0);

		if (index >= 1013)
		{
			// 1000 bytes per channel
			if (sonarSwitches.Pack_Number == 0)
			{
				// Flip the port side data 1000 bytes
		    		for (i = 0; i < 1000; i++)
					dataBuf[999 - i] = receiveBuf[i + 12];
			}

			if (sonarSwitches.Pack_Number == 2)
			{
				// Get the starboard size data
		    		for (i = 0; i < 12; i++)
			    		headerBuf[i] = receiveBuf[i];
		    		for (i = 0; i< 1001; i++)
			    		dataBuf[i + 1000] = receiveBuf[i + 12];

				// Ping function to write one ping to file
		    		WriteOnePingData(outfile);
			}

			// Switch packet number every ping
			if (sonarSwitches.Pack_Number == 0)
				sonarSwitches.Pack_Number = 2;
			else if (sonarSwitches.Pack_Number == 2)
				sonarSwitches.Pack_Number == 0;

			// Client send
			index = send(client, (void *)&(sonarSwitches), 27, 0);
	    	}
	    
		// NCurses stuff
		// Gets character from user input in stdin
	    	ch = getch();

		// If no user response, continue
		if (ch == ERR)
			continue;

		// If + is entered, allow user to change parameters
		else if (ch == '+')
		{	
			// Prompt user for range
			cout << "Enter a new range: ";
			cin >> temp;
			cin.ignore(256, '\n');
			sonarSwitches.Range = temp;

			// Set minimum and maximum values for range
			if(sonarSwitches.Range < 10)
				sonarSwitches.Range = 10;
			if(sonarSwitches.Range > 200)
				sonarSwitches.Range = 200;

			// Prompt user for frequency
			cout << "Enter a new frequency: ";
			cin >> temp;
			cin.ignore(256, '\n');
			sonarSwitches.Freq = temp;

			// Set minimum and maximum values for frequency
			if(sonarSwitches.Freq < 0)
				sonarSwitches.Freq = 0;
			if(sonarSwitches.Freq > 2)
				sonarSwitches.Freq = 2;
			if(sonarSwitches.Freq == 2 && sonarSwitches.Range > 50) //high freq limit range to 50m
				sonarSwitches.Range = 50;

			// Prompt user for gain
			cout << "Enter a new gain: ";
			cin >> temp;
			cin.ignore(256, '\n');
			sonarSwitches.StartGain = temp;

			// Set minimum and maximum values for gain
			if(sonarSwitches.StartGain < 0)
				sonarSwitches.StartGain = 0;
			if(sonarSwitches.StartGain > 40)
				sonarSwitches.StartGain = 40; //limit 0-40dB

			m_nRange = sonarSwitches.Range;
			m_nRangeIndex = GetRangeIndex(m_nRange);

			cout << "Mission continued with new parameters..." << endl;
		}

		// If ~ is entered, end recording
		else if (ch == '~')
		{
			// Restore terminal settings
			endwin();
			break;
		}
		// Restore terminal settings
		endwin();
	}
    
	// Close mission file
    	if(outfile)
		outfile.close();

    	return nRetCode;
}

void WriteOnePingData(fstream& outfile)
{
	// Write a formatted .872 file using data received from the YellowFin Side Scan Sonar

	// Retrieve number of milliseconds that have elapsed since the system was started
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	uint64_t GetTickCount = (uint64_t)(ts.tv_nsec / 1000000) + ((uint64_t)ts.tv_sec * 1000ull);


	BYTE Buf[1001];
	int nBytes, index, i;
	index = 0;
	for(i = 0; i < 1001; i++)
		Buf[i] = 0; // Init to all zeros
    
	// First 1000 bytes: ping header
	Buf[index++] = '8';
	Buf[index++] = '7';
	Buf[index++] = '2';
		
	Buf[index++] = 0; // Version: 0-current version 1.xx, 1-later version
		
	ping_number++;  // Index = 4;
	Buf[index++] = (BYTE)((ping_number & 0xFF000000) >> 24);
	Buf[index++] = (BYTE)((ping_number & 0x00FF0000) >> 16);
	Buf[index++] = (BYTE)((ping_number & 0x0000FF00) >> 8);
	Buf[index++] = (BYTE)((ping_number & 0x000000FF));      //PingNumber


	nBytes = m_nFileRecordSize; // Total bytes for this record, index = 8, 9
	Buf[index++] = (BYTE)(((nBytes) & 0xFF00) >> 8);
	Buf[index++] = (BYTE)((nBytes) & 0x00FF);

	nBytes = m_nDataPointsPerChannel; // Data points per channel, index = 10, 11
	Buf[index++] = (BYTE)(((nBytes) & 0xFF00) >> 8);
	Buf[index++] = (BYTE)((nBytes) & 0x00FF);

	Buf[index++] = 1; // Bytes per data point 12
	Buf[index++] = 8; // Data point bit depth 13
	Buf[index++] = 0; // (BYTE)(((gps_type & 0x03) << 4) | 0x04);// GPS strings type 14

	nBytes = m_nGPSDataOffset; // GPS string data start address offset 15-16
	Buf[index++] = (BYTE)(((nBytes) & 0xFF00) >> 8);
	Buf[index++] = (BYTE)((nBytes) & 0x00FF);

	// Event
	nBytes=0; // Event and annotation 17-18
	Buf[index++] = (BYTE)(((nBytes) & 0xFF00) >> 8);
	Buf[index++] = (BYTE)((nBytes) & 0x00FF);
   
       	// Get date and time
	time_t now = time(0);
	tm *ltm = localtime(&now);
	char MY_TIME[SIZE];
	char MY_DATE[SIZE];

	time(&now);

	ltm = localtime(&now);

	strftime(MY_DATE, sizeof(MY_DATE), "%d-%b-%Y", ltm);
	strftime(MY_TIME, sizeof(MY_TIME), "%X", ltm);

	for(i = 0; i < 11;i++)
		Buf[index++] = MY_DATE[i];// m_disk_date[i];Date DD-MMM-YYYY
	Buf[index++] = '\0';
	for(i = 0; i < 8;i++)
		Buf[index++] = MY_TIME[i];// m_disk_time[i];time HH:MM:SS
	Buf[index++] = '\0';

	timeval curTime;
	gettimeofday(&curTime, NULL);
	unsigned int milli = curTime.tv_usec / 1000;
	sprintf((char *)&Buf[index],".%03d", milli);

	index = 45;
	Buf[index++] = sonarSwitches.Freq;
	Buf[index++] = m_nRangeIndex;
	Buf[index++] = sonarSwitches.StartGain;
	Buf[index++] = 0;// In order to match with YellowFin

	Buf[index++] = (BYTE)(((m_nRepRate) & 0xFF00) >> 8);
	Buf[index++] = (BYTE)((m_nRepRate) & 0x00FF);
	nBytes = 1500 * 10; // Sound velocity
	Buf[index++] = (BYTE)(((nBytes) & 0xFF00) >> 8);
	Buf[index++] = (BYTE)((nBytes) & 0x00FF);

	for (i = 0; i< 12; i++)		// Start at index = 53
	Buf[index + i] = headerBuf[i];	// Add sonar header 12 bytes

	index = 70;
	Buf[index++] = 1;// AUV board
	Buf[index++] = m_nRange;

	unsigned int temp;
	temp = (unsigned int)(m_dAUVSpeed * 100);
	Buf[index++] = (BYTE)((temp & 0xFF00) >> 8);
	Buf[index++] = (BYTE)(temp & 0x00FF);

	// Write first 1000 bytes
	if(outfile)
	{
		outfile.write((char*)Buf, 1000);
		outfile.flush();
	}

	// Write out tje two channel data 1000 bytes and zero fill 1000 bytes
	if (outfile)
		outfile.write((char*)dataBuf, m_numDataPoints);

	// Zero fill for GPS
	for(i = 0; i < 1001; i++)
		Buf[i] = 0;	// Init to all zeros

	if(outfile)
	{
		outfile.write((char*)Buf, 1000);
		outfile.flush();
	}

	// 4094-4095
	// Pointer to a previous ping bytes of this ping + last ping
	nBytes = 4096 + 4096;
	Buf[94] = (BYTE)(((nBytes) & 0xFF00) >> 8);
	Buf[95] = (BYTE)((nBytes) & 0x00FF);

	if(outfile)
	{
		outfile.write((char*)Buf, 96);
		outfile.flush();
	}

	m_nRepRate = GetTickCount - m_nLastTime;
	m_nLastTime = GetTickCount;
}

int GetRangeIndex(int nRange)
{
	int index = 5;
	switch (nRange)
	{
		case 10: index = 5; break;
		case 20: index = 6; break;
		case 30: index = 7; break;
		case 40: index = 8; break;
		case 50: index = 9; break;
		case 60: index = 10; break;
		case 80: index = 11; break;
		case 100: index = 12; break;
		case 125: index = 13; break;
		case 150: index = 14; break;
		case 200: index = 15; break;
		default: index = 15;
	}
	return index;
}
