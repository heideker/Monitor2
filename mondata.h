/**********************************************************
*                                                         *                                                         *
*  Author: Alexandre Heideker                             *
*  e-mail: alexandre.heideker@ufabc.edu.br                *
*  UFABC - 2019                                           *
*  statistic data structure class                         *
***********************************************************/

using namespace std;
#define STAT_USER 0
#define STAT_NICE 1
#define STAT_SYSTEM 2
#define STAT_IDLE 3
#define STAT_IOWAIT 4
#define STAT_IRQ 5
#define STAT_SOFTIRQ 6
#define STAT_STEAL 7
#define STAT_GUEST 8
#define STAT_GUEST_NICE 9

class MonData{
    private:
        
    public:
        bool debug = false;
        std::string CPUPathStat = "/proc/stat";
        std::string NetworkPathStat = "/proc/net";
        std::string CPUPathArch = "/proc/cpuinfo";
        std::vector <std::string> ProcessNames;
        std::vector <std::string> DockerNames;
        bool DockerStat = false;
        bool Ethtool = false;
        bool Nstat = false;
        unsigned int Timestamp; //timestamp
        long MemorySize;
        long MemoryAvailable;
        long cpuOldStats[10];
        long cpuStats[10];
        float cpuLevel;
        std::string arch;
        networkData netData;
        std::vector<networkAdapter> NetAdapters;
        std::vector<networkAdapter> NetOffset;
        std::vector<processData> Processes;
        std::vector<processData> Dockers;
        std::vector<ethtoolData> EthData;
        std::vector<nstatData> nstData;
        std::string getJsonStorage();
        std::string getJsonNetworkStats();
        std::string getJsonNetworkAdapters();
        std::string getJsonProcess();
        std::string getTxtStorage();
        std::string getTxtNetworkStats();
        std::string getTxtNetworkAdapters();
        std::string getTxtProcess();
        unsigned int delta = 0;
        unsigned int nqaSamp = 0;
        //unsigned char cnt5 = 0;
        unsigned int nqaCnt1;
        unsigned int nqaCnt5;
        bool nqa1 = false;
        bool nqa5 = false;
        void Refresh();
};

void MonData::Refresh(){
    this->delta++;
    this->nqaSamp++;
    bool flagNQA = false;
    //Refresh system data
    this->Timestamp = std::time(0);
    struct sysinfo memInfo;
    sysinfo (&memInfo);
    this->MemorySize = memInfo.totalram;
    this->MemoryAvailable = memInfo.freeram;
    std::ifstream File;
    std::string line;
    File.open (this->CPUPathStat);
    if (File.is_open()) {
        std::string cpuId;
        File >> cpuId >> this->cpuStats[STAT_USER]
                >> this->cpuStats[STAT_NICE]
                >> this->cpuStats[STAT_SYSTEM]
                >> this->cpuStats[STAT_IDLE]
                >> this->cpuStats[STAT_IOWAIT]
                >> this->cpuStats[STAT_IRQ]
                >> this->cpuStats[STAT_SOFTIRQ]
                >> this->cpuStats[STAT_STEAL]
                >> this->cpuStats[STAT_GUEST]
                >> this->cpuStats[STAT_GUEST_NICE];
        long cpuOldIdle, cpuIdle, cpuOldAct, cpuAct, totalOld, total, totalDelta, idleDelta;

        cpuOldIdle = this->cpuOldStats[STAT_IDLE] + this->cpuOldStats[STAT_IOWAIT];
        cpuIdle = this->cpuStats[STAT_IDLE] + this->cpuStats[STAT_IOWAIT];

        cpuOldAct = this->cpuOldStats[STAT_USER] + this->cpuOldStats[STAT_NICE] + this->cpuOldStats[STAT_SYSTEM] + 
                    this->cpuOldStats[STAT_IRQ] + this->cpuOldStats[STAT_SOFTIRQ] + this->cpuOldStats[STAT_STEAL];
        cpuAct = this->cpuStats[STAT_USER] + this->cpuStats[STAT_NICE] + this->cpuStats[STAT_SYSTEM] + 
                    this->cpuStats[STAT_IRQ] + this->cpuStats[STAT_SOFTIRQ] + this->cpuStats[STAT_STEAL];
        totalOld = cpuOldIdle + cpuOldAct;
        total = cpuIdle + cpuAct;
        totalDelta = total - totalOld;
        idleDelta = cpuIdle - cpuOldIdle;

        if (totalDelta!=0) this->cpuLevel = (float) (totalDelta - idleDelta) / totalDelta; 
        if (this->debug) cout << cpuId << "  " << this->cpuLevel << endl;
        for (int j=0; j<10; j++){
            this->cpuOldStats[j] = this->cpuStats[j];
        }
        File.close();
    }

    //socket statistics
    networkData nd;
    nd.TCPtxQueue = 0;
    nd.TCPrxQueue = 0;
    nd.TCPMaxWindowSize = 0;
    nd.UDPtxQueue = 0;
    nd.UDPrxQueue = 0;
    nd.RxRing = 0;
    nd.TxRing = 0;
    std::string trash;
    File.open (this->NetworkPathStat + "/tcp");
    if (File.is_open()) {
        //header
        getline(File, line);
        while (! File.eof()){
            getline(File, line);
            replaceAll(line, ":", " ");
            std::stringstream ss(line);
            unsigned long v = 0;
            ss >> trash >> trash >> trash >> trash >> trash >> trash >> v;
            nd.TCPtxQueue += v;
            ss >> v;
            nd.TCPrxQueue += v;
            ss >> trash >> trash >> trash >> trash >> trash >> trash >> trash >> trash >> trash >> trash >> trash >> v;
            if (v > nd.TCPMaxWindowSize) nd.TCPMaxWindowSize = v;
        }
        File.close();
    }
    File.open (this->NetworkPathStat + "/tcp6");
    if (File.is_open()) {
        //header
        getline(File, line);
        while (! File.eof()){
            getline(File, line);
            replaceAll(line, ":", " ");
            std::stringstream ss(line);
            long v = 0;
            ss >> trash >> trash >> trash >> trash >> trash >> trash >> v;
            nd.TCPtxQueue += v;
            ss >> v;
            nd.TCPrxQueue += v;
            ss >> trash >> trash >> trash >> trash >> trash >> trash >> trash >> trash >> trash >> trash >> trash >> v;
            if (v > nd.TCPMaxWindowSize) nd.TCPMaxWindowSize = v;
        }
        File.close();
    }
    File.open (this->NetworkPathStat + "/udp");
    if (File.is_open()) {
        //header
        getline(File, line);
        while (! File.eof()){
            getline(File, line);
            replaceAll(line, ":", " ");
            std::stringstream ss(line);
            long v = 0;
            ss >> trash >> trash >> trash >> trash >> trash >> trash >> v;
            nd.UDPtxQueue += v;
            ss >> v;
            nd.UDPrxQueue += v;
        }
        File.close();
    }
    File.open (this->NetworkPathStat + "/udp6");
    if (File.is_open()) {
        //header
        getline(File, line);
        while (! File.eof()){
            getline(File, line);
            replaceAll(line, ":", " ");
            std::stringstream ss(line);
            long v = 0;
            ss >> trash >> trash >> trash >> trash >> trash >> trash >> v;
            nd.UDPtxQueue += v;
            ss >> v;
            nd.UDPrxQueue += v;
        }
        File.close();
    }


    std::string stringOut;

    //Nstat statistics
    this->nstData.clear();
    if (this->Nstat) {
        stringOut = run("nstat -z 2>null");
        //if (this->debug) cout << "nstat stats: " << stringOut << endl;
        std::stringstream ss(stringOut);
        std::string param;
        std::string value;
        std::string avg;
        nstatData ntd;
        if (ss >> param) {
            while (ss >> param >> value >> avg){
                if (this->debug) cout << param << "\t" << value <<  "\t" << avg << endl;
                ntd.parameter = param;
                ntd.value = atol(value.c_str());
                this->nstData.push_back(ntd);
            }
        }
    }


    //reading network adapters stats...
    this->NetAdapters.clear();
    File.open (this->NetworkPathStat + "/dev");
    if (File.is_open()) {
        //header
        getline(File, line);
        getline(File, line);
        networkAdapter na;
        bool offset = false;
        this->EthData.clear();
        if (this->NetOffset.size()>0) offset = true;
        while (! File.eof()){
            getline(File, line);
            if (line !="") {
                replaceAll(line, ":", " ");
                if (!offset) {
                    networkAdapter naos;
                    std::stringstream ss(line);
                    ss >> naos.Name >> naos.RxBytes >> naos.RxPackets >> naos.RxErrors;
                    ss >> naos.RxDrop >> naos.RxFifo >> trash >> trash >> trash;
                    ss >> naos.TxBytes >> naos.TxPackets >> naos.TxErrors;
                    ss >> naos.TxDrop >> naos.TxFifo;
                    this->NetOffset.push_back (naos);
                }
                std::stringstream ss(line);
                ss >> na.Name >> na.RxBytes >> na.RxPackets >> na.RxErrors;
                ss >> na.RxDrop >> na.RxFifo >> trash >> trash >> trash;
                ss >> na.TxBytes >> na.TxPackets >> na.TxErrors;
                ss >> na.TxDrop >> na.TxFifo;
                //appling offset
                for (auto p: this->NetOffset) {
                    if (p.Name==na.Name) {
                        na.RxBytes -= p.RxBytes;
                        na.RxPackets -= p.RxPackets;
                        na.RxErrors -= p.RxErrors;
                        na.RxDrop -= p.RxDrop;
                        na.RxFifo -= p.RxFifo;
                        na.TxBytes -= p.TxBytes;
                        na.TxPackets -= p.TxPackets;
                        na.TxErrors -= p.TxErrors;
                        na.TxDrop -= p.TxDrop;
                        na.TxFifo -= p.TxFifo;
                        break;
                    }
                }
                na.rxring = 0;
                na.txring = 0;

                //Ethtool statistics
                if (this->Ethtool) {
                    //register dump
                    char buff[128];
                    int i = runBin("ethtool -d " + na.Name +" raw on 2>null", buff, 128);
                    std::string param;
                    std::string value;
                    ethtoolData etd;
                    NICdump ndd;
                    if (i == 128) {
                        memset(&ndd, '\0', sizeof(ndd));
                        memcpy(&ndd, buff, sizeof(ndd));
                        etd.NIC = na.Name;
                        etd.parameter = "rdlen";
                        etd.value = ndd.rdlen;
                        this->EthData.push_back(etd);
                        etd.parameter = "rdh";
                        etd.value = ndd.rdh;
                        this->EthData.push_back(etd);
                        etd.parameter = "rdt";
                        etd.value = ndd.rdt;
                        this->EthData.push_back(etd);
                        etd.parameter = "tdlen";
                        etd.value = ndd.tdlen;
                        this->EthData.push_back(etd);
                        etd.parameter = "tdh";
                        etd.value = ndd.tdh;
                        this->EthData.push_back(etd);
                        etd.parameter = "tdt";
                        etd.value = ndd.tdt;
                        this->EthData.push_back(etd);
                        na.rxring = (ndd.rdh>=ndd.rdt? ndd.rdh-ndd.rdt : ndd.rdlen - (ndd.rdt - ndd.rdh));
                        if (na.rxring==2) na.rxring=0;
                        nd.RxRing += na.rxring;
                        na.txring = (ndd.tdh>=ndd.tdt? ndd.tdh-ndd.tdt : ndd.tdlen - (ndd.tdt - ndd.tdh));
                        nd.TxRing += na.txring;
                    }

                }
                this->NetAdapters.push_back (na);
            }
        }
        File.close();
    }

    if (nd.TCPrxQueue > 0 || nd.TCPtxQueue > 0 || nd.UDPrxQueue > 0 || nd.UDPtxQueue > 0 || nd.RxRing > 0 || nd.TxRing > 0 ) 
        flagNQA = true;

    this->netData = nd;


    //reading architeture info...
    File.open (this->CPUPathArch);
    if (File.is_open()) {
        while (! File.eof()){
            getline(File, line);
            if (line.substr(0, 10)=="model name"){
                this->arch = trim(ReplaceForbidden(line.substr(line.find(":")+2, line.length()-1)));
                break;
            }
        }
        File.close();
    }
    //reading processes stats...using ps shell command
    processData ps;
    if (this->ProcessNames.size()>1 || (this->ProcessNames.size()==1 && this->ProcessNames[0]!="") ) {
        this->Processes.clear();
        for (auto p: this->ProcessNames) {
            stringOut = run("ps  --no-headers -C " + p + " -o pid,%cpu,rss,vsz");
            std::stringstream ss(stringOut);
            while (ss >> ps.pid >> ps.cpu >> ps.DataMem >> ps.VirtMem){
                ps.Name = ReplaceForbidden(p);
                ps.DataMem *= 1000;
                ps.VirtMem *= 1000;
                if (this->debug) cout << ps.Name << "\t" << ps.cpu << "\t" << ps.DataMem << endl;
                this->Processes.push_back (ps);
            }
        }
    }
    if (this->DockerStat) {
        this->Dockers.clear();
        stringOut = run("docker stats --all --no-stream --format \"{{.Name}};{{.CPUPerc}};{{.MemUsage}}\"");
        if (this->debug) cout << "Docker Stats: " << stringOut << endl;
        if (stringOut.size()>0) {
            replaceAll(stringOut, " / ", ";");
            replaceAll(stringOut, "MiB", "E06");
            replaceAll(stringOut, "GiB", "E09");
            replaceAll(stringOut, "B", "");
            replaceAll(stringOut, "%", "");
            replaceAll(stringOut, ";", " ");
            std::stringstream ss(stringOut);
            float dm, n;
            ps.pid=0;
            ps.VirtMem=0;
            while (ss >> ps.Name >> ps.cpu >> dm >> n){
                ps.DataMem = (long) dm;
                if (this->debug) cout << ps.Name << "\t" << ps.cpu << "\t" << ps.DataMem << endl;
                if (this->DockerNames.size()>0) {
                    if (this->DockerNames[0] == "*") {
                        this->Dockers.push_back(ps);
                    } else {
                        for (auto d: this->DockerNames) {
                            if (d == ps.Name)
                                this->Dockers.push_back (ps);
                        }  
                    }
                }
            }
        }   
    }
    if (flagNQA) {
        this->nqaCnt1++;
        if (this->nqaSamp >= 5) {
            this->nqaCnt5++;
        }
    }
    if (this->nqaSamp >= 5) this->nqaSamp = 0;
    if (this->delta >= 30) {
        this->nqa1 = (this->nqaCnt1 == 30);
        this->nqa5 = (this->nqaCnt5 == 6);
        this->nqaCnt1 = 0;
        this->nqaCnt5 = 0;
        this->delta = 0;        
    }

}
std::string MonData::getJsonStorage(){
    //Not implemented
    return "[]";
}
std::string MonData::getTxtStorage(){
    return "Storage";
}
std::string MonData::getJsonNetworkStats(){
    std::ostringstream s;
    s << "{\"TCPrxQueue\":" << this->netData.TCPrxQueue 
        << ", \"TCPtxQueue\":" << this->netData.TCPtxQueue 
        << ", \"TCPMaxWindowSize\":" << this->netData.TCPMaxWindowSize 
        << ", \"UDPrxQueue\":" << this->netData.TCPtxQueue 
        << ", \"UDPtxQueue\":" << this->netData.TCPtxQueue << "}";
    return s.str();
}
std::string MonData::getJsonNetworkAdapters(){
    std::ostringstream s;
    s << "[";
    bool cy = false;
    for (auto p: this->NetAdapters) {
        if (cy) s << ",";
        cy = true;
        s << "{\"name\":\"" << p.Name << "\", \"rxBytes\":" << p.RxBytes 
            << ", \"rxPackets\":" << p.RxPackets
            << ", \"rxErrors\":" << p.RxErrors
            << ", \"txBytes\":" << p.TxBytes 
            << ", \"txPackets\":" << p.TxPackets
            << ", \"txErrors\":" << p.TxErrors << "}";
    }
    s << "]";
    return s.str();
}
std::string MonData::getTxtNetworkStats(){
    return "Network";
}
std::string MonData::getTxtNetworkAdapters(){
    return "Network";
}
std::string MonData::getJsonProcess(){
    std::ostringstream s;
    s << "[";
    bool cy = false;
    for (auto p: this->Processes) {
        if (cy) s << ",";
        cy = true;
        s << "{\"type\":\"system\", \"pid\":" << p.pid << ", \"name\": \"" << p.Name
            << "\", \"memory\": " << p.DataMem << ", \"cpu\":" << p.cpu << "}";
    }
    for (auto p: this->Dockers) {
        if (cy) s << ",";
        cy = true;
        s << "{\"type\":\"docker\", \"pid\":" << p.pid << ", \"name\": \"" << p.Name
            << "\", \"memory\": " << p.DataMem << ", \"cpu\":" << p.cpu << "}";
    }
    s << "]";
    return s.str();
}
std::string MonData::getTxtProcess(){
    return "Process";
}




