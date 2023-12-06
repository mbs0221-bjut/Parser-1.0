#ifndef _LEXER_H_
#define _LEXER_H_

#include <map>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <array>
#include <vector>
#include <queue>
#include <stack>
#include <list>

using namespace std;

enum Tag{
	IF = 256, ELSE, FOR, DO, WHILE, SWITCH, CASE, BREAK, CONTINUE,
	EXTENDS, IMPLEMENTS, AUTO, BASIC, REAL, ID, NUMBER, STRING, JSON, NUL,
	INCLUDE, DEFINE, STRUCT, CLASS, ENUM, LAMBDA, PRINT,
	AND, OR, EQ, NE, GE, BE, NEG, TRUE, FALSE,
	MEMBER, POINTER, INC, DEC, SHL, SHR, SIZEOF, FUNCTION,
	PUBLIC, PROTECTED, PRIVATE,
	STATEMENTS, STATEMENT,
	TEMP, ARRAY, INDEX
};

struct Token{
	int Tag;
	Token();
	Token(int tag);
	virtual string toString();
};

struct Number :Token{
	int value;
	Number(int value);
	virtual string toString();
};

struct Real :Token{
	double value;
	Real(double value);
	virtual string toString();
};

struct Word :Token{
	string word;
	static Word *Inc, *Dec, *And, *Or, *Not, *Eq, *Ne, *Ge, *Be, *Ptr;
	static Word *Neg, *Shl, *Shr, *True, *False, *Temp;
	Word();
	Word(int tag, string word);
	virtual string toString();
};

struct Type :Word{
	int width;
	static Type *Int, *Char, *Double, *Float, *Bool;
	static Type *Void, *Enum, *Json, *Null, *Member;
	Type();
	Type(Word word, int width);
	Type(int tag, string word, int width);
	static bool isType(Token *t);
	static Type *max(Type *type1, Type *type2);
	virtual string toString();
};

struct Temp :Type{
	static int count;
	int number;
	Temp();
	virtual string toString();
};

struct Array :Type{
	int size;
	Type *type;
	Array();
	Array(int size, Type *type);
	virtual string toString();
};

class Lexer{
private:
	int n_id = 0;
	int n_type;
	char peek = '\0';
	ifstream fin;
	map<string, Word*> keywords;
public:
	static int line, column;
	Lexer(string in);
	~Lexer();
 bool read(char c);
	void putWord(Type *type);
	Word *getWord(string name);
	Token *scan();
	Token *match_string();
	Token *match_id();
	Token *match_number();
	Token *match_other();
	Token *skip_comment();
	Token *match_hex();
	Token *match_oct();
};

#endif