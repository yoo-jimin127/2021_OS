#include <linux/kernel.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

typedef enum {
	OPRD_NUM,
	ADD, MINUS, MULTI, MOD,
	FINISH_INDICATOR
}ParsingElement;

ParsingElement parsingElement;
char tok = ' ';
long oprd_num;

long* add_func(long oprd1, long oprd2);
long* minus_func(long oprd1, long oprd2);
long* mul_func(long oprd1, long oprd2);
long* mod_func(long oprd1, long oprd2);

void parse_input();
int pick_operand();
long send_operand();
void print_errmsg(char *error_msg);

long* add_func(long oprd1, long oprd2) {
	long error_num;
	long* res;
	res = (long*)malloc(sizeof(long));
	error_num = syscall(443, oprd1, oprd2, res);

	if (error_num == -EFAULT) {
		return NULL;
	}

	return res;
}

long* minus_func(long oprd1, long oprd2) {
	long error_num;
	long* res;
	res = (long*)malloc(sizeof(long));
	error_num = syscall(444, oprd1, oprd2, res);

	if (error_num == -EFAULT) {
		return NULL;
	}

	return res;
}

long* mul_func(long oprd1, long oprd2) {
	long error_num;
	long* res;
	res = (long*)malloc(sizeof(long));
	error_num = syscall(445, oprd1, oprd2, res);

	if (error_num == -EFAULT) {
		return NULL;
	}

	return res;
}

long* mod_func(long oprd1, long oprd2) {
	long error_num;
	long* res;
	res = (long*)malloc(sizeof(long));
	error_num = syscall(446, oprd1, oprd2, res);

	if (error_num == -EFAULT) {
		return NULL;
	}

	return res;
}

void parse_input() {
	while(tok == ' ') {
		tok = getchar();
	}

	
	if (tok == '+') parsingElement = ADD;
	else if (tok == '-') parsingElement = MINUS;
	else if (tok == '*') parsingElement = MULTI;
	else if (tok == '%') parsingElement = MOD;
	else if (tok >= '0' && tok <= '9') {
		long operand_buf = 0;

		parsingElement = OPRD_NUM;

		while (tok >= '0' && tok <= '9') {
			operand_buf = (operand_buf * 10 + tok) - '0';
			tok = getchar();
		}

		oprd_num = operand_buf;
	}

	else if (tok == '(') {
		long operand_buf = 0;

		parsingElement = OPRD_NUM;
		tok = getchar();

		if (tok == '-') {
			tok = getchar();

			while (tok >= '0' && tok <= '9') {
				operand_buf = (operand_buf * 10 + tok) - '0';
				tok = getchar();
			}
		}

		else if (tok != ')') {
			print_errmsg("ERROR: negative operand is not permitted.");
		}

		else {
			operand_buf = 0 - tok;
		}

		tok = getchar();
	}

	else if (tok == '\n') {
		parsingElement = FINISH_INDICATOR;
	}

	else {
		print_errmsg("ERROR: input expression error.");
	}
}

int pick_operand() {
	if (parsingElement == OPRD_NUM) {
		int res = oprd_num;
		return res;
	}

	print_errmsg("ERROR: input expression pick_operand function.");
}

long send_operand() {
	long* res;
	long operand1, operand2;

	operand1 = pick_operand();

	while (parsingElement != FINISH_INDICATOR) {
		parse_input();

		if (parsingElement == ADD) {
			tok = getchar();
			parse_input();
			operand2 = pick_operand();

			res = add_func(operand1, operand2);
		}

		else if (parsingElement == MINUS) {
			tok = getchar();
			parse_input();
			operand2 = pick_operand();

			res = minus_func(operand1, operand2);	
		}

		else if (parsingElement == MULTI) {
			tok = getchar();
			parse_input();
			operand2 = pick_operand();

			res = mul_func(operand1, operand2);
		}

		else if (parsingElement == MOD) {
			tok = getchar();
			parse_input();
			operand2 = pick_operand();

			res = mod_func(operand1, operand2);
		}

		else if (parsingElement == FINISH_INDICATOR) {
			break;
		}

		else {
			print_errmsg("ERROR: operand2 is not appropriate.");
		}
	}
	
	if (res == NULL) {
		print_errmsg("ERROR: system call function is ERROR. res is not valid.");
	}

	return *res;
}

void print_errmsg(char *error_msg) {
	fprintf(stderr, "%s\n", error_msg);
	exit(1);
}

int main (void) {
	long res;

	parse_input();
	res = send_operand();
	printf("result of input expression : %ld\n", res);

	return 0;
}
