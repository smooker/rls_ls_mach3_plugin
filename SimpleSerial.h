#include <memory>
#include <iostream>
#include <stdio.h>
#include <string>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <chrono>
#include <thread>
#include <time.h>
#include <fstream>
#include <list>
#include <vector>


#define DEBUG

class SimpleSerial {

public:
    SimpleSerial(const LPCSTR& com_port);
    ~SimpleSerial();

    bool WriteSerialPort(const std::string& data_sent);
    double readValuefromScale();

    HANDLE OpenPort(LPCSTR ComPortName);
    bool CloseSerialPort();

    bool IsConnected() const;

    double getValue();
    std::string getSerNo();
    int getReading(HANDLE hComm, int lineNo);

    int errcnt = 0;

private:
    double Value;
    std::string SerNo;

    bool connected_;
    // 
    unsigned long int convert(char* input, uint8_t len);
    char* convertToBitString(unsigned long long value);

    HANDLE io_handler_;

    std::vector<std::string> lpBuffer;

    std::vector<char*> parts;

    char TempChar;              //Temporary character used for reading
    char SerialBuffer[256];     //Buffer for storing Rxed Data

    DWORD NoBytesRead;
    int i;
    long long unsigned int val;

    //
    char* ptr;
    char* nptr;
    char seps[4] = { 0x2c, 0x09, 0x0a, 0x0d };

    //
    char *test;

    //
    DWORD dNoOfBytesWritten;        // No of bytes written to the port
    DWORD dwEventMask;
    bool Status;

    // from parseposition... to be cleaned
    char out[130];
    char* outptr;
    double outdbl;

    //
    uint8_t tableCRC6[64] = {
                                0x00, 0x03, 0x06, 0x05, 0x0C, 0x0F, 0x0A, 0x09,
                                0x18, 0x1B, 0x1E, 0x1D, 0x14, 0x17, 0x12, 0x11,
                                0x30, 0x33, 0x36, 0x35, 0x3C, 0x3F, 0x3A, 0x39,
                                0x28, 0x2B, 0x2E, 0x2D, 0x24, 0x27, 0x22, 0x21,
                                0x23, 0x20, 0x25, 0x26, 0x2F, 0x2C, 0x29, 0x2A,
                                0x3B, 0x38, 0x3D, 0x3E, 0x37, 0x34, 0x31, 0x32,
                                0x13, 0x10, 0x15, 0x16, 0x1F, 0x1C, 0x19, 0x1A,
                                0x0B, 0x08, 0x0D, 0x0E, 0x07, 0x04, 0x01, 0x02 };

    uint8_t crcBiSS(uint32_t data);

    char str1[130];

    double parsePosition(unsigned long long);

    // trim from start (in place)
    inline void ltrim(std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
            }));
    }

    // trim from end (in place)
    inline void rtrim(std::string& s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
            }).base(), s.end());
    }
};