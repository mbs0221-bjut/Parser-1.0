#include "LLParser.h"

void main(){
	string in = "in.txt";
	string out = "out.xml";
	LLParser parser(in, out);
	parser.program();
	getchar();
}