/* Created by Nathan Johnston for Siemens
 * Built to run in tandem with a Kuka LBR Robot named Salma
 * Process is: run server.java on Salma, then run client.exe
 * on the MARS. Client.exe will ask for some information and then
 * connect to Salma's server. Once connected, Salma will move
 * to the first point and tell the MARS that it's moved. 
 * Salma will run lmacq, wait for that to finish, then run
 * Raptor85 in parallel while Salma moves to the next point. 
 * Once data has been acquired at all points, pslocate will run
 * as the last step. */

#include <iostream>
#include <unistd.h>
#include <sstream>
#include <Winsock2.h>
#include <string.h>
#include <sys/types.h>
#include <fstream>
#include <windows.h>
#include <errno.h>
#include <dirent.h>

using namespace std;

string server = "165.226.39.207";
int PORT = 30005;
string outFile = "test_point";
string outFileExt = ".lmb";
int count = 0;

/* Prints a custom error message as well as the system error message
 * String msg: custom error message
 * bool er: whether or not to print the system error message
 * bool ex: whether or not to kill the program */
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
        cout << "Press any key followed by enter to exit" << endl;
        cin >> msg;
        exit(-1);
    }
}

/* Converts an int to a string
 * int a: the int to convert */
string toString(int a){
    ostringstream temp;
    temp << a;
    return temp.str();
}

/* Runs the command "lmacq.exe -f file_name_iteration.lmb -t time" then
 * waits for the command to finish returning to main()
 * string time: the time in seconds to acquire data */
void lmacq(string time){
    PROCESS_INFORMATION pinfo;
    STARTUPINFO sinfo;
    ZeroMemory(&pinfo, sizeof(pinfo));
    ZeroMemory(&sinfo, sizeof(sinfo));
    string command = "lmacq.exe -f " + outFile + toString(count) + ".lmb -t " + time;
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

/* Runs the command "Raptor85.exe input_file.lmb output_file.s" in
 * parallel with main() */
void raptor(){
    PROCESS_INFORMATION pinfo;
    STARTUPINFO sinfo;
    ZeroMemory(&pinfo, sizeof(pinfo));
    ZeroMemory(&sinfo, sizeof(sinfo));
    string command = "Raptor85.exe " + outFile + toString(count) + ".lmb " + outFile + toString(count) + ".s";
    if(CreateProcess(NULL, (char *)command.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pinfo)){
        cout << "Running Raptor85..." << endl;
        CloseHandle(pinfo.hProcess);
        CloseHandle(pinfo.hThread);
    }
    else
        error("Failed to start Raptor 85", 1, 0);
}

/* Runs the command 
 * "pslocate.exe --filename input_file_iteration.s --raw_x 520 --raw_y 399 --raw_z 5189 --z_segm1 645"
 * for each .s file */
void pslocate(){
    for(int i=0; i<count; i++){
        PROCESS_INFORMATION pinfo;
        STARTUPINFO sinfo;
        ZeroMemory(&pinfo, sizeof(pinfo));
        ZeroMemory(&sinfo, sizeof(sinfo));
        string command = "pslocate.exe --filename " + outFile + toString(i) + ".s --raw_x 520 --raw_y 399 --raw_z 5189 --z_segm1 645";
        if(CreateProcess(NULL, (char *)command.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pinfo)){
            cout << "Running pslocate..." << endl;
            WaitForSingleObject(pinfo.hProcess, INFINITE);
            CloseHandle(pinfo.hProcess);
            CloseHandle(pinfo.hThread);
        }
        else
            error("Failed to start pslocate");
    }
}

/* Connects to Salma via sockets then sends the specified json file
 * Then continuously loops through acquiring data until Salma signals
 * that there are no more points to move to. Finally runs pslocate */
int main(){
    string file, fileToSend, fileName, time;
    int size, s = 0, total = 0;
    char r;
    bool runRaptor;
    WSADATA WSAData;
    SOCKET sock;
    struct sockaddr_in addr;

    cout << "Enter file name (Enter full path if file is not in this directory): " << endl;
    cin >> file;
    if(file.length() < 5 || file.substr(file.length()-5, file.length()-1) != ".json")
        error("File must be a .json file", 0);
        
    cout << "Enter a time in seconds for lmacq to run: " << endl;
    cin >> time;

    cout << "Would you like run lmacq exclusively? (y/n)" << endl;
    cin >> r;
    if(r == 'y')
        runRaptor = false;
    else
        runRaptor = true;

    if(WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
        error("WSAStartup failed");

    ifstream fin(file.c_str());
    if(fin){
        ostringstream os;
        os << fin.rdbuf();
        fileToSend = os.str();
        size = fileToSend.length();
    }
    else
        error("Couldn't open file");

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("Error opening socket\n" + WSAGetLastError());

    addr.sin_addr.s_addr = inet_addr(server.c_str());
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
        error("Connection failed");

    int sizeToSend = htonl(size);
    int num = send(sock, (const char *)&sizeToSend, 4, 0);
    total += num;

    num = send(sock, fileToSend.data(), size, 0);
    total += num;
    cout << "Total bytes written: " << total << endl;
    char buffer[32];

    /* Main loop */
    while(true){
        int num = recv(sock, buffer, sizeof(buffer), 0);
        buffer[num] = '\0';
        if(strcmp(buffer, "Moved") == 0){
            cout << "Collecting data..." << endl;
            lmacq(time);
            if(runRaptor)
                raptor();
            count++;
            string t = "True\n";
            cout << "Data collected. Telling Salma to move" << endl;
            int h = send(sock, t.data(), t.length(), 0);
            cout << "Sent" << endl;
        }
        else if(strcmp(buffer, "Done") == 0)
            break;
        else
            cout << buffer << endl;
        memset(buffer, 0, sizeof(buffer));
    }

    closesocket(sock);
    WSACleanup();
    if(runRaptor)
        pslocate();
    system("pause");
    return 0;
}
