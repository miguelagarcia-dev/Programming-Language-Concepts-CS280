#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdlib>
using namespace std;

int main() {
    string FileName;
    cout << "Enter the name of a file to read from:" << endl;
    cin >> FileName;

    ifstream file(FileName);
    
    if (!file.is_open()) {
        cout << endl;
        cout << "File cannot be opened: " << FileName << endl;
        exit(1);
    }
    
    //counters and commands 
    int total = 0;
    int commented = 0;
    int valid = 0;
    int invalid = 0;
    int DIR = 0;
    int CD = 0;
    int COPY = 0;
    int DEL = 0;
    
    string line;
    int lineNumber = 0;
    while (getline(file, line)) {
        total++;
        lineNumber++;
        
        // getting first word by trimming 
        size_t start = line.find_first_not_of(" \t");
        if (start == string::npos) {
            // Empty / whitespace-only 
            continue;
        }
        
        string trimmedLine = line.substr(start);
        
        // checking if line is a comment 
        if (trimmedLine.length() >= 2 && trimmedLine.substr(0, 2) == "::") {
            commented++;
            continue;
        }
        
        //  REM checking 
        string upperTrimmed = trimmedLine;
        for (char& c : upperTrimmed) {
            c = toupper(c);
        }
        if (upperTrimmed.length() >= 3 && upperTrimmed.substr(0, 3) == "REM") {
            if (upperTrimmed.length() == 3 || isspace(upperTrimmed[3])) {
                commented++;
                continue;
            }
        }
        
        stringstream ss(trimmedLine);
        string command;
        string originalCommand;
        ss >> command;
        originalCommand = command;  //  original case 
        
        for (char& c : command) {
            c = toupper(c);
        }
        
        //checking commands
        bool isValid = false;
        if (command == "DIR") {
            DIR++;
            isValid = true;
        } else if (command == "CD") {
            CD++;
            isValid = true;
        } else if (command == "COPY") {
            COPY++;
            isValid = true;
        } else if (command == "DEL") {
            DEL++;
            isValid = true;
        }
    
        if (isValid) {
            valid++;
        } else {
            if (invalid == 0) {
                cout << endl; 
            }
            cout << "Error: Unrecognizable command in line " << lineNumber << ": " << originalCommand << endl;
            cout << endl;  
            invalid++;
        }
    }
    
    // checking if file is empty 
     if (total == 0) {
        cout << "File is empty." << endl;
        file.close();
        return 0;
    }
    
    if (invalid == 0) {
        cout << endl;  
    }
    
    cout << "Total lines: " << total << endl;
    cout << "Commented lines: " << commented << endl;
    cout << "Valid Command lines: " << valid << endl;
    cout << "Invalid Command lines: " << invalid << endl;
    cout << "DIR commands: " << DIR << endl;
    cout << "CD commands: " << CD << endl;
    cout << "COPY commands: " << COPY << endl;
    cout << "DEL commands: " << DEL << endl;
    
    file.close();
    return 0;
}