#ifndef _INTER_H_
#define _INTER_H_

#include "lexer.h"

using namespace std;

//----------------------�ڵ�---------------------------------------
struct Node
{
	int line;
	int begin, end;
	Node();
	static int lable;				// ȫ�ֱ��
	static map<string, int> lables; // ȫ�ֱ��
	static int MakeLable();
	static string Lable(int lable);
	virtual string toString();
};

//----------------------�����Ե����-------------------------------
struct Statement : Node
{
	virtual string toString();
};

//----------------------����ʽ��-----------------------------------
struct Expr : Statement
{
	Token *token; // & | ~ && ||
	Type *type;
	Expr();
	Expr(Token *token, Type *type);
	virtual string toString();
};

struct Constant : Expr
{
	static Constant *True, *False;
	Constant(Token *token, Type *type);
	virtual string toString();
};

struct Id : Expr
{
	int offset;
	Id();
	Id(Id *id);
	Id(Word *word, Type *type);
	virtual string toString();
};

struct Member : Id
{
	Id *root;
	Member(Id *root, Word *word, Type *type, int off);
	virtual string toString();
};

//----------------------˫Ŀ�������ʽ��---------------------------
struct Binocular : Expr
{
	Expr *expr1, *expr2;
	Binocular(Token *token, Expr *expr1, Expr *expr2);
	virtual string toString();
};

//----------------------��Ŀ�������ʽ��---------------------------
struct Unary : Expr
{
	Expr *expr;
	Unary(Token *token, Expr *expr);
	virtual string toString();
};

struct Pointer : Expr
{
	Pointer(Id *root, Word *word);
};

struct Access : Expr
{
	Id *id;
	Expr *expr;
	Access(Id *id, Expr *expr, Type *type);
	virtual string toString();
};

//----------------------����---------------------------------------
struct CompareId
{
	bool operator()(const pair<string, Id> &id1, const pair<string, Id> &id2);
};

struct Environment
{
	CompareId comp;
	map<string, Id> table;
	int width = 0;
	static int lable;
	int nlable;
	Environment();
	void put(Id *id);
	Id *get(string name);
	string toString();
};

struct Statements : Statement
{
	vector<Statement *> stmts;
	Statements();
	void addStatement(Statement *stmt);
	virtual string toString();
};

struct Print : Statement
{
	Expr *expr;
	Print();
	Print(Expr *expr);
	virtual string toString();
};

struct Assign : Statement
{
	Id *id;
	Expr *expr;
	Assign();
	Assign(Id *id, Expr *expr);
	virtual string toString();
};

struct IfElse : Statement
{
	Expr *expr;
	IfElse();
	IfElse(Expr *expr);
	virtual string toString();
};

struct If : IfElse
{
	Statement *stmt;
	If();
	If(Expr *expr, Statement *stmt);
	virtual string toString();
};

struct Else : IfElse
{
	Statement *stmt1;
	Statement *stmt2;
	Else();
	Else(Expr *expr, Statement *stmt1, Statement *stmt2);
	virtual string toString();
};

struct Switch : Statement
{
	Expr *expr;
	map<int, Statement *> cases;
	Switch();
	void addCase(int i, Statement *stmts);
	virtual string toString();
};

struct DoWhile : Statement
{
	Statement *stmt;
	Expr *expr;
	DoWhile();
	DoWhile(Statement *stmt, Expr *expr);
	virtual string toString();
};

struct While : Statement
{
	Expr *expr;
	Statement *stmt;
	While();
	While(Expr *expr, Statement *stmt);
	virtual string toString();
};

struct For : Statement
{
	Statement *assign1;
	Expr *expr;
	Statement *assign2;
	Statement *stmt;
	For();
	For(Statement *assign1, Expr *expr, Statement *assign2, Statement *stmt);
	virtual string toString();
};

struct Break : Statement
{
	Statement *stmt;
	static stack<Statement *> cur;
	Break();
	virtual string toString();
};

struct Continue : Statement
{
	Statement *stmt;
	static stack<Statement *> cur;
	Continue();
	virtual string toString();
};

//----------------------�����Ե����-------------------------------
struct Nodes : Node
{
	vector<Node *> nodes;
	Nodes();
	Nodes(Nodes *nodes);
	void addNode(Node *node);
	void addNodes(Nodes *node);
	virtual string toString();
};

struct Declaration : Statement, Nodes
{
	Declaration(Nodes *nodes);
};

struct Function : Node, Type
{
	Type *type;
	string name;
	Statements body;
	Environment *env;
	Function(string name, Type *type);
	void addStatement(Statement *stmt);
	virtual string toString();
};

struct Enum : Node, Temp
{
	map<string, int> vars;
	int offset = 0;
	Enum();
	Enum(string name);
	void addVariable(string name);
	int getVariable(string name);
	virtual string toString();
};

struct Struct : Type
{
	Environment *env;
	Struct();
	Struct(string name);
	Id *getVariable(string name);
	virtual string toString();
};

struct Class : Nodes, Type
{
	Class *parrent;
	Environment *env;
	list<Function *> functions;
	Class(string name);
	Id *getVariable(string name);
	virtual string toString();
};

//----------------------JSON----------------------------------
struct JSONValue : Node
{
	virtual string toString();
};

struct JSONString : JSONValue
{
	string str;
	JSONString(string str);
	virtual string toString();
};

struct JSONInt : JSONValue
{
	int num;
	JSONInt(int num);
	virtual string toString();
};

struct JSONReal : JSONValue
{
	double real;
	JSONReal(double d);
	virtual string toString();
};

struct JSONPair : Node
{
	JSONString *name;
	JSONValue *node;
	JSONPair();
	JSONPair(JSONString *name, JSONValue *node);
	virtual string toString();
};

struct JSONObject : JSONValue
{
	vector<JSONPair *> json;
	void addPair(JSONPair *p);
	JSONValue *operator[](string name);
	virtual string toString();
};

struct JSONArray : JSONValue
{
	vector<Node *> json;
	void addNode(Node *p);
	virtual string toString();
};

struct Json : Node
{
	string name;
	Node *node;
	Json(string name, Node *node);
	virtual string toString();
};

#endif