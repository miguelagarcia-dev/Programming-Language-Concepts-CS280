#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <cctype>
using namespace std;

// checking if the part of the word starting after the symbol is letters, digits, or underscore
bool isValidStartingWord(const string& s, size_t start) {
    if (start >= s.length()) return false;
    for (size_t i = start; i < s.length(); i++) {
        char c = s[i];
        if (!isalnum(c) && c != '_')
            return false;
    }
    return true;
}

// checking the start type of the name (_,@,#)
int nameType(const string& word) {
    if (word.length() < 2) return 0;
    if (word[0] == '_' && isValidStartingWord(word, 1)) return 1;
    if (word[0] == '@' && isValidStartingWord(word, 1)) return 2;
    if (word[0] == '#' && isValidStartingWord(word, 1)) return 3;
    return 0;
}

int main(int argc, char* argv[]) {
    string FileName;
    bool flagAll = false;
    bool flagType1 = false;
    bool flagType2 = false;
    bool flagType3 = false;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-all") flagAll = true;
        else if (arg == "-t1") flagType1 = true;
        else if (arg == "-t2") flagType2 = true;
        else if (arg == "-t3") flagType3 = true;
        else if (FileName.empty()) FileName = arg;
    }

    // no file name given
    if (FileName.empty()) {
        cout << "NO SPECIFIED INPUT FILE NAME." << endl;
        return 0;
    }

    ifstream file(FileName);
    if (!file.is_open()) {
        cout << "CANNOT OPEN THE FILE " << FileName << endl;
        return 0;
    }

    // counters and commands
    int TotalWords = 0;
    int CountType1 = 0;
    int CountType2 = 0;
    int CountType3 = 0;
    set<string> UniqueType1;
    set<string> UniqueType2;
    set<string> UniqueType3;
    string word;

    // read words and classify (like getline loop in check, but word-by-word)
    while (file >> word) {
        TotalWords++;
        int typeOfWord = nameType(word);
        if (typeOfWord == 1) { CountType1++; UniqueType1.insert(word); }
        else if (typeOfWord == 2) { CountType2++; UniqueType2.insert(word); }
        else if (typeOfWord == 3) { CountType3++; UniqueType3.insert(word); }
    }

    // checking if file is empty (same idea as check.cpp: count was 0 after reading)
    if (TotalWords == 0) {
        cout << "The file is empty." << endl;
        file.close();
        return 0;
    }

    // output by flags
    bool NoFlags = !flagAll && !flagType1 && !flagType2 && !flagType3;
    if (NoFlags || flagAll) {
        cout << "Total number of words: " << TotalWords << endl;
    }
    if (flagAll) {
        cout << "Occurrences of Type1 Names (Starting with '_' character): " << CountType1 << endl;
        cout << "Occurrences of Type2 Names (Starting with '@' character): " << CountType2 << endl;
        cout << "Occurrences of Type3 Names (Starting with '#' character): " << CountType3 << endl;
    }
    if (flagType1) cout << "Count of Type1 Unique Names: " << UniqueType1.size() << endl;
    if (flagType2) cout << "Count of Type2 Unique Names: " << UniqueType2.size() << endl;
    if (flagType3) cout << "Count of Type3 Unique Names: " << UniqueType3.size() << endl;

    file.close();
    return 0;
}
