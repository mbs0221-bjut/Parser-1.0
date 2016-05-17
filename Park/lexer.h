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
	Token(){
		Tag = NULL;
	}
	Token(int tag){
		Tag = tag;
	}
	virtual string toString(){
		ostringstream fout;
		switch (Tag){
		case STRUCT:fout << "STRUCT"; break;
		case CLASS:fout << "CLASS"; break;
		case ENUM:fout << "ENUM"; break;
		case BASIC:fout << "BASIC"; break;
		case STRING:fout << "STRING"; break;
		case JSON:fout << "JSON"; break;
		case ID:fout << "ID"; break;
		case AND:fout << "AND"; break;
		case OR:fout << "OR"; break;
		case TRUE:fout << "TRUE"; break;
		case FALSE:fout << "FALSE"; break;
		case INC:fout << "INC"; break;
		case DEC:fout << "DEC"; break;
		case '+':fout << "ADD"; break;
		case '-':fout << "SUB"; break;
		case '*':fout << "MUL"; break;
		case '/':fout << "DIV"; break;
		case '%':fout << "MOD"; break;
		case '>':
		case '<':
		case GE:
		case BE:
		case EQ:
		case NE:fout << "CMP"; break;
		default:fout << (char)Tag; break;
		}
		return fout.str();
	}
};

struct Number :Token{
	int value;
	Number(int value){
		this->Tag = NUMBER;
		this->value = value;
	}
	virtual string toString(){
		ostringstream oss;
		oss << value;
		return oss.str();
	}
};

struct Real :Token{
	double value;
	Real(double value){
		this->Tag = REAL;
		this->value = value;
	}
	virtual string toString(){
		ostringstream oss;
		oss << value << "F";
		return oss.str();
	}
};

struct Word :Token{
	string word;
	static Word *Inc, *Dec, *And, *Or, *Not, *Eq, *Ne, *Ge, *Be, *Ptr;
	static Word *Neg, *Shl, *Shr, *True, *False, *Temp;
	Word(){
		this->Tag = NULL;
		this->word = "";
	}
	Word(int tag, string word) :Token(tag){
		this->word = word;
	}
	virtual string toString(){
		return word;
	}
};

Word *Word::Inc = new Word(INC, "INC");
Word *Word::Dec  = new Word(DEC, "DEC");
Word *Word::And = new Word(AND, "AND");
Word *Word::Or = new Word(OR, "OR");
Word *Word::Eq = new Word(EQ, "EQ");
Word *Word::Shl = new Word(SHL, "SHL");
Word *Word::Shr = new Word(SHR, "SHR");
Word *Word::Ne = new Word(NE, "NE");
Word *Word::Ge = new Word(GE, "GE");
Word *Word::Be = new Word(BE, "BE");
Word *Word::Ptr = new Word(POINTER, "POINTER");
Word *Word::Neg = new Word(NEG, "NEG");
Word *Word::True = new Word(TRUE, "TRUE");
Word *Word::False = new Word(FALSE, "FALSE");
Word *Word::Temp = new Word(TEMP, "TEMP");

struct Type :Word{
	int width;
	static Type *Int, *Char, *Double, *Float, *Bool;
	static Type *Void, *Enum, *Json, *Null, *Member;
	Type(){ Tag = BASIC; word = "type", width = 0; }
	Type(Word word, int width) :Word(word), width(width){}
	Type(int tag, string word, int width) :Word(tag, word), width(width){}
	static bool isType(Token* t){
		return t->Tag == BASIC || t->Tag == STRUCT || t->Tag == CLASS || t->Tag == ENUM;
	} 
	static Type *max(Type *type1, Type *type2){
		return type1->width > type2->width ? type1 : type2;
	}
	virtual string toString(){
		ostringstream oss;
		oss << word << ":" << width;
		return oss.str();
	}
};

Type *Type::Char = new Type(BASIC, "char", 1);
Type *Type::Int = new Type(BASIC, "int", 4);
Type *Type::Float = new Type(BASIC, "float", 8);
Type *Type::Double = new Type(BASIC, "double", 16);
Type *Type::Bool = new Type(BASIC, "bool", 1);
Type *Type::Void = new Type(BASIC, "void", 0);
Type *Type::Enum = new Type(ENUM, "enum", 4);
Type *Type::Json = new Type(JSON, "json", 0); 
Type *Type::Null = new Type(NUL, "null", 0);
Type *Type::Member = new Type(MEMBER, "member", 0);

struct Temp :Type{
	static int count;
	int number;//临时变量计数
	Temp() :Type(TEMP, "temp", 0){
		number = ++count;
	}
	virtual string toString(){
		return "T" + number;
	}
};

struct Array :Type{
	int size;
	Type *type;
	Array() :Type(ARRAY, "[]", 0), size(0){  }
	Array(int size, Type *type) :Type(ARRAY, "[]", size*type->width), size(size), type(type){  }
	virtual string toString(){
		ostringstream oss;
		oss << "[" << size << "]" << type->toString();
		return oss.str();
	}
};

int Temp::count = 0;

class Lexer{
private:
	int n_id = 0;
	int n_type;
	char peek = '\0';
	ifstream fin;
	map<string, Word*> keywords;
public:
	static int line, column;
	Lexer(string in)
	{
		fin = ifstream(in);
		if (fin.is_open()){
			cout << "Lexer construct" << endl;
		}
		keywords["include"] = new Word(INCLUDE, "INCLUDE");//match(INCLUDE)
		keywords["class"] = new Word(CLASS, "CLASS");//match(CLASS);
		keywords["struct"] = new Word(STRUCT, "STRUCT");
		keywords["extends"] = new Word(EXTENDS, "EXTENDS");
		keywords["implements"] = new Word(IMPLEMENTS, "IMPLEMENTS");
		keywords["lambda"] = new Word(LAMBDA, "LAMBDA");
		keywords["string"] = new Word(STRING, "STRING");
		keywords["auto"] = new Word(AUTO, "AUTO");
		keywords["print"] = new Word(PRINT, "PRINT");
		// 流程语句
		keywords["if"] = new Word(IF, "IF");
		keywords["else"] = new Word(ELSE, "ELSE");
		keywords["for"] = new Word(FOR, "FOR");
		keywords["do"] = new Word(DO, "DO");
		keywords["while"] = new Word(WHILE, "WHILE");
		keywords["switch"] = new Word(SWITCH, "SWITCH");
		keywords["case"] = new Word(CASE, "CASE");
		keywords["break"] = new Word(BREAK, "BREAK");
		keywords["continue"] = new Word(CONTINUE, "CONTINUE");
		keywords["sizeof"] = new Word(SIZEOF, "SIZEOF");
		// 保留字
		keywords["true"] = Word::True;
		keywords["false"] = Word::False;
		keywords["temp"] = Word::Temp;
		// 类型说明
		keywords["json"] = Type::Json;
		keywords["int"] = Type::Int;
		keywords["char"] = Type::Char;
		keywords["double"] = Type::Double;
		keywords["float"] = Type::Float;
		keywords["bool"] = Type::Bool;
		keywords["enum"] = Type::Enum;
		keywords["void"] = Type::Void;
		keywords["null"] = Type::Null;
	}
	~Lexer(){
		if (fin.is_open()){
			fin.close();
			cout << "Lexer deconstruct" << endl;
		}
	}
	bool read(char c){
		char a;
		fin.read(&a, sizeof(char));
		return a == c;
	}
	void putWord(Type *type){
		keywords[type->word] = type;
	}
	Word* getWord(string name){
		return keywords[name];
	}
	Token *scan(){//LL(1)
		if (fin.eof()){
			return new Token(EOF);
		} 
		while (fin.read(&peek, 1)){
			column++;
			if (peek == ' ' || peek == '\t')continue;
			else if (peek == '\n'){ column = 0; line++; }
			else if (peek == '/'){ 
				Token *t; 
				if (t = skip_comment()){
					return t;
				}
			}
			else break;
		}
		if (peek == '"'){// "THIS IS A TEST"
			return match_string();
		}
		if (isalpha(peek) || peek == '_'){// 
			return match_id();//a _
		}
		if (isdigit(peek)){
			if (peek == '0'){
				fin.read(&peek, 1);
				if (peek == 'x'){
					return match_hex();
				}
				else if (isdigit(peek) && peek >= '1'&&peek <= '7'){
					return match_oct();
				}
				fin.seekg(-1, ios_base::cur);// adsa+...
				return new Number(0);
			}
			else{
				return match_number();
			}
		}
		return match_other();
	}// a, b, c, int;
	Token *match_string(){
		string str;
		fin.read(&peek, 1);//
		while (peek != '"'){
			if (peek == '\\'){
				"\"";// ""
				fin.read(&peek, 1);// "\""
				switch (peek){
				case 'a':str.push_back('\a'); break;
				case 'b':str.push_back('\b'); break;
				case 'f':str.push_back('\f'); break;
				case 'n':str.push_back('\n'); break;
				case 'r':str.push_back('\r'); break;
				case 't':str.push_back('\t'); break;
				case 'v':str.push_back('\v'); break;
				case '\\':str.push_back('\\'); break;
				case '\'':str.push_back('\''); break;
				case '\"':str.push_back('"'); break;
				case '?':str.push_back('\?'); break;
				case '0':str.push_back('\0'); break;
				default:str.push_back('\\'); str.push_back(peek); break;
				}
			}
			else{
				str.push_back(peek);
			}
			fin.read(&peek, 1);
		}
		return new Word(STRING, str);// match(STRING); char a = "";match(BASIC); MATCH(ID); MATCH('='); MATCH(STRING);
	}
	Token *match_id(){
		string str;
		while (isalnum(peek) || peek == '_'){
			str.push_back(peek);
			fin.read(&peek, 1);
		}
		fin.seekg(-1, ios_base::cur);// adsa+...
		map<string, Word*>::iterator iter;
		iter = keywords.find(str);
		if (iter == keywords.end()){
			return new Word(ID, str);// a1211
		}
		else{
			return (*iter).second;// 
		}
	}
	Token *match_number(){
		int ivalue = 0;
		while (isdigit(peek)){
			ivalue = 10 * ivalue + peek - '0';// 123e-3
			fin.read(&peek, 1);
		}
		if (peek == 'e' || peek == 'E'){// 123e3
			double dvalue = ivalue;
			int positive = 1;
			int index = 0;
			fin.read(&peek, 1);
			if (peek == '-'){
				positive = -1;
			}
			else if (peek == '+'){
				positive = 1;
			}
			else {
				fin.seekg(-1, ios_base::cur);
			}
			fin.read(&peek, 1);
			while (isdigit(peek)){
				index = 10 * index + peek - '0';
				fin.read(&peek, 1);
			}
			fin.seekg(-1, ios_base::cur);
			if (positive == 1){
				while (index-- > 0) dvalue *= 10;
			}
			else{
				while (index-- > 0) dvalue /= 10;
			}
			return new Real(dvalue);
		}
		if (peek == '.'){// 23.89
			double dvalue = ivalue;
			double power = 1;
			fin.read(&peek, 1);
			while (isdigit(peek)){
				power *= 10;
				dvalue = dvalue + (peek - '0') / power;
				fin.read(&peek, 1);
			}
			if (peek == 'f' || peek == 'F'){
				return new Real(dvalue);
			}
			if (peek == 'e' || peek == 'E'){
				int positive = 1;
				int index = 0;
				fin.read(&peek, 1);
				if (peek == '-'){
					positive = -1;
					fin.read(&peek, 1);
				}
				if (peek == '+'){
					fin.read(&peek, 1);
				}
				while (isdigit(peek)){
					index = 10 * index + peek - '0';
					fin.read(&peek, 1);
				}
				fin.seekg(-1, ios_base::cur);
				if (positive == 1){
					while (index-- > 0) dvalue *= 10;
				}
				else{
					while (index-- > 0) dvalue /= 10;
				}
				return new Real(dvalue);
			}
			fin.seekg(-1, ios_base::cur);
			return new Real(dvalue);
		}
		fin.seekg(-1, ios_base::cur);
		return new Number(ivalue);
	}
	Token *match_other(){
		switch (peek){
		case '+':
			if (read('+'))return Word::Inc; else {
				fin.seekg(-1, ios_base::cur);
				return new Token('+');
			}
		case '-':
			if (read('-'))
				return Word::Dec;
			else 
				fin.seekg(-1, ios_base::cur);
			if (read('>'))
				return Word::Ptr;
			else
				fin.seekg(-1, ios_base::cur);
			return new Token('-');
		case '&':
			if (read('&'))return Word::And; else {
				fin.seekg(-1, ios_base::cur);
				return new Token('&');
			}
		case '|':
			if (read('|'))return Word::Or; else {
				fin.seekg(-1, ios_base::cur);
				return new Token('|');
			}
		case '!':
			if (read('='))return Word::Ne; else {
				fin.seekg(-1, ios_base::cur);
				return new Token('!');
			}
		case '=':
			if (read('='))return Word::Eq; else {
				fin.seekg(-1, ios_base::cur);
				return new Token('=');
			}
		case '>':
			if (read('='))
				return Word::Ge; 
			else 
				fin.seekg(-1, ios_base::cur);
			if (read('>'))
				return Word::Shr; 
			else
				fin.seekg(-1, ios_base::cur);
			return new Token('>');
		case '<':
			if (read('='))
				return Word::Be; 
			else 
				fin.seekg(-1, ios_base::cur);
			if (read('<'))
				return Word::Shl;
			else
				fin.seekg(-1, ios_base::cur);
			return new Token('<');
		default:
			return new Token(peek);
		}
	}
	Token *skip_comment(){
		if (peek == '/'){
			fin.read(&peek, 1);
			if (peek == '*'){
				do{
					fin.read(&peek, 1);
					if (peek == '\n')line++;
					if (peek == '*'){
						fin.read(&peek, 1);/**/
						if (peek == '\n')line++;
						if (peek == '/')break;
					}
				} while (peek != EOF);
				return nullptr;
			}
			else if (peek == '/')
			{
				while (!read('\n'));
				fin.seekg(-1, ios_base::cur);
				return nullptr;
			}
			else
			{
				fin.seekg(-1, ios_base::cur);
			}
			return new Token('/');
		}
		return nullptr;
	}
	Token *match_hex(){
		int num = 0;
		do{
			fin.read(&peek, 1);
			if (isdigit(peek)){
				num = 16 * num + peek - '0';
			}
			else if (tolower(peek) >= 'a'&&tolower(peek) <= 'f'){
				num = 16 * num + 10 + tolower(peek) - 'a';
			}
			else{
				fin.seekg(-1, ios_base::cur);// adsa+...
				return new Number(num);
			}
		} while (true);
	}
	Token *match_oct(){
		int num = 0;
		if (peek >= '1'&&peek <= '7'){
			num = 8 * num + peek - '0';
			fin.read(&peek, 1);
			do{
				if (peek >= '0'&&peek <= '7'){
					num = 8 * num + peek - '0';
				}else{
					fin.seekg(-1, ios_base::cur);// adsa+...
					return new Number(num);
				}
				fin.read(&peek, 1);
			} while (true);
		}else{
			fin.seekg(-1, ios_base::cur);// adsa+...
			return new Number(num);
		}
	}
};

int Lexer::line = 1;
int Lexer::column = 1;

#endif