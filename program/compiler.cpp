#include "LLParser.h"

int main(int argc, char *argv[])
{
	string in = "in.txt";
	string out = "out.xml";
	LLParser parser(in, out);
	parser.program();
	getchar();
}