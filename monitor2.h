/**********************************************************
*                                                         *
*  IMAIoT - Infrastructure Monitoring Agent for IoT       *
*                                                         *
*  Author: Alexandre Heideker                             *
*  e-mail: alexandre.heideker@ufabc.edu.br                *
*  UFABC - 2019                                           *
*  Ver. 0.2                                               *
***********************************************************/
using namespace std;


typedef struct {
    int pid;
    std::string Name;
    float cpu;
    long DataMem;
    long VirtMem;
} processData;
typedef struct {
    unsigned int ctrl;
    unsigned int status;
    unsigned int rctl;
    unsigned int rdlen;
    unsigned int rdh;
    unsigned int rdt;
    unsigned int rdtr;
    unsigned int tctl;
    unsigned int tdlen;
    unsigned int tdh;
    unsigned int tdt;
    unsigned int tidv;
    unsigned int phystatus;
    unsigned int phycontrol;
    char trash[16];
} NICdump;

typedef struct {
    long UDPtxQueue;
    long UDPrxQueue;
    long TCPtxQueue;
    long TCPrxQueue;
    unsigned int TCPMaxWindowSize; //only TCP
    unsigned int RxRing;
    unsigned int TxRing;
    unsigned int nqaCnt1;
    unsigned int nqaCnt5;
} networkData;

typedef struct {
    std::string Name;
    long RxBytes;
    long RxPackets;
    long RxErrors;
    long RxDrop;
    long RxFifo;
    long TxBytes;
    long TxPackets;
    long TxErrors;
    long TxDrop;
    long TxFifo;
    unsigned int rxring;
    unsigned int txring;
} networkAdapter;

typedef struct {
    std::string NIC;
    std::string parameter;
    long value;
} ethtoolData;

typedef struct {
    std::string parameter;
    long value;
} nstatData;

#include "mondata.h"


std::string NodeName = "JohnDoe";
bool debugMode = false;
std::string LogType = "";
std::string tag = "";
u_int16_t ExpTime = 0;
std::string LogFileName = "monitor.log";
bool ServerMode = false;
u_int16_t ServerPort = 8888;
u_int16_t SampplingTime = 1;
u_int16_t LogIntervall = 1;
bool DockerStat = false;
bool Ethtool = false;
bool Nstat = false;
std::vector <std::string> ProcessNames;
std::vector <std::string> DockerNames;
bool SincMode = false;
MonData MFstats;




void dumpVar(){
    cout << "NodeName:\t" << NodeName << endl;
    cout << "debugMode:\t" << debugMode << endl;
    cout << "LogType:\t" << LogType << endl;
    cout << "LogFileName:\t" << LogFileName << endl;
    cout << "LogIntervall:\t" << LogIntervall << endl;
    cout << "ServerMode:\t" << ServerMode << endl;
    cout << "ServerPort:\t" << ServerPort << endl;
    cout << "SampplingTime:\t" << SampplingTime << endl;
    cout << "DockerStat:\t" << DockerStat << endl;
    cout << "Ethtool:\t" << Ethtool << endl;
    cout << "Nstat:\t\t" << Nstat << endl;
    cout << "ProcessNames:\t";
    for (auto i: ProcessNames)
        std::cout << i << ' ';
    cout << endl;
    cout << "DockerNames:\t";
    for (auto i: DockerNames)
        std::cout << i << ' ';
    cout << endl;
}


bool parseVar(string token, string value){
    if (token == "debugMode") {
        if (value == "1")
            debugMode = true;
        else
            debugMode = false; 
    } else 
    if (token == "LogMode") {
            LogType = UCase(value);
    } else 
    if (token == "LogFileName") {
        LogFileName = value;
    } else 
    if (token == "NodeName") {
        NodeName = value;
    } else
    if (token == "ServerMode") {
        if (value == "1")
            ServerMode = true;
        else
            ServerMode = false; 
    } else
    if (token == "ServerPort") {
        ServerPort = stoi(value);
    } else 
    if (token == "SampplingTime") {
        SampplingTime = stoi(value);
    } else
    if (token == "LogIntervall") {
        LogIntervall = stoi(value);
    } else 
    if (token == "DockerStat") {
        DockerStat = false; 
        if (value == "1")
            DockerStat = true;
    } else
    if (token == "EthStat") {
        Ethtool = false; 
        if (value == "1")
            Ethtool = true;
    } else
    if (token == "Nstat") {
        Nstat = false; 
        if (value == "1")
            Nstat = true;
    } else
    if (token == "CPUPathStat") {
        MFstats.CPUPathStat = value;
    } else
    if (token == "CPUPathArch") {
        MFstats.CPUPathArch = value;
    } else      
    if (token == "NetworkPathStat") {
        MFstats.NetworkPathStat = value;
    } else
    if (token == "ProcessNames") {
        MFstats.ProcessNames = splitString(trim(value), ',');
    } else
    if (token == "DockerNames") {
        MFstats.DockerNames = splitString(trim(value), ',');
        DockerStat = true;
    } else
    if (token == "tag") {
        tag = value;
    } else
    if (token == "sinc") {
        SincMode = true;
    } else
    if (token == "ExpTime") {
        ExpTime = stoi(value);
    } else
    if (token == "help") {
/*
\n\t--LogMode=<JSON|TXT|SQL>
\n\t--LogFileName=<string>
\n\t--NodeName=<string>
\n\t--ServerMode\t\tActivate server mode
\n\t--ServerPort=<int>\t\tSocket TCP Port
\n\t--SampplingTime=<int>\t\tSamppling Stats Intervall (default each one second)
\n\t--LogIntervall=<int>\t\tLogging intervall (default each one second)
\n\t--CPUPathStat=\path
\n\t--CPUPathArch=\path
\n\t--NetworkPathStat=\path
\n\t--ProcessNames=process1,process2,..\t\tMonitored processess
\n\t--DockerNames=< * or docker1,docker2,.. >\t\tMonitored Dockers
\n\t--tag=<string>\t\tIdentification of experiment
\n\t--ExpTime=<int>\t\tSeconds of monitoring (default 0=infinite)
\n\t--debugMode
\n\t--sinc\t\tSincronous mode
\n\t--help\t\tThis help
*/

        cout << "\n\t--LogMode=<JSON|TXT|SQL>\n\t--LogFileName=<string>\n\t--NodeName=<string>"
            << "\n\t--EthStat=<1|0>\t\tActivate ethtool stats"
            << "\n\t--Nstat=<1|0>\t\tActivate nstat stats"
            << "\n\t--ServerMode\t\tActivate server mode\n\t--ServerPort=<int>\t\tSocket TCP Port"
            << "\n\t--SampplingTime=<int>\t\tSamppling Stats Intervall (default each one second)"
            << "\n\t--LogIntervall=<int>\t\tLogging intervall (default each one second)"
            << "\n\t--CPUPathStat=\\path\n\t--CPUPathArch=\\path\n\t--NetworkPathStat=\\path"
            << "\n\t--ProcessNames=process1,process2,..\t\tMonitored processess"
            << "\n\t--DockerNames=< * or docker1,docker2,.. >\t\tMonitored Dockers"
            << "\n\t--tag=<string>\t\tIdentification of experiment"
            << "\n\t--ExpTime=<int>\t\tSeconds of monitoring (default 0=infinite)\n\t--debugMode=<1|0>"
            << "\n\t--sinc\t\tSincronous mode\n\t--help\t\tThis help" << endl;
        return false;
    } else
    {
        cout << "Invalid argument: Token=" << token << "  Value=" << value << endl;
        cout << "\n\t--LogMode=<JSON|TXT|SQL>\n\t--LogFileName=<string>\n\t--NodeName=<string>"
            << "\n\t--EthStat=<1|0>\t\tActivate ethtool stats"
            << "\n\t--Nstat=<1|0>\t\tActivate nstat stats"
            << "\n\t--ServerMode\t\tActivate server mode\n\t--ServerPort=<int>\t\tSocket TCP Port"
            << "\n\t--SampplingTime=<int>\t\tSamppling Stats Intervall (default each one second)"
            << "\n\t--LogIntervall=<int>\t\tLogging intervall (default each one second)"
            << "\n\t--CPUPathStat=\\path\n\t--CPUPathArch=\\path\n\t--NetworkPathStat=\\path"
            << "\n\t--ProcessNames=process1,process2,..\t\tMonitored processess"
            << "\n\t--DockerNames=< * or docker1,docker2,.. >\t\tMonitored Dockers"
            << "\n\t--tag=<string>\t\tIdentification of experiment"
            << "\n\t--ExpTime=<int>\t\tSeconds of monitoring (default 0=infinite)\n\t--debugMode=<1|0>"
            << "\n\t--sinc\t\tSincronous mode\n\t--help\t\tThis help" << endl;
        return false;
    }
    return true;
}

bool readSetup(){
    bool error = false;
    ifstream File;
    string line;
    File.open ("setup.conf");
    if (File.is_open()) {
        string token;
        string value;
        while (!File.eof() && !error){
            getline(File, line);
            if (line[0] != '#' && line!="") {
                token = trim(line.substr(0, line.find("=")));
                value = trim(line.substr(line.find("=")+1, line.length()-1));
                error = !parseVar(token, value);
            }
        }
        File.close();
        return !error;
    } else {
        return false;
    }
}

bool readSetupFromCL(int argc, char *argv[]){
    int i;
    bool error = false;
    string token;
    string value;
    for (i=1; i<argc; i++) {
        string line(argv[i]);
        if (line[0] == '-' && line[1] == '-') {
            token = trim(line.substr(2, line.find("=")-2));
            value = trim(line.substr(line.find("=")+1, line.length()-1));
            error = !parseVar(token, value);
        } else {
            error = true;
        }
        if (error) break;
    }
    if (error) {
        return false;
    } else {
        return true;
    }
}
