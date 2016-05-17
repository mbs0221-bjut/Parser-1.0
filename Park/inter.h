#ifndef _INTER_H_
#define _INTER_H_

#include "lexer.h"
//----------------------节点---------------------------------------
struct Node{
	int line;
	int begin, end;
	Node(){ line = Lexer::line; begin = MakeLable(); end = MakeLable(); }
	static int lable;// 全局标号
	static map<string, int> lables;// 全局标号
	static int MakeLable(){ return Node::lable++; }
	static string Lable(int lable)
	{
		// 产生标号，如果有相同的标号，就不产生该标号
		ostringstream fout;
		fout << "L" << lable << ":" << endl;
		if (lables.find(fout.str()) == lables.end())
		{
			lables[fout.str()] = lable;
			return fout.str();
		}
		else
		{
			return "";
		}
	}
	virtual string toString(){//.code
		ostringstream fout;
		fout << "<" << begin << "-" << end << ">" << endl;
		return fout.str();
	}
};

map<string, int> Node::lables = map<string, int>();
int Node::lable = 0;
//----------------------陈述性的语句-------------------------------
struct Statement :Node{
	virtual string toString(){
		ostringstream fout;
		fout << "";
		return fout.str();
	}
};

//----------------------表达式类-----------------------------------
struct Expr :Statement{
	Token *token;// & | ~ && || 
	Type *type;
	Expr(){ token = nullptr; type = nullptr; }
	Expr(Token *token, Type *type) :token(token), type(type){}
	virtual string toString(){
		ostringstream s;
		switch (token->Tag){
		case NUMBER:s << token->toString(); break;
		case REAL:s << token->toString(); break;
		case ID:s << token->toString(); break;
		default:s << "T" << begin; break;
		}
		return s.str();
	}
};

struct Constant :Expr{
	static Constant *True, *False;
	Constant(Token *token, Type *type):Expr(token, type){}
	virtual string toString(){
		ostringstream fout;
		fout << "MOV T" << begin;
		fout << " " << token->toString() << endl;
		return fout.str();
	}
};

struct Constant* Constant::True = new Constant(Word::True, Type::Bool);
struct Constant* Constant::False = new Constant(Word::False, Type::Bool);

struct Id :Expr{
	int offset;
	Id(){
		token = new Token(' ');
		type = Type::Int;
		offset = 0;
	}
	Id(Id* id){
		token = id->token;
		type = id->type;
		offset = id->offset;
	}
	Id(Word *word, Type *type) 
		:Expr(word, type){ 
		offset = 0;
	}
	virtual string toString(){
		ostringstream fout;
		fout << "MOV T" << begin;
		fout << " " << token->toString() << endl;
		return fout.str();
	}
};

struct Member :Id{
	Id *root;
	Member(Id *root, Word *word, Type *type, int off) :Id(word, type), root(root){
		offset = off;
	}
	virtual string toString(){
		ostringstream fout;
		fout << "LEA SI,NUMS" << endl;
		fout << "MOV T" << begin;
		fout << " BYTE PTR " << root->token->toString();
		fout << "[SI + " << std::hex << offset << "H]" << endl;
		return fout.str();
	}
};

//----------------------双目运算表达式类---------------------------
struct Binocular :Expr{
	Expr *expr1, *expr2;
	Binocular(Token *token, Expr *expr1, Expr *expr2) 
		:Expr(token, Type::max(expr1->type, expr2->type))
		, expr1(expr1)
		, expr2(expr2){ }
	virtual string toString(){
		ostringstream fout;
		fout << expr1->toString();
		fout << expr2->toString();
		// E1.code | E2.code
		fout << token->toString();
		fout << " T" << begin;
		fout << " T" << expr1->begin;
		fout << " T" << expr2->begin << endl;
		// SUB T3 T1 T2
		return fout.str();
	}
};

//----------------------单目运算表达式类---------------------------
struct Unary :Expr{
	Expr *expr;
	Unary(Token *token, Expr *expr) :Expr(token, expr->type), expr(expr){  }
	virtual string toString(){
		ostringstream fout;
		fout << expr->toString();
		fout << token->toString();
		fout << " T" << begin;
		fout << " " << expr->begin << endl;
		return fout.str();
	}
};

struct Pointer :Expr{
	Pointer(Id *root, Word *word){
	}
};

struct Access :Expr{
	Id *id;
	Expr *expr;
	Access(Id *id, Expr *expr, Type *type):
		Expr(new Word(INDEX, "[]"), type),
		id(id), 
		expr(expr){}
	virtual string toString(){
		ostringstream fout;
		fout << expr->toString();
		fout << "MOV T" << begin << " " << id->token->toString();
		fout << "[ T" << expr->begin << " ]" << endl;
		return fout.str();
	}
};

//----------------------环境---------------------------------------
struct CompareId{
	bool operator ()(const pair<string, Id> &id1, const pair<string, Id> &id2){
		return id1.second.offset < id2.second.offset;
	}
};

struct Environment{
	CompareId comp;
	map<string, Id> table;
	int width = 0;
	static int lable;
	int nlable;
	Environment(){ nlable = lable++; }
	void put(Id *id){
		Id tid = *id;// 复制一份id
		tid.offset = width;// 计算内部偏移量
		width += tid.type->width;// 计算大小
		table[tid.token->toString()] = tid;
	}
	Id* get(string name){
		if (table.find(name) == table.end())
			return nullptr;
		return &table[name];
	}
	string toString(){
		ostringstream fout;
		if (!table.empty()){
			fout << "<env>" << endl;
			vector<pair<string, Id>> v(table.begin(), table.end());
			sort(v.begin(), v.end(), comp);
			vector<pair<string, Id>>::iterator iter;
			for (iter = v.begin(); iter != v.end(); iter++){
				fout << "" << iter->second.token->toString();
				fout << " width:" << iter->second.type->width;
				fout << " offset:" << iter->second.offset << endl;
			}
			fout << "</env>" << endl;
		}
		return fout.str();
	}
};

int Environment::lable = 0;

struct Statements :Statement{
	vector<Statement*> stmts;
	Statements(){  }
	void addStatement(Statement* stmt){
		stmts.push_back(stmt);
	}
	virtual string toString(){
		ostringstream fout;
		for (Statement* stmt : stmts){
			if (stmt)fout << stmt->toString();
		}
		return fout.str();
	}
};

struct Print :Statement{
	Expr *expr;
	Print(){
		//Tag = PRINT;
	}
	Print(Expr *expr) :expr(expr){
		//Tag = PRINT;
	}
	virtual string toString(){
		ostringstream fout;
		fout << expr->toString();
		fout << "print T" << expr->begin << endl;
		return fout.str();
	}
};

struct Assign :Statement{
	Id *id;
	Expr *expr;
	Assign(){  }
	Assign(Id *id, Expr *expr) :id(id), expr(expr){  }
	virtual string toString(){
		ostringstream fout;
		fout << expr->toString();
		fout << "MOV " << ((Word*)id->token)->word;
		fout << " T" << expr->begin << endl;
		return fout.str();
	}
};

struct IfElse :Statement{
	Expr *expr;
	IfElse(){  }
	IfElse(Expr *expr) :expr(expr){  }
	virtual string toString(){
		ostringstream fout;
		fout << expr->toString();
		return fout.str();
	}
};

struct If :IfElse{
	Statement* stmt;
	If(){  }
	If(Expr *expr, Statement* stmt) :IfElse(expr), stmt(stmt){  }
	virtual string toString(){
		ostringstream fout;
		// 语法制导定义
		expr->begin = begin;
		stmt->end = end;
		// 语法制导翻译
		fout << expr->toString();
		fout << "ifFalse T" << expr->begin << " goto L" << end << endl;
		fout << stmt->toString();
		fout << Lable(end);
		return fout.str();
	}
};

struct Else :IfElse{
	Statement* stmt1;
	Statement* stmt2;
	Else(){  }
	Else(Expr *expr, Statement* stmt1, Statement* stmt2) :IfElse(expr), stmt1(stmt1), stmt2(stmt2){  }
	virtual string toString(){
		ostringstream fout;
		// 语法制导定义
		expr->begin = begin;
		stmt2->end = end;
		// 语法制导翻译
		fout << expr->toString();
		fout << "ifFalse T" << expr->begin << " goto L" << stmt2->begin << endl;
		fout << stmt1->toString();
		fout << "goto L" << end << endl;
		fout << Lable(stmt2->begin);
		fout << stmt2->toString();
		fout << Lable(end);
		return fout.str();
	}
};

struct Switch :Statement{
	Expr* expr;
	map<int, Statement*> cases;
	Switch(){  }
	void addCase(int i, Statement *stmts)
	{
		cases[i] = stmts;
	}
	virtual string toString()
	{
		ostringstream fout;
		fout << "goto L" << expr->begin << endl;
		map<int, Statement*>::iterator iter;
		for (iter = cases.begin(); iter != cases.end(); iter++)
		{
			fout << Lable(iter->second->begin);
			fout << iter->second->toString();
		}
		fout << Lable(expr->begin);
		fout << expr->toString();
		for (iter = cases.begin(); iter != cases.end(); iter++)
		{
			fout << "if T" << expr->begin << "==" << iter->first;
			fout << " goto L" << iter->second->begin << endl;
		}
		fout << Lable(end);
		return fout.str();
	}
};

struct DoWhile :Statement{
	Statement* stmt;
	Expr *expr;
	DoWhile(){  }
	DoWhile(Statement* stmt, Expr *expr) : stmt(stmt), expr(expr){  }
	virtual string toString(){
		ostringstream fout;
		// 语法制导定义
		stmt->begin = begin;
		// 语法制导翻译
		fout << Lable(begin);
		fout << stmt->toString();
		fout << expr->toString();
		fout << "if T" << expr->begin << " goto L" << begin << endl;
		fout << Lable(end);
		return fout.str();
	}
};

struct While :Statement{
	Expr *expr;
	Statement* stmt;
	While(){  }
	While(Expr *expr, Statement* stmt) :expr(expr), stmt(stmt){  }
	virtual string toString(){
		ostringstream fout;
		// 语法制导定义
		expr->begin = begin;
		// 语法制导翻译
		fout << Lable(begin);
		fout << expr->toString();
		fout << "ifFalse T" << expr->begin << " goto L" << end << endl;
		fout << stmt->toString();
		fout << "goto L" << begin << endl;
		fout << Lable(end);
		return fout.str();
	}
};

struct For :Statement{
	Statement *assign1;
	Expr *expr;
	Statement *assign2;
	Statement *stmt;
	For(){  }
	For(Statement *assign1, Expr *expr, Statement *assign2, Statement* stmt) :assign1(assign1), expr(expr), assign2(assign2), stmt(stmt){}
	virtual string toString(){
		ostringstream fout;
		// 语法制导定义
		assign1->begin = begin;
		// 语法制导翻译
		fout << assign1->toString();
		fout << "goto L" << assign2->end << endl;
		fout << Lable(assign2->begin);
		fout << assign2->toString();
		fout << Lable(assign2->end);
		fout << expr->toString();
		fout << "ifFalse T" << expr->begin << " goto L" << end << endl;
		fout << stmt->toString();
		fout << "goto L" << assign2->begin << endl;
		fout << Lable(end);
		return fout.str();
	}
};

struct Break :Statement{
	Statement *stmt;
	static stack<Statement*> cur;
	Break(){
		stmt = Break::cur.top();
	}
	virtual string toString(){
		ostringstream fout;
		fout << "goto L" << stmt->end << endl;
		return fout.str();
	}
};

stack<Statement*> Break::cur = stack<Statement*>();

struct Continue:Statement{
	Statement *stmt;
	static stack<Statement*> cur;
	Continue(){
		stmt = Continue::cur.top();
	}
	virtual string toString(){
		ostringstream fout;
		fout << "goto L" << stmt->begin << endl;
		return fout.str();
	}
};

stack<Statement*> Continue::cur = stack<Statement*>();

//----------------------定义性的语句-------------------------------
struct Nodes :Node{
	vector<Node*> nodes;
	Nodes(){}
	Nodes(Nodes* nodes){
		if (nodes)nodes->addNodes(nodes);
	}
	void addNode(Node *node){
		nodes.push_back(node);
	}
	void addNodes(Nodes *node){
		nodes.push_back(node);
	}
	virtual string toString(){
		ostringstream fout;
		if (nodes.size() > 0){
			fout << "<nodes>" << endl;
			for (Node* node : nodes){
				if (node){
					fout << node->toString();
				}
			}
			fout << "</nodes>" << endl;
		}
		return fout.str();
	}
};

struct Declaration :Statement, Nodes{
	Declaration(Nodes* nodes) :Nodes(nodes){}
};

struct Function :Node, Type{
	Type *type;
	string name;
	Statements body;
	Environment *env;
	Function(string name, Type *type) :
		name(name), type(type){
		Tag = FUNCTION;
	}
	void addStatement(Statement* stmt){
		body.addStatement(stmt);
	}
	virtual string toString(){
		lables.clear();
		ostringstream fout;
		fout << "<function>" << endl;
		if (env)fout << env->toString();
		fout << name << " PROC" << endl;
		fout << body.toString();
		fout << name << " ENDP" << endl;
		fout << "</function>" << endl;
		return fout.str();
	}
};

struct Enum :Node, Temp{
	map<string, int> vars;
	int offset = 0;
	Enum(){
		Tag = ENUM;
		word = "noname" + begin;
	}
	Enum(string name){
		Tag = ENUM;
		word = name;
		width = Type::Int->width;
	}
	void addVariable(string name){
		vars[name] = offset;
		offset++;
	}
	int getVariable(string name){
		return vars[name];
	}
	virtual string toString(){
		ostringstream fout;
		fout << "<enum>" << endl;
		fout << "<name>" << word << "</name>" << endl;
		fout << "<items>" << endl;
		map<string, int>::iterator iter;
		for (iter = vars.begin(); iter != vars.end(); iter++){
			fout << "<item>" << (*iter).first << "--" << (*iter).second << "</item>" << endl;
		}
		fout << "</items>" << endl;
		fout << "</enum>" << endl;
		return fout.str();
	}
};

struct Struct : Type{
	Environment *env;
	Struct(){
		Tag = STRUCT;
		word = "[struct]";
		width = 0;
	}
	Struct(string name){
		Tag = STRUCT;
		word = name;
		width = 0;
	}
	Id* getVariable(string name){
		return env->get(name);
	}
	virtual string toString(){
		ostringstream fout;
		fout << env->toString();
		return fout.str();
	}
};

struct Class :Nodes, Type{
	Class* parrent;
	Environment *env;
	list<Function*> functions;
	Class(string name) {
		Tag = CLASS;
		word = name;
		parrent = nullptr;
	}
	Id* getVariable(string name){
		Id* id = nullptr;
		Class *pClass = this;
		while (pClass){
			id = pClass->env->get(name);
			if (id)break;
			pClass = pClass->parrent;
		}
		return id;
	}
	virtual string toString(){
		// 获取继承方法列表
		list<Function*> q;
		// 输出类
		ostringstream fout;
		fout << "<class>" << endl;
		fout << "<name>" << word << "</name>" << endl;
		fout << "<width>" << width << "</width>" << endl;
		// 输出类成员
		fout << "<envs>" << endl;
		Class *pClass = this;
		while (pClass){
			q.merge(pClass->functions);
			fout << pClass->env->toString();
			pClass = pClass->parrent;
		}
		fout << "</envs>" << endl;
		// 输出类方法
		fout << "<functions>" << endl;
		list<Function*>::iterator iter;
		for (iter = q.begin(); iter != q.end(); iter++){
			if (*iter){
				fout << (*iter)->toString();
			}
		}
		fout << "</functions>" << endl;
		fout << "</class>" << endl;
		return fout.str();
	}
};

//----------------------JSON----------------------------------
struct JSONValue :Node{
	virtual string toString(){
		return "JSONValue";
	}
};

struct JSONString :JSONValue{
	string str;
	JSONString(string str) :str(str){}
	virtual string toString(){
		ostringstream fout;
		fout << "<string>";
		fout << str;
		fout << "</string>";
		return fout.str();
	}
};

struct JSONInt :JSONValue{
	int num;
	JSONInt(int num) :num(num){}
	virtual string toString(){
		ostringstream fout;
		fout << "<begin>";
		fout << num;
		fout << "</begin>";
		return fout.str();
	}
};

struct JSONReal :JSONValue{
	double real;
	JSONReal(double d) :real(d){}
	virtual string toString(){
		ostringstream fout;
		fout << "<real>";
		fout << real;
		fout << "</real>";
		return fout.str();
	}
};

struct JSONPair :Node{
	JSONString *name;
	JSONValue* node;
	JSONPair(){}
	JSONPair(JSONString *name, JSONValue *node) :name(name), node(node){  }
	virtual string toString(){
		ostringstream fout;
		fout << "<pair>" << endl;
		fout << "<name>";
		fout << name->toString();
		fout << "</name>" << endl;
		fout << node->toString();
		fout << "</pair>";
		return fout.str();
	}
};

struct JSONObject :JSONValue{
	vector<JSONPair*> json;
	void addPair(JSONPair *p){
		json.push_back(p);
	}
	JSONValue* operator [](string name){
		vector<JSONPair*>::iterator it;
		for (it = json.begin(); it != json.end(); it++){
			if ((*it)->name->str == name){
				return (*it)->node;
			}
		}
	}
	virtual string toString(){
		ostringstream fout;
		fout << "<jsonobject>" << endl;
		for (JSONPair* node : json){
			if (node){
				fout << node->toString() << endl;
			}
		}
		fout << "</jsonobject>" << endl;
		return fout.str();
	}
};

struct JSONArray :JSONValue{
	vector<Node*> json;
	void addNode(Node *p){
		json.push_back(p);
	}
	virtual string toString(){
		ostringstream fout;
		fout << "<jsonarray>" << endl;
		for (Node* node : json){
			if (node){
				fout << node->toString() << endl;
			}
		}
		fout << "</jsonarray>" << endl;
		return fout.str();
	}
};

struct Json :Node{
	string name;
	Node* node;
	Json(string name, Node* node) :
		name(name), node(node){
	}
	virtual string toString(){
		ostringstream fout;
		fout << "<node>" << endl;
		fout << "<name>";
		fout << name;
		fout << "</name>" << endl;
		fout << node->toString();
		fout << "</node>" << endl;
		return fout.str();
	}
};

#endif