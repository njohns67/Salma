#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>

using namespace std;

class CSV{
    public:
        CSV(string, bool);
        CSV(ifstream&, bool);
        friend ostream& operator<<(ostream&, const CSV);
        vector<string> operator[](string);
        vector<string> operator[](int);
        int numCols = 1;
        int numRows = 0;

    private:
        void parseString(istringstream&, bool);
        string *headers;
        map<string, vector<string>> colsName;
        vector<vector<string>> colsIndex;
        vector<vector<string>> rows;
};

void CSV::parseString(istringstream &ss, bool noHeaders){
    int index, lastIndex;
    string line;
    bool first = true;
    while(getline(ss, line)){
        index = 0, lastIndex = 0;
        if(first && !noHeaders){
            while((index = line.find(',', index+1)) != string::npos)
                numCols++;
            headers = new string[numCols];
            index = 0;
            for(int i=0; i<numCols; i++){
                vector<string> temp;
                index = line.find(',', lastIndex);  //TODO: Check what happens when there's not item e.g. item, item,, item
                string header = line.substr(lastIndex, (index - lastIndex));
                if(header[0] == ' ')
                    header = header.substr(1, header.length()-1);
                headers[i] = header;
                colsName.insert(pair<string, vector<string>>(header, temp));
                lastIndex = index + 1;
                colsIndex.push_back(temp);
            }
            first = false;
            continue;
        }

        vector<string> temp;
        for(int i=0; i<numCols; i++){
            index = line.find(',', lastIndex);
            string word = line.substr(lastIndex, (index-lastIndex));
            if(word[0] == ' ')
                word = word.substr(1, word.length()-1);
            temp.push_back(word);
            colsIndex[i].push_back(word);
            if(!noHeaders)
                colsName[headers[i]].push_back(word);
            lastIndex = index + 1;
        }
        numRows++;
        rows.push_back(temp);
    }
}
CSV::CSV(string csvString, bool noHeaders = 0){
    istringstream ss(csvString);
    parseString(ss, noHeaders);
}

CSV::CSV(ifstream &fin, bool noHeaders = 0){
    string buf;
    if(!fin.is_open()){
        cerr << "File not open" << endl;
        exit(1);
    }
    fin.seekg(0, ios::end);
    buf.reserve(fin.tellg());
    fin.seekg(0, ios::beg);
    buf.assign((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    fin.close();
    istringstream ss(buf);
    parseString(ss, noHeaders);
}

ostream& operator<<(ostream& os, CSV csv){
    for(int i=0; i<csv.numRows; i++){
        for(int j=0; j<csv.numCols; j++){
            if(j != 0)
                cout << ", ";
            cout << csv.rows[i][j];
        }
        cout << endl;
    }
    return os;
}

vector<string> CSV::operator[](string header){
    if(colsName.count(header) == 0){
        cerr << "Invalid column name: " + header << endl;
        exit(1);
    }
    return colsName[header];
}

vector<string> CSV::operator[](int index){
    if(colsIndex.size() < index){
        cerr << "Column index too large" << endl;
        exit(1);
    }
    return colsIndex[index];
}