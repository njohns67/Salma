#include "client.h"

using namespace std;

void CSV::parseString(istringstream &ss, bool noHeaders){
    int index, lastIndex;
    string line;
    bool first = true;
    numCols = 1;
    numRows = 0;
    while(getline(ss, line)){
        index = 0, lastIndex = 0;
        if(first){
            first = false;
            while((index = line.find(',', index+1)) != string::npos)
                numCols++;
            for(int i=0; i<numCols; i++){
                vector<string> temp;
                colsIndex.push_back(temp);
            }
            if(!noHeaders){
                headers = new string[numCols];
                index = 0;
                for(int i=0; i<numCols; i++){
                    vector<string> temp;
                    index = line.find(',', lastIndex);  //TODO: Check what happens when there's not item e.g. item, item,, item
                    string header = line.substr(lastIndex, (index - lastIndex));
                    if(header[0] == ' ')
                        header.erase(0, 1);
                    headers[i] = header;
                    colsName.insert(pair<string, vector<string> >(header, temp));
                    lastIndex = index + 1;
                    colsIndex.push_back(temp);
                }
                continue;
            }
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
    convertToJSON();
}
CSV::CSV(string fileName, bool isFile, bool noHeaders){
    if(isFile){
        ifstream fin(fileName.c_str());
        string buf;
        fin.seekg(0, ios::end);
        buf.reserve(fin.tellg());
        fin.seekg(0, ios::beg);
        buf.assign((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
        fin.close();
        istringstream ss(buf);
        parseString(ss, noHeaders);
    }
    else{
        istringstream ss(fileName);
        parseString(ss, noHeaders);
    }
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

string CSV::convertToJSON(){
    jsonString = "{\n\t\"points\":[\n";
    for(int i=0; i<rows.size(); i++){
        jsonString += "\t\t{\n";
        for(int j=0; j<rows[i].size(); j++){
            jsonString += "\t\t\t\"" + headers[j] + "\": " + rows[i][j];
            if(j != rows[i].size()-1)
                jsonString += ",\n";
        }
        if(i != rows.size()-1)
            jsonString += "\n\t\t},\n";
        else
            jsonString += "\n\t\t}\n\t]\n}";
    }
    return jsonString;
}