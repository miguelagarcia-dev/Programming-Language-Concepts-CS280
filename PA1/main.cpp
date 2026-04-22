
#include <iostream>
#include <fstream>
#include <map>
#include "lex.h"
using namespace std;


int main(int argc, char* argv[]) {

    //  parseing did command-line arguments 
    string fileName;
    bool printAll = false;
    bool printNums = false;
    bool printStrs = false;
    bool printIds  = false;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (arg[0] == '-') {
            // it's a flag
            if (arg == "-all")      printAll  = true;
            else if (arg == "-num") printNums = true;
            else if (arg == "-str") printStrs = true;
            else if (arg == "-ids") printIds  = true;
            else {
                cout << "UNRECOGNIZED FLAG {" << arg << "}" << endl;
                return 0;
            }
        } else {
            // it's a filename
            if (!fileName.empty()) {
                cout << "ONLY ONE FILE NAME IS ALLOWED." << endl;
                return 0;
            }
            fileName = arg;
        }
    }

    if (fileName.empty()) {
        cout << "NO SPECIFIED INPUT FILE." << endl;
        return 0;
    }

    ifstream inputFile(fileName);
    if (!inputFile.is_open()) {
        cout << "CANNOT OPEN THE FILE " << fileName << endl;
        return 0;
    }


    // --- tokenize the file ---
    int linenum    = 1;
    int totalToks  = 0;
    int boolToks   = 0;  // count of TRUE/FALSE tokens
    int strToks    = 0;  // count of SCONST tokens

    // maps for counting occurrences of each unique lexeme
    map<string, int> identMap;   // identifiers
    map<string, int> kwMap;      // keywords
    map<string, int> intMap;     // integer constants
    map<string, int> realMap;    // real constants
    map<string, int> strMap;     // string constants

    while (true) {
        LexItem tok = getNextToken(inputFile, linenum);
        Token tk = tok.GetToken();

        if (tk == DONE)
            break;

        if (tk == ERR) {
            // print the error and stop — no summary on error
            cout << tok;
            return 0;
        }

        totalToks++;

        // -all: print every token as we see it
        if (printAll)
            cout << tok;

        // bucket the token into the right category
        if (tk == IDENT) {
            identMap[tok.GetLexeme()]++;
        }
        else if (tk == TRUE || tk == FALSE) {
            boolToks++;
        }
        else if (tk == ICONST) {
            intMap[tok.GetLexeme()]++;
        }
        else if (tk == RCONST) {
            realMap[tok.GetLexeme()]++;
        }
        else if (tk == SCONST) {
            strMap[tok.GetLexeme()]++;
            strToks++;
        }
        else if (tk == PROGRAM || tk == VAR   || tk == CONST   || tk == IF    ||
                 tk == THEN    || tk == ELSE   || tk == BEGIN   || tk == END   ||
                 tk == WRITE   || tk == WRITELN|| tk == READLN  || tk == INTEGER||
                 tk == REAL    || tk == BOOLEAN|| tk == CHAR    || tk == STRING ||
                 tk == AND     || tk == OR     || tk == NOT     || tk == IDIV  ||
                 tk == MOD) {
            kwMap[tok.GetLexeme()]++;
        }
    }

    // empty file check
    if (totalToks == 0) {
        cout << "Empty File." << endl;
        return 0;
    }

    // tally up the totals for the summary
    int identTotal = 0;
    for (auto& p : identMap)  identTotal += p.second;

    int kwTotal = 0;
    for (auto& p : kwMap)     kwTotal += p.second;

    int intTotal = 0;
    for (auto& p : intMap)    intTotal += p.second;

    int realTotal = 0;
    for (auto& p : realMap)   realTotal += p.second;

    // blank line before the summary block (always)
    cout << "\n";

    // linenum gets incremented for each \n (including trailing newline in file),
    // so subtract 1 to get the actual number of lines
    cout << "Lines: "                 << (linenum - 1)              << "\n";
    cout << "Total Tokens: "          << totalToks                  << "\n";
    cout << "Identifiers & Keywords: "<< (identTotal + kwTotal)     << "\n";
    cout << "Numbers: "               << (intTotal + realTotal)     << "\n";
    cout << "Booleans: "              << boolToks                   << "\n";
    cout << "Strings: "               << strToks                    << "\n";


    // --- optional flag output (order: -ids, -num, -str) ---
    bool anyFlagOn = printIds || printNums || printStrs;
    if (!anyFlagOn)
        return 0;

    // extra blank line between stats and flag sections — only when -all was printing above
    if (printAll)
        cout << "\n";

    // -ids: list identifiers then keywords (alphabetical, via map ordering)
    if (printIds) {
        if (!identMap.empty()) {
            cout << "IDENTIFIERS:\n";
            bool first = true;
            for (auto& p : identMap) {
                if (!first) cout << ", ";
                cout << p.first << " (" << p.second << ")";
                first = false;
            }
            cout << "\n";
        }
        if (!kwMap.empty()) {
            cout << "KEYWORDS:\n";
            bool first = true;
            for (auto& p : kwMap) {
                if (!first) cout << ", ";
                cout << p.first << " (" << p.second << ")";
                first = false;
            }
            cout << "\n";
        }
    }

    // -num: list integers then reals (map sorts keys as strings, so "100" < "2" < "45")
    if (printNums) {
        if (!intMap.empty()) {
            cout << "INTEGERS:\n";
            bool first = true;
            for (auto& p : intMap) {
                if (!first) cout << ", ";
                cout << p.first << " (" << p.second << ")";
                first = false;
            }
            cout << "\n";
        }
        if (!realMap.empty()) {
            cout << "REALS:\n";
            bool first = true;
            for (auto& p : realMap) {
                if (!first) cout << ", ";
                cout << p.first << " (" << p.second << ")";
                first = false;
            }
            cout << "\n";
        }
    }

    // -str: list string constants with surrounding single quotes
    if (printStrs) {
        if (!strMap.empty()) {
            cout << "STRINGS:\n";
            bool first = true;
            for (auto& p : strMap) {
                if (!first) cout << ", ";
                cout << "'" << p.first << "' (" << p.second << ")";
                first = false;
            }
            cout << "\n";
        }
    }

    return 0;
}
