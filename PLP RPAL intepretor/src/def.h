#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


//state
#define S_S			1	//state for finite state machine
#define ID_S	 	2
#define INT_S		3
#define STR_S		4
#define SPACE_S		5
#define COMMENT_S 	6
#define	PUNC_S		7
#define OP_S		8

#define END_S		9   //end state

#define ID_F	 	-2	//final state
#define INT_F		-3
#define STR_F		-4
#define	PUNC_F		-5
#define OP_F		-6

//keyword type
#define IN		10
#define GR		11
#define GE		12
#define LS		13
#define LE		14
#define EQ		15
#define NE		16
#define OR		17
#define FN		18

#define LET		19
#define NOT		20
#define NEG		21
#define XNIL	22
#define AND2	23//and
#define REC		24
#define AUG		25

#define TRUE	26

#define WHERE	27
#define FALSE	28
#define DUMMY	29

#define WITHIN	30


//token type
#define ID 		31
#define INT		32
#define STR		33
#define OP		34
#define DELETE	35
#define L_PAREN	36
#define R_PAREN	37
#define SEMI	38

//OP type
#define PERIOD	40
#define AND		41//&
#define TIMES	42
#define PLUS	43
#define MINUS	44
#define DIVIDE	45
#define ATX		46//@
#define BAR		47
#define EQUAL	48
#define COND	49
#define EXPO	50

//tree node
#define LAMBDA		51
#define TAU			52
#define GAMMA		53
#define COMMA		54
#define YSTAR		55

char *st_node_name[]={
	"lambda","tau","gamma",",","Y*"
};

#define FCN_FORM	56
#define EMPTY_PAREN	57

// macro constant
#define MAX_CONTENT_LEN 50
#define MAX_LINE_LEN 500

#define debug 0


//structures and global variables
FILE *fp; //file pointer

struct token {
	int type;	//it's a number for later switch of parser fn.
	char content[MAX_CONTENT_LEN]; //what information the token carries
};

struct token *t; //pinter for one token on demand


char line[MAX_LINE_LEN];
char *start=line, *here=line;


struct tree {
	int type;
	char content[MAX_CONTENT_LEN];
	struct tree *child;
	struct tree *sibling;
};

struct t_stack {
	struct tree *member;
	struct t_stack *next;
};

struct t_stack *top=NULL;



//optional varaibles
int print_list=0, print_ast=0, print_st=0;

#define MAX_SIBLING 100





