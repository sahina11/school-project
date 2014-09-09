
//like god create the first man XD, this is the basic element
struct atom *make_atom(int type){
	struct atom *adam;
	adam = (struct atom *)malloc(sizeof(struct atom));
	adam->type = type;
	adam->next =NULL;

	return(adam);
}

struct leta *make_leta(int index, struct tree *var){
	struct atom *it = make_atom(99);
	struct leta *l = (struct leta *)malloc(sizeof(struct leta));
	it->val.t=var;
	l->index = index;
	l->var = it;
	l->c =NULL;
	return l;
}

struct beta *make_beta(){
	struct beta *b=(struct beta *)malloc(sizeof(struct beta));
	b->T = NULL;
	b->F = NULL;
	return b;
}

struct delta *make_delta(){ //initialize d0
	struct delta *d =(struct delta *) malloc(sizeof(struct delta));
	d->index = 0;
	d->structure = NULL;
	d->next = NULL;
	return d;
}

struct atom *to_last_atom(struct atom *list){
	if (list ==NULL) return NULL;

	while(list->next !=NULL) list = list->next;
	return list;
}


struct delta *to_last_delta(struct delta *list){
	if (list ==NULL) return NULL;

	while(list->next !=NULL) list = list->next;
	return list;
}

//the function takes in st tree, and a mem location of delta
//and it will recursively traverse st tree, build delta contol structure
struct atom *make_ctrl_structure(struct tree *st, int *i, struct delta *d){

	struct atom *pa_temp;
	struct atom *p_atom;
	int counter =0; //used for counting tau

	if (st ==NULL) return NULL;

	p_atom=make_atom(GENERAL);

	switch(st->type){
	case ID: 
	case INT:
	case STR:
	case TRUE:
	case FALSE: 
	case XNIL:
	case YSTAR:
	case DUMMY : { 
		p_atom->val.t = st;
		p_atom->type=GENERAL;
		p_atom->next =NULL;
		return p_atom;
	}
	case LAMBDA : { /* Lambda function */ 
		(*i)++;
	
		p_atom->val.l = make_leta(*i,st->child);	
		
		p_atom->type=LAMBDA;
		d = to_last_delta(d);
		d->next = make_delta();
		d=d->next;
		d->index=*i;
		d->structure=make_ctrl_structure(st->child->sibling,i,d);
		return p_atom;

	}
	case NEG:
	case NOT : {/* Uniary operators */
		p_atom->val.t=st;
		p_atom->type=UNOP;
		p_atom->next=make_ctrl_structure(st->child,i,d);
		return p_atom;
	}
	case OR:
	case AND: {/* Logical binops */
		p_atom->val.t =st;
		p_atom->type = BINOPL;
		p_atom->next = make_ctrl_structure(st->child,i,d);

		pa_temp = p_atom;

		p_atom=to_last_atom(p_atom);
		p_atom->next = make_ctrl_structure(st->child->sibling,i,d);

		return pa_temp;
	}
	case GR:
	case GE:
	case LS:
	case LE:
	case EQ:
	case NE: 
	case PLUS:
	case MINUS:
	case TIMES:
	case DIVIDE:
	case EXPO : {/* Arithmatic binops */
		p_atom->val.t = st;
		p_atom->type = BINOPA;
		p_atom->next = make_ctrl_structure(st->child,i,d);
		pa_temp=p_atom;
		p_atom= to_last_atom(p_atom);
		p_atom->next=make_ctrl_structure(st->child->sibling,i,d);

		return pa_temp;
	}
	case AUG : { /*a binop. . .*/
		p_atom->val.t = st;
		p_atom->type=AUG;
		p_atom->next=make_ctrl_structure(st->child,i,d);
		pa_temp=p_atom;
		p_atom=to_last_atom(p_atom);
		p_atom->next=make_ctrl_structure(st->child->sibling,i,d);
		return(pa_temp);
	}
	case COND : { /* The conditional */
		p_atom->type = BETA;
		p_atom->val.b=make_beta(); /* The beta will contain the deltas */
		
		pa_temp=p_atom;
		
		(*i)++; 
		d=to_last_delta(d);
		d->next=make_delta();
		d=d->next;
		d->index=*i;
		d->structure=make_ctrl_structure(st->child->sibling,i,d);
		p_atom->val.b->T = d; /* True result delta */
		
		(*i)++;
		d=to_last_delta(d);
		d->next=make_delta();
		d=d->next;
		d->index=*i;
		d->structure=make_ctrl_structure(st->child->sibling->sibling,i,d);
		p_atom->val.b->F = d; /* False result delta */
		
		p_atom->next=make_ctrl_structure(st->child,i,d);
		
		return(pa_temp);
	}
	case GAMMA : { /* The multifunctional gamma */
		p_atom->val.t = st;
		p_atom->type=GAMMA;
		p_atom->next=make_ctrl_structure(st->child,i,d);
		pa_temp=p_atom;
		p_atom=to_last_atom(p_atom);
		p_atom->next=make_ctrl_structure(st->child->sibling,i,d);
		return(pa_temp);
	}
	case TAU :  { /* Create a tau atom */
		p_atom->type=TAU;
		pa_temp=p_atom;
		st = st->child;
		while (st!=NULL) {
			counter++; /* Counts the number of items in the tau */
			p_atom->next=make_ctrl_structure(st,i,d);
			p_atom=to_last_atom(p_atom);
			st=st->sibling;
		}

		//new ver
		struct tuple *tup;
		tup=(struct tuple *)malloc(sizeof(struct tuple));
		tup->index = counter;
		pa_temp->val.tup = tup;
		

		return(pa_temp);
	}
	default: printf("Don't know what do with %s\n",st->content);
		 exit(-1);
	}

}

