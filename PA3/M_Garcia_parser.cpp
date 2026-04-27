#include "parserInterpSP26.h"
#include <map>
#include <set>
#include <vector>
#include <limits>
#include <sstream>

using namespace std;

// tracks whether a name is declared
map<string, bool> defVar;
map<string, bool> defConst;

// holds the type of each variable, so we can typecheck assignments
map<string, Token> SymTable;

// stores the actual runtime value, so only populated after assignment
map<string, Value> ValTable;

// holds constant values, so filled in during ConstDef
map<string, Value> ConstTable;

// saves prog name, so we can catch things like "prog12 := x"
string ProgName = "";

// flips off when parsing a branch we're NOT taking, so an uninitialized var inside a skipped if-branch doesn't blow up
bool execMode = true;

namespace Parser {
    bool pushed_back = false;
    LexItem pushed_token;

    static LexItem GetNextToken(istream& in, int& line) {
        if (pushed_back) {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }

    static void PushBackToken(LexItem& t) {
        if (pushed_back) abort();
        pushed_back = true;
        pushed_token = t;
    }
}

static int error_count = 0;

int ErrCount() { return error_count; }

void ParseError(int line, string msg) {
    ++error_count;
    cout << line << ": " << msg << endl;
}


// 1. Prog ::= PROGRAM IDENT ; Block .
bool Prog(istream& in, int& line) {
    LexItem Tok = Parser::GetNextToken(in, line);

    if (Tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern.");
        cout << "(" << Tok.GetLexeme() << ")" << endl;
        return false;
    }
    if (Tok.GetToken() == DONE && Tok.GetLinenum() <= 1) {
        ParseError(line, "Empty File.");
        return false;
    }
    if (Tok != PROGRAM) {
        ParseError(line, "Missing PROGRAM Keyword.");
        return false;
    }

    Tok = Parser::GetNextToken(in, line);
    if (Tok != IDENT) {
        ParseError(line, "Missing Program Name.");
        return false;
    }

    ProgName = Tok.GetLexeme(); // saves it, so Variable() can reject it as an lvalue

    Tok = Parser::GetNextToken(in, line);
    if (Tok != SEMICOL) {
        ParseError(line, "Missing Semicolon in Program Header.");
        return false;
    }

    if (!Block(in, line)) {
        ParseError(line, "Incorrect Program Body.");
        return false;
    }

    Tok = Parser::GetNextToken(in, line);
    if (Tok != DOT) {
        Parser::PushBackToken(Tok);
    }

    cout << endl << endl;
    cout << "DONE" << endl;
    return true;
}


// 2. Block ::= [ DeclPart ] CompStmt
bool Block(istream& in, int& line) {
    LexItem Tok = Parser::GetNextToken(in, line);
    Token t = Tok.GetToken();

    // checks for optional const section
    if (t == CONST) {
        Parser::PushBackToken(Tok);
        if (!ConstPart(in, line)) {
            ParseError(line, "Incorrect Constant Definition Part.");
            return false;
        }
        Tok = Parser::GetNextToken(in, line);
        t = Tok.GetToken();
    }

    // checks for optional var section
    if (t == VAR) {
        Parser::PushBackToken(Tok);
        if (!VarPart(in, line)) {
            ParseError(line, "Incorrect Declaration Part.");
            return false;
        }
        Tok = Parser::GetNextToken(in, line);
        t = Tok.GetToken();
    }

    // BEGIN must follow, so push it back for CompStmt to consume
    if (t == BEGIN) {
        Parser::PushBackToken(Tok);
        if (!CompStmt(in, line)) {
            ParseError(line, "Incorrect Program Body.");
            return false;
        }
        return true;
    }

    ParseError(line, "Missing begin for program body.");
    return false;
}


// 4. ConstPart ::= CONST ConstDef { ; ConstDef } ;
bool ConstPart(istream& in, int& line) {
    LexItem Tok = Parser::GetNextToken(in, line);
    if (Tok != CONST) {
        ParseError(line, "Missing CONST keyword.");
        return false;
    }

    // first ConstDef is required, so fail right away if it's missing
    if (!ConstDef(in, line)) {
        ParseError(line, "Syntactic error in Constants Definitions Part.");
        return false;
    }

    Tok = Parser::GetNextToken(in, line);
    if (Tok != SEMICOL) {
        ParseError(line, "Missing semicolon in Constants Definitions Part.");
        return false;
    }

    // keeps reading more defs as long as we see an IDENT next
    Tok = Parser::GetNextToken(in, line);
    while (Tok == IDENT) {
        Parser::PushBackToken(Tok);
        if (!ConstDef(in, line)) {
            ParseError(line, "Syntactic error in Constants Definitions Part.");
            return false;
        }
        Tok = Parser::GetNextToken(in, line);
        if (Tok != SEMICOL) {
            ParseError(line, "Missing semicolon in Constants Definitions Part.");
            return false;
        }
        Tok = Parser::GetNextToken(in, line);
    }

    // whatever stopped the loop gets pushed back, so Block can consume it (VAR or BEGIN)
    Parser::PushBackToken(Tok);
    return true;
}


// 5. ConstDef ::= IDENT = Expr
bool ConstDef(istream& in, int& line) {
    LexItem Tok = Parser::GetNextToken(in, line);

    if (Tok != IDENT) {
        Parser::PushBackToken(Tok);
        return false;
    }

    string ConstName = Tok.GetLexeme();

    if (defConst.find(ConstName) != defConst.end()) {
        ParseError(line, "Constant Redefinition: " + ConstName);
        return false;
    }

    Tok = Parser::GetNextToken(in, line);
    if (Tok != EQ) {
        ParseError(line, "Incorrect constant definition syntax.");
        return false;
    }

    Value constVal;
    if (!Expr(in, line, constVal)) {
        ParseError(line, "Missing expression in constant definition.");
        return false;
    }

    defConst[ConstName] = true;
    ConstTable[ConstName] = constVal;
    return true;
}


// 6. VarPart ::= VAR DeclStmt { ; DeclStmt } ;
bool VarPart(istream& in, int& line) {
    LexItem Tok = Parser::GetNextToken(in, line);
    if (Tok != VAR) {
        ParseError(line, "Missing VAR keyword.");
        return false;
    }

    // first DeclStmt is required, so fail right away if it's missing
    if (!DeclStmt(in, line)) {
        ParseError(line, "Syntactic error in Declaration Block.");
        return false;
    }

    Tok = Parser::GetNextToken(in, line);
    if (Tok != SEMICOL) {
        ParseError(line, "Missing semicolon in Declaration Statement.");
        return false;
    }

    // keeps reading more decls as long as we see an IDENT next
    Tok = Parser::GetNextToken(in, line);
    while (Tok == IDENT) {
        Parser::PushBackToken(Tok);
        if (!DeclStmt(in, line)) {
            ParseError(line, "Syntactic error in Declaration Block.");
            return false;
        }
        Tok = Parser::GetNextToken(in, line);
        if (Tok != SEMICOL) {
            ParseError(line, "Missing semicolon in Declaration Statement.");
            return false;
        }
        Tok = Parser::GetNextToken(in, line);
    }

    // whatever stopped the loop gets pushed back, so Block can see BEGIN
    Parser::PushBackToken(Tok);
    return true;
}


// 7. DeclStmt ::= IDENT {, IDENT } : Type [:= Expr]
bool DeclStmt(istream& in, int& line) {
    vector<string> IdList;

    if (!IdentList(in, line, IdList)) {
        ParseError(line, "Incorrect identifiers list in Declaration Statement.");
        return false;
    }

    LexItem Tok = Parser::GetNextToken(in, line);
    if (Tok != COLON) {
        Parser::PushBackToken(Tok);
        return false;
    }

    Tok = Parser::GetNextToken(in, line);
    if (Tok != INTEGER && Tok != REAL && Tok != BOOLEAN && Tok != CHAR && Tok != STRING) {
        ParseError(line, "Incorrect Declaration Type: " + Tok.GetLexeme());
        return false;
    }

    Token varType = Tok.GetToken();

    // stamps the type on every identifier in this statement
    for (auto& vname : IdList) {
        SymTable[vname] = varType;
    }

    // checks for optional initializer, applies to ALL vars in the list (e.g. a, b : integer := 0)
    Tok = Parser::GetNextToken(in, line);
    if (Tok == ASSOP) {
        Value initVal;
        if (!Expr(in, line, initVal)) {
            ParseError(line, "Incorrect initialization expression.");
            return false;
        }

        // typechecks the init value, so real can absorb int, string can absorb char, everything else must match
        bool typeOK = false;
        Value storedVal = initVal;

        if (varType == INTEGER && initVal.IsInt()) {
            typeOK = true;
        } else if (varType == REAL && (initVal.IsReal() || initVal.IsInt())) {
            typeOK = true;
            if (initVal.IsInt()) storedVal = Value((double)initVal.GetInt());
        } else if (varType == STRING && (initVal.IsString() || initVal.IsChar())) {
            typeOK = true;
            if (initVal.IsChar()) storedVal = Value(string(1, initVal.GetChar()));
        } else if (varType == CHAR && initVal.IsChar()) {
            typeOK = true;
        } else if (varType == BOOLEAN && initVal.IsBool()) {
            typeOK = true;
        }

        if (!typeOK) {
            ParseError(line, "Illegal expression type for the assigned variable.");
            return false;
        }

        for (auto& vname : IdList) {
            ValTable[vname] = storedVal;
        }
    } else {
        Parser::PushBackToken(Tok);
    }

    return true;
}


// collects identifiers into a vector, so marks them in defVar
bool IdentList(istream& in, int& line, vector<string>& IdList) {
    LexItem Tok = Parser::GetNextToken(in, line);

    if (Tok != IDENT) {
        Parser::PushBackToken(Tok);
        return false;
    }

    string VarName = Tok.GetLexeme();

    if (VarName == ProgName) {
        ParseError(line, "Program Name is invalid identifier for a variable.");
        ParseError(line, "Incorrect identifiers list in Declaration Statement.");
        return false;
    }
    if (defConst.find(VarName) != defConst.end()) {
        ParseError(line, "Illegal use of a constant name as a variable: " + VarName);
        ParseError(line, "Incorrect identifiers list in Declaration Statement.");
        return false;
    }
    if (defVar.find(VarName) != defVar.end()) {
        ParseError(line, "Variable Redefinition: " + VarName);
        ParseError(line, "Incorrect identifiers list in Declaration Statement.");
        return false;
    }

    defVar[VarName] = true;
    IdList.push_back(VarName);

    Tok = Parser::GetNextToken(in, line);
    while (Tok == COMMA) {
        Tok = Parser::GetNextToken(in, line);
        if (Tok != IDENT) {
            ParseError(line, "Missing variable name after comma.");
            ParseError(line, "Incorrect identifiers list in Declaration Statement.");
            return false;
        }

        VarName = Tok.GetLexeme();

        if (VarName == ProgName) {
            ParseError(line, "Program Name is invalid identifier for a variable.");
            ParseError(line, "Incorrect identifiers list in Declaration Statement.");
            return false;
        }
        if (defConst.find(VarName) != defConst.end()) {
            ParseError(line, "Illegal use of a constant name as a variable: " + VarName);
            ParseError(line, "Incorrect identifiers list in Declaration Statement.");
            return false;
        }
        if (defVar.find(VarName) != defVar.end()) {
            ParseError(line, "Variable Redefinition: " + VarName);
            ParseError(line, "Incorrect identifiers list in Declaration Statement.");
            return false;
        }

        defVar[VarName] = true;
        IdList.push_back(VarName);
        Tok = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(Tok);
    return true;
}


// 9. Stmt ::= SimpleStmt | StructuredStmt
bool Stmt(istream& in, int& line) {
    LexItem Tok = Parser::GetNextToken(in, line);
    Parser::PushBackToken(Tok);

    if (Tok == IF || Tok == BEGIN) {
        return StructuredStmt(in, line);
    } else {
        return SimpleStmt(in, line);
    }
}


// 10. SimpleStmt ::= AssignStmt | ReadLnStmt | WriteLnStmt | WriteStmt
bool SimpleStmt(istream& in, int& line) {
    LexItem Tok = Parser::GetNextToken(in, line);

    if (Tok == WRITELN) {
        Parser::PushBackToken(Tok);
        if (!WriteLnStmt(in, line)) {
            ParseError(line, "Incorrect Simple Statement.");
            return false;
        }
        return true;
    }
    if (Tok == WRITE) {
        Parser::PushBackToken(Tok);
        if (!WriteStmt(in, line)) {
            ParseError(line, "Incorrect Simple Statement.");
            return false;
        }
        return true;
    }
    if (Tok == READLN) {
        Parser::PushBackToken(Tok);
        if (!ReadLnStmt(in, line)) {
            ParseError(line, "Incorrect Simple Statement.");
            return false;
        }
        return true;
    }
    if (Tok == IDENT) {
        Parser::PushBackToken(Tok);
        if (!AssignStmt(in, line)) {
            ParseError(line, "Incorrect Simple Statement.");
            return false;
        }
        return true;
    }

    ParseError(line, "Incorrect Simple Statement.");
    return false;
}


// 11. StructuredStmt ::= IfStmt | CompStmt
bool StructuredStmt(istream& in, int& line) {
    LexItem Tok = Parser::GetNextToken(in, line);

    if (Tok == IF) {
        Parser::PushBackToken(Tok);
        if (!IfStmt(in, line)) {
            ParseError(line, "Incorrect Structured Statement.");
            return false;
        }
        return true;
    }
    if (Tok == BEGIN) {
        Parser::PushBackToken(Tok);
        return CompStmt(in, line);
    }

    ParseError(line, "Incorrect Structured Statement.");
    return false;
}


// 12. CompStmt ::= BEGIN Stmt {; Stmt } END
bool CompStmt(istream& in, int& line) {
    LexItem Tok = Parser::GetNextToken(in, line);
    if (Tok != BEGIN) {
        ParseError(line, "Missing BEGIN.");
        return false;
    }

    // reads the next token right after Stmt even if it failed, so the
    // line counter moves forward and cascade error messages show the right line number
    bool status = Stmt(in, line);
    Tok = Parser::GetNextToken(in, line);

    while (status && Tok == SEMICOL) {
        Tok = Parser::GetNextToken(in, line);

        // trailing semicolon right before END is fine, so just return
        if (Tok == END) return true;

        if (Tok == DONE) {
            ParseError(line, "Missing end of compound statement.");
            return false;
        }
        if (Tok == ELSE) {
            ParseError(line, "Illegal Else-clause.");
            return false;
        }

        Parser::PushBackToken(Tok);
        status = Stmt(in, line);
        Tok = Parser::GetNextToken(in, line);
    }

    if (status && Tok == END) return true;

    if (Tok == DONE) {
        ParseError(line, "Missing end of compound statement.");
        Parser::PushBackToken(Tok);
        return false;
    }
    if (status && Tok == ELSE) {
        ParseError(line, "Illegal Else-clause.");
        return false;
    }
    if (!status) {
        ParseError(line, "Syntactic error in the statement.");
        Parser::PushBackToken(Tok);
        return false;
    }

    ParseError(line, "Missing semicolon.");
    return false;
}


// 13. WriteLnStmt ::= WRITELN ( ExprList )
bool WriteLnStmt(istream& in, int& line) {
    LexItem Tok = Parser::GetNextToken(in, line); // consume WRITELN

    Tok = Parser::GetNextToken(in, line);
    if (Tok != LPAREN) {
        ParseError(line, "Missing Left Parenthesis in WriteLn statement.");
        return false;
    }

    if (!ExprList(in, line)) {
        ParseError(line, "Missing expression list for WriteLn statement.");
        return false;
    }

    Tok = Parser::GetNextToken(in, line);
    if (Tok != RPAREN) {
        ParseError(line, "Missing Right Parenthesis for WriteLn statement.");
        return false;
    }

    if (execMode) cout << endl; // writeln adds a newline, so only print when executing
    return true;
}


// 14. WriteStmt ::= WRITE ( ExprList )
bool WriteStmt(istream& in, int& line) {
    LexItem Tok = Parser::GetNextToken(in, line); // consume WRITE

    Tok = Parser::GetNextToken(in, line);
    if (Tok != LPAREN) {
        ParseError(line, "Missing Left Parenthesis in Write statement.");
        return false;
    }

    if (!ExprList(in, line)) {
        ParseError(line, "Missing expression list for Write statement.");
        return false;
    }

    Tok = Parser::GetNextToken(in, line);
    if (Tok != RPAREN) {
        ParseError(line, "Missing Right Parenthesis for Write statement.");
        return false;
    }

    return true; // write does not add a newline
}


// 15. IfStmt ::= IF Expr THEN Stmt [ ELSE Stmt ]
bool IfStmt(istream& in, int& line) {
    LexItem Tok = Parser::GetNextToken(in, line); // consume IF

    Value condVal;
    if (!Expr(in, line, condVal)) {
        ParseError(line, "Missing if statement Logic Expression.");
        return false;
    }

    Tok = Parser::GetNextToken(in, line);
    if (Tok != THEN) {
        ParseError(line, "If Statement Syntax Error.");
        return false;
    }

    // checks boolean after consuming THEN, so if we fail here the next token
    // is already on the then-clause line and cascade error line numbers are correct
    if (execMode && !condVal.IsBool()) {
        ParseError(line, "Run-Time Error-Illegal Type for If statement condition.");
        return false;
    }

    // figures out which branch runs, so if not in execMode just parse both as skip
    bool cond = (execMode && condVal.IsBool()) ? condVal.GetBool() : false;

    bool savedExec = execMode;

    // then-clause: only runs if condition is true, otherwise parses in skip mode
    if (!cond) execMode = false;
    if (!Stmt(in, line)) {
        execMode = savedExec;
        ParseError(line, "Missing Statement for If-Then-Part.");
        return false;
    }
    execMode = savedExec;

    // else is optional, so only runs when condition was false
    Tok = Parser::GetNextToken(in, line);
    if (Tok == ELSE) {
        if (cond) execMode = false; // condition was true, so skip the else
        if (!Stmt(in, line)) {
            execMode = savedExec;
            ParseError(line, "Missing Statement for If-Else-Part.");
            return false;
        }
        execMode = savedExec;
    } else {
        Parser::PushBackToken(Tok);
    }

    return true;
}


// 16. AssignStmt ::= Variable := Expr
bool AssignStmt(istream& in, int& line) {
    LexItem idTok;
    if (!Variable(in, line, idTok)) {
        ParseError(line, "Missing Left-Hand Side Variable in Assignment statement.");
        return false;
    }

    LexItem Tok = Parser::GetNextToken(in, line);
    if (Tok != ASSOP) {
        ParseError(line, "Missing Assignment Operator.");
        return false;
    }

    Value rhsVal;
    if (!Expr(in, line, rhsVal)) {
        ParseError(line, "Missing Expression in Assignment Statement.");
        return false;
    }

    if (!execMode) return true; // skip mode, so just parse without storing anything

    string VarName = idTok.GetLexeme();
    Token varType  = SymTable[VarName];

    // same type rules as DeclStmt, so real eats int, string eats char, rest must match
    Value storedVal = rhsVal;
    bool typeOK     = false;

    if (varType == INTEGER && rhsVal.IsInt()) {
        typeOK = true;
    } else if (varType == REAL && rhsVal.IsInt()) {
        typeOK    = true;
        storedVal = Value((double)rhsVal.GetInt());
    } else if (varType == REAL && rhsVal.IsReal()) {
        typeOK = true;
    } else if (varType == STRING && rhsVal.IsString()) {
        typeOK = true;
    } else if (varType == STRING && rhsVal.IsChar()) {
        typeOK    = true;
        storedVal = Value(string(1, rhsVal.GetChar()));
    } else if (varType == CHAR && rhsVal.IsChar()) {
        typeOK = true;
    } else if (varType == CHAR && rhsVal.IsString()) {
        string s = rhsVal.GetString();
        if (s.length() == 1) {
            typeOK    = true;
            storedVal = Value(s[0]);
        }
    } else if (varType == BOOLEAN && rhsVal.IsBool()) {
        typeOK = true;
    }

    if (!typeOK) {
        ParseError(line, "Illegal expression type for the assigned variable.");
        return false;
    }

    ValTable[VarName] = storedVal;
    return true;
}


// 17. Variable ::= IDENT  (returns token so callers know the name)
bool Variable(istream& in, int& line, LexItem& idTok) {
    LexItem Tok = Parser::GetNextToken(in, line);

    if (Tok != IDENT) {
        ParseError(line, "Missing variable name.");
        return false;
    }

    string VarName = Tok.GetLexeme();
    idTok = Tok;

    if (VarName == ProgName) {
        ParseError(line, "Illegal use of program name as a variable: " + VarName);
        return false;
    }
    if (defConst.find(VarName) != defConst.end()) {
        ParseError(line, "Illegal use of constant name as a variable: " + VarName);
        return false;
    }
    if (defVar.find(VarName) == defVar.end()) {
        ParseError(line, "Undeclared Variable: " + VarName);
        return false;
    }

    return true;
}


// 18. ReadLnStmt ::= READLN ( VarList )
bool ReadLnStmt(istream& in, int& line) {
    LexItem Tok = Parser::GetNextToken(in, line); // consume READLN

    Tok = Parser::GetNextToken(in, line);
    if (Tok != LPAREN) {
        ParseError(line, "Missing Left Parenthesis in ReadLn statement.");
        return false;
    }

    vector<string> varNames;
    if (!VarList(in, line, varNames)) {
        ParseError(line, "Missing variable list for ReadLn statement.");
        return false;
    }

    Tok = Parser::GetNextToken(in, line);
    if (Tok != RPAREN) {
        ParseError(line, "Missing Right Parenthesis for ReadLn statement.");
        return false;
    }

    if (!execMode) return true; // skip mode, so don't actually touch stdin

    // reads values for each variable based on its declared type
    bool lastWasString = false;
    for (auto& vname : varNames) {
        Token varType = SymTable[vname];
        lastWasString = false;

        if (varType == INTEGER) {
            // skips whitespace, grabs the number
            int v;
            cin >> v;
            ValTable[vname] = Value(v);
        } else if (varType == REAL) {
            // same as int but accepts decimals, so uses double
            double v;
            cin >> v;
            ValTable[vname] = Value(v);
        } else if (varType == STRING) {
            // reads everything from current position to end of line
            string s;
            getline(cin, s);
            ValTable[vname] = Value(s);
            lastWasString = true;
        } else if (varType == CHAR) {
            // grabs the very next character with no whitespace skipping
            char c;
            cin.get(c);
            ValTable[vname] = Value(c);
        } else if (varType == BOOLEAN) {
            // skips whitespace then reads "true" or "false"
            string s;
            cin >> s;
            for (char& c : s) c = tolower((unsigned char)c);
            if (s == "true") {
                ValTable[vname] = Value(true);
            } else if (s == "false") {
                ValTable[vname] = Value(false);
            } else {
                ParseError(line, "Run-Time Error-Invalid Boolean input.");
                return false;
            }
        }
    }

    // readln skips to the next line after reading, unless string already ate the newline
    if (!lastWasString) {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    return true;
}


// 19. ExprList ::= Expr { , Expr }
// collects all values first then prints, so partial output is avoided if one expr fails
bool ExprList(istream& in, int& line) {
    vector<Value> vals;

    Value exprVal;
    if (!Expr(in, line, exprVal)) {
        ParseError(line, "Missing Expression.");
        return false;
    }
    vals.push_back(exprVal);

    LexItem Tok = Parser::GetNextToken(in, line);
    while (Tok == COMMA) {
        if (!Expr(in, line, exprVal)) {
            ParseError(line, "Missing Expression.");
            return false;
        }
        vals.push_back(exprVal);
        Tok = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(Tok);

    // only prints if actually executing, so skipped branches produce no output
    if (execMode) {
        for (auto& v : vals) cout << v;
    }
    return true;
}


// 20. VarList ::= Variable {, Variable }  collects var names for ReadLn
bool VarList(istream& in, int& line, vector<string>& VarNames) {
    LexItem idTok;
    if (!Variable(in, line, idTok)) {
        ParseError(line, "Missing variable.");
        return false;
    }
    VarNames.push_back(idTok.GetLexeme());

    LexItem Tok = Parser::GetNextToken(in, line);
    while (Tok == COMMA) {
        if (!Variable(in, line, idTok)) {
            ParseError(line, "Missing variable.");
            return false;
        }
        VarNames.push_back(idTok.GetLexeme());
        Tok = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(Tok);
    return true;
}


// 21. Expr ::= SimpleExpr [ ( = | < | > ) SimpleExpr ]
bool Expr(istream& in, int& line, Value& retVal) {
    if (!SimpleExpr(in, line, retVal)) return false;

    LexItem Tok = Parser::GetNextToken(in, line);
    if (Tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern.");
        cout << "(" << Tok.GetLexeme() << ")" << endl;
        return false;
    }

    if (Tok == EQ || Tok == LTHAN || Tok == GTHAN) {
        Token op = Tok.GetToken();
        Value RHS;
        if (!SimpleExpr(in, line, RHS)) {
            ParseError(line, "Missing operand after operator.");
            return false;
        }

        if (op == EQ)         retVal = (retVal == RHS);
        else if (op == LTHAN) retVal = (retVal < RHS);
        else                  retVal = (retVal > RHS);

        if (execMode && retVal.IsErr()) {
            ParseError(line, "Illegal operand types for an arithmetic or logical operator.");
            return false;
        }

        // no cascading relational operators allowed, so check and reject
        Tok = Parser::GetNextToken(in, line);
        if (Tok == EQ || Tok == LTHAN || Tok == GTHAN) {
            ParseError(line, "Illegal Relational Expression.");
            return false;
        }
        Parser::PushBackToken(Tok);
    } else {
        Parser::PushBackToken(Tok);
    }

    return true;
}


// 22. SimpleExpr ::= Term { ( + | - | OR ) Term }
bool SimpleExpr(istream& in, int& line, Value& retVal) {
    if (!Term(in, line, retVal)) return false;

    LexItem Tok = Parser::GetNextToken(in, line);
    if (Tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern.");
        cout << "(" << Tok.GetLexeme() << ")" << endl;
        return false;
    }

    while (Tok == PLUS || Tok == MINUS || Tok == OR) {
        Token op = Tok.GetToken();
        Value RHS;
        if (!Term(in, line, RHS)) {
            ParseError(line, "Missing operand after operator.");
            return false;
        }

        if (op == OR) {
            if (execMode && (!retVal.IsBool() || !RHS.IsBool())) {
                ParseError(line, "Illegal operand types for an arithmetic or logical operator.");
                return false;
            }
            retVal = retVal || RHS;
        } else if (op == PLUS) {
            retVal = retVal + RHS;
        } else {
            retVal = retVal - RHS;
        }

        if (execMode && retVal.IsErr()) {
            ParseError(line, "Illegal operand types for an arithmetic or logical operator.");
            return false;
        }

        Tok = Parser::GetNextToken(in, line);
        if (Tok.GetToken() == ERR) {
            ParseError(line, "Unrecognized Input Pattern.");
            cout << "(" << Tok.GetLexeme() << ")" << endl;
            return false;
        }
    }

    Parser::PushBackToken(Tok);
    return true;
}


// 23. Term ::= SFactor { ( * | / | DIV | MOD | AND ) SFactor }
bool Term(istream& in, int& line, Value& retVal) {
    if (!SFactor(in, line, retVal)) return false;

    LexItem Tok = Parser::GetNextToken(in, line);
    if (Tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern.");
        cout << "(" << Tok.GetLexeme() << ")" << endl;
        return false;
    }

    while (Tok == MULT || Tok == DIV || Tok == IDIV || Tok == MOD || Tok == AND) {
        Token op = Tok.GetToken();
        Value RHS;
        if (!SFactor(in, line, RHS)) {
            ParseError(line, "Missing operand after operator.");
            return false;
        }

        if (op == MULT) {
            retVal = retVal * RHS;
            if (execMode && retVal.IsErr()) {
                ParseError(line, "Illegal operand types for an arithmetic or logical operator.");
                return false;
            }
        } else if (op == DIV) {
            if (execMode) {
                double divisor = RHS.IsInt() ? (double)RHS.GetInt()
                               : RHS.IsReal() ? RHS.GetReal() : 0;
                if (divisor == 0.0) {
                    ParseError(line, "Run-Time Error-Illegal division by Zero");
                    return false;
                }
            }
            retVal = retVal / RHS;
            if (execMode && retVal.IsErr()) {
                ParseError(line, "Illegal operand types for an arithmetic or logical operator.");
                return false;
            }
        } else if (op == IDIV) {
            if (execMode) {
                if (!retVal.IsInt() || !RHS.IsInt()) {
                    ParseError(line, "Invalid non-integer operands for the DIV operator.");
                    return false;
                }
                if (RHS.GetInt() == 0) {
                    ParseError(line, "Run-Time Error-Illegal division by Zero");
                    return false;
                }
            }
            retVal = retVal.idiv(RHS);
        } else if (op == MOD) {
            if (execMode) {
                if (!retVal.IsInt() || !RHS.IsInt()) {
                    ParseError(line, "Invalid non-integer operands for the MOD operator.");
                    return false;
                }
                if (RHS.GetInt() == 0) {
                    ParseError(line, "Run-Time Error-Illegal division by Zero");
                    return false;
                }
            }
            retVal = retVal % RHS;
        } else { // AND
            if (execMode && (!retVal.IsBool() || !RHS.IsBool())) {
                ParseError(line, "Illegal operand types for an arithmetic or logical operator.");
                return false;
            }
            retVal = retVal && RHS;
        }

        Tok = Parser::GetNextToken(in, line);
        if (Tok.GetToken() == ERR) {
            ParseError(line, "Unrecognized Input Pattern.");
            cout << "(" << Tok.GetLexeme() << ")" << endl;
            return false;
        }
    }

    Parser::PushBackToken(Tok);
    return true;
}


// 24. SFactor ::= [( - | + | NOT )] Factor
bool SFactor(istream& in, int& line, Value& retVal) {
    // tracks sign: 0=none, 1=unary+, -1=unary-, 2=NOT
    int Sign = 0;

    LexItem Tok = Parser::GetNextToken(in, line);
    if (Tok == MINUS)     Sign = -1;
    else if (Tok == PLUS) Sign =  1;
    else if (Tok == NOT)  Sign =  2;
    else                  Parser::PushBackToken(Tok);

    return Factor(in, line, Sign, retVal);
}


// 25. Factor ::= IDENT | ICONST | RCONST | SCONST | BCONST | CCONST | (Expr)
bool Factor(istream& in, int& line, int sign, Value& retVal) {
    LexItem Tok = Parser::GetNextToken(in, line);

    if (Tok == IDENT) {
        string Name = Tok.GetLexeme();

        if (defConst.find(Name) != defConst.end()) {
            retVal = ConstTable[Name];
        } else if (defVar.find(Name) != defVar.end()) {
            // variable is declared but might not be assigned yet
            if (ValTable.find(Name) == ValTable.end()) {
                if (execMode) {
                    // catches uninitialized use, so only errors when actually executing
                    ParseError(line, "Using a variable before being assinged: " + Name);
                    return false;
                }
                retVal = Value(); // dummy VERR, so parsing can continue in skip mode
            } else {
                retVal = ValTable[Name];
            }
        } else {
            ParseError(line, "Undeclared Variable: " + Name);
            return false;
        }
    } else if (Tok == ICONST) {
        retVal = Value(stoi(Tok.GetLexeme()));
    } else if (Tok == RCONST) {
        retVal = Value(stod(Tok.GetLexeme()));
    } else if (Tok == SCONST) {
        retVal = Value(Tok.GetLexeme());
    } else if (Tok == BCONST) {
        // lexSP26.cpp maps true/false to BCONST, so lexeme is the original word
        string lex = Tok.GetLexeme();
        for (char& c : lex) c = tolower((unsigned char)c);
        retVal = Value(lex == "true");
    } else if (Tok == CCONST) {
        string lex = Tok.GetLexeme();
        retVal = Value(lex.empty() ? '\0' : lex[0]);
    } else if (Tok == LPAREN) {
        if (!Expr(in, line, retVal)) {
            ParseError(line, "Missing expression after Left Parenthesis.");
            return false;
        }
        Tok = Parser::GetNextToken(in, line);
        if (Tok != RPAREN) {
            ParseError(line, "Missing right parenthesis after expression.");
            return false;
        }
    } else if (Tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern.");
        cout << "(" << Tok.GetLexeme() << ")" << endl;
        return false;
    } else {
        ParseError(line, "Missing operand after operator.");
        return false;
    }

    // applies unary sign/NOT, so runtime checks are skipped if not in exec mode
    if (sign == 1) {
        if (execMode && !retVal.IsInt() && !retVal.IsReal()) {
            ParseError(line, "Illegal Operand Type for Sign/NOT Operator.");
            return false;
        }
        // unary + doesn't change the value
    } else if (sign == -1) {
        if (execMode && !retVal.IsInt() && !retVal.IsReal()
                     && !retVal.IsString() && !retVal.IsChar()) {
            ParseError(line, "Illegal Operand Type for Sign/NOT Operator.");
            return false;
        }
        retVal = -retVal;
    } else if (sign == 2) {
        if (execMode && !retVal.IsBool()) {
            ParseError(line, "Illegal operand type for NOT operator.");
            return false;
        }
        retVal = !retVal;
    }

    return true;
}
