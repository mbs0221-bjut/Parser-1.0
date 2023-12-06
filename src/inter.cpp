#include <algorithm>
#include "lexer.h"
#include "inter.h"

using namespace std;

//----------------------节点---------------------------------------
Node::Node()
{
    line = Lexer::line;
    begin = MakeLable();
    end = MakeLable();
}
int Node::MakeLable() { return Node::lable++; }
string Node::Lable(int lable)
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
string Node::toString()
{ //.code
    ostringstream fout;
    fout << "<" << begin << "-" << end << ">" << endl;
    return fout.str();
}

map<string, int> Node::lables = map<string, int>();
int Node::lable = 0;
//----------------------陈述性的语句-------------------------------
string Statement::toString()
{
    ostringstream fout;
    fout << "";
    return fout.str();
}

//----------------------表达式类-----------------------------------
Expr ::Expr()
{
    token = nullptr;
    type = nullptr;
}
Expr ::Expr(Token *token, Type *type) : token(token), type(type) {}
string Expr ::toString()
{
    ostringstream s;
    switch (token->Tag)
    {
    case NUMBER:
        s << token->toString();
        break;
    case REAL:
        s << token->toString();
        break;
    case ID:
        s << token->toString();
        break;
    default:
        s << "T" << begin;
        break;
    }
    return s.str();
}

Constant::Constant(Token *token, Type *type) : Expr(token, type) {}

string Constant::toString()
{
    ostringstream fout;
    fout << "MOV T" << begin;
    fout << " " << token->toString() << endl;
    return fout.str();
}

struct Constant *Constant::True = new Constant(Word::True, Type::Bool);
struct Constant *Constant::False = new Constant(Word::False, Type::Bool);
Id ::Id()
{
    token = new Token(' ');
    type = Type::Int;
    offset = 0;
}
Id ::Id(Id *id)
{
    token = id->token;
    type = id->type;
    offset = id->offset;
}
Id ::Id(Word *word, Type *type)
    : Expr(word, type)
{
    offset = 0;
}
string Id ::toString()
{
    ostringstream fout;
    fout << "MOV T" << begin;
    fout << " " << token->toString() << endl;
    return fout.str();
}

Member::Member(Id *root, Word *word, Type *type, int off) : Id(word, type), root(root)
{
    offset = off;
}
string Member::toString()
{
    ostringstream fout;
    fout << "LEA SI,NUMS" << endl;
    fout << "MOV T" << begin;
    fout << " BYTE PTR " << root->token->toString();
    fout << "[SI + " << std::hex << offset << "H]" << endl;
    return fout.str();
}
//----------------------双目运算表达式类---------------------------
Binocular::Binocular(Token *token, Expr *expr1, Expr *expr2)
    : Expr(token, Type::max(expr1->type, expr2->type)), expr1(expr1), expr2(expr2) {}
string Binocular::toString()
{
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

//----------------------单目运算表达式类---------------------------
Unary::Unary(Token *token, Expr *expr) : Expr(token, expr->type), expr(expr) {}
string Unary::toString()
{
    ostringstream fout;
    fout << expr->toString();
    fout << token->toString();
    fout << " T" << begin;
    fout << " " << expr->begin << endl;
    return fout.str();
}

Pointer::Pointer(Id *root, Word *word)
{
}

Access::Access(Id *id, Expr *expr, Type *type) : Expr(new Word(INDEX, "[]"), type),
                                                 id(id),
                                                 expr(expr) {}
string Access::toString()
{
    ostringstream fout;
    fout << expr->toString();
    fout << "MOV T" << begin << " " << id->token->toString();
    fout << "[ T" << expr->begin << " ]" << endl;
    return fout.str();
}

//----------------------环境---------------------------------------
bool CompareId::operator()(const pair<string, Id> &id1, const pair<string, Id> &id2)
{
    return id1.second.offset < id2.second.offset;
}

Environment::Environment() { nlable = lable++; }
void Environment::put(Id *id)
{
    Id tid = *id;             // 复制一份id
    tid.offset = width;       // 计算内部偏移量
    width += tid.type->width; // 计算大小
    table[tid.token->toString()] = tid;
}
Id *Environment::get(string name)
{
    if (table.find(name) == table.end())
        return nullptr;
    return &table[name];
}
string Environment::toString()
{
    ostringstream fout;
    if (!table.empty())
    {
        fout << "<env>" << endl;
        vector<pair<string, Id>> v(table.begin(), table.end());
        sort(v.begin(), v.end(), comp);
        vector<pair<string, Id>>::iterator iter;
        for (iter = v.begin(); iter != v.end(); iter++)
        {
            fout << "" << iter->second.token->toString();
            fout << " width:" << iter->second.type->width;
            fout << " offset:" << iter->second.offset << endl;
        }
        fout << "</env>" << endl;
    }
    return fout.str();
}

int Environment::lable = 0;

Statements::Statements() {}
void Statements::addStatement(Statement *stmt)
{
    stmts.push_back(stmt);
}
string Statements::toString()
{
    ostringstream fout;
    for (Statement *stmt : stmts)
    {
        if (stmt)
            fout << stmt->toString();
    }
    return fout.str();
}

Print::Print()
{
    // Tag = PRINT;
}
Print::Print(Expr *expr) : expr(expr)
{
    // Tag = PRINT;
}
string Print::toString()
{
    ostringstream fout;
    fout << expr->toString();
    fout << "print T" << expr->begin << endl;
    return fout.str();
}

Assign::Assign() {}
Assign::Assign(Id *id, Expr *expr) : id(id), expr(expr) {}
string Assign::toString()
{
    ostringstream fout;
    fout << expr->toString();
    fout << "MOV " << ((Word *)id->token)->word;
    fout << " T" << expr->begin << endl;
    return fout.str();
}

IfElse::IfElse() {}
IfElse::IfElse(Expr *expr) : expr(expr) {}
string IfElse::toString()
{
    ostringstream fout;
    fout << expr->toString();
    return fout.str();
}

If::If() {}
If::If(Expr *expr, Statement *stmt) : IfElse(expr), stmt(stmt) {}
string If::toString()
{
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

Else::Else() {}
Else::Else(Expr *expr, Statement *stmt1, Statement *stmt2) : IfElse(expr), stmt1(stmt1), stmt2(stmt2) {}
string Else::toString()
{
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

Switch::Switch() {}
void Switch::addCase(int i, Statement *stmts)
{
    cases[i] = stmts;
}
string Switch::toString()
{
    ostringstream fout;
    fout << "goto L" << expr->begin << endl;
    map<int, Statement *>::iterator iter;
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
DoWhile::DoWhile() {}
DoWhile::DoWhile(Statement *stmt, Expr *expr) : stmt(stmt), expr(expr) {}
string DoWhile::toString()
{
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
While::While() {}
While::While(Expr *expr, Statement *stmt) : expr(expr), stmt(stmt) {}
string While::toString()
{
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

For::For() {}
For::For(Statement *assign1, Expr *expr, Statement *assign2, Statement *stmt) : assign1(assign1), expr(expr), assign2(assign2), stmt(stmt) {}
string For::toString()
{
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

Break::Break()
{
    stmt = Break::cur.top();
}
string Break::toString()
{
    ostringstream fout;
    fout << "goto L" << stmt->end << endl;
    return fout.str();
}

stack<Statement *> Break::cur = stack<Statement *>();
Continue::Continue()
{
    stmt = Continue::cur.top();
}
string Continue::toString()
{
    ostringstream fout;
    fout << "goto L" << stmt->begin << endl;
    return fout.str();
}

stack<Statement *> Continue::cur = stack<Statement *>();

//----------------------定义性的语句-------------------------------
Nodes::Nodes() {}
Nodes::Nodes(Nodes *nodes)
{
    if (nodes)
        nodes->addNodes(nodes);
}
void Nodes::addNode(Node *node)
{
    nodes.push_back(node);
}
void Nodes::addNodes(Nodes *node)
{
    nodes.push_back(node);
}
string Nodes::toString()
{
    ostringstream fout;
    if (nodes.size() > 0)
    {
        fout << "<nodes>" << endl;
        for (Node *node : nodes)
        {
            if (node)
            {
                fout << node->toString();
            }
        }
        fout << "</nodes>" << endl;
    }
    return fout.str();
}

Declaration::Declaration(Nodes *nodes) : Nodes(nodes) {}

Function::Function(string name, Type *type) : name(name), type(type)
{
    Tag = FUNCTION;
}
void Function::addStatement(Statement *stmt)
{
    body.addStatement(stmt);
}
string Function::toString()
{
    lables.clear();
    ostringstream fout;
    fout << "<function>" << endl;
    if (env)
        fout << env->toString();
    fout << name << " PROC" << endl;
    fout << body.toString();
    fout << name << " ENDP" << endl;
    fout << "</function>" << endl;
    return fout.str();
}
Enum::Enum()
{
    Tag = ENUM;
    word = "noname" + begin;
}
Enum::Enum(string name)
{
    Tag = ENUM;
    word = name;
    width = Type::Int->width;
}
void Enum::addVariable(string name)
{
    vars[name] = offset;
    offset++;
}
int Enum::getVariable(string name)
{
    return vars[name];
}
string Enum::toString()
{
    ostringstream fout;
    fout << "<enum>" << endl;
    fout << "<name>" << word << "</name>" << endl;
    fout << "<items>" << endl;
    map<string, int>::iterator iter;
    for (iter = vars.begin(); iter != vars.end(); iter++)
    {
        fout << "<item>" << (*iter).first << "--" << (*iter).second << "</item>" << endl;
    }
    fout << "</items>" << endl;
    fout << "</enum>" << endl;
    return fout.str();
}
Struct::Struct()
{
    Tag = STRUCT;
    word = "[struct]";
    width = 0;
}
Struct::Struct(string name)
{
    Tag = STRUCT;
    word = name;
    width = 0;
}
Id *Struct::getVariable(string name)
{
    return env->get(name);
}
string Struct::toString()
{
    ostringstream fout;
    fout << env->toString();
    return fout.str();
}
Class::Class(string name)
{
    Tag = CLASS;
    word = name;
    parrent = nullptr;
}
Id *Class::getVariable(string name)
{
    Id *id = nullptr;
    Class *pClass = this;
    while (pClass)
    {
        id = pClass->env->get(name);
        if (id)
            break;
        pClass = pClass->parrent;
    }
    return id;
}
string Class::toString()
{
    // 获取继承方法列表
    list<Function *> q;
    // 输出类
    ostringstream fout;
    fout << "<class>" << endl;
    fout << "<name>" << word << "</name>" << endl;
    fout << "<width>" << width << "</width>" << endl;
    // 输出类成员
    fout << "<envs>" << endl;
    Class *pClass = this;
    while (pClass)
    {
        q.merge(pClass->functions);
        fout << pClass->env->toString();
        pClass = pClass->parrent;
    }
    fout << "</envs>" << endl;
    // 输出类方法
    fout << "<functions>" << endl;
    list<Function *>::iterator iter;
    for (iter = q.begin(); iter != q.end(); iter++)
    {
        if (*iter)
        {
            fout << (*iter)->toString();
        }
    }
    fout << "</functions>" << endl;
    fout << "</class>" << endl;
    return fout.str();
}

//----------------------JSON----------------------------------
string JSONValue::toString()
{
    return "JSONValue";
}

JSONString::JSONString(string str) : str(str) {}
string JSONString::toString()
{
    ostringstream fout;
    fout << "<string>";
    fout << str;
    fout << "</string>";
    return fout.str();
}

JSONInt::JSONInt(int num) : num(num) {}
string JSONInt::toString()
{
    ostringstream fout;
    fout << "<begin>";
    fout << num;
    fout << "</begin>";
    return fout.str();
}

JSONReal::JSONReal(double d) : real(d) {}
string JSONReal::toString()
{
    ostringstream fout;
    fout << "<real>";
    fout << real;
    fout << "</real>";
    return fout.str();
}

JSONPair::JSONPair() {}
JSONPair::JSONPair(JSONString *name, JSONValue *node) : name(name), node(node) {}
string JSONPair::toString()
{
    ostringstream fout;
    fout << "<pair>" << endl;
    fout << "<name>";
    fout << name->toString();
    fout << "</name>" << endl;
    fout << node->toString();
    fout << "</pair>";
    return fout.str();
}

void JSONObject::addPair(JSONPair *p)
{
    json.push_back(p);
}
JSONValue *JSONObject::operator[](string name)
{
    vector<JSONPair *>::iterator it;
    for (it = json.begin(); it != json.end(); it++)
    {
        if ((*it)->name->str == name)
        {
            return (*it)->node;
        }
    }
}
string JSONObject::toString()
{
    ostringstream fout;
    fout << "<jsonobject>" << endl;
    for (JSONPair *node : json)
    {
        if (node)
        {
            fout << node->toString() << endl;
        }
    }
    fout << "</jsonobject>" << endl;
    return fout.str();
}

void JSONArray::addNode(Node *p)
{
    json.push_back(p);
}
string JSONArray::toString()
{
    ostringstream fout;
    fout << "<jsonarray>" << endl;
    for (Node *node : json)
    {
        if (node)
        {
            fout << node->toString() << endl;
        }
    }
    fout << "</jsonarray>" << endl;
    return fout.str();
}

Json::Json(string name, Node *node) : name(name), node(node)
{
}
string Json::toString()
{
    ostringstream fout;
    fout << "<node>" << endl;
    fout << "<name>";
    fout << name;
    fout << "</name>" << endl;
    fout << node->toString();
    fout << "</node>" << endl;
    return fout.str();
}