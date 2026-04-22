

#include "parserSP26.h"
#include <map>
#include <set>

// maps to keep track of defined variables and constants across the whole program
map<string, bool> defVar;
map<string, bool> defConst;

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
		if (pushed_back) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;
	}
}

static int error_count = 0;

int ErrCount()
{
	return error_count;
}

void ParseError(int line, string msg)
{
	++error_count;
	cout << line << ": " << msg << endl;
}


// 1. Prog ::= PROGRAM IDENT ; Block .
bool Prog(istream& in, int& line) {
	LexItem Tok = Parser::GetNextToken(in, line);

	if (Tok != PROGRAM) {
		ParseError(line, "Missing PROGRAM keyword.");
		return false;
	}

	Tok = Parser::GetNextToken(in, line);
	if (Tok != IDENT) {
		ParseError(line, "Missing Program Name.");
		return false;
	}

	string ProgName = Tok.GetLexeme();

	Tok = Parser::GetNextToken(in, line);
	if (Tok != SEMICOL) {
		ParseError(line, "Missing semicolon after program name.");
		return false;
	}

	if (!Block(in, line)) {
		ParseError(line, "Incorrect Program Body.");
		return false;
	}

	Tok = Parser::GetNextToken(in, line);

	// trailing dot is optional so just put it back if its not there.
	if (Tok != DOT) {
		Parser::PushBackToken(Tok);
	}

	cout << "Program Name: " << ProgName << "\n";

	// sort variable names before printing, ouput is az
	cout << "Declared Variables:" << "\n";
	set<string> SortedVars;
	for (auto& Entry : defVar)
		SortedVars.insert(Entry.first);

	bool FirstVar = true;
	for (auto& V : SortedVars) {
		if (!FirstVar) cout << ", ";
		cout << V;
		FirstVar = false;
	}
	cout << "\n";

	// same as above but for constants
	cout << "\n" << "Defined Constants:" << "\n";
	set<string> SortedConsts;
	for (auto& Entry : defConst)
		SortedConsts.insert(Entry.first);

	bool FirstConst = true;
	for (auto& C : SortedConsts) {
		if (!FirstConst) cout << ", ";
		cout << C;
		FirstConst = false;
	}
	cout << "\n";

	cout << "\n" << "DONE" << "\n";
	return true;
}


// 2. Block ::= [ DeclPart ] CompStmt
bool Block(istream& in, int& line) {
	if (!DeclPart(in, line)) {
		return false;
	}

	if (!CompStmt(in, line)) {
		ParseError(line, "Incorrect Program Body.");
		return false;
	}

	return true;
}


// 3. DeclPart ::= [ ConstPart ] [ VarPart ]
bool DeclPart(istream& in, int& line) {
	LexItem Tok = Parser::GetNextToken(in, line);

	if (Tok == CONST) {
		Parser::PushBackToken(Tok);
		if (!ConstPart(in, line)) {
			ParseError(line, "Incorrect Constant Definition Part.");
			return false;
		}
		Tok = Parser::GetNextToken(in, line);
	}

	if (Tok == VAR) {
		Parser::PushBackToken(Tok);
		if (!VarPart(in, line)) {
			ParseError(line, "Incorrect Declaration Part.");
			return false;
		}
	} else {
		// put the token back for whoever calls us next
		Parser::PushBackToken(Tok);
	}

	return true;
}


// 4. ConstPart ::= CONST ConstDef { ; ConstDef } ;
bool ConstPart(istream& in, int& line) {
	LexItem Tok = Parser::GetNextToken(in, line);

	if (Tok != CONST) {
		ParseError(line, "Missing CONST keyword.");
		return false;
	}

	if (!ConstDef(in, line)) {
		ParseError(line, "Syntactic error in Constants Definitions Part.");
		return false;
	}

	Tok = Parser::GetNextToken(in, line);
	if (Tok != SEMICOL) {
		ParseError(line, "Missing semicolon.");
		return false;
	}

	// keep reading more defintions as long as identifiers keep showing up
	Tok = Parser::GetNextToken(in, line);
	while (Tok == IDENT) {
		Parser::PushBackToken(Tok);
		if (!ConstDef(in, line)) {
			ParseError(line, "Syntactic error in Constants Definitions Part.");
			return false;
		}
		Tok = Parser::GetNextToken(in, line);
		if (Tok != SEMICOL) {
			ParseError(line, "Missing semicolon.");
			return false;
		}
		Tok = Parser::GetNextToken(in, line);
	}

	Parser::PushBackToken(Tok);
	return true;
}


// 5. ConstDef ::= IDENT = Expr
bool ConstDef(istream& in, int& line) {
	LexItem Tok = Parser::GetNextToken(in, line);

	if (Tok != IDENT) {
		ParseError(line, "Missing constant name.");
		return false;
	}

	string ConstName = Tok.GetLexeme();

	// defining the same constant twice is not allowed
	if (defConst.find(ConstName) != defConst.end()) {
		ParseError(line, "Constant Redefinition: " + ConstName);
		return false;
	}

	Tok = Parser::GetNextToken(in, line);

	// constant definitions use = not :=
	if (Tok != EQ) {
		ParseError(line, "Incorrect constant definition syntax.");
		return false;
	}

	if (!Expr(in, line)) {
		ParseError(line, "Missing expression in constant definition.");
		return false;
	}

	defConst[ConstName] = true;
	return true;
}


// 6. VarPart ::= VAR DeclStmt { ; DeclStmt } ;
// same structure as rule 4
bool VarPart(istream& in, int& line) {
	LexItem Tok = Parser::GetNextToken(in, line);

	if (Tok != VAR) {
		ParseError(line, "Missing VAR keyword.");
		return false;
	}

	if (!DeclStmt(in, line)) {
		ParseError(line, "Syntactic error in Declaration Block.");
		return false;
	}

	Tok = Parser::GetNextToken(in, line);
	if (Tok != SEMICOL) {
		ParseError(line, "Missing semicolon.");
		return false;
	}

	Tok = Parser::GetNextToken(in, line);
	while (Tok == IDENT) {
		Parser::PushBackToken(Tok);
		if (!DeclStmt(in, line)) {
			ParseError(line, "Syntactic error in Declaration Block.");
			return false;
		}
		Tok = Parser::GetNextToken(in, line);
		if (Tok != SEMICOL) {
			ParseError(line, "Missing semicolon.");
			return false;
		}
		Tok = Parser::GetNextToken(in, line);
	}

	Parser::PushBackToken(Tok);
	return true;
}


// 7. DeclStmt ::= IDENT {, IDENT } : Type [:= Expr]
bool DeclStmt(istream& in, int& line) {
	LexItem Tok = Parser::GetNextToken(in, line);

	if (Tok != IDENT) {
		ParseError(line, "Missing variable name in declaration.");
		return false;
	}

	string VarName = Tok.GetLexeme();

	// a constant name cannot be reused as a variable
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

	defVar[VarName] = false;

	Tok = Parser::GetNextToken(in, line);
	while (Tok == COMMA) {
		Tok = Parser::GetNextToken(in, line);
		if (Tok != IDENT) {
			ParseError(line, "Missing variable name after comma.");
			ParseError(line, "Incorrect identifiers list in Declaration Statement.");
			return false;
		}

		VarName = Tok.GetLexeme();

		// same checks as before
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

		defVar[VarName] = false;
		Tok = Parser::GetNextToken(in, line);
	}

	// after the identifier list we need a colon before the type
	if (Tok != COLON) {
		ParseError(line, "Missing comma in declaration statement.");
		ParseError(line, "Incorrect identifiers list in Declaration Statement.");
		return false;
	}

	Tok = Parser::GetNextToken(in, line);
	if (Tok != INTEGER && Tok != REAL && Tok != BOOLEAN && Tok != CHAR && Tok != STRING) {
		ParseError(line, "Incorrect Declaration Type: " + Tok.GetLexeme());
		return false;
	}

	// initializer is optional
	Tok = Parser::GetNextToken(in, line);
	if (Tok == ASSOP) {
		if (!Expr(in, line)) {
			ParseError(line, "Missing expression in variable initialization.");
			return false;
		}
	} else {
		Parser::PushBackToken(Tok);
	}

	return true;
}


// 9. Stmt ::= SimpleStmt | StructuredStmt
bool Stmt(istream& in, int& line) {
	LexItem Tok = Parser::GetNextToken(in, line);
	Parser::PushBackToken(Tok);

	// IF and BEGIN are the TWO structured statement starters
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
		if (!CompStmt(in, line)) {
			return false;
		}
		return true;
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

	if (!Stmt(in, line)) {
		ParseError(line, "Syntactic error in the statement.");
		return false;
	}

	Tok = Parser::GetNextToken(in, line);
	while (Tok == SEMICOL) {
		Tok = Parser::GetNextToken(in, line);

		// semicolon right before END is fine
		if (Tok == END) {
			return true;
		}

		if (Tok == DONE) {
			ParseError(line, "Missing end of compound statement.");
			return false;
		}

		// ELSE after a semicolon means theres no matching if
		if (Tok == ELSE) {
			ParseError(line, "Illegal Else-clause.");
			return false;
		}

		Parser::PushBackToken(Tok);

		if (!Stmt(in, line)) {
			ParseError(line, "Syntactic error in the statement.");
			return false;
		}
		Tok = Parser::GetNextToken(in, line);
	}

	if (Tok == END) {
		return true;
	}

	if (Tok == DONE) {
		ParseError(line, "Missing end of compound statement.");
		return false;
	}

	// anything else HERE means a semicolon was forgotten
	ParseError(line, "Missing semicolon.");
	return false;
}


// 13. WriteLnStmt ::= WRITELN ( ExprList )
bool WriteLnStmt(istream& in, int& line) {
	LexItem Tok = Parser::GetNextToken(in, line);

	Tok = Parser::GetNextToken(in, line);
	if (Tok != LPAREN) {
		ParseError(line, "Missing Left Parenthesis in WriteLn statement.");
		return false;
	}

	if (!ExprList(in, line)) {
		ParseError(line, "Missing expression list in WriteLn statement.");
		return false;
	}

	Tok = Parser::GetNextToken(in, line);
	if (Tok != RPAREN) {
		ParseError(line, "Missing Right Parenthesis in WriteLn statement.");
		return false;
	}

	return true;
}


// 14. WriteStmt ::= WRITE ( ExprList )
// same as rule 13
bool WriteStmt(istream& in, int& line) {
	LexItem Tok = Parser::GetNextToken(in, line);

	Tok = Parser::GetNextToken(in, line);
	if (Tok != LPAREN) {
		ParseError(line, "Missing Left Parenthesis in Write statement.");
		return false;
	}

	if (!ExprList(in, line)) {
		ParseError(line, "Missing expression list in Write statement.");
		return false;
	}

	Tok = Parser::GetNextToken(in, line);
	if (Tok != RPAREN) {
		ParseError(line, "Missing Right Parenthesis in Write statement.");
		return false;
	}

	return true;
}


// 15. IfStmt ::= IF Expr THEN Stmt [ ELSE Stmt ]
bool IfStmt(istream& in, int& line) {
	LexItem Tok = Parser::GetNextToken(in, line);

	if (!Expr(in, line)) {
		ParseError(line, "Missing if statement Logic Expression.");
		return false;
	}

	Tok = Parser::GetNextToken(in, line);
	if (Tok != THEN) {
		// update line so the cascade of errors from callers shows the right number
		line = Tok.GetLinenum();
		ParseError(line, "If Statement Syntax Error.");
		return false;
	}

	if (!Stmt(in, line)) {
		ParseError(line, "Missing Statement for If-Then-Part.");
		return false;
	}

	// else is optional
	Tok = Parser::GetNextToken(in, line);
	if (Tok == ELSE) {
		if (!Stmt(in, line)) {
			ParseError(line, "Missing Statement for Else-Part.");
			return false;
		}
	} else {
		Parser::PushBackToken(Tok);
	}

	return true;
}


// 16. AssignStmt ::= Variable := Expr
bool AssignStmt(istream& in, int& line) {
	if (!Variable(in, line)) {
		ParseError(line, "Missing Left-Hand Side Variable in Assignment statement.");
		return false;
	}

	LexItem Tok = Parser::GetNextToken(in, line);
	if (Tok != ASSOP) {
		ParseError(line, "Missing Assignment Operator.");
		return false;
	}

	if (!Expr(in, line)) {
		ParseError(line, "Missing Expression in Assignment Statement.");
		return false;
	}

	return true;
}


// 17. Variable ::= IDENT
bool Variable(istream& in, int& line) {
	LexItem Tok = Parser::GetNextToken(in, line);

	if (Tok != IDENT) {
		ParseError(line, "Missing variable name.");
		return false;
	}

	string VarName = Tok.GetLexeme();

	if (defVar.find(VarName) == defVar.end() && defConst.find(VarName) == defConst.end()) {
		ParseError(line, "Undeclared Variable: " + VarName);
		return false;
	}

	// constants cannot appear on the left side of an assignment
	if (defConst.find(VarName) != defConst.end()) {
		ParseError(line, "Illegal use of a constant name as a variable: " + VarName);
		return false;
	}

	return true;
}


// 18. ReadLnStmt ::= READLN ( VarList )
// same as rule 13 but reads into variables
bool ReadLnStmt(istream& in, int& line) {
	LexItem Tok = Parser::GetNextToken(in, line);

	Tok = Parser::GetNextToken(in, line);
	if (Tok != LPAREN) {
		ParseError(line, "Missing Left Parenthesis in ReadLn statement.");
		return false;
	}

	if (!VarList(in, line)) {
		ParseError(line, "Missing variable list in ReadLn statement.");
		return false;
	}

	Tok = Parser::GetNextToken(in, line);
	if (Tok != RPAREN) {
		ParseError(line, "Missing Right Parenthesis in ReadLn statement.");
		return false;
	}

	return true;
}


// 19. ExprList ::= Expr { , Expr }
bool ExprList(istream& in, int& line) {
	if (!Expr(in, line)) {
		return false;
	}

	LexItem Tok = Parser::GetNextToken(in, line);
	while (Tok == COMMA) {
		if (!Expr(in, line)) {
			return false;
		}
		Tok = Parser::GetNextToken(in, line);
	}

	Parser::PushBackToken(Tok);
	return true;
}


// 20. VarList ::= Variable {, Variable }
// same as rule 19 but for variables
bool VarList(istream& in, int& line) {
	if (!Variable(in, line)) {
		return false;
	}

	LexItem Tok = Parser::GetNextToken(in, line);
	while (Tok == COMMA) {
		if (!Variable(in, line)) {
			return false;
		}
		Tok = Parser::GetNextToken(in, line);
	}

	Parser::PushBackToken(Tok);
	return true;
}


// 21. Expr ::= SimpleExpr [ ( = | < | > ) SimpleExpr ]
bool Expr(istream& in, int& line) {
	if (!SimpleExpr(in, line)) {
		return false;
	}

	int PrevLine = line;
	LexItem Tok = Parser::GetNextToken(in, line);
	if (Tok == EQ || Tok == LTHAN || Tok == GTHAN) {
		if (!SimpleExpr(in, line)) {
			return false;
		}

		// cascading like a < b < c is NOT allowed.
		PrevLine = line;
		Tok = Parser::GetNextToken(in, line);
		if (Tok == EQ || Tok == LTHAN || Tok == GTHAN) {
			ParseError(line, "Illegal Relational Expression.");
			return false;
		}
		line = PrevLine;
		Parser::PushBackToken(Tok);
	} else {
		line = PrevLine;
		Parser::PushBackToken(Tok);
	}

	return true;
}


// 22. SimpleExpr ::= Term { ( + | - | OR ) Term }
bool SimpleExpr(istream& in, int& line) {
	if (!Term(in, line)) {
		return false;
	}

	int PrevLine = line;
	LexItem Tok = Parser::GetNextToken(in, line);
	while (Tok == PLUS || Tok == MINUS || Tok == OR) {
		if (!Term(in, line)) {
			return false;
		}
		PrevLine = line;
		Tok = Parser::GetNextToken(in, line);
	}

	if (Tok == ERR) {
		ParseError(line, "Unrecognized Input Pattern.");
		cout << "(" << Tok.GetLexeme() << ")" << "\n";
		return false;
	}

	line = PrevLine;
	Parser::PushBackToken(Tok);
	return true;
}


// 23. Term ::= SFactor { ( * | / | DIV | MOD | AND ) SFactor }
bool Term(istream& in, int& line) {
	if (!SFactor(in, line)) {
		return false;
	}

	int PrevLine = line;
	LexItem Tok = Parser::GetNextToken(in, line);
	while (Tok == MULT || Tok == DIV || Tok == IDIV || Tok == MOD || Tok == AND) {
		if (!SFactor(in, line)) {
			return false;
		}
		PrevLine = line;
		Tok = Parser::GetNextToken(in, line);
	}

	line = PrevLine;
	Parser::PushBackToken(Tok);
	return true;
}


// 24. SFactor ::= [( - | + | NOT )] Factor
bool SFactor(istream& in, int& line) {
	// 0 = no sign, 1 = positive, -1 = negative, 2 = NOT
	int Sign = 0;

	LexItem Tok = Parser::GetNextToken(in, line);
	if (Tok == MINUS) {
		Sign = -1;
	} else if (Tok == PLUS) {
		Sign = 1;
	} else if (Tok == NOT) {
		Sign = 2;
	} else {
		Parser::PushBackToken(Tok);
	}

	if (!Factor(in, line, Sign)) {
		return false;
	}

	return true;
}


// 25. Factor ::= IDENT | ICONST | RCONST | SCONST | BCONST | CCONST | (Expr)
bool Factor(istream& in, int& line, int sign) {
	LexItem Tok = Parser::GetNextToken(in, line);

	if (Tok == IDENT) {
		string Name = Tok.GetLexeme();
		if (defVar.find(Name) == defVar.end() && defConst.find(Name) == defConst.end()) {
			ParseError(line, "Undeclared Variable: " + Name);
			return false;
		}
		return true;
	}

	if (Tok == ICONST || Tok == RCONST || Tok == SCONST || Tok == BCONST || Tok == CCONST) {
		return true;
	}

	if (Tok == LPAREN) {
		if (!Expr(in, line)) {
			ParseError(line, "Missing expression after Left Parenthesis.");
			return false;
		}
		Tok = Parser::GetNextToken(in, line);
		if (Tok != RPAREN) {
			ParseError(line, "Missing right parenthesis after expression.");
			return false;
		}
		return true;
	}

	if (Tok == ERR) {
		ParseError(line, "Unrecognized Input Pattern.");
		cout << "(" << Tok.GetLexeme() << ")" << "\n";
		return false;
	}

	ParseError(line, "Missing operand after operator.");
	return false;
}
