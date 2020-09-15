#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <windows.h>

using namespace std;

const string pointValues[] = {"x", "y", "z"};

vector<vector<string> > readCSV(string fileName){
    ifstream fin(fileName.c_str());
    if(!fin.is_open()){
        cerr << "Could not open file\n";
        exit(1);
    }
    vector<vector<string> > rows;
    string temp, line, word;
    getline(fin, line);
    while(getline(fin, line)){
        vector<string> row;
        stringstream s(line);
        while(getline(s, word, ',')){
            if(word[0] == ' ')
                word.erase(0, 1);
            row.push_back(word);
        }
        rows.push_back(row);
    }
    return rows;
}

string convertToJSON(vector<vector<string> > csv){
    string jsonString = "{\n\t\"points\":[\n";
    for(int i=0; i<csv.size(); i++){
        jsonString += "\t\t{\n";
        for(int j=0; j<csv[i].size(); j++){
            jsonString += "\t\t\t\"" + pointValues[j] + "\": " + csv[i][j];
            if(j != csv[i].size()-1)
                jsonString += ",\n";
        }
        if(i != csv.size()-1)
            jsonString += "\n\t\t},\n";
        else
            jsonString += "\n\t\t}\n\t]\n}";
    }
    cout << jsonString << endl;
    return jsonString;
}

void writeJSON(string jsonString, string fileName){
    ofstream fout(fileName.c_str());
    fout << jsonString;
    fout.close();
}


int main(){
    string fileName;
    char t;
    printf("Enter file name:\n");
    cin >> fileName;

    vector<vector<string> > rows = readCSV(fileName);
    for(int i=0; i<rows.size(); i++){
        for(int j=0; j<rows[i].size(); j++){
            cout << rows[i][j] << endl;
        }
    }
    string jsonString = convertToJSON(rows);
    writeJSON(jsonString, fileName.substr(0, fileName.length() - 4) + ".json");
}
