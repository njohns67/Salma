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

void error(string msg, bool er=1, bool ex=1){
    fprintf(stderr, "Error: %s\n", msg);
    if(er){
        wchar_t *s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&s, 0, NULL);
        fprintf(stderr, "%S\n", s);
        LocalFree(s);
    }
    if(ex)
        exit(-1);
}

void checkFiles(){
    struct dirent *entry;
    string path = ".";
    DIR *dir = opendir(path.c_str());
    int i = 0;
    if (dir == NULL) {
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        cout << entry->d_name << endl;
        i++;
    }
    cout << "Number of files is " << i << endl;
    closedir(dir);
}

int lmacq(){
    PROCESS_INFORMATION pinfo;
    STARTUPINFO sinfo;
    ZeroMemory(&pinfo, sizeof(pinfo));
    ZeroMemory(&sinfo, sizeof(sinfo));

    if(CreateProcess(NULL, "sleep.exe", NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pinfo)){
        cout << "Running lmacq..." << endl;
        WaitForSingleObject(pinfo.hProcess, INFINITE);
        cout << "Done" << endl;
        CloseHandle(pinfo.hProcess);
        CloseHandle(pinfo.hThread);
        ZeroMemory(&pinfo, sizeof(pinfo));
        ZeroMemory(&sinfo, sizeof(sinfo));
    }
    else{
        error("Failed to start lmacq");
        return 1;
    }
    if(CreateProcess(NULL, "concurrent.exe", NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pinfo))
        cout << "Running Raptor85..." << endl;
    else
        error("Failed to start Raptor 85", 1, 0);
}

int main(){
    string file, fileToSend, fileName;
    int s = 0;
    int total = 0;
    int size;
    WSADATA WSAData;
    SOCKET sock;
    struct sockaddr_in addr;

    cout << "Enter file name (Enter full path if file is not in this directory): " << endl;
    cin >> file;
    if(file.length() < 5 || file.substr(file.length()-5, file.length()-1) != ".json")
        error("File must be a .json file", 0);
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
    while(true){
        int num = recv(sock, buffer, sizeof(buffer), 0);
        buffer[num] = '\0';
        cout << "Buffer is: " << buffer << endl;
        if(strcmp(buffer, "Moved") == 0){
            cout << "Collecting data..." << endl;
            lmacq();
            //Sleep(5000);
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
    return 0;
}
