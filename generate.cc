#define BIG_N 100
#define SMALL_N 100

#define XORSHIFT_SEED 20190820

#undef GENERATE_BINARY_OPS
#undef GENEXORSHIFT_SEEDRATE_UNARY_OPS
#undef GENERATE_TERNARY_OPS
#undef GENERATE_CONCAT_OPS
#undef GENERATE_REPEAT_OPS
#undef GENERATE_WIDEEXPR
#undef GENERATE_PARTSEL

#undef GENERATE_EXPRESSIONS
#undef GENERATE_EMBEDDED
#undef GENERATE_LONGER
#undef GENERATE_FLIPFLOP
#define GENERATE_STATEMACHINE

// Use 'make gen_samples'
// #define ONLY_SAMPLES

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string>
#include <iostream>
#include <cmath>

const char *arg_types[][3] = {
	{ "{dir} {name}", "{name}", "4" },	// 00
	{ "{dir} {name}", "{name}", "5" },	// 01
	{ "{dir} {name}", "{name}", "6" },	// 02
	{ "{dir} {name}", "{name}", "4" },	// 03
	{ "{dir} {name}", "{name}", "5" },	// 04
	{ "{dir} {name}", "{name}", "6" },	// 05
};

const char *small_arg_types[][3] = {
	{ "{dir} {name}", "{name}", "1" },	// 00
	{ "{dir} {name}", "{name}", "2" },	// 01
	{ "{dir} {name}", "{name}", "3" },	// 02
	{ "{dir} signed {name}", "{name}", "1" },	// 03
	{ "{dir} signed {name}", "{name}", "2" },	// 04
	{ "{dir} signed {name}", "{name}", "3" },	// 05
};

// See Table 5-1 (page 42) in IEEE Std 1364-2005
// for a list of all Verilog oprators.
const char *builtin_gate[]={
	"and",
	"nand",
	"nor",
	"or",
	"xnor",
	"xor",
};
const char *bitwise_ops[]={
	"&",	// 1
	"|",	// 2
	"^",	// 3
	"^~",	// 4
};
const char *binary_ops[] = {
	"+",	// 00 y = &x
	"-",	// 01
	"*",	// 02
	"/",	// 03
	"%",	// 04
	"**",	// 05
	">",	// 06
	">=",	// 07
	"<",	// 08
	"<=",	// 09
	"&&",	// 10
	"||",	// 11
	"==",	// 12
	"!=",	// 13
	"===",	// 14
	"!==",	// 15
	"&",	// 16
	"|",	// 17
	"^",	// 18
	"^~",	// 19
	"<<",	// 20
	">>",	// 21
	"<<<",	// 22
	">>>",	// 23
};

const char *unary_ops[] = {
	"+",	// 00
	"-",	// 01
	"!",	// 02
	"~",	// 03
	"&",	// 04
	"~&",	// 05
	"|",	// 06
	"~|",	// 07
	"^",	// 08
	"~^",	// 09
};

#define SIZE(_list) int(sizeof(_list) / sizeof(*_list))

void strsubst(std::string &str, const std::string &match, const std::string &replace)
{
	size_t pos;
	while ((pos = str.find(match)) != std::string::npos)
		str.replace(pos, match.size(), replace);
}

uint32_t xorshift32(uint32_t seed = 0) {
	static uint32_t x = 314159265 + XORSHIFT_SEED;
	if (seed) {
		x = (seed << 16) + XORSHIFT_SEED;
		for (int i = 0; i < 10; i++)
			xorshift32();
	}
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return x;
}


void print_expression(FILE *f, int budget, int iter, uint32_t mask, bool avoid_undef, bool avoid_signed, bool in_param)
{
	bool avoid_mult_div_mod = false;
	int num_binary_ops = SIZE(binary_ops);//24
	int num_unary_ops = SIZE(unary_ops);//10
	int num_arg_types = SIZE(arg_types);//6
	int num_modes = 10;
	int i, j, mode;
	const char *p;

	assert(budget >= 0);
	//The function will stop will do not statify the condition
	if (budget == 0) {
		if (in_param)
			goto print_constant;
		char var_char;
		int var_index;
		if (!avoid_undef && (xorshift32() % 256) > (mask >> 24)) {
			var_char='b';
			var_index = xorshift32() % num_arg_types;
		} else {
			var_char = 'a' + (xorshift32() % 2);
			var_index = xorshift32() % num_arg_types;
		}
		if (avoid_signed && (var_index % num_arg_types) >= num_arg_types/2)
			var_index -= num_arg_types/2;
		fprintf(f, "%c%d", var_char, var_index);
		return;
	}

	while ((mask & ~((~0) << num_modes)) == 0)
		mask = xorshift32() & (in_param ? ~4 : ~0);

	if ((mask & 3) != 0)
		avoid_mult_div_mod = true;

	do {
		mode = xorshift32() % num_modes;
	} while (((1 << mode) & mask) == 0);

	budget--;
	switch (mode)
	{
	case 0:	
	case 1:
		// this mode number is used to determine avoid_mult_div_mod
		/*i = (xorshift32() % 4) + 1;
		fprintf(f, "{%d{", i);
		print_expression(f, budget / i, mask, avoid_undef, avoid_signed, in_param);
		fprintf(f, "}}");
		break;*/
		/*fprintf(f, "{");
			if	(xorshift32()%10<=5)
			fprintf(f,"a%d",xorshift32()%num_arg_types);
			else
			fprintf(f,"b%d",xorshift32()%num_arg_types);
		fprintf(f,"}");
		break;*/
	case 2:
		// this mode number is masked out if in_param is set during mask generation
		/*/if (avoid_signed)
			fprintf(f, "(");
		else
			fprintf(f, "%s(", xorshift32() % 3 == 0 ? "" :
					xorshift32() % 2 == 0 ? "" : "");
		print_expression(f, budget, mask, avoid_undef, true, in_param);
		fprintf(f, ")");
		break;*/
	case 3:
	//Own created case
		fprintf(f,"(");
		//fprintf(f, "a%d %s b%d",xorshift32()%num_arg_types, bitwise_ops [xorshift32() % SIZE(bitwise_ops)],xorshift32() % num_arg_types);
			if	(xorshift32()%10<=5)
			fprintf(f,"a%d",xorshift32()%num_arg_types);
			else
			fprintf(f,"b%d",xorshift32()%num_arg_types);
		fprintf(f, "%s",bitwise_ops [xorshift32() % SIZE(bitwise_ops)]);
			if	(xorshift32()%10<=5)
			fprintf(f,"a%d",xorshift32()%num_arg_types);
			else
			fprintf(f,"b%d",xorshift32()%num_arg_types);
		fprintf(f, ")");
		break;
	case 4:
		fprintf(f,"~");
			if	(xorshift32()%10<=5)
			fprintf(f,"a%d",xorshift32()%num_arg_types);
			else
			fprintf(f,"b%d",xorshift32()%num_arg_types);
			break;
	case 5:
	/*	fprintf(f, "(");
		do { 	
			p = binary_ops[xorshift32() % num_binary_ops];
		} while ((avoid_mult_div_mod && (!strcmp(p, "*") || !strcmp(p, "/") || !strcmp(p, "%"))) ||
				(avoid_undef && (!strcmp(p, "/") || !strcmp(p, "%"))));
		if (!strcmp(p, "===") || !strcmp(p, "!=="))
			avoid_undef = true;
		if (!strcmp(p, "**")) {
			fprintf(f, "%s'd2 %s ", arg_types[xorshift32() % num_arg_types][2], p);
			print_expression(f, budget < 3 ? std::max(budget-1, 0) : 2, mask, avoid_undef, true, in_param);
		} else
		if (!strcmp(p, "/") || !strcmp(p, "%")) {
			print_expression(f, budget < 3 ? std::max(budget-1, 0) : 2, mask, avoid_undef, avoid_signed, in_param);
			fprintf(f, "%s", p);
			print_expression(f, 0, mask, avoid_undef, avoid_signed, in_param);
		} else {
			if (!strcmp(p, "*"))
				budget = budget < 4 ? budget : 4;
			print_expression(f, budget/2, mask, avoid_undef, avoid_signed, in_param);
			fprintf(f, "%s", p);
			print_expression(f, budget/2, mask, avoid_undef, avoid_signed, in_param);
		}
		fprintf(f, ")");
		break;*/
	case 6:
	case 7:
	/*	fprintf(f, "(%s", bitwise_ops[xorshift32() % SIZE(bitwise_ops)]);
		print_expression(f, budget, mask, avoid_undef, avoid_signed, in_param);
		fprintf(f, ")");
		break;*/
	case 8:
		fprintf(f, "(");
		print_expression(f, budget / 3, iter,mask, avoid_undef, avoid_signed, in_param);
		fprintf(f, "?");
		print_expression(f, budget / 3,iter, mask, avoid_undef, avoid_signed, in_param);
		fprintf(f, ":");
		print_expression(f, budget / 3, iter,mask, avoid_undef, avoid_signed, in_param);
		fprintf(f, ")");
		break;
	case 9:
print_constant:
		fprintf(f, "(");
		i = (xorshift32() % 4) + 2;
	/*/	if (xorshift32() % 2 == 0 && !avoid_signed)
			fprintf(f, "%s%d'sd%d", xorshift32() % 2 == 0 ? "-" : "", i, xorshift32() % (1 << (i-1)));
		else*/
			fprintf(f, "1'b%d",  xorshift32() % 1);
		fprintf(f, ")");
		break;
	}
}

void print_embedded(FILE *f, int budget, int iter, int NumOfsub, int NUM_EMBPORT, int NUM_SUBMODULE, uint32_t mask, bool avoid_undef, bool avoid_signed, bool in_param)
{
	bool avoid_mult_div_mod = false;
	int num_binary_ops = SIZE(binary_ops);//24
	int num_unary_ops = SIZE(unary_ops);//10
	int num_arg_types = SIZE(arg_types);//6
	int num_modes = 10;
	int i, j, mode;
	const char *p;

	assert(budget >= 0);
	//The function will stop will do not statify the condition
	if (budget == 0) {
		if (in_param)
			goto print_constant;
		char var_char;
		int var_index;
		if (!avoid_undef && (xorshift32() % 256) > (mask >> 24)) {
			var_char='a';
			var_index = xorshift32() % NUM_EMBPORT;
		} else {
			var_char = 'a' + (xorshift32() % 2);
			var_index = xorshift32() % NUM_EMBPORT;
		}
		if (avoid_signed && (var_index % NUM_EMBPORT) >= NUM_EMBPORT/2)
			var_index -= NUM_EMBPORT/2;
		fprintf(f, "%c%d_sub%d", var_char, var_index,NumOfsub);
		return;
	}

	while ((mask & ~((~0) << num_modes)) == 0)
		mask = xorshift32() & (in_param ? ~4 : ~0);

	if ((mask & 3) != 0)
		avoid_mult_div_mod = true;

	do {
		mode = xorshift32() % num_modes;
	} while (((1 << mode) & mask) == 0);

	budget--;
	switch (mode)
	{
	case 0:
			
	case 1:

	case 2:
		
	case 3:
	//Own created case
		fprintf(f,"(");
		//fprintf(f, "a%d %s b%d",xorshift32()%num_arg_types, bitwise_ops [xorshift32() % SIZE(bitwise_ops)],xorshift32() % num_arg_types);
			fprintf(f,"a%d_sub%d",xorshift32()%NUM_EMBPORT,NumOfsub);
			fprintf(f, "%s",bitwise_ops [xorshift32() % SIZE(bitwise_ops)]);
			fprintf(f,"a%d_sub%d",xorshift32()%NUM_EMBPORT,NumOfsub);
		fprintf(f, ")");
		break;
	case 4:
		fprintf(f,"~");
		fprintf(f,"a%d_sub%d",xorshift32()%NUM_EMBPORT,NumOfsub);
			break;
	case 5:
	
	case 6:
	case 7:

	case 8:
		fprintf(f, "(");
		print_embedded(f, budget / 3, iter, NumOfsub, NUM_EMBPORT,NUM_SUBMODULE,mask, avoid_undef, avoid_signed, in_param);
		fprintf(f, "?");
		print_embedded(f, budget / 3,iter, NumOfsub,NUM_EMBPORT,NUM_SUBMODULE,mask, avoid_undef, avoid_signed, in_param);
		fprintf(f, ":");
		print_embedded(f, budget / 3, iter,NumOfsub, NUM_EMBPORT,NUM_SUBMODULE,mask, avoid_undef, avoid_signed, in_param);
		fprintf(f, ")");
		break;
	case 9:
print_constant:
		fprintf(f, "(");
		i = (xorshift32() % 4) + 2;
	/*/	if (xorshift32() % 2 == 0 && !avoid_signed)
			fprintf(f, "%s%d'sd%d", xorshift32() % 2 == 0 ? "-" : "", i, xorshift32() % (1 << (i-1)));
		else*/
			fprintf(f, "1'b%d",  xorshift32() % 1);
		fprintf(f, ")");
		break;
	}
}

void print_wideexpr(FILE *f, bool is_signed, int max_depth)
{
	static const char *prefix_ops[] = { "+", "-", "!", "~", "&", "~&", "|", "~|", "^", "~^" };
	static const char *combine_ops[] = { "+", "-", "&", "|", "^", "^~" };
	static const char *shift_ops[] = { "<<", "<<<", ">>", ">>>" };
	static const char *compare_ops[] = { "<", "<=", "==", "!=", ">=", ">" };
	int mode, k;

	if (max_depth <= 0)
		mode = xorshift32() % 2;
	else if (is_signed)
		mode = xorshift32() % 7;
	else
		mode = xorshift32() % 10;

	switch (mode)
	{
	/* block 1: terminal nodes */
	case 0:
		k = (xorshift32() % 6) + 1;
		fprintf(f, "%d'%sb", k, is_signed || (xorshift32() % 2 == 0) ? "s" : "");
		for (int i = 0; i < k; i++) {
	#if 0
			if (xorshift32() % 15)
				fprintf(f, "%d", xorshift32() % 2);
			else
				fprintf(f, "x");
	#else
			fprintf(f, "%d", xorshift32() % 2);
	#endif
		}
		break;
	case 1:
		fprintf(f, "%c%d", is_signed || (xorshift32() % 2 == 0) ? 's' : 'u', xorshift32() % 8);
		break;

	/* block 2: nodes for signed and unsigned expressions */
	case 2:
		fprintf(f, "%s(", prefix_ops[xorshift32() % (is_signed ? 2 : SIZE(prefix_ops))]);
		print_wideexpr(f, xorshift32() % 2 == 0, max_depth-1);
		fprintf(f, ")");
		break;
	case 3:
		fprintf(f, "(");
		print_wideexpr(f, is_signed, max_depth-1);
		fprintf(f, ")%s(", combine_ops[xorshift32() % SIZE(combine_ops)]);
		print_wideexpr(f, is_signed, max_depth-1);
		fprintf(f, ")");
		break;
	case 4:
		fprintf(f, "(");
		print_wideexpr(f, is_signed, max_depth-1);
		fprintf(f, ")%s(", shift_ops[xorshift32() % SIZE(shift_ops)]);
		print_wideexpr(f, xorshift32() % 2 == 0, max_depth-1);
		fprintf(f, ")");
		break;
	case 5:
		fprintf(f, "(ctrl[%d]?", xorshift32() % 8);
		print_wideexpr(f, is_signed, max_depth-1);
		fprintf(f, ":");
		print_wideexpr(f, is_signed, max_depth-1);
		fprintf(f, ")");
		break;
	case 6:
		fprintf(f, "%s(", is_signed || (xorshift32() % 2 == 0) ? "$signed" : "$unsigned");
		is_signed = xorshift32() % 2 == 0;
		print_wideexpr(f, is_signed, max_depth-1);
		fprintf(f, ")");
		break;

	/* block 3: nodes for unsigned expressions */
	case 7:
		is_signed = xorshift32() % 2 == 0;
		fprintf(f, "(");
		print_wideexpr(f, is_signed, max_depth-1);
		fprintf(f, ")%s(", compare_ops[xorshift32() % SIZE(compare_ops)]);
		print_wideexpr(f, is_signed, max_depth-1);
		fprintf(f, ")");
		break;
	case 8:
		k = (xorshift32() % 4) + 1;
		for (int i = 0; i < k; i++) {
			fprintf(f, i == 0 ? "{" : ",");
			is_signed = xorshift32() % 2 == 0;
			print_wideexpr(f, is_signed, max_depth-1);
		}
		fprintf(f, "}");
		break;
	case 9:
		k = (xorshift32() % 4) + 1;
		fprintf(f, "{%d{", k);
		is_signed = xorshift32() % 2 == 0;
		print_wideexpr(f, is_signed, max_depth-1);
		fprintf(f, "}}");
		break;

	default:
		abort();
	}
}

void print_partsel(FILE *f, int max_var, int depth)
{
	char var_name[16];
	if (xorshift32() % 2)
		snprintf(var_name, 16, "x%d", xorshift32() % max_var);
	else
		snprintf(var_name, 16, "p%d", xorshift32() % 4);

	switch (xorshift32() % (depth > 4 ? 5 : 10))
	{
	case 0:
		fprintf(f, "%s", var_name);
		break;
	case 1:
		if (xorshift32() % 2)
			fprintf(f, "%s[%d]", var_name, 8 + xorshift32() % 16);
		else
			fprintf(f, "%s[%d + s%d]", var_name, 4 + xorshift32() % 16, xorshift32() % 4);
		break;
	case 2:
		fprintf(f, "%s[%d + s%d %c: %d]", var_name, xorshift32() % 32, xorshift32() % 4,
				"+-"[xorshift32() % 2], xorshift32() % 8 + 1);
		break;
	case 3:
		// avoid constant out-of-bounds part select: most tools will produce an error
		if (xorshift32() % 2)
			fprintf(f, "%s[%d +: %d]", var_name, 8 + xorshift32() % 12, 1 + xorshift32() % 4);
		else
			fprintf(f, "%s[%d -: %d]", var_name, 12 + xorshift32() % 12, 1 + xorshift32() % 4);
		break;
	case 4:
	case 5:
	case 6:
		fprintf(f, "(");
		print_partsel(f, max_var, depth+1);
		fprintf(f, " %c ", "+-|&^"[xorshift32() % 5]);
		print_partsel(f, max_var, depth+1);
		fprintf(f, ")");
		break;
	case 7:
		fprintf(f, "{");
		print_partsel(f, max_var, depth+1);
		fprintf(f, ", ");
		print_partsel(f, max_var, depth+1);
		fprintf(f, "}");
		break;
	case 8:
		fprintf(f, "{2{");
		print_partsel(f, max_var, depth+1);
		fprintf(f, "}}");
		break;
	case 9:
		fprintf(f, "(%sctrl[%d] %s %sctrl[%d] %s %sctrl[%d] ? ",
			xorshift32() % 2 ? "!" : "", xorshift32() % 4, xorshift32() % 2 ? "||" : "&&",
			xorshift32() % 2 ? "!" : "", xorshift32() % 4, xorshift32() % 2 ? "||" : "&&",
			xorshift32() % 2 ? "!" : "", xorshift32() % 4);
		print_partsel(f, max_var, depth+1);
		fprintf(f, " : ");
		print_partsel(f, max_var, depth+1);
		fprintf(f, ")");
		break;
	default:
		abort();
	}
}

int main()
{
    std::cout<<"Project Start"<<std::endl;
	mkdir("rtl", 0777);

#ifdef GENERATE_BINARY_OPS
	for (int ai = 0; ai < SIZE(arg_types); ai++)
	for (int bi = 0; bi < SIZE(arg_types); bi++)
	for (int yi = 0; yi < SIZE(arg_types); yi++)
	for (int oi = 0; oi < SIZE(binary_ops); oi++)
	{
#ifdef ONLY_SAMPLES
		if (xorshift32() % 20)
			continue;
#endif
		std::string a_decl = arg_types[ai][0];
		strsubst(a_decl, "{dir}", "input");
		strsubst(a_decl, "{name}", "a");

		std::string b_decl = arg_types[bi][0];
		strsubst(b_decl, "{dir}", "input");
		strsubst(b_decl, "{name}", "b");

		std::string y_decl = arg_types[yi][0];
		strsubst(y_decl, "{dir}", "output");
		strsubst(y_decl, "{name}", "y");

		std::string a_ref = arg_types[ai][1];
		strsubst(a_ref, "{dir}", "input");
		strsubst(a_ref, "{name}", "a");

		std::string b_ref = arg_types[bi][1];
		strsubst(b_ref, "{dir}", "input");
		strsubst(b_ref, "{name}", "b");

		std::string y_ref = arg_types[yi][1];
		strsubst(y_ref, "{dir}", "output");
		strsubst(y_ref, "{name}", "y");

		char buffer[1024];
		snprintf(buffer, 1024, "rtl/binary_ops_%02d%02d%02d%02d.v", ai, bi, yi, oi);

		FILE *f = fopen(buffer, "w");
		fprintf(f, "module binary_ops_%02d%02d%02d%02d(a, b, y);\n", ai, bi, yi, oi);
		fprintf(f, "  %s;\n", a_decl.c_str());
		fprintf(f, "  %s;\n", b_decl.c_str());
		fprintf(f, "  %s;\n", y_decl.c_str());
		if (!strcmp(binary_ops[oi], "**"))
			fprintf(f, "  assign %s = %s%s ** %s;\n", y_ref.c_str(), arg_types[ai][2],
					strstr(arg_types[ai][0], " signed ") ? "'sd2" : "'d2", b_ref.c_str());
		else
			fprintf(f, "  assign %s = %s %s %s;\n", y_ref.c_str(),
					a_ref.c_str(), binary_ops[oi], b_ref.c_str());
		fprintf(f, "endmodule\n");
		fclose(f);
	}
#endif

#ifdef GENERATE_UNARY_OPS
	for (int ai = 0; ai < SIZE(arg_types); ai++)
	for (int yi = 0; yi < SIZE(arg_types); yi++)
	for (int oi = 0; oi < SIZE(unary_ops); oi++)
	{
#ifdef ONLY_SAMPLES
		if (xorshift32() % 20)
			continue;
#endif
		std::string a_decl = arg_types[ai][0];
		strsubst(a_decl, "{dir}", "input");
		strsubst(a_decl, "{name}", "a");

		std::string y_decl = arg_types[yi][0];
		strsubst(y_decl, "{dir}", "output");
		strsubst(y_decl, "{name}", "y");

		std::string a_ref = arg_types[ai][1];
		strsubst(a_ref, "{dir}", "input");
		strsubst(a_ref, "{name}", "a");

		std::string y_ref = arg_types[yi][1];
		strsubst(y_ref, "{dir}", "output");
		strsubst(y_ref, "{name}", "y");

		char buffer[1024];
		snprintf(buffer, 1024, "rtl/unary_ops_%02d%02d%02d.v", ai, yi, oi);

		FILE *f = fopen(buffer, "w");
		fprintf(f, "module unary_ops_%02d%02d%02d(a, y);\n", ai, yi, oi);
		fprintf(f, "  %s;\n", a_decl.c_str());
		fprintf(f, "  %s;\n", y_decl.c_str());
		fprintf(f, "  assign %s = %s %s;\n", y_ref.c_str(),
				unary_ops[oi], a_ref.c_str());
		fprintf(f, "endmodule\n");
		fclose(f);
	}
#endif

#ifdef GENERATE_TERNARY_OPS
	for (int ai = 0; ai < SIZE(small_arg_types); ai++)
	for (int bi = 0; bi < SIZE(arg_types); bi++)
	for (int ci = 0; ci < SIZE(arg_types); ci++)
	for (int yi = 0; yi < SIZE(arg_types); yi++)
	{
#ifdef ONLY_SAMPLES
		if (xorshift32() % 20)
			continue;
#endif
		if (!strcmp(small_arg_types[ai][2], "3"))
			continue;
		if (!strcmp(arg_types[bi][2], "6"))
			continue;
		if (!strcmp(arg_types[ci][2], "6"))
			continue;

		std::string a_decl = small_arg_types[ai][0];
		strsubst(a_decl, "{dir}", "input");
		strsubst(a_decl, "{name}", "a");

		std::string b_decl = arg_types[bi][0];
		strsubst(b_decl, "{dir}", "input");
		strsubst(b_decl, "{name}", "b");

		std::string c_decl = arg_types[ci][0];
		strsubst(c_decl, "{dir}", "input");
		strsubst(c_decl, "{name}", "c");

		std::string y_decl = arg_types[yi][0];
		strsubst(y_decl, "{dir}", "output");
		strsubst(y_decl, "{name}", "y");

		std::string a_ref = small_arg_types[ai][1];
		strsubst(a_ref, "{dir}", "input");
		strsubst(a_ref, "{name}", "a");

		std::string b_ref = arg_types[bi][1];
		strsubst(b_ref, "{dir}", "wire");
		strsubst(b_ref, "{name}", "b");

		std::string c_ref = arg_types[ci][1];
		strsubst(c_ref, "{dir}", "wire");
		strsubst(c_ref, "{name}", "c");

		std::string y_ref = arg_types[yi][1];
		strsubst(y_ref, "{dir}", "output");
		strsubst(y_ref, "{name}", "y");

		char buffer[1024];
		snprintf(buffer, 1024, "rtl/ternary_ops_%02d%02d%02d%02d.v", ai, bi, ci, yi);

		FILE *f = fopen(buffer, "w");
		fprintf(f, "module ternary_ops_%02d%02d%02d%02d(a, b, c, y);\n", ai, bi, ci, yi);
		fprintf(f, "  %s;\n", a_decl.c_str());
		fprintf(f, "  %s;\n", b_decl.c_str());
		fprintf(f, "  %s;\n", c_decl.c_str());
		fprintf(f, "  %s;\n", y_decl.c_str());
		fprintf(f, "  assign %s = %s ? %s : %s;\n", y_ref.c_str(),
				a_ref.c_str(), b_ref.c_str(), c_ref.c_str());
		fprintf(f, "endmodule\n");
		fclose(f);
	}
#endif

#ifdef GENERATE_CONCAT_OPS
	for (int ai = 0; ai < SIZE(small_arg_types); ai++)
	for (int bi = 0; bi < SIZE(small_arg_types); bi++)
	for (int yi = 0; yi < SIZE(arg_types); yi++)
	{
#ifdef ONLY_SAMPLES
		if (xorshift32() % 20)
			continue;
#endif
		std::string a_decl = small_arg_types[ai][0];
		strsubst(a_decl, "{dir}", "input");
		strsubst(a_decl, "{name}", "a");

		std::string b_decl = small_arg_types[bi][0];
		strsubst(b_decl, "{dir}", "input");
		strsubst(b_decl, "{name}", "b");

		std::string y_decl = arg_types[yi][0];
		strsubst(y_decl, "{dir}", "output");
		strsubst(y_decl, "{name}", "y");

		std::string a_ref = small_arg_types[ai][1];
		strsubst(a_ref, "{dir}", "input");
		strsubst(a_ref, "{name}", "a");

		std::string b_ref = small_arg_types[bi][1];
		strsubst(b_ref, "{dir}", "input");
		strsubst(b_ref, "{name}", "b");

		std::string y_ref = arg_types[yi][1];
		strsubst(y_ref, "{dir}", "output");
		strsubst(y_ref, "{name}", "y");

		char buffer[1024];
		snprintf(buffer, 1024, "rtl/concat_ops_%02d%02d%02d.v", ai, bi, yi);

		FILE *f = fopen(buffer, "w");
		fprintf(f, "module concat_ops_%02d%02d%02d(a, b, y);\n", ai, bi, yi);
		fprintf(f, "  %s;\n", a_decl.c_str());
		fprintf(f, "  %s;\n", b_decl.c_str());
		fprintf(f, "  %s;\n", y_decl.c_str());
		fprintf(f, "  assign %s = {%s, %s};\n", y_ref.c_str(), a_ref.c_str(), b_ref.c_str());
		fprintf(f, "endmodule\n");
		fclose(f);
	}
#endif

#ifdef GENERATE_REPEAT_OPS
	for (int a = 0; a < 4; a++)
	for (int bi = 0; bi < SIZE(small_arg_types); bi++)
	for (int yi = 0; yi < SIZE(arg_types); yi++)
	{
#ifdef ONLY_SAMPLES
		if (xorshift32() % 20)
			continue;
#endif
		std::string b_decl = small_arg_types[bi][0];
		strsubst(b_decl, "{dir}", "input");
		strsubst(b_decl, "{name}", "b");

		std::string y_decl = arg_types[yi][0];
		strsubst(y_decl, "{dir}", "output");
		strsubst(y_decl, "{name}", "y");

		std::string b_ref = small_arg_types[bi][1];
		strsubst(b_ref, "{dir}", "input");
		strsubst(b_ref, "{name}", "b");

		std::string y_ref = arg_types[yi][1];
		strsubst(y_ref, "{dir}", "output");
		strsubst(y_ref, "{name}", "y");

		char buffer[1024];
		snprintf(buffer, 1024, "rtl/repeat_ops_%02d%02d%02d.v", a, bi, yi);

		FILE *f = fopen(buffer, "w");
		fprintf(f, "module repeat_ops_%02d%02d%02d(b, y);\n", a, bi, yi);
		fprintf(f, "  %s;\n", b_decl.c_str());
		fprintf(f, "  %s;\n", y_decl.c_str());
		fprintf(f, "  assign %s = {%d{%s}};\n", y_ref.c_str(), a, b_ref.c_str());
		fprintf(f, "endmodule\n");
		fclose(f);
	}
#endif

#ifdef GENERATE_LONGER
	for (int i = 0; i < BIG_N; i++)
	{
//#ifdef ONLY_SAMPLES
	//	if (i == SMALL_N)
		//	break;
//#endif
		int sizetyp=SIZE(arg_types);//Check the size of arg_type
		xorshift32(i);
		char buffer[1024];
		snprintf(buffer, 1024, "rtl/longer_%d.v", i);

		FILE *f = fopen(buffer, "w");
		for  (int k=0; k< 1; k++)
		{
		fprintf(f, "module longer_%d_%d(", i,k);

		for (char var = 'a'; var <= 'b'; var++)
		for (int j = 0; j < SIZE(arg_types); j++)
			fprintf(f, "%c%d, ", var, j);

			for(int j = 0; j < SIZE(arg_types)*300; j++ )
			{
			if( j != SIZE(arg_types)*300-1)
				fprintf(f, "y%d,",j);
			else
				fprintf(f,"y%d",j);
			}

            fprintf(f, ");\n");

		for (char var = 'a'; var <= 'y'; var++) {
			for (int j = 0; j <  SIZE(arg_types)*(var == 'y' ? 300: 1); j++) {
				std::string decl = arg_types[j % SIZE(arg_types)][0];
				strsubst(decl, "{dir}", var == 'y' ? "output" : "input");
				snprintf(buffer, 1024, "%c%d", var, j);
				strsubst(decl, "{name}", buffer);
				fprintf(f, "  %s;\n", decl.c_str());
			}
			if (var == 'b')
				var = 'x';
			fprintf(f, "\n");
		}

		/*/int total_y_size = 0;
		for (int j = 0; j < SIZE(arg_types)*3; j++)
			total_y_size++ //= atoi(arg_types[j % SIZE(arg_types)][2]);
		fprintf(f, "  output [%d:0] y;\n", total_y_size-1);

		fprintf(f, "  assign y = {");
		for (int j = 0; j < SIZE(arg_types)*3; j++)
			fprintf(f, "%sy%d", j ? "," : "", j);
		fprintf(f, "};\n");
		fprintf(f, "\n");*/

		/*for (int j = 0; j < SIZE(arg_types)*3; j++) {
			std::string decl = arg_types[j % SIZE(arg_types)][0];
			strsubst(decl, "{dir}", "localparam");
			snprintf(buffer, 1024, "p%d", j);
			strsubst(decl, "{name}", buffer);
			fprintf(f, "  %s = ", decl.c_str());
			print_expression(f, 1 + xorshift32() % 15, 0, false, false, true);
			fprintf(f, ";\n");
		}
		fprintf(f, "\n");*/

		for (int j = 0; j < SIZE(arg_types)*300; j++) {
			if(xorshift32()%10<8)
			{
			fprintf(f, "  assign y%d = ", j);
			print_expression(f, 1 + xorshift32() % 20,j/*16*/, 0, false, true, false);
			fprintf(f, ";\n");
			}
			else
			{
				 fprintf(f,"  %s(y%d, ", builtin_gate[xorshift32()%SIZE(builtin_gate)],j);
	 			for (int i=0;i< 1+xorshift32()%5;i++)
				 {
					if	(xorshift32()%10<=5)
						fprintf(f,"a%d, ",xorshift32()%SIZE(arg_types));
					else
						fprintf(f,"b%d, ",xorshift32()%SIZE(arg_types));
				 }
				if	(xorshift32()%10<=5)
					fprintf(f,"a%d)",xorshift32()%SIZE(arg_types));
				else
					fprintf(f,"b%d)",xorshift32()%SIZE(arg_types));
					fprintf(f,";\n");
			}
		}

		fprintf(f, "endmodule\n");
		}
		fclose(f);
	}
#endif


#ifdef GENERATE_EMBEDDED
	for (int i = 0; i < BIG_N; i++)
	{  int NUM_SUBMODULE=xorshift32()%10;
		int NUM_EMBPORT=xorshift32()%10;
		int sizetyp=SIZE(arg_types);//Check the size of arg_type
		xorshift32(i);
		char buffer[1024];
		snprintf(buffer, 1024, "rtl/embedded_%d.v", i);

		FILE *f = fopen(buffer, "w");
		for  (int k=0; k< NUM_SUBMODULE; k++)
		{
			fprintf(f, "module  submodule_%d(",k);

	
		for (int j = 0; j < NUM_EMBPORT; j++)
			fprintf(f, "a%d_sub%d, ",j,k );

		for(int j = 0; j < NUM_EMBPORT; j++ )
			{
			if( j != NUM_EMBPORT-1)
				fprintf(f, "y%d_sub%d, ",j,k);
			else
				fprintf(f,"y%d_sub%d",j,k);
			}

            fprintf(f, ");\n");

		for (char var = 'a'; var < 'y'; var++) {
						if (var == 'b')
							var = 'y';
			for (int j = 0; j < NUM_EMBPORT; j++) {
				std::string decl = arg_types[j % SIZE(arg_types)][0];
				strsubst(decl, "{dir}", var == 'y' ? "output" : "input");
				snprintf(buffer, 1024, "%c%d", var, j);
				strsubst(decl, "{name}", buffer);
				fprintf(f, "  %s_sub%d;\n", decl.c_str(),k);
			}
			fprintf(f, "\n");
		}

		for (int j = 0; j < NUM_EMBPORT; j++) {
			if (xorshift32()%10<8)
			{
			fprintf(f, "  assign y%d_sub%d = ", j,k);
			print_embedded(f, 1 + xorshift32() % 20,j/*16*/,k,NUM_EMBPORT,NUM_SUBMODULE, 0, false, true, false);
			fprintf(f, ";\n");			
			}
					
			else
			{
			fprintf(f,"  %s", builtin_gate[xorshift32()%SIZE(builtin_gate)]);
			fprintf(f,"(y%d_sub%d,",j,k);
			fprintf(f,"a%d_sub%d,",xorshift32()%NUM_EMBPORT,k);	 
			fprintf(f,"a%d_sub%d)",xorshift32()%NUM_EMBPORT,k);
			fprintf(f,";\n");
			}
			
		}

		fprintf(f, "endmodule\n");
		fprintf(f,"\n");
		}


		fprintf(f, "module  mainmodule_%d(",i);
	//	int mod=1;
		//mod = 1+ (xorshift32()%4);
//		switch(mod){
//			case 1:
			char var ='a';
			for (int j = 0; j < NUM_EMBPORT*NUM_SUBMODULE; j++)
			fprintf(f, "%c%d, ", var, j);

			for(int j = 0; j < NUM_EMBPORT*NUM_SUBMODULE; j++ )
			{
			if( j != NUM_EMBPORT*NUM_SUBMODULE-1)
				fprintf(f, "y%d, ",j);
			else
				fprintf(f,"y%d",j);
			}

            fprintf(f, ");\n");

		for (char var = 'a'; var <= 'y'; var++) {
				if (var == 'b')
				var = 'y';
			for (int j = 0; j <  NUM_EMBPORT*NUM_SUBMODULE; j++) {
				std::string decl = arg_types[j % SIZE(arg_types)][0];
				strsubst(decl, "{dir}", var == 'y' ? "output" : "input");
				snprintf(buffer, 1024, "%c%d", var, j);
				strsubst(decl, "{name}", buffer);
				fprintf(f, "  %s;\n", decl.c_str());
			}
		
			fprintf(f, "\n");
		}
		for (int j=0; j< NUM_SUBMODULE; j++){
			//informal
			if (xorshift32()%10>=5){
				fprintf(f,"submodule_%d (",j);
				for (int k=NUM_EMBPORT*j; k<NUM_EMBPORT*(j+1);k++){
					fprintf (f,"a%d,",k);
				}
				for (int k= NUM_EMBPORT*j; k<NUM_EMBPORT*(j+1);k++){
					if (k != (j+1)*NUM_EMBPORT-1)
						fprintf(f,"y%d,",k);
					else
					{
						fprintf(f,"y%d);\n",k);
					}
				}
			}
			
			//formal
			else
			{
				fprintf(f,"submodule_%d (",j);
				for (int k=0; k<NUM_EMBPORT;k++){
					fprintf (f,".a%d_sub%d(a%d),",k,j,j*NUM_EMBPORT+k);
				}
				for (int k =0; k<NUM_EMBPORT;k++){
					if (k != NUM_EMBPORT-1)
						fprintf(f,".y%d_sub%d(y%d),",k,j,j*NUM_EMBPORT+k);
					else
					{
						fprintf(f,".y%d_sub%d(y%d));\n",k,j,j*NUM_EMBPORT+k);
					}
				}
			}			
		}
		fprintf(f, "endmodule\n");


		
		fclose(f);
	}
#endif

#ifdef GENERATE_EXPRESSIONS
	for (int i = 0; i < BIG_N; i++)
	{
//#ifdef ONLY_SAMPLES
	//	if (i == SMALL_N)
		//	break;
//#endif
		int sizetyp=SIZE(arg_types);//Check the size of arg_type
		xorshift32(i);
		char buffer[1024];
		snprintf(buffer, 1024, "rtl/expression_%d.v", i);

		FILE *f = fopen(buffer, "w");
		fprintf(f, "module expression_%d(", i);

		for (char var = 'a'; var <= 'b'; var++)
		for (int j = 0; j < SIZE(arg_types); j++)
			fprintf(f, "%c%d, ", var, j);
			for(int j = 0; j < SIZE(arg_types)*3; j++ )
			{
			if( j != SIZE(arg_types)*3-1)
				fprintf(f, "y%d,",j);
			else
				fprintf(f,"y%d",j);
			}

            fprintf(f, ");\n");

		for (char var = 'a'; var <= 'y'; var++) {
			for (int j = 0; j <  SIZE(arg_types)*(var == 'y' ? 3 : 1); j++) {
				std::string decl = arg_types[j % SIZE(arg_types)][0];
				strsubst(decl, "{dir}", var == 'y' ? "output" : "input");
				snprintf(buffer, 1024, "%c%d", var, j);
				strsubst(decl, "{name}", buffer);
				fprintf(f, "  %s;\n", decl.c_str());
			}
			if (var == 'b')
				var = 'x';
			fprintf(f, "\n");
		}
		for (int j = 0; j < SIZE(arg_types)*3; j++) {
			if(xorshift32()%10<8)
			{
			fprintf(f, "  assign y%d = ", j);
			print_expression(f, 1 + xorshift32() % 20,j/*16*/, 0, false, true, false);
			fprintf(f, ";\n");
			}
			else
			{
				 fprintf(f,"  %s(y%d, ", builtin_gate[xorshift32()%SIZE(builtin_gate)],j);
	 			for (int i=0;i< 1+xorshift32()%5;i++)
				 {
					if	(xorshift32()%10<=5)
						fprintf(f,"a%d, ",xorshift32()%SIZE(arg_types));
					else
						fprintf(f,"b%d, ",xorshift32()%SIZE(arg_types));
				 }
				if	(xorshift32()%10<=5)
					fprintf(f,"a%d)",xorshift32()%SIZE(arg_types));
				else
					fprintf(f,"b%d)",xorshift32()%SIZE(arg_types));
					fprintf(f,";\n");
			}
		}

		fprintf(f, "endmodule\n");
		fclose(f);
	}
#endif

#ifdef GENERATE_WIDEEXPR
	for (int i = 0; i < BIG_N; i++)
	{
#ifdef ONLY_SAMPLES
		if (i == SMALL_N)
			break;
#endif
		xorshift32(BIG_N + i);

		char buffer[1024];
		snprintf(buffer, 1024, "rtl/wideexpr_%05d.v", i);

		FILE *f = fopen(buffer, "w");
		fprintf(f, "module wideexpr_%05d(ctrl, ", i);

		for (int i = 0; i < 2; i++)
		for (int j = 0; j < 8; j++)
			fprintf(f, "%c%d, ", "us"[i], j);
		fprintf(f, "y);\n");

		fprintf(f, "  input [7:0] ctrl;\n");
		for (int i = 0; i < 2; i++)
		for (int j = 0; j < 8; j++)
			fprintf(f, "  input %s[%d:0] %c%d;\n", i ? "signed " : "", j, "us"[i], j);

		fprintf(f, "  output [127:0] y;\n");

		for (int j = 0; j < 8; j++)
			fprintf(f, "  wire [15:0] y%d;\n", j);

		fprintf(f, "  assign y = {");
		for (int j = 0; j < 8; j++)
			fprintf(f, "%sy%d", j ? "," : "", j);
		fprintf(f, "};\n");

		for (int j = 0; j < 8; j++) {
			bool is_signed = xorshift32() % 2 == 0;
			fprintf(f, "  assign y%d = ", j);
			print_wideexpr(f, is_signed, 5 + (xorshift32() % 5));
			fprintf(f, ";\n");
		}

		fprintf(f, "endmodule\n");
		fclose(f);
	}
#endif

#ifdef GENERATE_FLIPFLOP
 for( int i = 0; i < BIG_N; i++){
	 int sizetyp=SIZE(arg_types);//Check the size of arg_type
		xorshift32(i);
		char buffer[1024];
		snprintf(buffer, 1024, "rtl/flipflop_%d.v", i);
		int NUM_FF = xorshift32()%10 + 1;
		int trigger = xorshift32()%4;
		FILE *f = fopen(buffer, "w");
		fprintf(f, "module flipflop_%d(", i);

		switch (trigger)
		{
		case 1:
			fprintf(f, "clk, reset,  ");
			break;
		case 2:
			fprintf(f, "clk ,reset_n, ");
			break;
		case 3:
			fprintf(f, "clk_n, reset, ");
			break;
		case 4:
			fprintf(f, "clk_n, reset_n, ");
			break;
		default:
			fprintf(f, "clk, reset,  ");
			break;
		}

            fprintf(f, "a, y);\n");
				switch (trigger)
		{
		case 1:
			fprintf(f, "  input clk, reset;\n");
			break;
		case 2:
			fprintf(f, "  input clk ,reset_n;\n");
			break;
		case 3:
			fprintf(f, " input clk_n, reset;\n");
			break;
		case 4:
			fprintf(f, "  input clk_n, reset_n;\n ");
			break;
		default:
			fprintf(f, "  input clk, reset; \n");
			break;
		}

		for (char var = 'a'; var <= 'y'; var++) {
				std::string decl = arg_types [0][0];
				if (var=='a')
				strsubst(decl, "{dir}", "input");
				else if(var=='y')
				strsubst(decl, "{dir}", "output reg");
				else
				strsubst(decl, "{dir}", "reg");			
				snprintf(buffer, 1024, "%c", var);
				strsubst(decl, "{name}", buffer);
				fprintf(f, "  %s;\n", decl.c_str());
			
			if (var == 97+NUM_FF)
				var = 'x';
		}
			switch (trigger)
		{
		case 1:
			fprintf(f, "always @(posedge  clk or posedge reset)\n");
			break;
		case 2:
			fprintf(f, "always @(posedge  clk or negedge reset_n)\n");
			break;
		case 3:
			fprintf(f, "always @(negedge  clk or posedge reset)\n");
			break;
		case 4:
			fprintf(f, "always @(negedge  clk or negedge reset)\n");
			break;
		default:
			fprintf(f, "always @(posedge  clk or posedge reset)\n");
			break;
		}
		fprintf(f,"begin\n");
		
		if ((trigger==2)||(trigger==4))	
			fprintf(f,"	if (!reset_n)\n");
		else
			fprintf(f,"	if (reset)\n");
		fprintf(f,"		begin\n");
		fprintf(f,"			");
			for (char j = 'b'; j < 'b'+NUM_FF; j++)
			{
				if (j == 'b'+NUM_FF - 1)
				{
					fprintf(f,"%c <= 1'b0;\n",j);
					break;
				}
				fprintf(f,"%c <= 1'b0; ",j);
			}
		fprintf(f,"		end\n");
		fprintf(f,"	else\n");
		fprintf(f,"		begin\n");
		fprintf(f,"			");
		char j_pre;
			for (char j = 'b'; j <='y'; j++)
			{
				if (j == 'y')
				{
					fprintf(f,"y <= %c;\n",j_pre);
					break;
				}
				fprintf(f,"%c <= %c; ",j,j-1);
				j_pre = j;
				if (j == 'a'+NUM_FF)
				j = 'x';
			}
		fprintf(f,"		end\n");
		fprintf(f,"end\n");
		fprintf(f, "endmodule\n");
		fclose(f);
 }
#endif

#ifdef GENERATE_STATEMACHINE
 for( int i = 0; i < BIG_N; i++){
		xorshift32(i);
		char buffer[1024];
		snprintf(buffer, 1024, "rtl/FSM_%d.v", i);
		int NUM_FSM = xorshift32()%10 + 1;
		int NUM_IN = 2*NUM_FSM;

		int trigger = xorshift32()%4;
		FILE *f = fopen(buffer, "w");
		fprintf(f, "module FSM_%d(", i);

		switch (trigger)
		{
		case 1:
			fprintf(f, "clk, reset,  ");
			break;
		case 2:
			fprintf(f, "clk ,reset_n, ");
			break;
		case 3:
			fprintf(f, "clk_n, reset, ");
			break;
		case 4:
			fprintf(f, "clk_n, reset_n, ");
			break;
		default:
			fprintf(f, "clk, reset,  ");
			break;
		}

for(char i = 'a'; i < 'a'+NUM_IN; i++)
{
	fprintf(f,"%c, ",i);
}
            fprintf(f, "y);\n");
				switch (trigger)
		{
		case 1:
			fprintf(f, "  input clk, reset;\n");
			break;
		case 2:
			fprintf(f, "  input clk ,reset_n;\n");
			break;
		case 3:
			fprintf(f, " input clk_n, reset;\n");
			break;
		case 4:
			fprintf(f, "  input clk_n, reset_n;\n ");
			break;
		default:
			fprintf(f, "  input clk, reset; \n");
			break;
		}

		for (char var = 'a'; var <= 'y'; var++) {
				std::string decl = arg_types [0][0];
				if(var =='y')
				strsubst(decl, "{dir}", "output ");
				else
				strsubst(decl, "{dir}", "input");			
				snprintf(buffer, 1024, "%c", var);
				strsubst(decl, "{name}", buffer);
				fprintf(f, "  %s;\n", decl.c_str());
			
			if (var == 96+NUM_IN)
				var = 'x';
		}
		fprintf(f,"  reg [%d:0] p_state, n_state;\n",int(log2(NUM_FSM)));
		fprintf(f,"parameter ");
		for (int i = 0; i <= NUM_FSM; i++)
		{
			if (i == NUM_FSM)
			{
				fprintf(f,"S%d = %d'd%d;\n",i, int((log2(NUM_FSM))+1),i);
				break;
			}
			else
			{
			fprintf(f,"S%d = %d'd%d, ",i,int((log2(NUM_FSM))+1),i);
			}
		}
		
			switch (trigger)
		{
		case 1:
			fprintf(f, "always @(posedge clk or posedge reset)\n");
			break;
		case 2:
			fprintf(f, "always @(posedge clk or negedge reset_n)\n");
			break;
		case 3:
			fprintf(f, "always @(negedge clk_n or posedge reset)\n");
			break;
		case 4:
			fprintf(f, "always @(negedge clk_n or negedge reset_n)\n");
			break;
		default:
			fprintf(f, "always @(posedge clk or posedge reset)\n");
			break;
		}
		fprintf(f,"begin\n");
		
		if ((trigger==2)||(trigger==4))	
			fprintf(f,"	if(!reset_n)\n");
		else
			fprintf(f,"	if(reset)\n");
		fprintf(f,"		begin\n");
		fprintf(f,"			p_state <= S0;\n");
		fprintf(f,"		end\n");
		fprintf(f,"	else\n");
		fprintf(f,"		begin\n");
		fprintf(f,"			p_state <= n_state;\n");
		fprintf(f,"		end\n");
		fprintf(f,"end\n");
fprintf(f,"always @(p_state or ");
for(char i = 'a'; i <='a'+NUM_IN; i++)
{
	if (i == 'a'+NUM_IN-1)
	{
		fprintf(f,"%c)\n",i);
		break;
	}
	else
	fprintf(f,"%c or ",i);
}
fprintf(f,"  begin\n");
fprintf(f,"case (p_state)\n");
char in = 'a';
for (int i = 0; i <=NUM_FSM; i++)
{	
	if (i == NUM_FSM-1)
	{
	fprintf(f,"S%d: begin\n",i);
	fprintf(f,"		if (%c %s %c)\n", in,binary_ops [xorshift32() % SIZE(binary_ops)],in+1);
	fprintf(f,"		n_state = S%d;\n",xorshift32()%NUM_FSM);
	fprintf(f,"		else\n");
	fprintf(f,"		n_state = S%d;\n",i);
	fprintf(f,"end\n");	
	fprintf(f,"default: n_state = S0;\n");
	fprintf(f,"endcase\n");
	break;
	}
	
	fprintf(f,"S%d: begin\n",i);
	fprintf(f,"		if (%c %s %c)\n", in,binary_ops [xorshift32() % SIZE(binary_ops)],in+1);
	fprintf(f,"		n_state = S%d;\n",i+1);
	fprintf(f,"		else\n");
	fprintf(f,"		n_state = S%d;\n",i);
	fprintf(f,"end\n");
	in = in + 2;
}
		fprintf(f,"end\n");
if (xorshift32()%10>5)
{
	fprintf(f, "assign y = (p_state == S%d);\n",xorshift32()%NUM_FSM);
}
else
	{
		fprintf(f,"assign y = (%c %s p_state == S%d);\n",'a'+rand()%NUM_IN-1, bitwise_ops[xorshift32()%SIZE(bitwise_ops)], xorshift32()%NUM_FSM);
	}
		fprintf(f, "endmodule\n");
		fclose(f);
 }
#endif

#ifdef GENERATE_PARTSEL
	for (int i = 0; i < BIG_N; i++)
	{
#ifdef ONLY_SAMPLES
		if (i == SMALL_N)
			break;
#endif
		xorshift32(2*BIG_N + i);

		char buffer[1024];
		snprintf(buffer, 1024, "rtl/partsel_%05d.v", i);

		FILE *f = fopen(buffer, "w");
		fprintf(f, "module partsel_%05d(ctrl, s0, s1, s2, s3, ", i);

		for (int i = 0; i < 4; i++)
			fprintf(f, "x%d, ", i);
		fprintf(f, "y);\n");

		fprintf(f, "  input [3:0] ctrl;\n");
		fprintf(f, "  input [2:0] s0;\n");
		fprintf(f, "  input [2:0] s1;\n");
		fprintf(f, "  input [2:0] s2;\n");
		fprintf(f, "  input [2:0] s3;\n");

		for (int i = 0; i < 4; i++)
			fprintf(f, "  input %s[31:0] x%d;\n", xorshift32() % 2 ? "signed " : "", i);

		for (int i = 4; i < 16; i++)
			if (xorshift32() % 2)
				fprintf(f, "  wire %s[%d:%d] x%d;\n", xorshift32() % 2 ? "signed " : "",
						xorshift32() % 8, 31 - xorshift32() % 8, i);
			else
				fprintf(f, "  wire %s[%d:%d] x%d;\n", xorshift32() % 2 ? "signed " : "",
						31 - xorshift32() % 8, xorshift32() % 8, i);

		fprintf(f, "  output [127:0] y;\n");
		for (int i = 0; i < 4; i++)
			fprintf(f, "  wire %s[31:0] y%d;\n", xorshift32() % 2 ? "signed " : "", i);

		fprintf(f, "  assign y = {");
		for (int i = 0; i < 4; i++)
			fprintf(f, "%sy%d", i ? "," : "", i);
		fprintf(f, "};\n");

		for (int i = 0; i < 4; i++)
			if (xorshift32() % 2)
				fprintf(f, "  localparam %s[%d:%d] p%d = %d;\n", xorshift32() % 2 ? "signed " : "",
						xorshift32() % 8, 31 - xorshift32() % 8, i, xorshift32() % 1000000000);
			else
				fprintf(f, "  localparam %s[%d:%d] p%d = %d;\n", xorshift32() % 2 ? "signed " : "",
						31 - xorshift32() % 8, xorshift32() % 8, i, xorshift32() % 1000000000);

		for (int i = 4; i < 20; i++) {
			fprintf(f, "  assign %c%d = ", i < 16 ? 'x' : 'y', i < 16 ? i : i - 16);
			print_partsel(f, i < 16 ? i : 16, 0);
			fprintf(f, ";\n");
		}

		fprintf(f, "endmodule\n");
		fclose(f);
	}
#endif

	return 0;
}
