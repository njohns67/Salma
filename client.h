#include <iostream>
//#include <unistd.h>
#include <sstream>
#include <Winsock2.h>
#include <string.h>
#include <sys/types.h>
#include <fstream>
#include <windows.h>
#include <errno.h>
#include <dirent.h>
#include <vector>
#include <map>
#include "json.hpp"

//using string = string;
using json = nlohmann::json;
using namespace std;

class CSV{
    public:
        CSV(string, bool = true, bool = false);
        string convertToJSON();
        friend ostream& operator<<(ostream&, const CSV);
        vector<string> operator[](string);
        vector<string> operator[](int);
        int numCols;
        int numRows;

    private:
        void parseString(istringstream&, bool);
        string *headers;
        map<string, vector<string> > colsName;
        vector<vector<string> > colsIndex;
        vector<vector<string> > rows;
        string jsonString;
};
