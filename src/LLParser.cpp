#include <iomanip>

#include "inter.h"
#include "LLParser.h"
void LLParser::pushEnv()
{
    if (env)
    {
        envs.push_front(env);
        env = new Environment();
    }
    else
    {
        cout << "<< envs null!" << endl;
    }
}
void LLParser::popEnv()
{
    if (!envs.empty())
    {
        env = envs.front();
        envs.pop_front();
    }
    else
    {
        cout << ">> envs empty!" << endl;
    }
}
void LLParser::move()
{
    look = lexer->scan();
}
void LLParser::match(int c)
{
    if (look->Tag == c)
    {
        move();
    }
    else if (look->Tag == TEMP)
    {
        fout << lexer->line << ":"
             << "," << lexer->column;
        fout << ":'" << look->toString() << "' no decl" << endl;
    }
    else
    {
        fout << lexer->line << ":"
             << "," << lexer->column;
        fout << ":'" << (char)c << "' not matched" << endl;
    }
}
LLParser::LLParser(string in, string out)
{
    lexer = new Lexer(in);
    fout = ofstream(out);
    env = new Environment();
    if (fout.is_open())
    {
        cout << "Parser construct" << endl;
    }
}
LLParser::~LLParser()
{
    if (fout.is_open())
    {
        cout << "Parser deconstruct" << endl;
        fout.close();
    }
}
void LLParser::program()
{
    cout << "Parse start!" << endl;
    move();
    while (look->Tag != '#')
    {
        Node *node = def();
        if (node != nullptr)
            nodes.addNode(node);
    }
    string s;
    fout << "<codes>" << endl;
    fout << env->toString();
    fout << nodes.toString();
    fout << "</codes>" << endl;
    cout << "Parser complete!" << endl;
}
// 外部结构
Node *LLParser::def()
{
    Node *node = nullptr;
    switch (look->Tag)
    {
    case BASIC:
        node = def_var();
        break;
    case CLASS:
        node = def_class();
        break;
    case STRUCT:
        node = def_struct();
        break;
    case ENUM:
        node = def_enum();
        break;
    case JSON:
        node = def_json();
        break;
    case ID:
        node = def_assign();
        break;
    case AUTO:
        node = stmt_auto();
        break;
    case LAMBDA:
        lambda_expr();
        break;
    case PRINT:
        match(PRINT);
        break;
    default:
        move();
        break;
    }
    return node;
}
Node *LLParser::def_assign()
{
    if (look->Tag == ID)
    {
        if (env)
        {
            Id *id = env->get(look->toString());
            if (id->type == Type::Json)
            {
                match(ID);
                if (look->Tag == '=')
                {
                    match('=');
                    Json *json = new Json(id->token->toString(), json_value());
                    match(';');
                    return json;
                }
                // if (look->Tag == '['){
                //	match('[');
                //	Id* id = new Id((Word*)look, Type::Json);
                //	match(ID);
                //	match(']');
                //	match(';');
                //	return id;
                // }
            }
        }
    }
    return nullptr;
}
Node *LLParser::def_json()
{
    match(JSON);
    Token *token = look;
    match(ID);
    Id *id = new Id((Word *)token, Type::Json);
    putId(id);
    if (look->Tag == '=')
    {
        match('=');
        Json *json = new Json(token->toString(), json_value());
        match(';');
        return json;
    }
    else if (look->Tag == ';')
    {
        match(';');
    }
    return nullptr;
}
Node *LLParser::def_var()
{
    Nodes *nodes = new Nodes();
    if (Type::isType(look))
    {
        Type *type = (Type *)look;
        match(look->Tag);
        while (look->Tag == ID)
        {
            Token *token = look;
            Id *id = new Id((Word *)token, type);
            match(ID);
            if (look->Tag == '(' && type->Tag != ENUM)
            {
                Node *func = def_func(id->token->toString(), type);
                nodes->addNode(func);
            }
            else if (look->Tag == '[')
            {
                // int a[5][4][3][2];
                Array *arr = (Array *)type;
                while (look->Tag == '[')
                {
                    match('[');
                    Number *num = (Number *)look;
                    match(NUMBER);
                    match(']');
                    arr = new Array(num->value, arr);
                }
                // root = [2];
                // [2]->[3]->[4]->[5]->int; src
                Array *head, *temp;
                head = arr;
                temp = (Array *)arr->type;
                arr->width = arr->size * type->width; // arr[2]*4
                while (temp->Tag == ARRAY)
                {
                    arr->type = temp->type;                       // [2]->[4]
                    temp->type = head;                            // [3]->[2]
                    temp->width = temp->size * temp->type->width; // [3]*width;
                    head = temp;                                  // [3]->[2]->[4]
                    temp = (Array *)arr->type;
                }
                arr->type = temp;
                // [5]->[4]->[3]->[2]->int; dst
                id->type = head; // 将id的类型设为新的数组类型
                // cout << "find Array:" << head->toString() << endl;
                putId(id);
            }
            else
            {
                putId(id);
            }
            if (look->Tag == ',')
            {
                match(',');
            }
        }
        if (look->Tag == ';')
            match(';');
    }
    return nodes;
}
Node *LLParser::def_func(string name, Type *type)
{
    Function *func = new Function(name, type);
    pushEnv();
    if (look->Tag == '(')
    {
        match('(');
        while (Type::isType(look))
        {
            Type *type = (Type *)look;
            match(look->Tag);
            Id *id = new Id((Word *)look, type);
            putId(id);
            match(ID);
            if (look->Tag == ',')
                match(',');
        }
        match(')');
        if (look->Tag == '{')
        {
            Statement *sts = stmts();
            if (sts)
                func->addStatement(sts);
        }
        else
        {
            match(';');
        }
    }
    func->env = env;
    popEnv();
    return func;
}
Node *LLParser::def_class()
{
    match(CLASS);
    Class *current = new Class(look->toString());
    match(ID);
    pushEnv();
    if (look->Tag == EXTENDS)
    {
        match(EXTENDS);
        Class *p = (Class *)look;
        env->width += p->env->width;
        current->parrent = (Class *)look;
        match(CLASS);
    }
    if (look->Tag == IMPLEMENTS)
    {
        match(IMPLEMENTS);
    }
    match('{');
    while (look->Tag != '}')
    {
        if (Type::isType(look))
        {
            Nodes *t = (Nodes *)def_var();
            vector<Node *>::iterator iter;
            for (iter = t->nodes.begin(); iter != t->nodes.end(); iter++)
            {
                current->functions.push_front((Function *)(*iter));
            }
        }
    }
    match('}');
    match(';');
    current->width = env->width;
    current->env = env;
    popEnv();
    lexer->putWord(current);
    return current;
}
Node *LLParser::def_struct()
{
    Struct *st = nullptr;
    match(STRUCT);
    if (look->Tag == '{')
    {
        st = new Struct();
    }
    else if (look->Tag == ID)
    {
        st = new Struct(look->toString());
        match(ID);
    }
    else
    {
        return nullptr;
    }
    pushEnv();
    match('{');
    while (Type::isType(look))
    {
        Type *type = (Type *)look;
        match(look->Tag);
        while (look->Tag == ID)
        {
            Id *id = new Id((Word *)look, type);
            putId(id);
            match(ID);
            if (look->Tag == ',')
            {
                match(',');
            }
        }
        match(';');
    }
    match('}');
    st->width = env->width;
    st->env = env;
    popEnv();
    lexer->putWord(st);
    Nodes *nodes = new Nodes();
    while (look->Tag == ID)
    {
        putId(new Id((Word *)look, st));
        match(ID);
        if (look->Tag == ',')
        {
            match(',');
        }
    }
    match(';');
    return nodes;
}
Node *LLParser::def_interface()
{
    return nullptr;
}
Node *LLParser::def_enum()
{
    Enum *e = nullptr;
    match(ENUM);
    if (look->Tag == '{')
    {
        e = new Enum();
    }
    else if (look->Tag == ID)
    {
        e = new Enum(look->toString());
        match(ID);
    }
    else
    {
        return e;
    }
    match('{');
    while (look->Tag == ID)
    {
        e->addVariable(look->toString());
        match(ID);
        if (look->Tag == ',')
        {
            match(',');
        }
    }
    match('}');
    lexer->putWord(e);
    while (look->Tag == ID)
    {
        putId(new Id((Word *)look, e));
        match(ID);
        if (look->Tag == ',')
        {
            match(',');
        }
    }
    match(';');
    return e;
}
// 查找变量
void LLParser::putId(Id *id)
{
    if (env)
    {
        int size = envs.size();
        cout << setw(8 * size) << "Env[" << env->nlable << "]<<" << id->token->toString() << endl;
        env->put(id);
    }
    else
    {
        cout << "<error>";
        cout << lexer->line << ":no env";
        cout << "</error>" << endl;
    }
}
Id *LLParser::getId()
{
    Id *id = nullptr;
    if (env)
    {
        int size = envs.size();
        id = env->get(look->toString());
        if (id == nullptr)
        {
            list<Environment *>::iterator iter;
            for (iter = envs.begin(); iter != envs.end(); iter++)
            {
                id = (*iter)->get(look->toString());
                if (id != nullptr)
                    break;
            }
        }
        if (id == nullptr)
        {
            fout << lexer->line << ": " << ((Word *)look)->toString();
            fout << " undeclared!" << endl;
        }
    }
    else
    {
        cout << "No Environment!" << endl;
    }
    return id;
}
// lambda表达式
void LLParser::lambda_expr()
{
    if (look->Tag == ID)
    {
        match(ID);
    }
    else if (look->Tag == '(')
    {
        match('(');
        if (look->Tag == LAMBDA)
        {
            match(LAMBDA);
            match(ID);
            match('.');
            lambda_expr();
            match(')');
        }
        else
        {
            lambda_expr();
            lambda_expr();
            match(')');
        }
    }
    else if (look->Tag == LAMBDA)
    {
        match(LAMBDA);
        match(ID);
        match('.');
        lambda_expr();
    }
}
// 内部结构
Statement *LLParser::stmts()
{
    if (look->Tag == '{')
    {
        match('{');
        Statements *sts = new Statements();
        while (look->Tag != '}')
        {
            Statement *st = stmt();
            if (st)
                sts->addStatement(st);
        }
        match('}');
        return sts;
    }
    else
    {
        Statement *sts = stmt();
        return sts;
    }
}
Statement *LLParser::stmt()
{
    Statement *st = (Statement *)nullptr;
    switch (look->Tag)
    {
    case ';':
        move();
        break;
    case '{':
        st = stmts();
        break;
    case BASIC:
        st = stmt_var();
        break;
    case IF:
        st = stmt_if();
        break;
    case WHILE:
        st = stmt_while();
        break;
    case DO:
        st = stmt_do();
        break;
    case FOR:
        st = stmt_for();
        break;
    case ID:
        st = stmt_assign();
        break;
    case AUTO:
        st = stmt_auto();
        break;
    case PRINT:
        st = stmt_print();
        break;
    case SWITCH:
        st = stmt_switch();
        break;
    case BREAK:
        st = stmt_break();
        break;
    case CONTINUE:
        st = stmt_continue();
        break;
    default:
        st = stmt_assign();
        break;
    }
    return st;
}
Statement *LLParser::stmt_var()
{
    Nodes *nodes = new Nodes();
    if (look->Tag == BASIC)
    {
        Type *type = (Type *)look;
        Id *id;
        match(BASIC);
        id = new Id((Word *)look, type);
        putId(id);
        match(ID);
        while (look->Tag == ',')
        {
            match(',');
            id = new Id((Word *)look, type);
            putId(id);
            match(ID);
        }
        match(';');
    }
    return new Declaration(nodes);
}
Statement *LLParser::stmt_if()
{
    match(IF);
    match('(');
    Expr *expr = boolean();
    match(')');
    pushEnv();
    Statement *stmt1 = stmts();
    popEnv();
    If *st = new If(expr, stmt1);
    if (look->Tag == ELSE)
    {
        match(ELSE);
        pushEnv();
        Statement *stmt2 = stmts();
        popEnv();
        Else *st = new Else(expr, stmt1, stmt2);
        return st;
    }
    return st;
}
Statement *LLParser::stmt_for()
{
    For *st = new For();
    match(FOR);
    match('(');
    st->assign1 = stmt_assign();
    match(';');
    st->expr = boolean();
    match(';');
    st->assign2 = stmt_assign();
    match(')');
    pushEnv();
    Break::cur.push(st);
    Continue::cur.push(st->assign2); // continue直接跳转到第二条赋值语句开头执行
    st->stmt = stmts();
    Continue::cur.pop();
    Break::cur.pop();
    popEnv();
    return st;
}
Statement *LLParser::stmt_while()
{
    While *st = new While();
    match(WHILE);
    match('(');
    st->expr = boolean();
    match(')');
    pushEnv();
    Break::cur.push(st);
    Continue::cur.push(st); // continue 跳转到语句的开头执行
    st->stmt = stmts();
    Continue::cur.pop();
    Break::cur.pop();
    popEnv();
    return st;
}
Statement *LLParser::stmt_do()
{
    DoWhile *st = new DoWhile();
    match(DO);
    pushEnv();
    Break::cur.push(st);
    Continue::cur.push(st); // continue 跳转到语句开头执行
    st->stmt = stmts();
    Continue::cur.pop();
    Break::cur.pop();
    popEnv();
    match(WHILE);
    match('(');
    st->expr = boolean();
    match(')');
    return st;
}
Statement *LLParser::stmt_assign()
{
    Assign *st = new Assign();
    st->id = getId();
    match(ID);
    match('=');
    st->expr = boolean();
    return st;
}
Statement *LLParser::stmt_auto()
{
    Assign *st = new Assign();
    match(AUTO);
    Token *token = look;
    match(ID);
    match('=');
    st->expr = boolean();
    Id *id = new Id((Word *)token, st->expr->type);
    putId(id);
    st->id = id;
    return st;
}
Statement *LLParser::stmt_print()
{
    Print *st = new Print();
    match(PRINT);
    st->expr = boolean();
    match(';');
    return st;
}
Statement *LLParser::stmt_switch()
{
    Switch *st = new Switch();
    match(SWITCH);
    match('(');
    st->expr = boolean();
    match(')');
    match('{');
    Break::cur.push(st);
    while (look->Tag == CASE)
    {
        match(CASE);
        int i = ((Number *)look)->value;
        match(NUMBER);
        match(':');
        st->addCase(i, stmts());
    }
    Break::cur.pop();
    match('}');
    return st;
}
Statement *LLParser::stmt_break()
{
    Break *st = new Break();
    match(BREAK);
    match(';');
    return st;
}
Statement *LLParser::stmt_continue()
{
    Continue *st = new Continue();
    match(CONTINUE);
    match(';');
    return st;
}
// 算术逻辑
Expr *LLParser::boolean()
{
    Expr *left = join();
    while (look->Tag == OR)
    {
        Token *token = look;
        match(OR);
        left = new Binocular(token, left, join());
    }
    return left;
}
Expr *LLParser::join()
{
    Expr *left = equality();
    while (look->Tag == AND)
    {
        Token *token = look;
        match(AND);
        left = new Binocular(token, left, equality());
    }
    return left;
}
Expr *LLParser::equality()
{
    Expr *left = relation();
    while (look->Tag == EQ || look->Tag == NE)
    {
        Token *token = look;
        match(look->Tag);
        left = new Binocular(token, left, relation());
    }
    return left;
}
Expr *LLParser::relation()
{
    Expr *left = shift();
    if (look->Tag == '>' || look->Tag == '<' || look->Tag == GE || look->Tag == BE)
    {
        Token *token = look;
        match(look->Tag);
        left = new Binocular(token, left, shift());
    }
    return left;
}
Expr *LLParser::shift()
{
    Expr *left = expr();
    if (look->Tag == SHL || look->Tag == SHR)
    {
        Token *token = look;
        match(look->Tag);
        left = new Binocular(token, left, expr());
    }
    return left;
}
Expr *LLParser::expr()
{
    Expr *left = term();
    while (look->Tag == '+' || look->Tag == '-')
    {
        Token *token = look;
        match(look->Tag);
        left = new Binocular(token, left, term());
    }
    return left;
}
Expr *LLParser::term()
{
    Expr *left = unary();
    while (look->Tag == '*' || look->Tag == '/' || look->Tag == '%')
    {
        Token *token = look;
        match(look->Tag);
        left = new Binocular(token, left, unary());
    }
    return left;
}
Expr *LLParser::unary()
{
    Expr *expr;
    Token *token = look;
    Type *type;
    switch (look->Tag)
    {
    case '*':
    case '&':
    case '!':
    case '~':
    case INC:
    case DEC:
        match(look->Tag);
        expr = new Unary(token, unary());
        break;
    case '-':
        match(look->Tag);
        expr = new Unary(Word::Neg, unary());
        break;
    case SIZEOF:
        match(look->Tag);
        match('(');
        type = (Type *)look;
        match(look->Tag);
        match(')');
        expr = new Constant(new Number(type->width), Type::Int);
        break;
    default:
        expr = factor();
        break;
    }
    return expr;
}
Expr *LLParser::factor()
{
    Expr *left = nullptr;
    switch (look->Tag)
    {
    case '(':
        match('(');
        left = boolean();
        match(')');
        break;
    case ID:
        left = getId();
        if (left->type->Tag == STRUCT || left->type->Tag == CLASS)
        {
            left = member();
        }
        else if (left->type->Tag == ARRAY)
        {
            left = access();
        }
        else
        {
            match(ID);
        }
        break;
    case NUMBER:
        left = new Constant(look, Type::Int);
        match(NUMBER);
        break;
    case REAL:
        left = new Constant(look, Type::Float);
        match(REAL);
        break;
    case TRUE:
        left = Constant::True;
        match(TRUE);
        break;
    case FALSE:
        left = Constant::False;
        match(FALSE);
        break;
    }
    return left;
}
Expr *LLParser::access()
{
    Id *id = getId();
    match(ID);
    if (look->Tag == '[')
    {
        Expr *index, *width, *expr1, *expr2, *location;
        Type *type = id->type;
        match('[');
        index = boolean();
        match(']');
        type = ((Array *)type)->type;                             // 数组元素的类型
        width = new Constant(new Number(type->width), Type::Int); // 数组元素的宽度
        expr1 = new Binocular(new Token('*'), index, width);      // A[5][4][3]
        location = expr1;
        while (look->Tag == '[')
        {
            match('[');
            index = boolean();
            match(']');
            type = ((Array *)type)->type;                             // 数组元素的类型
            width = new Constant(new Number(type->width), Type::Int); // 数组元素的宽度
            expr1 = new Binocular(new Token('*'), index, width);      // 偏移量
            expr2 = new Binocular(new Token('+'), location, expr1);   // 总的偏移量
            location = expr2;                                         // 迭代计算位置
        }
        return new Access(id, location, type);
    }
    return id;
}
Expr *LLParser::member()
{
    Id *root;
    Id *left;
    int offset = 0;
    root = left = getId();
    match(ID);
    while (look->Tag == '.')
    {
        match('.');
        Word *w = (Type *)lexer->getWord(left->type->word);
        if (w->Tag == CLASS)
        {
            left = ((Class *)w)->getVariable(look->toString());
            offset += left->offset;
            match(ID);
        }
        else if (w->Tag == STRUCT)
        {
            left = ((Struct *)w)->getVariable(look->toString());
            offset += left->offset;
            match(ID);
        }
        else
        {
            match(ID);
        }
    }
    left->token->Tag = MEMBER;
    left = new Member(root, (Word *)left->token, left->type, offset);
    return left;
}
// JSON处理
JSONValue *LLParser::json_value()
{
    JSONValue *node = nullptr;
    switch (look->Tag)
    {
    case '[':
        node = json_array();
        break;
    case '{':
        node = json_object();
        break;
    default:
        break;
    }
    return node;
}
JSONArray *LLParser::json_array()
{
    JSONArray *arr = nullptr;
    if (look->Tag == '[')
    {
        arr = new JSONArray();
        match('[');
        arr->addNode(json_object());
        while (look->Tag == ',')
        {
            match(',');
            arr->addNode(json_object());
        }
        match(']');
    }
    return arr;
}
JSONObject *LLParser::json_object()
{
    JSONObject *obj = new JSONObject();
    if (look->Tag == '{')
    {
        match('{');
        JSONPair *p = json_pair();
        if (p)
            obj->addPair(p);
        while (look->Tag == ',')
        {
            match(',');
            p = json_pair();
            if (p)
                obj->addPair(p);
        }
        match('}');
        return obj;
    }
    return nullptr;
}
JSONPair *LLParser::json_pair()
{
    if (look->Tag == STRING)
    {
        JSONString *attr = json_string();
        match(':');
        JSONValue *value = json_attr();
        if (attr && value)
        {
            return new JSONPair(attr, value);
        }
    }
    return nullptr;
}
JSONValue *LLParser::json_attr()
{
    JSONValue *node;
    switch (look->Tag)
    {
    case '[':
        node = json_array();
        break;
    case '{':
        node = json_object();
        break;
    case STRING:
        node = json_string();
        break;
    case NUMBER:
        node = new JSONInt(((Number *)look)->value);
        match(NUMBER);
        break;
    case REAL:
        node = new JSONReal(((Real *)look)->value);
        match(REAL);
        break;
    case TRUE:
        node = new JSONString(((Word *)look)->word);
        match(TRUE);
        break;
    case FALSE:
        node = new JSONString(((Word *)look)->word);
        match(FALSE);
        break;
    case NUL:
        node = new JSONString(((Type *)look)->word);
        match(NUL);
        break;
    default:
        node = nullptr;
        break;
    }
    return node;
}
JSONString *LLParser::json_string()
{
    JSONString *js = new JSONString(((Word *)look)->word);
    match(STRING);
    return js;
}