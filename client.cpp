/* Created by Nathan Johnston for Siemens
 * Built to run in tandem with a Kuka LBR Robot we named Salma
 * Process is: run server.java on Salma, then run client.exe
 * on the MARS. Client.exe will ask for some information and then
 * connect to Salma's server. Once connected, Salma will move
 * to the first point and tell the MARS that it's moved. 
 * Salma will run lmacq, wait for that to finish, then run
 * Raptor85 in parallel while Salma moves to the next point. 
 * Once data has been acquired at all points, pslocate will run
 * as the last step.
 * This will only run on windows, though the only non portable code is 
 * the port I/O 
 * Dependencies: Nlohmann's C++ JSON parser, found at 
 * https://github.com/nlohmann/json */

#include "client.h"

using json = nlohmann::json;
using namespace std;

string server = "165.226.39.175";
int PORT = 30005;
string outFile = "point0";
string outFileExt = ".lmb";
int iters = 0;

/* <summary> 
 * Prints a custom error message as well as the system error message
 * String msg: custom error message 
 * </summary>
 * <param name = "er">Whether or not to print the system error message</param>
 * <param name = "ex">Whether or not to kill the program</param> */
void error(string msg, bool er=1, bool ex=1){
    fprintf(stderr, "Error: %s\n", msg.c_str());
    if(er){
        wchar_t *s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&s, 0, NULL);
        fprintf(stderr, "%S\n", s);
        LocalFree(s);
    }
    if(ex){
        system("pause");
        exit(-1);
    }
}

/* <summary> 
 * Converts an int to a string and prepends
 * a certain number of 0s
 * </summary>
 * <param name = "a"> The int to convert </param> */
string toString(int a){
    ostringstream temp;
    temp << a;
    if(a < 10)
        return "00" + temp.str();
    else if(a < 100)
        return "0" + temp.str();
    else
        return temp.str();
}

/* <summary>
 * Runs the command "lmacq.exe -f file_name_iteration.lmb -t time" then
 * waits for the command to finish returning to main()
 * </summary>
 * <param name = "time"> The time in seconds to acquire data </param> */
void lmacq(string time){
    PROCESS_INFORMATION pinfo;
    STARTUPINFO sinfo;
    ZeroMemory(&pinfo, sizeof(pinfo));
    ZeroMemory(&sinfo, sizeof(sinfo));
    string command = "lmacq.exe -f " + outFile + toString(iters) + ".lmb -t " + time;
    if(CreateProcess(NULL, (char *)command.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pinfo)){
        cout << "Running lmacq..." << endl;
        WaitForSingleObject(pinfo.hProcess, INFINITE);
        cout << "Done" << endl;
        CloseHandle(pinfo.hProcess);
        CloseHandle(pinfo.hThread);
    }
    else
        error("Failed to start lmacq");
}

/* <summary>
 * Runs the command "Raptor85.exe input_file.lmb output_file.s" in
 * parallel with main() 
 * </summary> */
PROCESS_INFORMATION raptor(PROCESS_INFORMATION pinfo){
    STARTUPINFO sinfo;
    ZeroMemory(&pinfo, sizeof(pinfo));
    ZeroMemory(&sinfo, sizeof(sinfo));
    string command = "Raptor85.exe " + outFile + toString(iters) + ".lmb " + outFile + toString(iters) + ".s";
    if(CreateProcess(NULL, (char *)command.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pinfo)){
        cout << "Running Raptor85..." << endl;
        return pinfo;
    }
    else
        error("Failed to start Raptor85", 1, 0);
}

void rebin(){
    PROCESS_INFORMATION pinfo;
    STARTUPINFO sinfo;
    ZeroMemory(&pinfo, sizeof(pinfo));
    ZeroMemory(&sinfo, sizeof(sinfo));
    string command = "Rebin.exe " + outFile + toString(iters) + ".lmb " + outFile + toString(iters) + ".s";
    if(CreateProcess(NULL, (char *)command.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pinfo)){
        cout << "Running Rebin..." << endl;
        CloseHandle(pinfo.hProcess);
        CloseHandle(pinfo.hThread);
    }
    else
        error("Failed to start Rebin", 1, 0);
}

/* <summary> 
 * Runs the command 
 * "pslocate.exe --filename input_file_iteration.s --raw_x 520 --raw_y 399 --raw_z 5189 --z_segm1 645"
 * for each .s file 
 * </summary> */
 //Raptor: --filename *.s --raw_x 520 --raw_y 399 --raw_z 5189 --z_segm1 645
 //Vision: --filename *.s --raw_x 520 --raw_y 399 --raw_z 1293 --z_segm1 159
void pslocate(string args){
    PROCESS_INFORMATION pinfo;
    STARTUPINFO sinfo;
    ZeroMemory(&pinfo, sizeof(pinfo));
    ZeroMemory(&sinfo, sizeof(sinfo));
    string command = "pslocate.exe " + args;
    if(CreateProcess(NULL, (char *)command.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pinfo)){
        cout << "Running pslocate..." << endl;
        WaitForSingleObject(pinfo.hProcess, INFINITE);
        CloseHandle(pinfo.hProcess);
        CloseHandle(pinfo.hThread);
    }
    else
        error("Failed to start pslocate");
}

void transform(){
    PROCESS_INFORMATION pinfo;
    STARTUPINFO sinfo;
    ZeroMemory(&pinfo, sizeof(pinfo));
    ZeroMemory(&sinfo, sizeof(sinfo));
    if(CreateProcess(NULL, (char *)"PSF_CoordTransformation.exe", NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pinfo)){
        cout << "Running PSF_CoordTransformation..." << endl;
        WaitForSingleObject(pinfo.hProcess, INFINITE);
        CloseHandle(pinfo.hProcess);
        CloseHandle(pinfo.hThread);
    }
    else
        error("Failed to start PSF_CoordTransformation");
}

/* <summary>
 * Connects to Salma via sockets then sends the specified json file
 * Then continuously loops through acquiring data until Salma signals
 * that there are no more points to move to. Finally runs pslocate 
 * </summary>
 * <param name = "-ip"> Command line arg to specify Salma's ip address </param> */
int main(int argc, char *argv[]){
    string fileToSend, time, file, mode, pslocateArgs;
    int size, s = 0, total = 0;
    char temp;
    bool runLmacq = false, runRaptor = false, runRebin = false, runPslocate = false, runTransform = false, calibrate = false;
    WSADATA WSAData;
    SOCKET sock;
    struct sockaddr_in addr;
    PROCESS_INFORMATION pinfo;

    for(int i=1; i<argc; i++){
        if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")){
            cout << "Command line arguments:" << endl
                 << "--ip: Salma's IP address to connect to" << endl
                 << "-h || --help: Displays this help information" << endl                 
                 << "-f || --file: The file to send to Salma (.csv or .json)" << endl
                 << "-t || --time: The time in seconds to run lmacq" << endl
                 << "--psargs: The argument string with which to run pslocate. Default is: " << endl
                 << "\t\"--filename *.s --raw_x 520 --raw_y 399 --raw_z 5189 --z_segm1 645\"" << endl
                 << "This MUST be enclosed in quotes. You can also use \"--psargs vision\" or" << endl
                 << "\"--psargs raptor\" for the default arguments needed for each system" << endl
                 << "-m || --mode: The operating mode to run the program in. Operating modes are:" << endl
                 << "\t1. lmacq" << endl
                 << "\t2. raptor85" << endl
                 << "\t3. rebin" << endl
                 << "\t4. pslocate" << endl
                 << "\t5. PSF_CoordTransformation" << endl
                 << "\t6. Only move to points" << endl
                 << "\t7. Calibrate Salma"
                 << "Example: \"--mode 1 3 4\" will run lmacq, rebin, and pslocate" << endl;
            return 0;
        }

        else if(!strcmp(argv[i], "--ip"))
            server = string(argv[++i]);
        
        else if(!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file"))
            file = argv[++i];
        
        else if(!strcmp(argv[i], "-m") || !strcmp(argv[i], "--mode")){
            mode = "";
            for(++i; i<argc; i++){
                if(argv[i][0] == '-')
                    break;
                else
                    mode += argv[i];
            }
            i--;
        }

        else if(!strcmp(argv[i], "-t") || !strcmp(argv[i], "--time"))
            time = argv[++i];
        
        else if(!strcmp(argv[i], "-c") || !strcmp(argv[i], "--calibrate")){
            calibrate = true;
            runLmacq = true;
            runTransform = true;
            runPslocate = true;
            break;
        }

        else if(!strcmp(argv[i], "--psargs"))
            pslocateArgs = argv[++i];
    
        else{
            cout << "Unknown argument: " << argv[i] << endl;
            cout << "Terminating..." << endl;
            system("pause");
            return 0;
        }
    }

    if(file.empty()){
        cout << "Enter file name (Enter full path if file is not in this directory): " << endl;
        getline(cin, file);
    }
    if(file.length() < 5 || (file.substr(file.length()-5, file.length()-1) != ".json" && file.substr(file.length()-4, file.length()-1) != ".csv"))
        error("File must be a JSON or CSV file", 0);
    
    if(mode.empty()){
        cout << "Select programs to run, each separated by a space (e.g. 1 3 4):" << endl
            << "1. lmacq" << endl
            << "2. raptor85" << endl
            << "3. rebin" << endl
            << "4. pslocate" << endl
            << "5. PSF_CoordTransformation" << endl
            << "6. Only move to points" << endl
            << "7. Calibrate" << endl;
        getline(cin, mode);
    }
    for(int i=0; i<mode.length(); i++){
        char c = mode[i];
        switch(c){
            case '1':
                runLmacq = true;
                break;
            case '2':
                runRaptor = true;
                break;
            case '3':
                runRebin = true;
                break;
            case '4':
                runPslocate = true;
                break;
            case '5':
                runTransform = true;
                break;
            case '7':
                runLmacq = true;
                runPslocate = true;
                runTransform = true;
                break;
            default:
                break;
        }
    }

    if(runLmacq && time.empty()){
        cout << "Enter a time in seconds for lmacq to run: " << endl;
        getline(cin, time);
    }

    if(runPslocate){
        cout << pslocateArgs << endl;
        if(pslocateArgs.empty()){
            cout << "Use default pslocate arguments for vision (y/n)? (--filename *.s --raw_x 520 --raw_y 399 --raw_z 1293 --z_segm1 159)" << endl;
            cin >> temp;
            if(temp == 'n'){
                cout << "Enter argument string to use:" << endl;
                getline(cin, pslocateArgs);
            }
            else
                pslocateArgs = "--filename *.s --raw_x 520 --raw_y 399 --raw_z 1293 --z_segm1 159";
        }
        else if(!strcmp(pslocateArgs.c_str(), "vision"))
            pslocateArgs = "--filename *.s --raw_x 520 --raw_y 399 --raw_z 1293 --z_segm1 159";
        else if(!strcmp(pslocateArgs.c_str(), "raptor"))
            pslocateArgs = "--filename *.s --raw_x 520 --raw_y 399 --raw_z 5189 --z_segm1 645";
        cout << pslocateArgs << endl;
    }
    
    if(WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
        error("WSAStartup failed");

    if(file[file.length()-1] == 'v'){   //If the file is a csv
        CSV csv(file);
        fileToSend = csv.convertToJSON();
        size = fileToSend.length();
    }
    else if(file[file.length()-1] == 'n'){   //If the file is a json
        ifstream fin(file.c_str());
        json j;
        fin >> j;
        fin.seekg(0, ios::beg);
        if(j.find("ip") != j.end())
            server = j["ip"];
        if(fin){
            ostringstream os;
            os << fin.rdbuf();
            fileToSend = os.str();
            size = fileToSend.length();
        }
        else
            error("Couldn't open file");
        fin.close();
    }
    else{
        error("Input file must be a json or csv");
    }

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("Error opening socket\n" + WSAGetLastError());

    addr.sin_addr.s_addr = inet_addr(server.c_str());
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
        error("Connection failed");

    if(calibrate){
        int calibrateCode = htonl(-1);
        int num = send(sock, (const char *)&calibrateCode, 4, 0);
        total += num;
    }
    int sizeToSend = htonl(size);
    int num = send(sock, (const char *)&sizeToSend, 4, 0);
    total += num;

    num = send(sock, fileToSend.data(), size, 0);
    total += num;
    cout << "Total bytes written: " << total << endl;
    char buffer[32];

    /* Main loop */
    while(true){
        num = recv(sock, buffer, sizeof(buffer), 0);
        buffer[num] = '\0';
        if(!strcmp(buffer, "Moved")){
            cout << "Collecting data..." << endl;
            if(runLmacq)
                lmacq(time);
            if(runRaptor)
                pinfo = raptor(pinfo);
            if(runRebin)
                rebin();
            iters++;
            string t = "True\n";
            cout << "Data collected. Telling Salma to move" << endl;
            int h = send(sock, t.data(), t.length(), 0);
            cout << "Sent" << endl;
        }
        else if(!strcmp(buffer, "Done")){
            if(calibrate){
                memset(buffer, 0, sizeof(buffer));
                continue;
            }
            break;
        }
        else if(!strcmp(buffer, "Calibrated"))
            break;
        else
            cout << buffer << endl;
        memset(buffer, 0, sizeof(buffer));
    }

    closesocket(sock);
    WSACleanup();
    if(runRaptor){
        WaitForSingleObject(pinfo.hProcess, INFINITE);
        CloseHandle(pinfo.hProcess);
        CloseHandle(pinfo.hThread);
    }
    if(runPslocate)
        pslocate(pslocateArgs);
    if(runTransform)
        transform();
    system("pause");
    return 0;
}
