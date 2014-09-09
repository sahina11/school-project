#define GENERAL	888 /* General things which stack by CSE Rule 1 */
#define BINOPA	100 /* Arithmatic BINOP */
#define BINOPL  101 /* Logical BINOP */
#define UNOP	110 /* Unary operators  */
#define BETA	200 /* The beta structure (for conditionals) */
#define ETA		300 /* The eta structure (for recursion) */
#define ENVIRO	400 /* The environment marker type */
#define EMPTY	-1  /* Signfies empty stack */
#define UNKNOWN -2  /* Unknown variable */
#define V_INT   1024
#define V_BOOL  1025


// item is a linked list will solve the problem of multiple varialbes 
// in lambda expression and environment structure
struct atom{ //linked list, basic building block of every data structure above
	int type;
	union{
		struct leta *l;
		struct environment *env;
		struct beta *b;
		struct tree *t; // store tree nodes: id, int, +-*/.....		
		struct tuple *tup;
		
		int value;//for v_int and v_bool
	} val;
	struct atom *next;
};

struct leta{ // lambda and eta
	int index;
	struct environment *c;
	struct atom *var;
};

struct beta{
	struct delta *T;
	struct delta *F;
};

struct tuple{
	int index;
	struct atom *list;
};

//--------------------------------------------------------
struct delta{
	int index;
	struct atom *structure;
	struct delta *next;
};


struct environment{ 
	int index;
	struct atom *var;
	struct atom *var_to;
	
	struct environment *parent;
	struct environment *child;
	struct environment *sibling;
};


