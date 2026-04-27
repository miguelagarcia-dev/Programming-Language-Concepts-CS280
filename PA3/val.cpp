#include "val_SP26.h"

// operator+: numeric add, or string/char concat
Value Value::operator+(const Value& op) const {
    // string concat, works for any combo of string/char
    if ((IsString() || IsChar()) && (op.IsString() || op.IsChar())) {
        string L = IsString() ? GetString() : string(1, GetChar());
        string R = op.IsString() ? op.GetString() : string(1, op.GetChar());
        return Value(L + R);
    }
    if (IsInt() && op.IsInt())   return Value(GetInt() + op.GetInt());
    if (IsReal() && op.IsReal()) return Value(GetReal() + op.GetReal());
    if (IsInt() && op.IsReal())  return Value((double)GetInt() + op.GetReal());
    if (IsReal() && op.IsInt())  return Value(GetReal() + (double)op.GetInt());
    return Value(); // mismatched types, so returns VERR
}

// operator-: numeric subtract (binary)
Value Value::operator-(const Value& op) const {
    if (IsInt() && op.IsInt())   return Value(GetInt() - op.GetInt());
    if (IsReal() && op.IsReal()) return Value(GetReal() - op.GetReal());
    if (IsInt() && op.IsReal())  return Value((double)GetInt() - op.GetReal());
    if (IsReal() && op.IsInt())  return Value(GetReal() - (double)op.GetInt());
    return Value();
}

// operator*: numeric multiply
Value Value::operator*(const Value& op) const {
    if (IsInt() && op.IsInt())   return Value(GetInt() * op.GetInt());
    if (IsReal() && op.IsReal()) return Value(GetReal() * op.GetReal());
    if (IsInt() && op.IsReal())  return Value((double)GetInt() * op.GetReal());
    if (IsReal() && op.IsInt())  return Value(GetReal() * (double)op.GetInt());
    return Value();
}

// operator/: real division, always returns VREAL, so caller checks div-by-zero
Value Value::operator/(const Value& op) const {
    double divisor;
    if (op.IsInt())       divisor = op.GetInt();
    else if (op.IsReal()) divisor = op.GetReal();
    else return Value();

    if (divisor == 0.0) return Value(); // caller catches this before calling

    double dividend;
    if (IsInt())       dividend = GetInt();
    else if (IsReal()) dividend = GetReal();
    else return Value();

    return Value(dividend / divisor);
}

// idiv: integer division, so both sides must be int
Value Value::idiv(const Value& oper) const {
    if (!IsInt() || !oper.IsInt()) return Value();
    if (oper.GetInt() == 0) return Value();
    return Value(GetInt() / oper.GetInt());
}

// operator%: modulus, so both sides must be int and sign follows left operand
Value Value::operator%(const Value& op) const {
    if (!IsInt() || !op.IsInt()) return Value();
    if (op.GetInt() == 0) return Value();
    return Value(GetInt() % op.GetInt());
}

// operator==: equality, so works across compatible numeric types
Value Value::operator==(const Value& op) const {
    if (IsInt() && op.IsInt())       return Value(GetInt() == op.GetInt());
    if (IsReal() && op.IsReal())     return Value(GetReal() == op.GetReal());
    if (IsInt() && op.IsReal())      return Value((double)GetInt() == op.GetReal());
    if (IsReal() && op.IsInt())      return Value(GetReal() == (double)op.GetInt());
    if (IsString() && op.IsString()) return Value(GetString() == op.GetString());
    if (IsChar() && op.IsChar())     return Value(GetChar() == op.GetChar());
    if (IsBool() && op.IsBool())     return Value(GetBool() == op.GetBool());
    return Value();
}

// operator>: greater than
Value Value::operator>(const Value& op) const {
    if (IsInt() && op.IsInt())       return Value(GetInt() > op.GetInt());
    if (IsReal() && op.IsReal())     return Value(GetReal() > op.GetReal());
    if (IsInt() && op.IsReal())      return Value((double)GetInt() > op.GetReal());
    if (IsReal() && op.IsInt())      return Value(GetReal() > (double)op.GetInt());
    if (IsString() && op.IsString()) return Value(GetString() > op.GetString());
    if (IsChar() && op.IsChar())     return Value(GetChar() > op.GetChar());
    return Value();
}

// operator<: less than
Value Value::operator<(const Value& op) const {
    if (IsInt() && op.IsInt())       return Value(GetInt() < op.GetInt());
    if (IsReal() && op.IsReal())     return Value(GetReal() < op.GetReal());
    if (IsInt() && op.IsReal())      return Value((double)GetInt() < op.GetReal());
    if (IsReal() && op.IsInt())      return Value(GetReal() < (double)op.GetInt());
    if (IsString() && op.IsString()) return Value(GetString() < op.GetString());
    if (IsChar() && op.IsChar())     return Value(GetChar() < op.GetChar());
    return Value();
}

// operator&&: logical AND, both must be bool
Value Value::operator&&(const Value& op) const {
    if (IsBool() && op.IsBool()) return Value(GetBool() && op.GetBool());
    return Value();
}

// operator||: logical OR, both must be bool
Value Value::operator||(const Value& op) const {
    if (IsBool() && op.IsBool()) return Value(GetBool() || op.GetBool());
    return Value();
}

// operator!: logical NOT
Value Value::operator!() const {
    if (IsBool()) return Value(!GetBool());
    return Value();
}

// unary operator-: negates numbers, trims strings/chars
Value Value::operator-() const {
    if (IsInt())    return Value(-GetInt());
    if (IsReal())   return Value(-GetReal());
    if (IsString()) return Trim();
    if (IsChar())   return Trim();
    return Value();
}

// Trim: strips trailing spaces from a string or char operand
Value Value::Trim() const {
    if (IsString()) {
        string s = GetString();
        size_t last = s.find_last_not_of(' ');
        if (last == string::npos) return Value(string(""));
        return Value(s.substr(0, last + 1));
    }
    if (IsChar()) {
        char c = GetChar();
        // trimming a single char, so if it's a space it just disappears
        if (c == ' ') return Value(string(""));
        return Value(string(1, c));
    }
    return Value();
}
