#ifndef __LLPARSER_H__
#define __LLPARSER_H__

#include "inter.h"
#include <iomanip>

class LLParser
{
private:
	Lexer *lexer;
	Token *look;
	ofstream fout;
	list<Environment *> envs;
	Environment *env;
	Nodes nodes;
	void pushEnv();
	void popEnv();
	void move();
	void match(int c);

public:
	LLParser(string in, string out);
	~LLParser();
	void program();

protected:
	Node *def();
	Node *def_assign();
	Node *def_json();
	Node *def_var();
	Node *def_func(string name, Type *type);
	Node *def_class();
	Node *def_struct();
	Node *def_interface();
	Node *def_enum();
	// Identifier
	void putId(Id *id);
	Id *getId();
	// Lambda
	void lambda_expr();
	// Statement
	Statement *stmts();
	Statement *stmt();
	Statement *stmt_var();
	Statement *stmt_if();
	Statement *stmt_for();
	Statement *stmt_while();
	Statement *stmt_do();
	Statement *stmt_assign();
	Statement *stmt_auto();
	Statement *stmt_print();
	Statement *stmt_switch();
	Statement *stmt_break();
	Statement *stmt_continue();
	// Expression
	Expr *boolean();
	Expr *join();
	Expr *equality();
	Expr *relation();
	Expr *shift();
	Expr *expr();
	Expr *term();
	Expr *unary();
	Expr *factor();
	Expr *access();
	Expr *member();
	// Json Value
	JSONValue *json_value();
	JSONArray *json_array();
	JSONObject *json_object();
	JSONPair *json_pair();
	JSONValue *json_attr();
	JSONString *json_string();
};

#endif