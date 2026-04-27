/* Implementation of Recursive-Descent Parser
	for a Basic Pasacal-Like Language
 * parser.cpp
 * Programming Assignment 2
 * Spring 2026
*/
#include "lex.h"
#include "parser.h"

map<string, bool> defVar;
map<string, Token> SymTable;
map<string, bool> defConst;

namespace Parser {
	bool pushed_back = false;
	LexItem	pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		if( pushed_back ) {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem & t) {
		if( pushed_back ) {
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



//Program is: Prog ::= PROGRAM IDENT ; Block .
bool Prog(istream& in, int& line)
{
	bool f1, f2;
	string progName;
	LexItem tok = Parser::GetNextToken(in, line);
		
	if (tok.GetToken() == PROGRAM) {
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == IDENT) {
			progName = tok.GetLexeme();
			if (!(defVar.find(progName->second))
			{
				defVar[progName] = true;
				
			}
			tok = Parser::GetNextToken(in, line);
			if (tok.GetToken() == SEMICOL) {
				f1 = Block(in, line); 
			
				if(!f1) {
					ParseError(line, "Incorrect Program Body.");
					return false;
				}
											
				tok = Parser::GetNextToken(in, line);
				if(tok.GetToken() != DOT)
				{
					ParseError(line, "Incorrect Syntax for End of Program.");
					return false;
				}
				cout << "Program Name: " << progName << endl;
				if( defVar.size() > 0 ) {
					cout << "Declared Variables:" << endl;
					auto it = defVar.begin();
					cout << it->first ;
					for( it++; it != defVar.end(); it++ )
						cout << ", " << it->first ;
					
				}
				cout << endl <<endl;
				
				if( defConst.size() > 0 ) {
					cout << "Defined Constants:" << endl;
					auto it = defConst.begin();
					cout << it->first ;
					for( it++; it != defConst.end(); it++ )
						cout << ", " << it->first ;
					
				}
				cout << endl <<endl;
				cout << "DONE" << endl;
				return true;
			}
			else
			{
				ParseError(line-1, "Missing Semicolon in Program Header.");
				return false;
			}
		}
		else
		{
			ParseError(line, "Missing Program Name.");
			return false;
		}
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern.");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else if(tok.GetToken() == DONE && tok.GetLinenum() <= 1){
		ParseError(line, "Empty File.");
		return true;
	}
	ParseError(line, "Missing PROGRAM Keyword.");
	return false;
}//End of Prog

//	Block ::= [ DeclPart ] CompStmt
bool Block(istream& in, int& line) {
	
	bool f1, f2, f3;
	LexItem tok = Parser::GetNextToken(in, line);
	Token t = tok.GetToken();
	
	if( t == CONST)
	{
		Parser::PushBackToken(tok);
		f1 = ConstPart(in, line);
		if(!f1)
		{
			ParseError(line, "Incorrect Constant Definition Part.");
			return false;
		}
		tok = Parser::GetNextToken(in, line);
		t = tok.GetToken();
	}
	
	if( t == VAR)
	{
		Parser::PushBackToken(tok);
		
		f2 = VarPart(in, line);
		
		if(!f2)
		{
			ParseError(line, "Incorrect Declaration Part.");
			return false;
		}
	}
	
	tok = Parser::GetNextToken(in, line);
	t = tok.GetToken();
	if(t == BEGIN)
	{
		f3 = CompStmt(in, line);
		if(!f3)
		{
			ParseError(line, "Incorrect Program Body.");
			return false;
		}
		return true;
	}
	ParseError(line, "Missing begin for program body.");
	return false;
}//End Block

//	ConstPart ::= CONST ConstDef { ; ConstDef  } ;
bool ConstPart(istream& in, int& line) {
	bool status = false;
	LexItem tok;
	
	LexItem t = Parser::GetNextToken(in, line);
	if(t == CONST)
	{
		status = ConstDef(in, line);
		
		while(status)
		{
			tok = Parser::GetNextToken(in, line);
			
			if(tok != SEMICOL)
			{
				
				ParseError(line, "Missing semicolon in Constants Definitions Part.");
				return false;
			}
			status = ConstDef(in, line);
		}
		
		tok = Parser::GetNextToken(in, line);
		
		if(tok == BEGIN || tok == VAR)
		{
			Parser::PushBackToken(tok);
			return true;
		}
		else 
		{
			ParseError(line, "Syntactic error in Constants Definitions Part.");
			return false;
		}
	}
	else
	{
		ParseError(line, "Non-recognizable Constants Definitions Part.");
		return false;
	}
	
}//end of ConstPart function


//VarPart ::= VAR DeclStmt { ; DeclStmt }
bool VarPart(istream& in, int& line) {
	bool status = false;
	LexItem tok;
	
	LexItem t = Parser::GetNextToken(in, line);
	if(t == VAR)
	{
		status = DeclStmt(in, line);
		
		while(status)
		{
			tok = Parser::GetNextToken(in, line);
			if(tok != SEMICOL)
			{
				//line--;
				ParseError(line, "Missing semicolon in Declaration Statement.");
				return false;
			}
			status = DeclStmt(in, line);
		}
		
		tok = Parser::GetNextToken(in, line);
		if(tok == BEGIN )
		{
			Parser::PushBackToken(tok);
			return true;
		}
		else 
		{
			ParseError(line, "Syntactic error in Declaration Block.");
			return false;
		}
	}
	else
	{
		ParseError(line, "Non-recognizable Declaration Part.");
		return false;
	}
	
}//end of VarPart function

bool ConstDef(istream& in, int& line)
{
	bool flag;
	LexItem tok = Parser::GetNextToken(in, line);
	
	string identstr;
	
	if(tok == IDENT)
	{
		//set IDENT lexeme to the type tok value
		identstr = tok.GetLexeme();
		
		if (!(defConst.find(identstr)->second))
		{
			
			defConst[identstr] = true;
			
		}	
		else
		{
			string str =  tok.GetLexeme();
			str = "Constant Redefinition: " + str;
			ParseError(line, str);
			return false;
		}
		
	}
	else
	{
		Parser::PushBackToken(tok);
		return false;
	}
	tok = Parser::GetNextToken(in, line);
	if(tok == EQ)
	{
		flag = Expr(in, line);
		if(!flag)
		{
			ParseError(line, "Incorrect Constant Value.");
			return false;
		}
		return true;
	}
	else
	{
		ParseError(line, "Incorrect constant definition syntax.");
		return false;
	}
}//end of constant definition

//DeclStmt ::= IDENT {, IDENT } : Type [:= Expr]
bool DeclStmt(istream& in, int& line)
{
	LexItem t;
	
	bool status = IdentList(in, line);
	bool flag;
	
	if (!status)
	{
		ParseError(line, "Incorrect identifiers list in Declaration Statement.");
		return status;
	}
	
	t = Parser::GetNextToken(in, line);
	if(t == COLON)
	{
		t = Parser::GetNextToken(in, line);
		if(t == INTEGER || t == REAL || t == STRING || t == BOOLEAN || t == CHAR)
		{
			t = Parser::GetNextToken(in, line);
			if(t == ASSOP)
			{
				flag = Expr(in, line);
				if(!flag)
				{
					ParseError(line, "Incorrect initialization expression.");
					return false;
				}
			}
			else
			{
				Parser::PushBackToken(t);
			}
			return true;
		}
		else
		{
			string str =  t.GetLexeme();
			str = "Incorrect Declaration Type: " + str;
			ParseError(line, str);
			return false;
		}
	}
	else
	{
		Parser::PushBackToken(t);
		return false;
	}
	
}//End of DeclStmt

//IdList:= IDENT {,IDENT}
bool IdentList(istream& in, int& line) {
	bool status;
	string identstr;
	
	LexItem tok = Parser::GetNextToken(in, line);
	if(tok == IDENT)
	{
		identstr = tok.GetLexeme();
		if(identstr == progName)
		{
			ParseError(line, "Program Name is invalid identifier for a variable.");
			return false;
		}
		if(!(defConst.find(identstr)->second))
		{
			if (!(defVar.find(identstr)->second))
			{
				defVar[identstr] = true;
				
			}	
			else
			{
				string str = tok.GetLexeme();
				str = "Variable Redefinition: " + str;
				ParseError(line, str);
				return false;
			}
		}
		else
		{
			string str = tok.GetLexeme();
			str = "Illegal use of a constant name as a variable: " + str;
			ParseError(line, str);
			return false;
		}
		
		
	}
	else
	{
		Parser::PushBackToken(tok);
		
		return true;
	}
	
	tok = Parser::GetNextToken(in, line);
	
	if (tok == COMMA) {
		status = IdentList(in, line);	
	}
	
	else if(tok == COLON)
	{
		Parser::PushBackToken(tok);
		return true;
	}
	else if(tok == IDENT)
	{
		ParseError(line, "Missing comma in declaration statement.");
		return false;
	}
	else {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	return status;
}//End of IdentList
	

//Stmt is either a simple statement or a structured one
//Stmt ::= SimpleStmt | StructuredStmt 
bool Stmt(istream& in, int& line) {
	bool status = false;
	
	LexItem t = Parser::GetNextToken(in, line);
	if(t == WRITELN || t == WRITE || t == IDENT || t == READLN)
	{
		Parser::PushBackToken(t);
		status = SimpleStmt(in, line);
		if (!status)
		{
			ParseError(line, "Incorrect Simple Statement.");
			return status;
		}
		return status;
	}
	else if( t == IF || t == BEGIN)
	{
		Parser::PushBackToken(t);
		status = StructuredStmt(in, line);
		if (!status)
		{
			ParseError(line, "Incorrect Structured Statement.");
			return status;
		}
		return status;
	}
	else 
	{
	
		Parser::PushBackToken(t);
		return true;
	}
	return status;
}//End of Stmt
	
//StructuredStmt ::= IfStmt | CompoundStmt
bool StructuredStmt(istream& in, int& line) {
	bool status;
	
	LexItem t = Parser::GetNextToken(in, line);
	switch( t.GetToken() ) {

	case IF: //Keyword consumed
		status = IfStmt(in, line);
		
		break;

	case BEGIN: //Keyword consumed
		status = CompStmt(in, line);
		break;
		
	default:
		;
	}

	return status;
}//End of StructuredStmt


//CompStmt ::= BEGIN Stmt {; Stmt } END
bool CompStmt(istream& in, int& line) {
	bool status;
	LexItem tok;
	
	status = Stmt(in, line);
	tok = Parser::GetNextToken(in, line);
	while(status && tok == SEMICOL)
	{
		status = Stmt(in, line);
		tok = Parser::GetNextToken(in, line);
		
	}
	
	if(status && tok == END)
	{
		
		return true;
	}	
	else if(status && tok == ELSE)
	{
		
		ParseError(line, "Illegal Else-clause.");
		return false;
		
	}
	else if( tok == DONE)
	{
		
		line--;
		ParseError(line, "Missing end of compound statement.");
		return false;
	}
	else if(status && tok != SEMICOL)
	{
		line--;
		
		ParseError(line, "Missing semicolon.");
		return false;
	}		
	if(!status)
	{
		ParseError(line, "Syntactic error in the statement.");
		Parser::PushBackToken(tok);	
		return false;
	}
	
}//end of CompStmt

//SimpleStmt ::= AssignStmt | WriteLnStmt | WriteStmt |ReadLnStmt
bool SimpleStmt(istream& in, int& line) {
	bool status;
	
	LexItem t = Parser::GetNextToken(in, line);
	
	switch( t.GetToken() ) {

	case WRITELN: //Keyword is consumed
		status = WriteLnStmt(in, line);
		
		break;

	case WRITE: //Keyword is consumed
		status = WriteStmt(in, line);
		break;

	case IDENT: //Keyword is not consumed
		Parser::PushBackToken(t);
        status = AssignStmt(in, line);
		
		break;
		
	case READLN: //Keyword is consumed
		status = ReadLnStmt(in, line);
		break;
	default:
	;	
	}

	return status;
}//End of SimpleStmt

//	ReadLnStmt ::= ReadLnStmt ( VarList )
bool ReadLnStmt(istream& in, int& line) {
	LexItem t;
		
	t = Parser::GetNextToken(in, line);
	if( t != LPAREN ) {
		
		ParseError(line, "Missing Left Parenthesis in ReadLn statement.");
		return false;
	}
	
	bool ex = VarList(in, line);
	
	if( !ex ) {
		ParseError(line, "Missing variable list for ReadLn statement.");
		return false;
	}
	
	t = Parser::GetNextToken(in, line);
	if(t != RPAREN ) {
		
		ParseError(line, "Missing Right Parenthesis for ReadLn statement.");
		return false;
	}
	
	return ex;
}//End of ReadLnStmt

//WriteLnStmt ::= writeln (ExprList) 
bool WriteLnStmt(istream& in, int& line) {
	LexItem t;
	
	
	t = Parser::GetNextToken(in, line);
	if( t != LPAREN ) {
		
		ParseError(line, "Missing Left Parenthesis in WriteLn statement.");
		return false;
	}
	
	bool ex = ExprList(in, line);
	
	if( !ex ) {
		ParseError(line, "Missing expression list for WriteLn statement.");
		return false;
	}
	
	t = Parser::GetNextToken(in, line);
	if(t != RPAREN ) {
		
		ParseError(line, "Missing Right Parenthesis for WriteLn statement.");
		return false;
	}
	
	return ex;
}//End of WriteLnStmt

//WriteStmt ::= write (ExprList) 
bool WriteStmt(istream& in, int& line) {
	LexItem t;
		
	t = Parser::GetNextToken(in, line);
	if( t != LPAREN ) {
		
		ParseError(line, "Missing Left Parenthesis in Write statement.");
		return false;
	}
	
	bool ex = ExprList(in, line);
	
	if( !ex ) {
		ParseError(line, "Missing expression list for Write statement.");
		return false;
	}
	
	t = Parser::GetNextToken(in, line);
	if(t != RPAREN ) {
		
		ParseError(line, "Missing Right Parenthesis for Write statement.");
		return false;
	}
	
	return ex;
}//End of WriteStmt

//IfStmt ::= IF Expr THEN Stmt [ ELSE Stmt ] 
bool IfStmt(istream& in, int& line) {
	bool ex=false, status ; 
	LexItem t;
			
	ex = Expr(in, line);
	if( !ex ) {
		ParseError(line, "Missing if statement Logic Expression.");
		return false;
	}
	
	t = Parser::GetNextToken(in, line);
	if(t != THEN)
	{
		ParseError(line, "If Statement Syntax Error.");
		return false;
	}
	status = Stmt(in, line);
	if(!status)
	{
		ParseError(line, "Missing Statement for If-Then-Part.");
		return false;
	}
	t = Parser::GetNextToken(in, line);
	if( t == ELSE ) {
		status = Stmt(in, line);
		if(!status)
		{
			ParseError(line, "Missing Statement for If-Else-Part.");
			return false;
		}
		return true;
	}
		
	Parser::PushBackToken(t);// semicolon pushed back or anything else
	return true;
}//End of IfStmt function

//Variable ::= IDENT
bool Variable(istream& in, int& line)
{
	string identstr;
	
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == IDENT){
		identstr = tok.GetLexeme();
		if(!(defConst.find(identstr)->second))
		{
			if (!(defVar.find(identstr)->second))
			{
				ParseError(line, "Undeclared Variable: "+ identstr);
				return false;
			}	
			return true;
		}
		else
		{
			ParseError(line, "Illegal use of a constant name as a variable");
			return false;
		}
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern.");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	return false;
}//End of Variable

//AssignStmt:= Variable := Expr
bool AssignStmt(istream& in, int& line) {
	bool varstatus = false, status = false;
	LexItem t;
	
	varstatus = Variable( in, line);
	
	if (varstatus){
		t = Parser::GetNextToken(in, line);
		
		if (t == ASSOP){
			status = Expr(in, line);
			if(!status) {
				ParseError(line, "Missing Expression in Assignment Statement.");
				return status;
			}
			
		}
		else if(t.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern.");
			cout << "(" << t.GetLexeme() << ")" << endl;
			return false;
		}
		else {
			ParseError(line, "Missing Assignment Operator.");
			return false;
		}
	}
	else {
		ParseError(line, "Missing Left-Hand Side Variable in Assignment statement.");
		return false;
	}
	return status;	
}

//	VarList ::= Variable {, Variable }
bool VarList(istream& in, int& line) {
	bool status = false;
	
	status = Variable(in, line);
	if(!status){
		ParseError(line, "Missing variable.");
		return false;
	}
	
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == COMMA) {
		status = VarList(in, line);
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern.");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}//VarList

//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
	bool status = false;
	
	status = Expr(in, line);
	if(!status){
		ParseError(line, "Missing Expression.");
		return false;
	}
	
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == COMMA) {
		
		status = ExprList(in, line);
		
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern.");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}//ExprList


//Expr ::= RelExpr ::= SimpleExpr  [ ( = | < | > ) SimpleExpr ]
bool Expr(istream& in, int& line) {
	LexItem tok;
	bool t1 = SimpleExpr(in, line);
		
	if( !t1 ) {
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern.");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	if ( tok == LTHAN || tok == GTHAN || tok == EQ) 
	{
		t1 = SimpleExpr(in, line);
		if( !t1 ) 
		{
			ParseError(line, "Missing operand after operator.");
			return false;
		}
		
		tok = Parser::GetNextToken(in, line);
		
		if(tok == LTHAN || tok == GTHAN || tok == EQ)
		{
			ParseError(line, "Illegal Relational Expression.");
			return false;
		}
		else if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern.");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}		
		
	}
	Parser::PushBackToken(tok);
	return true;
}//End of RelExpr

//Expr:= Term {(+|- | OR) Term}
bool SimpleExpr(istream& in, int& line) {
	
	bool t1 = Term(in, line);
	LexItem tok;
	
	if( !t1 ) {
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern.");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	while ( tok == PLUS || tok == MINUS || tok == OR) 
	{
		t1 = Term(in, line);
		if( !t1 ) 
		{
			ParseError(line, "Missing operand after operator.");
			return false;
		}
		
		tok = Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern.");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}		
		
	}
	Parser::PushBackToken(tok);
	return true;
}//End of Expr

//Term:= SFactor {( * | / | DIV | MOD | AND) SFactor}
bool Term(istream& in, int& line) {
	
	bool t1 = SFactor(in, line);
	LexItem tok;
	
	if( !t1 ) {
		return false;
	}
	
	tok	= Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern.");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
	}
	while ( tok == MULT || tok == DIV  || tok == IDIV || tok == MOD || tok == AND)
	{
		t1 = SFactor(in, line);
		
		if( !t1 ) {
			ParseError(line, "Missing operand after operator.");
			return false;
		}
		
		tok	= Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern.");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}
		
	}
	Parser::PushBackToken(tok);
	return true;
}//End of Term

//SFactor ::= [( - | + | NOT )] Factor
bool SFactor(istream& in, int& line)
{
	LexItem t = Parser::GetNextToken(in, line);
	bool status;
	int sign = 0;
	if(t == MINUS )
	{
		sign = -1;
	}
	else if(t == PLUS)
	{
		sign = 1;
	}
	else if(t == NOT)
	{
		sign = -2;//sign is a NOT op for logic operands
	}
	else
		Parser::PushBackToken(t);
		
	status = Factor(in, line, sign);
	return status;
}//End of SFactor

//Factor ::= IDENT |  ICONST | RCONST | SCONST | BCONST | (Expr)
bool Factor(istream& in, int& line, int sign) {
	
	LexItem tok = Parser::GetNextToken(in, line);
	
	if( tok == IDENT ) {
		
		string lexeme = tok.GetLexeme();
		
		if(!(defConst.find(lexeme)->second))
		{
			if (!(defVar.find(lexeme)->second))
			{
				ParseError(line, "Using Undefined Variable.");
				return false;	
			}
		}
		
		return true;
	}
	else if( tok == ICONST ) {
		
		return true;
	}
	else if( tok == SCONST ) {
		
		return true;
	}
	else if( tok == RCONST ) {
		
		return true;
	}
	else if( tok == BCONST ) {
		
		return true;
	}
	else if( tok == LPAREN ) {
		bool ex = Expr(in, line);
		if( !ex ) {
			ParseError(line, "Missing expression after Left Parenthesis.");
			return false;
		}
		if( Parser::GetNextToken(in, line) == RPAREN )
			return ex;
		else 
		{
			Parser::PushBackToken(tok);
			ParseError(line, "Missing right parenthesis after expression.");
			return false;
		}
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern.");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	Parser::PushBackToken(tok);
	
	return false;
}



