#include <iostream>
#include <string>
using namespace std;
#include "val_SP26.h"


Value Value::operator+(const Value& op) const
{
	if (IsErr() || op.IsErr())
		return Value();

	// numeric, int/real mix is fine
	if (IsInt() && op.IsInt())
		return Value(GetInt() + op.GetInt());
	if (IsInt() && op.IsReal())
		return Value((double)GetInt() + op.GetReal());
	if (IsReal() && op.IsInt())
		return Value(GetReal() + (double)op.GetInt());
	if (IsReal() && op.IsReal())
		return Value(GetReal() + op.GetReal());

	// string/char concat, char has to become a string first
	if (IsString() && op.IsString())
		return Value(GetString() + op.GetString());
	if (IsString() && op.IsChar())
		return Value(GetString() + op.GetChar());
	if (IsChar() && op.IsString())
		return Value(string(1, GetChar()) + op.GetString());
	if (IsChar() && op.IsChar())
		return Value(string(1, GetChar()) + string(1, op.GetChar()));

	return Value();
}


Value Value::operator/(const Value& op) const
{
	if (IsErr() || op.IsErr())
		return Value();

	// always real, even int/int
	if (IsInt() && op.IsInt())
		return Value((double)GetInt() / op.GetInt());
	if (IsInt() && op.IsReal())
		return Value((double)GetInt() / op.GetReal());
	if (IsReal() && op.IsInt())
		return Value(GetReal() / (double)op.GetInt());
	if (IsReal() && op.IsReal())
		return Value(GetReal() / op.GetReal());

	return Value();
}


Value Value::operator<(const Value& op) const
{
	if (IsErr() || op.IsErr())
		return Value();

	// same int/real widening as operator+
	if (IsInt() && op.IsInt())
		return Value(GetInt() < op.GetInt());
	if (IsInt() && op.IsReal())
		return Value((double)GetInt() < op.GetReal());
	if (IsReal() && op.IsInt())
		return Value(GetReal() < (double)op.GetInt());
	if (IsReal() && op.IsReal())
		return Value(GetReal() < op.GetReal());

	if (IsString() && op.IsString())
		return Value(GetString() < op.GetString());
	if (IsBool() && op.IsBool())
		return Value(GetBool() < op.GetBool());
	if (IsChar() && op.IsChar())
		return Value(GetChar() < op.GetChar());

	return Value();
}


Value Value::Trim() const
{
	// only strings, everything else is an error
	if (!IsString())
		return Value();

	string Result = GetString();
	while (!Result.empty() && Result.back() == ' ')
		Result.pop_back();

	return Value(Result);
}
