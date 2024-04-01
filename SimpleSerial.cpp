//#ifdef _MSC_VER
#include "stdafx.h"
//#endif

#include "SimpleSerial.h"
#include <windows.h>
#include <strsafe.h>

using namespace std;

SimpleSerial::SimpleSerial(const LPCSTR& com_port):
connected_(false)
{
    std::setlocale(LC_CTYPE, "");

    test = (char*)calloc(1, sizeof(test));

    //init
    //std::vector<std::string> lpBuffer = { "M6", "B26\r", "e", "s", "r", "4" }; //RABOTI DO M6 - 1.1MHz
    lpBuffer.clear();
    lpBuffer.push_back("M5");
    lpBuffer.push_back("B26\r");
    lpBuffer.push_back("r");

    //printf("before open\n");
    io_handler_ = OpenPort(com_port);
    //getReading(io_handler_, 2);    
    SendAndReceive("M5");
    SendAndReceive("B26\r");
    SerNo = SendAndReceive("r");

    //debug
    //CString str;
    //str.Format("SerNo: %s", SerNo.c_str() );
    //AfxMessageBox(str);
    //debug


    //printf("const exit\n");
}

SimpleSerial::~SimpleSerial()
{
    free(test);
    if (connected_)
    {
        connected_ = false;
        CloseHandle(io_handler_);
    }
}

LPTSTR SimpleSerial::SendRS232(LPTSTR cmd)
{
    unsigned long dNoOfBytesWritten = 0;            // No of bytes written to the port

    Status = WriteFile(hComm,                       // Handle to the Serial port
        cmd,                                        // Data to be written to the port
        strlen(cmd),                                //No of bytes to write  //fixme warn c4267 conversion from  size_t to DWORD, possible loss of data
        &dNoOfBytesWritten,                         //Bytes written
        NULL);

    if (Status == FALSE) {
        printf("WRITE FALSE\n");
        //ErrorExit(TEXT("WriteFile"));
        return "VGZ";
    }
    //rtrim((*i));
    return "VGZ";
}

//
LPTSTR SimpleSerial::ReceiveRS232()
{
    //char SerialBuffer[256];                                     //Buffer for storing Rxed Data
    memset(SerialBuffer, 0x00, sizeof(SerialBuffer));           //malloc ?

    clearComm();

    Status = SetCommMask(hComm, EV_RXCHAR);

    if (Status == FALSE) {
        //debug
        CString str;
        str.Format("error 32_1");
        AfxMessageBox(str);
        //debug
        return "VGZ";
    }

    Status = WaitCommEvent(hComm, &dwEventMask, NULL);
    if (Status == FALSE) {
        //debug
        CString str;
        str.Format("error 33_1");
        AfxMessageBox(str);
        //debug
        return "VGZ2";
    }

    clearComm();

    do {
        Status = ReadFile(hComm,            //Handle of the Serial port
            &SerialBuffer,                  //Temporary character
            sizeof(SerialBuffer),           //Size of TempChar
            &NoBytesRead,                   //Number of bytes read
            NULL);

        //check bytes read

        //debug
        CString str;
        str.Format("str: %s", SerialBuffer);
        //AfxMessageBox(str);
        //debug

        if (Status == FALSE) {
            //debug
            CString str;
            str.Format("err rcsrs232 sb: %s", SerialBuffer);
            AfxMessageBox(str);
            //debug
            clearComm();
            return "VGZ";
        }
    } while ( NoBytesRead > 0);

    //debug
//    CString str;
//    str.Format("rcv: %s", SerialBuffer);
//    AfxMessageBox(str);
    //debug

    return SerialBuffer;
}


LPTSTR SimpleSerial::SendAndReceive(LPTSTR cmd)
{
    LPTSTR res;

    SendRS232(cmd);
    res = ReceiveRS232();

    //debug
    //CString str;
    //str.Format("err rcsrs232 res: %s", res);
    //AfxMessageBox(str);
    //debug

    return res;
}

void SimpleSerial::ErrorExit(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw);
}

double SimpleSerial::getValue()
{
    return Value;
}

std::string SimpleSerial::getSerNo()
{
    return SerNo;
}

char* SimpleSerial::convertToBitString(unsigned long long value)
{
    memset(str1, '0', sizeof(str1));

    for (int i = 0; i < 64; i++)
    {
        if ((1ull << i) & value)
            str1[63 - i] = '1';     //UZHASNA SHITNIA
    }
    str1[64] = 0x00;
    str1[65] = 0x00;
    return &str1[0];
}

double SimpleSerial::readValuefromScale()
{
    Value = -11.13;
    lpBuffer.clear();
    lpBuffer.push_back("4");
    getReading(io_handler_, 0);
    if (Value < 0) {
        //debug
        CString str;
        str.Format("error 37");
        //AfxMessageBox(str);
        //debug
    }

    return Value;
}

bool SimpleSerial::WriteSerialPort(const std::string& data_sent)
{
    return false;
}

bool SimpleSerial::CloseSerialPort()
{
    if (connected_)
    {
        connected_ = false;
        CloseHandle(io_handler_);
        return true;
    }
    else
    {
        return false;
    }
}

bool SimpleSerial::IsConnected() const
{
    return connected_;
}

HANDLE SimpleSerial::OpenPort(LPCSTR ComPortName)
{
    std::string s = "\\\\.\\";
    s = s + ComPortName;

    //printf("Opening serial port %s\n", s.c_str());

    hComm = CreateFileA(s.c_str(),                //port name
        GENERIC_READ | GENERIC_WRITE, //Read/Write
        0,                            // No Sharing
        NULL,                         // No Security
        OPEN_EXISTING,                // Open existing port only
        0,                            // Non Overlapped I/O
        NULL);                        // Null for Comm Devices

    if (hComm == INVALID_HANDLE_VALUE) {
        printf("Error in opening serial port %s\n", s.c_str());
        //debug
        CString str;
        str.Format("error 1");
        //AfxMessageBox(str);
        //debug
    }
    else {
        printf("opening serial port successful\n");
    }

    //printf("before freeze on dcbserialparams\n");

    DCB dcbSerialParams = { 0 };                    // Initializing DCB structure.... freeze on debug
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    bool Status = GetCommState(hComm, &dcbSerialParams);

    dcbSerialParams.BaudRate = CBR_115200;          // Setting BaudRate = 9600
    dcbSerialParams.ByteSize = 8;                   // Setting ByteSize = 8
    dcbSerialParams.StopBits = ONESTOPBIT;          // Setting StopBits = 1
    dcbSerialParams.Parity = NOPARITY;              // Setting Parity = None
    dcbSerialParams.fDsrSensitivity = FALSE;		// Setup the flow control
    dcbSerialParams.fOutxCtsFlow = FALSE;			// NoFlowControl:
    dcbSerialParams.fOutxDsrFlow = FALSE;
    dcbSerialParams.fOutX = FALSE;
    dcbSerialParams.fInX = FALSE;
    dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;
    dcbSerialParams.fTXContinueOnXoff = FALSE;
    dcbSerialParams.fOutX = FALSE;
    dcbSerialParams.fInX = FALSE;
    dcbSerialParams.fNull = FALSE;
    dcbSerialParams.fAbortOnError = TRUE;
    dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;

    if (!SetCommState(hComm, &dcbSerialParams)) {// Set new state
        //std::cout << "Tuka si eba maikata 1!\n";
        //debug
        CString str;
        str.Format("error 2");
        //AfxMessageBox(str);
        //debug
    }

    //std::cout << "Port set!\n";

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 10; // in milliseconds
    timeouts.ReadTotalTimeoutConstant = 1; // in milliseconds
    timeouts.ReadTotalTimeoutMultiplier = 1; // in milliseconds
    timeouts.WriteTotalTimeoutConstant = 1; // in milliseconds
    timeouts.WriteTotalTimeoutMultiplier = 1; // in milliseconds

    if (!SetCommTimeouts(hComm, &timeouts)) {
        std::cout << "Tuka si eba maikata 2!\n";
    }
    clearComm();
    return hComm;
}

uint8_t SimpleSerial::crcBiSS(uint32_t data)
{
    //printf("\nCRC2:%x\n", data);          //debug
    uint8_t crc;
    uint32_t tmp;
    tmp = (data >> 30) & 0x00000003;
    crc = ((data >> 24) & 0x0000003F);
    tmp = crc ^ tableCRC6[tmp];
    crc = ((data >> 18) & 0x0000003F);
    tmp = crc ^ tableCRC6[tmp];
    crc = ((data >> 12) & 0x0000003F);
    tmp = crc ^ tableCRC6[tmp];
    crc = ((data >> 6) & 0x0000003F);
    tmp = crc ^ tableCRC6[tmp];
    crc = (data & 0x0000003F);
    tmp = crc ^ tableCRC6[tmp];
    crc = tableCRC6[tmp];
    return crc;
}

//convert from hex to actual position (double)
double SimpleSerial::parsePosition(unsigned long long input)
{
    //0000000000001100000000000000000000100000000100111100101000100010
    //0000000000000000000000000000000000000000000000000011001011000000
    //signal value
    outdbl = -11.111;

    outptr = &out[0];

    memcpy(out, convertToBitString(input), 130);    //here checkme... mai e fixme

    //printf("\r\nBIN:%s\r\n", convertToBitString(input));

    //has to begin with 11 e.g. 0xc0
    // zeroes.... any number
    // 1 START BIT
    // 0 CDS
    // 26 bits position
    // 2 bits status
    // 6 bits CRC
    // rest - ignored

    //printf("CHECK1:");            //debug
    
    //begin
    if ((out[0] == '1') && (out[1] == '1')) {
        outptr += 2;

        //zeroes skip
        //printf("OK \nZEROES:");   //debug
        while (*outptr == '0')
        {
            //printf("0");          //debug
            outptr++;
        }
        //printf("\n");             //debug

        //detection of start condition
        if ((*outptr == '1') && (*(outptr + 1) == '0')) {
            //printf("START CONDITION\n");      //debug
            outptr += 2;
        }

        //actual position
        //printf("POS:");           //debug
        for (int i = 0; i < 26; i++) {
            //printf("%c", *outptr);    //debug
            outptr++;
        }
        //printf("\n");

        //
        unsigned long int res = convert(outptr - 26, 26);
        //double res2 = res * 0.2442 / 1000;                //veroiatno ot kriv vint - greshni rezutati
        double res2 = res * 0.244140625 / 1000;             //tova e viarnata stoinost.. koito chete pdf
        //printf("POS_%s: %.8fmm\n", SerNo.c_str(), res2);               //actual reading debug

        //printf("\n");
        
        // status processing
        //printf("STATUS:");                //debug
        uint8_t status = 0;
        for (int i = 1; i >= 0; i--) {
            //printf("%c", *outptr);        //debug
            if (*outptr == '1')
                status += (1 << i);
            if (*outptr == '0')
                res2 = -11.111;         //error signalling
            outptr++;
        }
        //printf(":%d", status);          //debug
        //printf("\n");
        
        //CRC stuff
        //printf("CRC:");                  //debug
        //CRC bits are inverted!
        for (int i = 0; i < 6; i++) {
            //printf("%c", *outptr);        //debug
            outptr++;
        }
        //printf("\n");                     //debug

        uint8_t crc_recv = (uint8_t)(63 - convert(outptr - 6, 6));
        //printf("CRC:0x%hhx\n", crc_recv);

        unsigned long int res3;
        res3 = (res << 2) | (status & 0x03);   //fixme... what  ?
        uint8_t crc_calc = crcBiSS(res3);
        //printf("CRCC: %x\n", crc_calc);       //debug
        if (crc_calc == crc_recv) {
            //printf("CRCs: OK");               //debug
            outdbl = res2;                      //getting valid result only on valid crc
        }
        else {
            //printf("CRCs: ERR!");             //debug
        }
        //printf("\n");
    }
    else {
        printf("ERR %d\n", errcnt++);
        //debug
        CString str;
        str.Format("error 3\n%s:", out);
        //AfxMessageBox(str);
        //debug
    }
    return outdbl;
}

//null terminated
unsigned long int SimpleSerial::convert(char* input, uint8_t len) {
    //printf("CNV_%d:", len);
    char* charptr = input;
    uint8_t pow2 = len;
    unsigned long int result = 0;

    //check for length

    while (pow2 > 0) {
        //printf("%c", *charptr);
        if (*charptr == '1') {
            result += pow(2, pow2 - 1);     //fixme c4244 possible loss of data
        }
        charptr++;
        pow2--;
    }
    //printf(":0x%lx\n", result);
    return result;
}


void SimpleSerial::clearComm()
{
    DWORD dwError;
    COMSTAT commstat;

    ClearCommError(hComm, &dwError, &commstat);
    if (dwError > 0) {
        //debug
        CString str;
        str.Format("error 36_%d", dwError);
        AfxMessageBox(str);
        //debug
    }
}

int SimpleSerial::getReading(HANDLE hComm, int lineNo)     //line no to get the value
{
    int charsToComplete = 17;

    clearComm();

    // Position (18/26/32/36 bits)
    for (auto i = lpBuffer.begin(); i != lpBuffer.end(); ++i) {
        dNoOfBytesWritten = 0;              // No of bytes written to the port

        Status = WriteFile(hComm,           // Handle to the Serial port
            (*i).c_str(),                   // Data to be written to the port
            (*i).size(),                    //No of bytes to write  //fixme warn c4267 conversion from  size_t to DWORD, possible loss of data
            &dNoOfBytesWritten,             //Bytes written
            NULL);

        if (Status == FALSE) {
            printf("WRITE FALSE\n");
            //debug
            //CString str;
            //str.Format("error writing to serial port %d", GetLastError());
            //AfxMessageBox(str);
            //debug

            //ErrorExit(TEXT("WriteFile"));
            return 1;
        }
        rtrim((*i));
        //printf("O2D: %s (%d, %d)\n", (*i).c_str(), (*i).size(), dNoOfBytesWritten);
        //std::cout << "Wrote " << dNoOfBytesWritten << " chars. Waiting for char\n";
    }

    Status = SetCommMask(hComm, EV_RXCHAR);

    if (Status == FALSE) {
        //debug
        CString str;
        str.Format("error 32");
        AfxMessageBox(str);
        //debug
        return 2;
    }

    Status = WaitCommEvent(hComm, &dwEventMask, NULL);
    if (Status == FALSE) {
        //debug
        CString str;
        str.Format("error 33");
        AfxMessageBox(str);
        //debug
        return 3;
    }

    //cut from here
    //i = 0;

    memset(SerialBuffer, 0x00, sizeof(SerialBuffer));

    //printf("stc: %d\n", sizeof(TempChar));

    do {
        Status = ReadFile(hComm,             //Handle of the Serial port
            &SerialBuffer ,              //Temporary character
            sizeof(SerialBuffer),       //Size of TempChar
            &NoBytesRead,           //Number of bytes read
            NULL);

        if (Status == FALSE) {
            //debug
            CString str;
            str.Format("error 34");
            AfxMessageBox(str);
            //debug
            clearComm();
            return 3;
        }

    } while ( NoBytesRead == 0 );
    
    //split cut from here

    ptr = strtok_s(SerialBuffer, seps, &nptr);  //split

    parts.clear();

    while (ptr != NULL)
    {
        //cout << ptr << endl; // print the string token  
        printf("OUT_%02d:%s\n", parts.size(), ptr);
        parts.push_back(ptr);
        ptr = strtok_s(NULL, seps, &nptr);
    }

    //get reading 
    if (lineNo == 0) {                  //fixme. refactor lineNo name
        Value = -11.1111;
        //char* test; //cut from here

        if (strlen(parts[lineNo]) != 16) 
        {
            //debug
            CString str;
            str.Format("strlen:%s", parts[lineNo]);
            AfxMessageBox(str);
            //debug

        }
        val = strtoull(parts[lineNo], &test, 16);
        //printf("TEST: %llx\n", val);
        Value = parsePosition(val);
        if (Value < 0.0) {
            //debug
            CString str;
            str.Format("x1 = %g\nx2 = %g\n y = %g", 1, 1, 1);
            AfxMessageBox(str);
            //debug
        }

        return 0;
    }

    // get serial no
    if (lineNo == 2) {
        Value = -11.17;  //error inidcation
        //printf("pc:%d\n", parts.size());
        SerNo = parts[lineNo];          //fixme... check for serial number 5 hex characters
        if (strlen(SerNo.c_str()) != 6 ) {
            //debug
            CString str;
            //str.Format("serno: %s\n%s\n%s\n%s", SerNo.c_str(), parts[0], parts[1], parts[2]);
            str.Format("serno: %s\n", SerNo.c_str());
            AfxMessageBox(str);
            //debug
        }
        return 0;
    }
    return 0;
}
