#include "lex.h"
#include <map>
#include <cctype>
using namespace std;

static string tokenName(Token t) {
    switch (t) {
        case PROGRAM: return "PROGRAM";
        case VAR:     return "VAR";
        case CONST:   return "CONST";
        case IF:      return "IF";
        case THEN:    return "THEN";
        case ELSE:    return "ELSE";
        case BEGIN:   return "BEGIN";
        case END:     return "END";
        case WRITE:   return "WRITE";
        case WRITELN: return "WRITELN";
        case READLN:  return "READLN";
        case INTEGER: return "INTEGER";
        case REAL:    return "REAL";
        case BOOLEAN: return "BOOLEAN";
        case CHAR:    return "CHAR";
        case STRING:  return "STRING";
        case AND:     return "AND";
        case OR:      return "OR";
        case NOT:     return "NOT";
        case IDIV:    return "IDIV";
        case MOD:     return "MOD";
        case TRUE:    return "TRUE";
        case FALSE:   return "FALSE";
        case IDENT:   return "IDENT";
        case ICONST:  return "ICONST";
        case RCONST:  return "RCONST";
        case SCONST:  return "SCONST";
        case CCONST:  return "CCONST";
        case BCONST:  return "BCONST";
        case PLUS:    return "PLUS";
        case MINUS:   return "MINUS";
        case MULT:    return "MULT";
        case DIV:     return "DIV";
        case ASSOP:   return "ASSOP";
        case EQ:      return "EQ";
        case LTHAN:   return "LTHAN";
        case GTHAN:   return "GTHAN";
        case COMMA:   return "COMMA";
        case SEMICOL: return "SEMICOL";
        case LPAREN:  return "LPAREN";
        case RPAREN:  return "RPAREN";
        case COLON:   return "COLON";
        case DOT:     return "DOT";
        case LBRACE:  return "LBRACE";
        case RBRACE:  return "RBRACE";
        case ERR:     return "ERR";
        case DONE:    return "DONE";
        default:      return "UNKNOWN";
    }
}

ostream& operator<<(ostream& out, const LexItem& tok) {
    Token t = tok.GetToken();
    string name = tokenName(t);

    if (t == ERR) {
        out << "ERR: Error in line (" << tok.GetLinenum() << ") " << tok.GetLexeme();
    } else if (t == IDENT) {
        out << "IDENT: <" << tok.GetLexeme() << ">";
    } else if (t == ICONST || t == RCONST || t == TRUE || t == FALSE || t == BCONST) {
        out << name << ": (" << tok.GetLexeme() << ")";
    } else if (t == SCONST || t == CCONST) {
        out << name << ": '" << tok.GetLexeme() << "'";
    } else {
        out << name << ": \"" << tok.GetLexeme() << "\"";
    }
    out << "\n";
    return out;
}

LexItem id_or_kw(const string& lexeme, int linenum) {
    static map<string, Token> kwMap = {
        {"program", PROGRAM}, {"var", VAR},     {"const", CONST},
        {"if", IF},           {"then", THEN},   {"else", ELSE},
        {"begin", BEGIN},     {"end", END},     {"write", WRITE},
        {"writeln", WRITELN}, {"readln", READLN},
        {"integer", INTEGER}, {"real", REAL},   {"boolean", BOOLEAN},
        {"char", CHAR},       {"string", STRING},
        {"and", AND},         {"or", OR},       {"not", NOT},
        {"div", IDIV},        {"mod", MOD},
        {"true", TRUE},       {"false", FALSE}
    };

    // Keywords are case-insensitive (for exanple END == end)
    string lower = lexeme;
    for (char& c : lower) c = tolower((unsigned char)c);

    auto it = kwMap.find(lower);
    if (it != kwMap.end())
        return LexItem(it->second, lower, linenum);

    return LexItem(IDENT, lexeme, linenum);
}

LexItem getNextToken(istream& in, int& linenum) {
    char ch;

    // Skipping whitespace, counting newlines
    bool found = false;
    while (in.get(ch)) {
        if (ch == '\n') linenum++;
        if (!isspace((unsigned char)ch)) { found = true; break; }
    }
    if (!found) return LexItem(DONE, "", linenum);

    // Letters -> identifier or keyword
    if (isalpha((unsigned char)ch)) {
        string lexeme;
        lexeme += ch;
        int p;
        while ((p = in.peek()) != EOF &&
               (isalnum((unsigned char)p) || p == '_' || p == '$')) {
            in.get(ch);
            lexeme += ch;
        }
        return id_or_kw(lexeme, linenum);
    }

    // Digits -> integer or real
    if (isdigit((unsigned char)ch)) {
        string lexeme;
        lexeme += ch;

        // Collect integer digits
        while (in.peek() != EOF && isdigit((unsigned char)in.peek())) {
            in.get(ch);
            lexeme += ch;
        }

        // Checking for decimal point
        if (in.peek() == '.') {
            in.get(ch); // consume '.'
            if (in.peek() != EOF && isdigit((unsigned char)in.peek())) {
                // Real number: consume fraction digits
                lexeme += '.';
                while (in.peek() != EOF && isdigit((unsigned char)in.peek())) {
                    in.get(ch);
                    lexeme += ch;
                }

                // Second dot is invalid float
                if (in.peek() == '.') {
                    in.get(ch);
                    lexeme += '.';
                    while (in.peek() != EOF && isdigit((unsigned char)in.peek())) {
                        in.get(ch);
                        lexeme += ch;
                    }
                    string errMsg = "Invalid floating-point constant \"" + lexeme + "\"";
                    return LexItem(ERR, errMsg, linenum);
                }

                // Checking for exponent
                if (in.peek() == 'E' || in.peek() == 'e') {
                    in.get(ch);
                    lexeme += ch;

                    int p2 = in.peek();
                    if (p2 == '+' || p2 == '-') {
                        in.get(ch);
                        lexeme += ch;
                        int p3 = in.peek();
                        // Another sign after sign: error
                        if (p3 == '+' || p3 == '-') {
                            in.get(ch);
                            lexeme += ch;
                            string errMsg = "Invalid exponent for floating-point constant \"" + lexeme + "\"";
                            return LexItem(ERR, errMsg, linenum);
                        }
                        // E/e after sign: error
                        if (p3 == 'E' || p3 == 'e') {
                            in.get(ch);
                            lexeme += ch;
                            string errMsg = "Invalid exponent for floating-point constant \"" + lexeme + "\"";
                            return LexItem(ERR, errMsg, linenum);
                        }
                        // No digits after sign: error
                        if (p3 == EOF || !isdigit((unsigned char)p3)) {
                            string errMsg = "Invalid exponent for floating-point constant \"" + lexeme + "\"";
                            return LexItem(ERR, errMsg, linenum);
                        }
                        // Digits
                        while (in.peek() != EOF && isdigit((unsigned char)in.peek())) {
                            in.get(ch);
                            lexeme += ch;
                        }
                        // Another E/e after exponent digits: error
                        if (in.peek() == 'E' || in.peek() == 'e') {
                            in.get(ch);
                            lexeme += ch;
                            string errMsg = "Invalid exponent for floating-point constant \"" + lexeme + "\"";
                            return LexItem(ERR, errMsg, linenum);
                        }
                        return LexItem(RCONST, lexeme, linenum);
                    }

                    // No sign: check for another E/e (double exponent indicator)
                    if (p2 == 'E' || p2 == 'e') {
                        in.get(ch);
                        lexeme += ch;
                        string errMsg = "Invalid exponent for a floating-point constant \"" + lexeme + "\"";
                        return LexItem(ERR, errMsg, linenum);
                    }

                    // No digits: error
                    if (p2 == EOF || !isdigit((unsigned char)p2)) {
                        string errMsg = "Invalid exponent for a floating-point constant \"" + lexeme + "\"";
                        return LexItem(ERR, errMsg, linenum);
                    }

                    // Digits
                    while (in.peek() != EOF && isdigit((unsigned char)in.peek())) {
                        in.get(ch);
                        lexeme += ch;
                    }
                    // Another E/e after exponent digits: error
                    if (in.peek() == 'E' || in.peek() == 'e') {
                        in.get(ch);
                        lexeme += ch;
                        string errMsg = "Invalid exponent for floating-point constant \"" + lexeme + "\"";
                        return LexItem(ERR, errMsg, linenum);
                    }
                    return LexItem(RCONST, lexeme, linenum);
                }

                return LexItem(RCONST, lexeme, linenum);
            } else {
                // Next char after '.' is not a digit: put back '.' and return ICONST
                in.putback('.');
                return LexItem(ICONST, lexeme, linenum);
            }
        }

        return LexItem(ICONST, lexeme, linenum);
    }

    // Single quote -> string or char literal
    if (ch == '\'') {
        string content;
        while (true) {
            if (!in.get(ch)) {
                // EOF inside string literal
                string errMsg = "New line is not allowed within string literal \"'" + content + "\"";
                return LexItem(ERR, errMsg, linenum);
            }
            if (ch == '\n') {
                // Newline inside string literal (do NOT increment linenum before returning error)
                string errMsg = "New line is not allowed within string literal \"'" + content + "\"";
                return LexItem(ERR, errMsg, linenum);
            }
            if (ch == '\'') {
                if (content.length() == 1)
                    return LexItem(CCONST, content, linenum);
                return LexItem(SCONST, content, linenum);
            }
            content += ch;
        }
    }

    // Curly-brace comment { ... }
    if (ch == '{') {
        while (in.get(ch)) {
            if (ch == '\n') linenum++;
            if (ch == '}')
                return getNextToken(in, linenum);
        }
        string errMsg = "Missing closing symbol(s) for a comment \"{\"";
        return LexItem(ERR, errMsg, linenum);
    }

    // Parenthesis or paren-star comment (* ... *)
    if (ch == '(') {
        if (in.peek() == '*') {
            in.get(ch); // consume '*'
            while (in.get(ch)) {
                if (ch == '\n') linenum++;
                if (ch == '(') {
                    if (in.peek() == '*') {
                        string errMsg = "Invalid nesting of comments \"(*(*\"";
                        return LexItem(ERR, errMsg, linenum);
                    }
                }
                if (ch == '*') {
                    if (in.peek() == ')') {
                        in.get(ch); // consume ')'
                        return getNextToken(in, linenum);
                    }
                }
            }
            string errMsg = "Missing closing symbol(s) for a comment \"(*\"";
            return LexItem(ERR, errMsg, linenum);
        }
        return LexItem(LPAREN, "(", linenum);
    }

    // Colon or assignment
    if (ch == ':') {
        if (in.peek() == '=') {
            in.get(ch);
            return LexItem(ASSOP, ":=", linenum);
        }
        return LexItem(COLON, ":", linenum);
    }

    // Single-character tokens
    switch (ch) {
        case '+': return LexItem(PLUS,   "+", linenum);
        case '-': return LexItem(MINUS,  "-", linenum);
        case '*': return LexItem(MULT,   "*", linenum);
        case '/': return LexItem(DIV,    "/", linenum);
        case '=': return LexItem(EQ,     "=", linenum);
        case '<': return LexItem(LTHAN,  "<", linenum);
        case '>': return LexItem(GTHAN,  ">", linenum);
        case ',': return LexItem(COMMA,  ",", linenum);
        case ';': return LexItem(SEMICOL,";", linenum);
        case ')': return LexItem(RPAREN, ")", linenum);
        case '.': return LexItem(DOT,    ".", linenum);
        default: {
            string errMsg = "Invalid character for starting a token \"";
            errMsg += ch;
            errMsg += "\"";
            return LexItem(ERR, errMsg, linenum);
        }
    }
}
