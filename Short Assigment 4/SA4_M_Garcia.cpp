#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <cctype>
#include <iomanip>
#include <sstream>
using namespace std;

// trimming trailing zeros from floating-point numbers
static string formatFloat(double val) {
    if (val == (long long)val)
        return to_string((long long)val);
    ostringstream oss;
    oss << fixed << setprecision(6) << val;
    string s = oss.str();
    size_t n = s.size();
    while (n > 0 && s[n - 1] == '0') n--;
    if (n > 0 && s[n - 1] == '.') n--;
    return s.substr(0, n);
}

// 0=skip, 1=integer, 2=float, 3=invalid float, 4=invalid exp
int parseNumericLiteral(const string& line, size_t start, string& token) {
    token = "";
    if (start >= line.length()) return 0;
    size_t i = start;
    char c = line[i];
    if (c != '+' && c != '-' && c != '.' && !isdigit(c))
        return 0;

    size_t begin = i;
    if (c == '+' || c == '-') {
        i++;
        if (i >= line.length()) { token = line.substr(begin, 1); return 0; }
        c = line[i];
    }
    if (c == '.') return 0;
    if (!isdigit(c)) return 0;
    while (i < line.length() && isdigit(line[i])) i++;
    if (i >= line.length()) {
        token = line.substr(begin, i - begin);
        return 1;
    }
    c = line[i];
    if (c != '.' && c != 'e' && c != 'E') {
        token = line.substr(begin, i - begin);
        return 1;
    }
    if (c == '.') {
        i++;
        if (i >= line.length() || !isdigit(line[i])) {
            token = line.substr(begin, i - 1 - begin);
            return 1;
        }
        while (i < line.length() && isdigit(line[i])) i++;
        if (i < line.length() && line[i] == '.') {
            token = line.substr(begin, i - begin) + ".";
            return 3;
        }
        if (i < line.length() && (line[i] == 'e' || line[i] == 'E')) {
            i++;
            if (i < line.length() && (line[i] == '+' || line[i] == '-')) i++;
            size_t exp_start = i;
            while (i < line.length() && isdigit(line[i])) i++;
            if (i == exp_start) {
                while (i < line.length()) {
                    char t = line[i];
                    if (t == 'e' || t == 'E') { i++; token = line.substr(begin, i - begin); return 4; }
                    if (t == '+' || t == '-') { i++; token = line.substr(begin, i - begin); return 4; }
                    if (isdigit(t) || t == '.') break;
                    i++;
                }
                token = line.substr(begin, i - begin);
                return 4;
            }
            if (i < line.length() && (line[i] == '+' || line[i] == '-')) {
                i++; token = line.substr(begin, i - begin); return 4;
            }
            if (i < line.length() && (line[i] == 'e' || line[i] == 'E')) {
                i++; token = line.substr(begin, i - begin); return 4;
            }
            token = line.substr(begin, i - begin);
            return 2;
        }
        token = line.substr(begin, i - begin);
        return 2;
    }
    if (c == 'e' || c == 'E') {
        i++;
        if (i < line.length() && (line[i] == '+' || line[i] == '-')) i++;
        size_t exp_start = i;
        while (i < line.length() && isdigit(line[i])) i++;
        if (i == exp_start) {
            while (i < line.length()) {
                char t = line[i];
                if (t == 'e' || t == 'E') { i++; token = line.substr(begin, i - begin); return 4; }
                if (t == '+' || t == '-') { i++; token = line.substr(begin, i - begin); return 4; }
                if (isdigit(t) || t == '.') break;
                i++;
            }
            token = line.substr(begin, i - begin);
            return 4;
        }
        if (i < line.length() && (line[i] == '+' || line[i] == '-')) {
            i++; token = line.substr(begin, i - begin); return 4;
        }
        if (i < line.length() && (line[i] == 'e' || line[i] == 'E')) {
            i++; token = line.substr(begin, i - begin); return 4;
        }
        token = line.substr(begin, i - begin);
        return 2;
    }
    token = line.substr(begin, i - begin);
    return 1;
}

int main(int argc, char* argv[]) {
    string FileName;
    bool flagAll = false;
    bool flagInt = false;
    bool flagReal = false;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg[0] == '-') {
            if (arg == "-all") flagAll = true;
            else if (arg == "-int") flagInt = true;
            else if (arg == "-real") flagReal = true;
            else {
                cout << "UNRECOGNIZED FLAG " << arg << endl;
                return 0;
            }
        }
        else if (FileName.empty()) FileName = arg;
    }

    if (FileName.empty()) {
        cout << "NO SPECIFIED INPUT FILE NAME." << endl;
        return 0;
    }

    ifstream file(FileName);
    if (!file.is_open()) {
        cout << "CANNOT OPEN THE FILE " << FileName << endl;
        return 0;
    }

    string line;
    getline(file, line);
    if (line.empty() && file.eof()) {
        cout << "File is empty." << endl;
        file.close();
        return 0;
    }

    int TotalLines = 1;
    int InvalidCount = 0;
    map<string, int> AllOccurrences;
    map<string, long long> IntegerValues;
    map<string, double> FloatValues;

    for (;;) {
        size_t pos = 0;
        while (pos < line.length()) {
            string token;
            int kind = parseNumericLiteral(line, pos, token);
            if (kind == 0) {
                pos++;
                continue;
            }
            if (kind == 1) {
                string key = (token.length() > 0 && (token[0] == '+' || token[0] == '-')) ? token.substr(1) : token;
                AllOccurrences[key]++;
                long long val = stoll(token);
                IntegerValues[key] = (val < 0) ? -val : val;
                pos += token.length();
                continue;
            }
            if (kind == 2) {
                string key = (token.length() > 0 && (token[0] == '+' || token[0] == '-')) ? token.substr(1) : token;
                AllOccurrences[key]++;
                double val = stod(token);
                FloatValues[key] = (val < 0) ? -val : val;
                pos += token.length();
                continue;
            }
            if (kind == 3) {
                string errToken = (token.length() > 0 && (token[0] == '+' || token[0] == '-')) ? token.substr(1) : token;
                cout << "Line " << TotalLines << ": Invalid floating-point literal \"" << errToken << "\"" << endl;
                InvalidCount++;
                pos += token.length();
                continue;
            }
            if (kind == 4) {
                string errToken = (token.length() > 0 && (token[0] == '+' || token[0] == '-')) ? token.substr(1) : token;
                cout << "Line " << TotalLines << ": Invalid exponent for a numeric literal: \"" << errToken << "\"" << endl;
                InvalidCount++;
                pos += token.length();
                continue;
            }
            pos++;
        }
        if (!getline(file, line)) break;
        TotalLines++;
    }

    cout << "Total Number of Lines: " << TotalLines << endl;
    cout << "Number of Integer Literals: " << IntegerValues.size() << endl;
    cout << "Number of Floating-Point Literals: " << FloatValues.size() << endl;

    if (flagAll || flagInt || flagReal) cout << endl;
    if (flagAll) {
        cout << "List of All Numeric Literals and their Number of Occurrences:" << endl;
        for (auto& p : AllOccurrences)
            cout << "\"" << p.first << "\": " << p.second << endl;
        if (flagInt || flagReal) cout << endl;
    }
    if (flagInt) {
        cout << "List of Integer Literals and their Values:" << endl;
        for (auto& p : IntegerValues)
            cout << "\"" << p.first << "\": " << p.second << endl;
        if (flagReal) cout << endl;
    }
    if (flagReal) {
        cout << "List of Floating-Point Literals and their Values:" << endl;
        for (auto& p : FloatValues)
            cout << "\"" << p.first << "\": " << formatFloat(p.second) << endl;
    }

    file.close();
    return 0;
}
