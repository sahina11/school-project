#define DE 0
//sub support function
struct atom *wind(struct atom *list){ //return the last atom of the list
	if (list ==NULL) return NULL;

	struct atom *pa;
	pa = list;
	while(pa->next != NULL){
		pa = pa->next;
	}

	return pa;
}

struct delta *wind_d(struct delta *list){ //return the last delta of the list
	if (list ==NULL) return NULL;

	struct delta *pd;
	pd = list;
	while(pd->next != NULL){
		pd = pd->next;
	}

	return pd;
}

struct environment *make_env(int *index){ //initialize an env ptr
	struct environment *env;
	env = (struct environment *)malloc(sizeof(struct environment));
	env->index = *index;

	env->var = NULL;
	env->var_to = NULL;
	env->parent = NULL;
	env->child = NULL;
	env->sibling = NULL;
	(*index)++;
	

	return env;
}

struct atom *make_env_atom(struct environment *env){
	struct atom *pa;
	pa =(struct atom *)malloc(sizeof(struct atom));

	pa->type = ENVIRO;
	pa->val.env = env;
	pa->next = NULL;

	return pa;
}

struct atom *copy_atom(struct atom *source){
	if (source ==NULL) return NULL;

	struct atom *pa;
	pa = (struct atom *)malloc(sizeof (struct atom));

	pa->type = source->type;
	pa->val = source->val;
	pa->next = NULL;

	return pa;
}

struct atom *copy_atom_list(struct atom *source){ //return the head of list
	
	struct atom *cp;

	if(source ==NULL)
		return NULL;
	else{
		cp = copy_atom(source);
		cp->next = copy_atom_list(source->next);	
	}
	
	return cp;
}

struct atom *chop(struct atom *list){//return the head of chopped list
	if (list ==NULL) return NULL;

	struct atom *pa;
	pa = list;

	if(pa->next ==NULL) return NULL;

	while (pa->next->next !=NULL) {
		pa = pa ->next; 
	}
	pa->next =NULL;
	//because of this, I can change the link in rule functions without passing 
	//s_head or c_head by reference 

	
	return list;
}


struct atom *delta_n_structure (struct delta *d_head, int index){ //return copied head of delta structure
	struct delta *ptr;
	ptr = d_head;
	int i;
	for(i = index;i>0;i--){
		if (ptr == NULL) return NULL;
		else
			ptr = ptr->next;
	}

	return copy_atom_list(ptr->structure);
}


int true_type(struct atom *pa){
	switch (pa->type){
		case GENERAL:
		case UNOP:
		case BINOPA:
		case BINOPL: return (pa->val.t->type);
		default: return (pa->type);
	}
}

struct atom *look_up(struct atom *var, struct environment *e_cur){
	// printf("in lookup\n");
	if(e_cur->index ==0)
		return NULL;
	// printf("var:%s vs. var in env:%s\n",var->val.t->content,e_cur->var->val.t->content );


	if(strcmp(var->val.t->content,e_cur->var->val.t->content) ==0){
		// printf("going to return var_to:%p\n",e_cur->var_to);
		return (e_cur->var_to);
	}
	else{
		// printf("go to parent\n");
		return look_up(var,e_cur->parent);		
	}
}

struct atom *cr_int_atom(int value){
	struct atom *adam;
	adam = (struct atom *)malloc(sizeof(struct atom));
	adam->type=V_INT;
	adam->val.value = value;
	adam->next =NULL;

	return adam;
}
struct atom *cr_bool_atom(int value){
	struct atom *adam;
	adam = (struct atom *)malloc(sizeof(struct atom));
	adam->type =V_BOOL;
	adam->val.value = value;
	adam->next =NULL;

	return adam;
}

int stoi(char *str){ //can handle only value within int
	char neg=0;
	int value = 0;
	char *p;

	p =str;
	if(*p =='-'){
		neg =1;
		p++;
	}

	while(*p != '\0'){
		value = value*10 + (*p-'0');
		p++;
	}
		
	if (neg){
		return -value;
	}
	else
		return value;

}

struct environment *find_new_env(struct atom *s_head){
	// printf("into find_new_env\n");

	if(s_head == NULL){
		printf("NULL s_head\n");
		return NULL;
	}
	struct atom *tail,*clone;
	clone = copy_atom_list(s_head);
	tail = wind(clone);



	if(tail->type ==ENVIRO){
		return tail->val.env;
	}
	else
		return find_new_env(chop(clone));

}

//---------------------------------
void cout_envtree(struct environment *envtree, int level){
	dots(level);
	printf("Env(%d)\n",envtree->index);

	if(envtree->child != NULL) cout_envtree(envtree->child,level+1);
	if(envtree->sibling != NULL) cout_envtree(envtree->sibling,level);
}

void cout_path2parent(struct environment *env){
	if(env->index ==0){
		printf("env(%d)\n",env->index );
		return;
	}
	printf("env(%d)-->",env->index );
	cout_path2parent(env->parent);
}
void cout_delta(struct delta *d_head){
	struct atom *pa;
	while(d_head != NULL){
		printf("delta %d:   ",d_head->index);
		pa = d_head->structure;
		while(pa != NULL){
			switch(pa->type){
				case AUG:{
					printf("aug  ");
					break;
				}
				case BETA:{
					printf("delta(%d)  ",pa->val.b->T->index);
					printf("delta(%d)  ",pa->val.b->F->index);
					printf("Beta  ");
					break;
				}
				case ETA:{
					printf("eta  ");
					break;
				}
				case TAU:{
					printf("tau(%d)  ",pa->val.tup->index);
					break;
				}
				case GENERAL:
				case BINOPA:
				case BINOPL:
				case UNOP:{
					switch (pa->val.t->type){
						case ID:{
							printf("id:%s  ",pa->val.t->content );
							break;	
						}
						case STR:{
							printf("str:%s  ",pa->val.t->content );
							break;		
						}
						default:
						printf("%s  ",pa->val.t->content);
					}
					break;
				}
				case GAMMA:{
					printf("Gamma  ");
					break;
				}
				case LAMBDA:{
					struct atom *var_ptr;
					var_ptr= pa->val.l->var;
					printf("Lambda(%d):",pa->val.l->index);
					while(var_ptr!=NULL){
						printf("var(%s)  ",var_ptr->val.t->content );	
						var_ptr=var_ptr->next;
					}
					
					break;
				}

			}
			pa = pa->next;	
		}
		printf("\n");
		d_head=d_head->next;
	}
}
void c_atom(struct atom *pa){
	// printf("in c_atom\n");
	switch(pa->type){
		case V_INT:{
			printf("int(%d)  ",pa->val.value);
			break;
		}
		case V_BOOL:{
			if(pa->val.value ==1)
				printf("bool(TRUE)  ");
			else
				printf("bool(FALSE)  ");
			break;
		}
		case BETA:{
			printf("delta(%d)  ",pa->val.b->T->index);
			printf("delta(%d)  ",pa->val.b->F->index);
			printf("Beta  ");
			break;
		}
		case ETA:{
			struct atom *var_ptr;
			var_ptr= pa->val.l->var;
			printf("Eta(%d):",pa->val.l->index);
			printf("var(%s),env(%d)  ",var_ptr->val.t->content,pa->val.l->c->index );
			break;
		}
		case TAU:{
			printf("tau(%d)  ",pa->val.tup->index);
			break;
		}
		case GENERAL:
		case BINOPA:
		case BINOPL:
		case UNOP:{
			switch (pa->val.t->type){
				case ID:{
					printf("id:%s  ",pa->val.t->content );
					break;	
				}
				case STR:{
					printf("str:%s  ",pa->val.t->content );
					break;		
				}
				default:
				printf("%s  ",pa->val.t->content);
			}
			break;
		}
		case GAMMA:{
			printf("Gamma  ");
			break;
		}
		case LAMBDA:{
			struct atom *var_ptr;
			var_ptr= pa->val.l->var;
			printf("Lambda(%d):",pa->val.l->index);
			while(var_ptr!=NULL){
				printf("var(%s)  ",var_ptr->val.t->content );	
				var_ptr=var_ptr->next;
			}
			break;
		}
		case ENVIRO:{
			printf("Env(%d)  ",pa->val.env->index );
			break;
		}
	}
}

void c_list(struct atom *l_head){
	if(l_head ==NULL)
		printf("NULL list\n");
	struct atom *pa;
	pa = l_head;
	while(pa != NULL){
		c_atom(pa);
		pa = pa->next;	
	}	
}

void c_list_reverse(struct atom *l_head){
	if(l_head ==NULL)
		printf("NULL list\n");
	struct atom *pa,*pa_ptr;
	pa = copy_atom_list(l_head);
	pa_ptr = wind(pa);

	while(pa_ptr != NULL){
		switch(pa_ptr->type){
			case V_INT:{
				printf("int(%d)  ",pa_ptr->val.value);
				break;
			}
			case V_BOOL:{
				if(pa_ptr->val.value ==0)
					printf("bool(F)  ");
				else
					printf("bool(T)  ");
				break;
			}
			case BETA:{
				printf("delta(%d)  ",pa_ptr->val.b->T->index);
				printf("delta(%d)  ",pa_ptr->val.b->F->index);
				printf("Beta  ");
				break;
			}
			case ETA:{
				struct atom *var_ptr;
				var_ptr= pa_ptr->val.l->var;
				printf("Eta(%d):",pa_ptr->val.l->index);
				printf("var(%s),env(%d)  ",var_ptr->val.t->content,pa_ptr->val.l->c->index );	

				// printf("eta  ");
				break;
			}
			case TAU:{
				printf("tau(%d)  ",pa_ptr->val.tup->index);
				break;
			}
			case GENERAL:
			case BINOPA:
			case BINOPL:
			case UNOP:{
				switch (pa_ptr->val.t->type){
					case ID:{
						printf("id:%s  ",pa_ptr->val.t->content );
						break;	
					}
					case STR:{
						printf("str:%s  ",pa_ptr->val.t->content );
						break;		
					}
					default:
					printf("%s  ",pa_ptr->val.t->content);
				}
				break;
			}
			case GAMMA:{
				printf("Gamma  ");
				break;
			}
			case LAMBDA:{
				struct atom *var_ptr;
				var_ptr= pa_ptr->val.l->var;
				printf("Lambda(%d):",pa_ptr->val.l->index);
				while(var_ptr!=NULL){
					printf("var(%s),env(%d)  ",var_ptr->val.t->content,pa_ptr->val.l->c->index );	
					var_ptr=var_ptr->next;
				}				

				break;
			}
			case ENVIRO:{
				printf("Env(%d)  ",pa_ptr->val.env->index );
				break;
			}
		}
		
		pa= chop(pa);
		pa_ptr = wind(pa);
	}	//used for cout stack
}

void c_panel(struct atom *c_head, struct atom *s_head){
	printf("\t");
	printf("control list:");
	c_list(c_head);
	printf("\n");

	printf("\t");
	printf("stack list:");
	c_list_reverse(s_head);
	printf("\n\n");

}
//-----------------------------


//rules
void rule1(struct atom *c_head, struct atom *s_head, struct environment *e_cur){
	if(DE)
		printf("rule1:\n");
	
	struct atom *pc,*ps;
	pc = wind(c_head);
	ps= wind(s_head);

	c_head=chop(c_head);

	
	if(true_type(pc)==ID){
		if (strcmp(pc->val.t->content, "Print")==0){
			ps->next = pc;
			return;
		}
		else if(strcmp(pc->val.t->content, "Stern")==0){
			ps->next = pc;
			return;
		}
		else if(strcmp(pc->val.t->content, "Stem")==0){
			ps->next = pc;
			return;
		}
		else if(strcmp(pc->val.t->content, "Conc")==0){
			ps->next = pc;
			return;
		}
		else if(strcmp(pc->val.t->content, "Order")==0){
			ps->next = pc;
			return;
		}
		struct atom *finding;
		// printf("going to lookup\n");
		finding= look_up(pc,e_cur);

		// printf("what I reveive:%p\n",finding);
		ps->next = finding;
		// printf("ps->next=%p\n",ps->next );

		if(ps->next ==NULL)
			printf("lookup failed\n");
	}
	else if(true_type(pc)==INT){
		int si;
		si = stoi(pc->val.t->content);
		ps->next=cr_int_atom(si);
	}
	else if(true_type(pc)== TAU){ // = rule9 tuple formation
		struct atom *s_tail;
		s_tail = wind(s_head);
		if(s_tail->type == ENVIRO){
			s_tail->next = pc;
			return;
		}
		else if(s_tail->type == TAU){
			s_tail->next =pc;
			return;
		}

		struct atom *list_head,*list_atch, *list_ptr;// prepare to link list
		
		list_head=wind(s_head);
		list_atch=list_head;
		
		int n=pc->val.tup->index;

		while(n-1>0){
			chop(s_head);
			list_ptr=wind(s_head);

			list_atch->next = list_ptr;
			list_atch=list_atch->next;
			n--;
		}
		chop(s_head); //cut off the last tuple member left on the stack
		if(DE){
			printf("\tthe atom should be:\n");
			printf("\tatom adr:%p\n",pc );
			printf("\ttau=%d\n",pc->val.tup->index );
			printf("\tval.tup->list=%p\n",list_head);
		}
		struct tuple *tup;
		tup = (struct tuple *)malloc(sizeof(struct tuple));
		tup->list = list_head;
		tup->index = pc->val.tup->index;

		pc->val.tup = tup;
		pc->next =NULL;


		s_tail=wind(s_head);
		s_tail->next = pc;

		if(DE){
			s_tail=s_tail->next;
			printf("\tin the forged atom:\n");
			printf("\ts_tail adr:%p\n",s_tail );
			printf("\ttau=%d\n",s_tail->val.tup->index );
			printf("\ts_tail->val.tup->list=%p\n",s_tail->val.tup->list);
		}
				

	}
	else if (true_type(pc)==DUMMY){
		;
	}
	else
		ps->next = pc;

}

void rule2(struct atom *c_head,struct atom *s_head, struct environment *e_cur){
	if(DE)
		printf("rule2:\n");
	struct atom *pc,*ps;
	pc = wind(c_head);
	ps= wind(s_head);

	c_head=chop(c_head);

	pc->val.l->c = e_cur;
	ps->next = pc;
}


void rule4(struct atom *c_head,struct atom *s_head,struct delta *d_head,int *index,struct environment **e_cur){
	if(DE)
		printf("rule4:\n");
	struct atom *c_tail,*s_tail,*orand, *lambda,*env_atom,*copied_env_atom;
	struct environment *new_env;
	c_head=chop(c_head); //remove gamma
	c_tail=wind(c_head);

	lambda=wind(s_head);
	s_head=chop(s_head);
	orand=wind(s_head);
	s_head=chop(s_head);
	s_tail=wind(s_head);

	//make env and graft it
	new_env=make_env(index);
	new_env->var = lambda->val.l->var;
	new_env->var_to = orand;
	new_env->parent = lambda->val.l->c;
	*e_cur=new_env; //update current environment

	//grafting env tree
	struct environment *parent,*ptr;
	parent = lambda->val.l->c;
	if(parent->child == NULL)
		parent->child = new_env;
	else{
		ptr = parent->child;
		while(ptr->sibling !=NULL)
			ptr = ptr->sibling;

		ptr->sibling=new_env;
	}


	//make env atom, copied, and attach to the end of stack and ctrl
	env_atom=make_env_atom(new_env);
	copied_env_atom = copy_atom(env_atom);
	c_tail->next = env_atom;
	c_tail=wind(c_head);
	s_tail->next = copied_env_atom;
	
	//attach delta structure at the end of ctrl
	c_tail->next = delta_n_structure(d_head,lambda->val.l->index);




}




void rule5(struct atom **c_head,struct atom *s_head,struct environment **e_cur){
	if(DE)
		printf("rule5:\n");
	
	// struct atom *c_env;
	// c_env = wind(*c_head);

	// int env_num = c_env->val.env->index;
	// struct atom *s_tail, *new_head;

	// if(*s_head == NULL){
	// 	printf("error: Null stack\n");
	// 	exit;		
	// }
	// else if((*s_head)->type == ENVIRO && (*s_head)->val.env->index == env_num){
	// 	chop(*c_head);
	// 	*s_head= (*s_head)->next;
	// }
	// else{
	// 	struct atom *new_tail, *new_head;
	// 	for(new_tail=*s_head;
	// 		new_tail->next->type != ENVIRO || new_tail->next->val.env->index != env_num;
	// 		new_tail=new_tail->next){
	// 		if(new_tail== NULL){
	// 			printf("Didn't find according ENVIRO\n");
	// 			exit;
	// 		}
	// 	}

	// 	new_head = new_tail->next->next;
	// 	new_tail->next = new_head;
	// }


	

//------------------------------------------------
	// struct atom *s_tail, *c_tail; 
	// s_tail=wind(s_head);
	// c_tail=wind(*c_head);

	// if(c_tail->type ==ENVIRO &&
	// 	s_tail->type == ENVIRO &&
	// 	c_tail->val.env->index == s_tail->val.env->index){

	// 	chop(*c_head);
	// 	chop(s_head);
	// 	return;
	// }

	
	struct atom *ptail,*value;
	value= wind(s_head); //reserve value atom

	chop(s_head); 
	if(s_head->next ==NULL){
		*s_head =*value;
		*c_head=chop(*c_head);
		return;
	}
	s_head=chop(s_head);
	*c_head=chop(*c_head); //clear env atom in ctrl
	ptail=wind(s_head);

	*e_cur=find_new_env(s_head);

	
	ptail=wind(s_head);
	
	ptail->next = value; //attach value atom at the end of stack

}


void rule6(struct atom *c_head,struct atom *s_head){
	if(DE)
		printf("rule6:\n");
	struct atom *op, *or1, *or2;
	struct atom *s_tail;

	op=wind(c_head);
	c_head=chop(c_head);

	or1=wind(s_head);
	s_head=chop(s_head);
	or2=wind(s_head);
	s_head=chop(s_head);
	s_tail=wind(s_head);

	if(true_type(or1)==V_INT){//arithmetica biop
		switch(true_type(op)){
			case PLUS  :{
				s_tail->next = cr_int_atom(or1->val.value+or2->val.value);
				break;
			}
			case MINUS :{
				s_tail->next = cr_int_atom(or1->val.value-or2->val.value);
				break;
			}
			case TIMES :{
				s_tail->next = cr_int_atom(or1->val.value*or2->val.value);
				break;
			}
			case DIVIDE:{
				s_tail->next = cr_int_atom(or1->val.value/or2->val.value);
				break;
			}
			case EXPO  :{
				s_tail->next = cr_int_atom(pow(or1->val.value,or2->val.value));
				break;
			}
			case GR :{
				s_tail->next = cr_bool_atom(or1->val.value>or2->val.value?1:0);
				break;
			}	
			case GE :{
				s_tail->next = cr_bool_atom(or1->val.value>=or2->val.value?1:0);
				break;
			} 	
			case LS :{
				s_tail->next = cr_bool_atom(or1->val.value<or2->val.value?1:0);
				break;
			} 	
			case LE :{
				s_tail->next = cr_bool_atom(or1->val.value<=or2->val.value?1:0);
				break;
			} 	
			case EQ :{
				s_tail->next = cr_bool_atom(or1->val.value==or2->val.value?1:0);
				break;
			} 	
			case NE :{
				s_tail->next = cr_bool_atom(or1->val.value!=or2->val.value?1:0);
				break;
			} 	
			default :{
				printf("Error:dealing with arithmetic BINOP\n");
				exit(-1);
			}
		}
	}
	else if(true_type(or1)==V_BOOL){//logical biop
		if(true_type(op)==OR){
			s_tail->next=cr_bool_atom(or1->val.value || or2->val.value);
		}
		else if(true_type(op)==AND){
			s_tail->next=cr_bool_atom(or1->val.value && or2->val.value);
		}
		else{
			printf("Error:dealing with logical BINOP\n");
			exit(-1);
		}
			
	}

}



void rule7(struct atom *c_head,struct atom *s_head){
	if(DE)
		printf("rule7:\n");
	struct atom *pc,*s_value,*s_tail;
	pc = wind(c_head);
	s_value= wind(s_head);

	c_head=chop(c_head);
	s_head=chop(s_head);
	s_tail = wind(s_head);

	if(true_type(pc) ==NOT){
		if(true_type(s_value) == TRUE){
			s_tail->next =cr_bool_atom(0);
		}
		else if (s_value->val.t->type == FALSE){
			s_tail->next =cr_bool_atom(1);
		}
	}
	else{
		s_tail->next = cr_int_atom(-s_value->val.value);
	}
}

void rule8(struct atom *c_head,struct atom *s_head){
	if(DE)
		printf("rule8:\n");

	struct atom *beta,*s_value,*c_tail;
	beta = wind(c_head);
	s_value= wind(s_head);
	c_head=chop(c_head);
	s_head=chop(s_head);
	c_tail = wind(c_head);

	if(s_value->val.value){
		c_tail->next = copy_atom_list(beta->val.b->T->structure);
	}
	else{
		c_tail->next = copy_atom_list(beta->val.b->F->structure);		
	}
}

void rule12(struct atom *c_head,struct atom *s_head){
	if(DE)
		printf("rule12:\n");
	struct atom *s_tail;
	c_head=chop(c_head); //chop gamma
	s_head=chop(s_head); //chop ystar

	s_tail=wind(s_head); //tail == lambda
	s_tail->type=ETA;
}

void rule13(struct atom *c_head,struct atom *s_head){
	if(DE)
		printf("rule13:\n");
	struct atom *c_tail, *s_tail, *lambda;

	c_tail=wind(c_head);
	c_tail->next = make_atom(GAMMA);

	s_tail=wind(s_head);
	lambda = copy_atom(s_tail);
	lambda->type=LAMBDA;

	s_tail->next = lambda;

}

void print_atom(struct atom *c_head, struct atom *s_head){
	if(DE)
		printf("in print_atom\n");
	struct atom *s_tail;
	chop(s_head);
	s_tail= wind(s_head);
	chop(s_head);
	chop(c_head);
	if(DE)
		printf("true type=%d\n",true_type(s_tail));
	switch (true_type(s_tail)){
		case V_INT:{
			printf("%d",s_tail->val.value); break;
		}
		case STR:{ //still no tuple
			char *string;
			int len;
			len = strlen(s_tail->val.t->content);
			string = malloc(len);
			strncpy(string,s_tail->val.t->content,len-1);
			string = string+1;

			while(*string != '\0'){
				if(*string == '\\'){
					if(*(string+1)=='n'){
						printf("\n");
						string+=2;
					}
					else if(*(string+1)=='t'){
						printf("\t");
						string+=2;
					}
				}
				else{
					putchar(*string);
					string++;
				}
			}
			// printf("\n");
			break;
		}
		case DUMMY:printf("dummy");break;
		case TRUE:printf("true");break;
		case FALSE:printf("false");break;
		case XNIL: printf("nil"); break;
		case LAMBDA:printf("[lambda closure: %s: %d]",
			s_tail->val.l->var->val.t->content, 
			s_tail->val.l->index);
	}
}

void print_t(struct atom *s_tail){
	if(DE){
		printf("in print_t\n");
		if(s_tail==NULL){
			printf("in print_t: try to pring out NULL tuple atom\n");
		}
		printf("\ttrue_type atom=%d\n",true_type(s_tail) );
	}

	switch (true_type(s_tail)){
		case V_INT:{
			printf("%d",s_tail->val.value); break;
		}
		case STR:{ 
			char *string;
			int len;

			len = strlen(s_tail->val.t->content);
			string = malloc(len);
			strncpy(string,s_tail->val.t->content,len-1);
			string = string+1;

			while(*string != '\0'){
				if(*string == '\\'){
					if(*(string+1)=='n'){
						printf("\n");
						string+=2;
					}
					else if(*(string+1)=='t'){
						printf("\t");
						string+=2;
					}
				}
				else{
					putchar(*string);
					string++;
				}
			}
			break;
		}
		case DUMMY:printf("dummy");break;
		case TRUE:printf("true");break;
		case FALSE:printf("false");break;
		case XNIL: printf("nil"); break;
		case LAMBDA:printf("[lambda closure: %s: %d]",
			s_tail->val.l->var->val.t->content, 
			s_tail->val.l->index);
	}
}
void print_tau(struct atom *c_head, struct atom *s_head){
	if(DE)
		printf("in print_tau\n");

	chop(c_head); //rid of gamma
	chop(s_head); //rid of print

	struct atom *tau_atom;
	tau_atom = wind(s_head);
	int n = tau_atom->val.tup->index;
	

	struct atom *list_ptr=(struct atom *)malloc(sizeof(struct atom));
	
	list_ptr=tau_atom->val.tup->list;

	if(DE){
		printf("\ttuple number:%d\n",tau_atom->val.tup->index);
		printf("\tatom type:%d\n",tau_atom->type);	
		if(list_ptr==NULL)
			printf("NULL list_ptr\n");	
	}

	printf("(");
	while(n>0){
		if(DE){
			printf("\tn=%d\n",n );
			printf("\tlist_ptr type add=%p\n",list_ptr);
		}

		print_t(list_ptr);
		list_ptr=list_ptr->next;
		if(n>1)
			printf(", ");

		n--;
	}

	printf(")");

	// struct atom *s_ptr;
	// s_ptr= wind(s_head); //now it is tau atom
	// int n = s_ptr->val.tau;//keep track of how many thing to print
	// chop(s_head); //rid of tau atom

	// printf("(");

	// while(n>0){
	// 	s_ptr = wind(s_head); //atom to be priting
	// 	chop(s_head);
	// 	print_t(s_ptr);
	// 	if(n>1)
	// 		printf(", ");

	// 	n--;
	// }

	// printf(")\n");
}


void stern(struct atom *c_head, struct atom *s_head){
	if(DE)
		printf("in stern\n");
	struct atom *str_atom;
	char *string, *ptr;

	chop(c_head);
	chop(s_head);
	str_atom=wind(s_head);
	string = str_atom->val.t->content;

	for(ptr = string+1; *ptr!='\0';ptr++){
		*ptr=*(ptr+1);
	}
}

void stem(struct atom *c_head, struct atom *s_head){
	if(DE)
		printf("in stem\n");
	struct atom *str_atom;
	char *string;

	chop(c_head);
	chop(s_head);
	str_atom=wind(s_head);
	string = str_atom->val.t->content;
	*(string+2)= '\'';
	*(string+3)='\0';

}

void conc(struct atom *c_head, struct atom *s_head){
	if(DE){
		printf("in conc\n");
	}
	struct atom *str_atom_1, *str_atom_2;
	char *str1, *str2;

	chop(c_head); //delete first gamma
	chop(c_head); //second one
	chop(s_head); //delete conc atom

	str_atom_1= wind(s_head);
	chop(s_head);
	str_atom_2=wind(s_head);

	str1 = str_atom_1->val.t->content;
	*(str1 + strlen(str1)-1)='\0'; //get rid of last '\'
	str2 = str_atom_2->val.t->content;

	struct tree *cat_str;
	cat_str=calloc(1,sizeof(struct tree));
	cat_str->type = STR;
	strcat(cat_str->content,str1);
	strcat(cat_str->content,str2+1); //get rid of the first '\' of str2

	str_atom_2->val.t = cat_str;

}

void order(struct atom *c_head, struct atom *s_head){
	chop(c_head);//rid of gamma
	chop(s_head);//rid of order

	struct atom *tau, *int_atom, *s_tail;
	tau = wind(s_head);
	int_atom=cr_int_atom(tau->val.tup->index);

	chop(s_head); //rid of tau atom
	s_tail=wind(s_head);
	s_tail->next = int_atom;

	if(DE){
		printf("int_atom order=%d\n",s_tail->next->val.value );
	}
}

struct atom *second_last_atom(struct atom *head){
	if (head==NULL)
		return NULL;
	else if (head->next == NULL)
		return NULL;
	else if (head->next->next == NULL)
		return head;
	else
		return second_last_atom(head->next);
}

void aug(struct atom *c_head, struct atom *s_head){
	struct atom *tuple, *one, *s_tail;

	chop(c_head); //rid of aug
	tuple = wind(s_head);
	chop(s_head); //rid of tuple
	one = wind(s_head);
	chop(s_head); //rid of the one
	s_tail = wind(s_head); //attach point

	if(DE){
		printf("in aug\n");
		printf("\tbefore aug:\n");	
		printf("\ttup->index=%d\n",tuple->val.tup->index );
		printf("\tatom to be attach=%p\n", one );
	}

	struct atom *list_atch;
	for(list_atch=tuple->val.tup->list;list_atch->next != NULL; list_atch=list_atch->next){
		;
	}

	list_atch->next = one;
	tuple->val.tup->index++;

	s_tail->next = tuple;

	if(DE){
		printf("\tafter aug:\n");
		printf("\tnew tup->index=%d\n",s_tail->next->val.tup->index );
		for(list_atch=s_tail->next->val.tup->list;
			list_atch->next != NULL;
			list_atch=list_atch->next){
			;
		}
		printf("\tattached atom:%p\n",list_atch);
	}

}
//-------------------------cse machine---------------------------
struct atom *cse(struct atom *c_head, struct atom *s_head,struct delta *d_head,struct environment *e_cur,int *index){
	// printf("in cse\n");
	struct atom *c_tail,*s_tail;

	while(c_head!=NULL){
		c_tail=wind(c_head);
		switch(c_tail->type){
			case AUG:{
				aug(c_head,s_head);

				break;
			}
			case TAU:{
				if(DE){
					printf("\tcse, c=tau(%d)\n",c_tail->val.tup->index);
				}

				rule1(c_head,s_head,e_cur);
				if(DE) c_panel(c_head,s_head);
				
				break;
			}
			case GENERAL:{
				if(DE){
					printf("\tcse, c=general\n");
				}

				rule1(c_head,s_head,e_cur);
				if(DE) c_panel(c_head,s_head);

				break;
			}
			case LAMBDA:{
				if(DE){
					printf("\tcse, c=lambda(%d)\n",c_tail->val.l->index);
				}
				rule2(c_head,s_head,e_cur);
				if(DE) c_panel(c_head,s_head);

				break;
			}
			case GAMMA:{
				if(DE){
					printf("\tcse, c=GAMMA\n");
				}
				
				s_tail=wind(s_head);
				if(s_tail->type == LAMBDA){
					if(DE){
						printf("\tcse, s=lambda(%d)\n",s_tail->val.l->index);
					}
					rule4(c_head,s_head,d_head,index,&e_cur);
					if(DE) c_panel(c_head,s_head);

					break;
				}
				else if(s_tail->type == TAU){//tuple selection == rule10
					
					
					chop(c_head); //rid of gamma
					struct atom *tuple, *selector, *s_tail;
					
					tuple = wind(s_head);
					chop(s_head);
					selector = wind(s_head);
					chop(s_head);
					s_tail = wind(s_head);

					if(DE){
						printf("\tcse, s=tau(%d)\n",s_tail->val.tup->index );
						printf("in tuple selection: tuple(%d) || selector =%d\n",s_tail->val.tup->index,selector->val.value);

					}

					int n = tuple->val.tup->index;
					struct atom *one;
					int sel=selector->val.value -1;

					if(selector->type != V_INT){
						printf("Selector type error:%d\n",selector->type );
						exit(-1);
					}
					else if(selector->val.value > n){
						printf("out of tuple(%d), you select %dth member\n",n,selector->val.value );
						exit(-1);
					}
					else if(selector->val.value <1){
						printf("selector error: you choose %dth member?\n",selector->val.value );
						exit(-1);
					}
					else{						
						for(one = tuple->val.tup->list;sel>0;one = one->next){
							if(one == NULL){
								printf("sth wrong, the one is NULL\n");
								exit(-1);
							}
						}
					}

					one->next = NULL; //cut of possible ensuing list
					s_tail->next = one; //attach the one
					if(DE) c_panel(c_head,s_head);


				}
				else if(s_tail->type == GENERAL){
					if(s_tail->val.t->type == YSTAR){
						if(DE){
							printf("\tcse, s=ystar\n");
						}
						rule12(c_head,s_head);
						if(DE) c_panel(c_head,s_head);

						break;
					}
					else if(strcmp(s_tail->val.t->content,"Print")==0){
						if(DE){
							printf("\tcse, s=print\n");
						}
						struct atom *second_last;
						second_last = second_last_atom(s_head);
						if(second_last->type ==TAU){
							print_tau(c_head,s_head);
							if(DE) c_panel(c_head,s_head);

							break;
						}
						else{
							print_atom(c_head,s_head);
							if(DE) c_panel(c_head,s_head);

							break;
						}
					}
					else if(strcmp(s_tail->val.t->content,"Stern")==0){
						if(DE){
							printf("\tcse, s=stern\n");
						}
						stern(c_head,s_head);
						if(DE) c_panel(c_head,s_head);

						break;
					}
					else if(strcmp(s_tail->val.t->content,"Stem")==0){
						if(DE){
							printf("\tcse, s=stem\n");
						}
						stem(c_head,s_head);
						if(DE) c_panel(c_head,s_head);

						break;
					}
					else if(strcmp(s_tail->val.t->content,"Conc")==0){
						if(DE){
							printf("\tcse, s=conc\n");
						}
						conc(c_head,s_head);
						if(DE) c_panel(c_head,s_head);

						break;
					}
					else if(strcmp(s_tail->val.t->content,"Order")==0){
						if(DE){
							printf("\tcse, s=order\n");
						}
						order(c_head,s_head);
						if(DE) c_panel(c_head,s_head);

						break;
					}
				}
				else if(s_tail->type == ETA){	
					if(DE){
						printf("\tcse, s=eta(%d)\n",s_tail->val.l->index);
					}
					rule13(c_head,s_head);
					if(DE) c_panel(c_head,s_head);

					break;
				}
				else{
					printf("Unknown s_tail type %d\n",s_tail->type); exit(-1);
					break;
				}

			}
			case ENVIRO:{
				if(DE){
					printf("\tcse, c=env\n");
				}
				rule5(&c_head,s_head,&e_cur);
				if(DE) c_panel(c_head,s_head);

				break;
			}
			case BINOPA: 
			case BINOPL:{
				if(DE){
					printf("\tcse, c=bi op\n");
				}
				rule6(c_head,s_head);
				if(DE) c_panel(c_head,s_head);

				break;
			}
			case UNOP:{
				if(DE){
					printf("\tcse, c=uni op\n");
				}
				rule7(c_head,s_head);
				if(DE) c_panel(c_head,s_head);

				break;	
			}
			case BETA:{
				if(DE){
					printf("\tcse, c=beta\n");
				}
				rule8(c_head,s_head);
				if(DE) c_panel(c_head,s_head);

				break;
			}
			default: printf("Unknown c_tail type %d\n",c_tail->type); exit(-1);
		}
	}
	printf("\n");
	return s_head;
}


















