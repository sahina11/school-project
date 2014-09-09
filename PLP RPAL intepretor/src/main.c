#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "def.h"
#include "token.c"
#include "ast.c"
#include "st.c"

#include "cse.h"
#include "delta.c"
#include "rule.c"

int main(int argc, char *argv[]){
	if((fp= fopen(argv[argc-1],"r")) ==NULL){
		printf("File \"%s\" not found!\n",argv[argc-1]);
		exit(-1);
	}

	t= calloc(1,sizeof(struct token));

	struct tree *p_tree_ast;


	if(argc <2){
		printf("\n\nUsage: %s [-l] [-ast] [-st] filename\n\n",argv[0]);
		exit(-1);
	}

	int i;
	for(i=1;i<argc-1;i++){
		if(strcmp("-l",argv[i]) == 0) print_list++;
		else if(strcmp("-ast",argv[i]) == 0) print_ast++;
		else if(strcmp("-st",argv[i]) == 0) print_st++;
	}

	p_tree_ast = ast();
	if(print_ast){
		traverse_tree(p_tree_ast,0);
	}

	st(p_tree_ast);
	if(print_st){
		traverse_tree(p_tree_ast,0);
	}

	struct delta *d_temp=make_delta();
	struct delta *d_head = d_temp;
	struct atom *pa;
	i=0;
	d_head->index = i;
	d_head->structure = make_ctrl_structure(p_tree_ast,&i,d_temp);

	struct atom *c_head;
	struct atom *s_head;
	struct environment *e_head, *e_cur;
	int env_index=0;
	int *index;
	index = &env_index;

	//initilize the context
	e_head = make_env(index); //PE on tree
	e_cur = e_head;

	c_head = make_env_atom(e_head); //atom PE in ctrl
	s_head = copy_atom(c_head);
	c_head->next = delta_n_structure(d_head,0);

	struct atom *result;
	result = cse(c_head,s_head,d_head,e_cur,index);
	
	if(result!=NULL && result->type == V_INT){
		printf("%d\n",result->val.value );
	}

	fclose(fp);
	return 0;
}
