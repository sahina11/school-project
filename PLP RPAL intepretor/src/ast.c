struct tree *pop(){ //return top tree ptr on the stack, set the next tree on the stack to be on top
	if(top == NULL){
		printf("Stack error, nothing to pop!\n");
	}
	struct tree *obj;
	obj = top->member;
	top = top->next;

	return(obj);
}

void push(struct tree *obj){
	struct t_stack *newtop = (struct t_stack *) malloc (sizeof(struct t_stack));

	newtop->member = obj;
	newtop->next = top;

	top = newtop;
}


void bt(int type,char *content,int n){ //build tree

	//printf("\t\tbuilding tree \" %s \" with %d children\n",content,n);//test

	struct tree *pre,*new,*temp;
	pre = NULL;

	new = calloc(1,sizeof(struct tree)); //only ptr need to contain content need memory block


	while(n>0){  //if the tree has children
		if(pre == NULL) //no pre, first iteration
			pre = pop();
		else{
			temp = pop();
			temp->sibling = pre;
			pre = temp;
		}
		n--;
	}

	new->type = type;
	strcpy(new->content,content);

	new->child = pre; //graft child, if not child, NULL will be grafted
	new->sibling = NULL; //no siblings for root node



	push(new);
}

int nt(void){ //return current token type no.
	return(t->type);
}

void read(int type){
	if(type != t->type){
		printf("Error, expected token type =%d, but found token type =%d\n",type,t->type);
		exit(-1);
	}

	//printf("\t\tread(\"  %s  \")\n",t->content);

	scan();
}

int Vl(void){ //Returns 1 if it was able to create a variable node
	//printf("in Vl().....\n");
	int n=0;
	if(nt() == ID){
		bt(ID,t->content,0);
		read(ID);
		while(nt() == COMMA){
			read(COMMA);
			bt(ID,t->content,0);
			read(ID);
			n++;
		}
		if(n>0) bt(COMMA,",",n+1);
		return(1);

	}
	return(0);
}

int Vb(void){ //Returns 1 if it was able to create a variable node
	//printf("in Vb().....\n");
	if(nt()==ID){
		bt(ID,t->content,0);
		read(ID);
		return(1);
	}
	else if(nt()==L_PAREN){
		read(L_PAREN);
		if(nt()==ID){
			Vl();
			read(R_PAREN);
			return(1);
		}
		else if(nt()==R_PAREN){
			bt(EMPTY_PAREN,"()",0);
			read(R_PAREN);
			return(1);
		}
	}

	return(0);
}

void D(void);
void E(void);

void Db(void){
	//printf("in Db().....\n");
	int n=0;
	if(nt() == ID){
		bt(ID,t->content,0);
		read(ID);

		if(nt()==COMMA){
			while(nt()==COMMA){
				read(COMMA);
				bt(ID,t->content,0);
				read(ID);
				n++;
			}
			if(n>0) bt(COMMA,",",n+1);
		}

		if(nt()==EQUAL){
			read(EQUAL);
			E();
			bt(EQUAL,"=",2);
		}
		else if(nt() == ID || nt() == L_PAREN){
			n=0;
			do {Vb();
				n++;
			}while(nt() == ID || nt() == L_PAREN);

			read(EQUAL);
			E();
			bt(FCN_FORM,"function_form",n+2);
		}
	}
	else if(nt() == L_PAREN){
		read(L_PAREN);
		D();
		read(R_PAREN);
	}
}

void Dr(void){
	//printf("in Dr().....\n");

	if(nt() == REC){
		read(REC);
		Db();
		bt(REC,"rec",1);
	}

	Db();
}

void Da(void){
	//printf("in Da().....\n");

	Dr();
	int n=0;

	while(nt() == AND2){
		read(AND2);
		Dr();
		n++;
	}

	if(n>0) bt(AND2,"and",n+1);
}

void D(void){
	//printf("in D().....\n");

	Da();
	if(nt() == WITHIN){
		read(WITHIN);
		D();
		bt(WITHIN,"within",2);
	}
}

int Rn(void){
	//printf("in Rn().....\n");

	switch(nt()){
	case ID:
	case INT:
	case STR:
	case TRUE:
	case FALSE:
	case XNIL:{
		bt(t->type,t->content,0);
		read(nt());
		return(1);
	}
	case L_PAREN:{
		read(L_PAREN);
		E();
		read(R_PAREN);
		return(1);
	}
	}//end of switch

	if(nt() == DUMMY){
		bt(t->type,t->content,0);
		read(DUMMY);
		return(1);
	}

	return(0);
}

void R(void){
	//printf("in R().....\n");

		Rn();
		while(Rn()==1){
			bt(GAMMA,"gamma",2);
		}

}

void Ap(void){
	//printf("in Ap().....\n");

	R();

	while(nt() == ATX){
		read(ATX);
		bt(ID,t->content,0);
		read(ID);
		R();
		bt(ATX,"@",3);
	}
}

void Af(void){
	//printf("in Af().....\n");

	Ap();
	if(nt() == EXPO){
		read(EXPO);
		Af();
		bt(EXPO,"**",2);
	}
}

void At(void){
	//printf("in At().....\n");

	Af();
	int temp;
	temp = nt();
	while((temp == TIMES) || (temp == DIVIDE)){
		read(temp);
		Af();
		bt(temp,(temp == TIMES?"*":"/"),2);
		temp = nt();
	}
}

void A(void){
	//printf("in A().....\n");

	if(nt()==MINUS){
		read(MINUS);
		At();
		bt(NEG,"neg",1);
	}
	else if(nt()==PLUS){
		read(PLUS);
		At();
	}
	else
		At();

	int temp;
	temp = nt();

	while((temp == PLUS) || (temp == MINUS)){
		read(temp);
		At();
		bt(temp,(temp==PLUS?"+":"-"),2);
		temp = nt();
	}
}


void Bp(void){
	//printf("in Bp().....\n");

	A();

	switch (nt()){
	case GR:{
		read(GR);
		A();
		bt(GR,"gr",2);
		break;
	}
	case GE:{
		read(GE);
		A();
		bt(GE,"ge",2);
		break;
	}
	case LS:{
		read(LS);
		A();
		bt(LS,"ls",2);
		break;
	}
	case LE:{
		read(LE);
		A();
		bt(LE,"le",2);
		break;
	}
	case EQ:{
		read(EQ);
		A();
		bt(EQ,"eq",2);
		break;
	}
	case NE:{
		read(NE);
		A();
		bt(NE,"ne",2);
		break;
	}
	}//end of switch
}

void Bs(void){
	//printf("in Bs().....\n");

	if(nt() == NOT){
		read(NOT);
		Bp();
		bt(NOT,"not",1);
	}

	Bp();
}

void Bt(void){
	//printf("in Bt().....\n");

	Bs();

	while(nt()==AND){
		read(AND);
		Bs();
		bt(AND,"&",2);
	}
}


void B(void){
	//printf("in B().....\n");

	Bt();

	while(nt()==OR){
		read(OR);
		Bt();
		bt(OR,"or",2);
	}
}


void Tc(void){
	//printf("in Tc().....\n");

	B();

	if(nt()==COND){
		read(COND);
		Tc();
		read(BAR);
		Tc();
		bt(COND,"->",3);
	}
}

void Ta(void){
	//printf("in Ta().....\n");

	Tc();

	while(nt()==AUG){
		read(AUG);
		Tc();
		bt(AUG,"aug",2);
	}
}

void T(void){
	//printf("in T().....\n");

	int n=0;
	Ta();

	while(nt()==COMMA){
		read(COMMA);
		Ta();
		n++;
	}

	if(n>0) bt(TAU,"tau",n+1);

}

void Ew(void){
	//printf("in Ew().....\n");

	T();
	if(nt()==WHERE){
		read(WHERE);
		Dr();
		bt(WHERE,"where",2);
	}
}

void E(void){
	//printf("in E().....\n");

	if (nt()==LET){
		read(LET);
		D();
		read(IN);
		E();
		bt(LET,"let",2);
	}
	else if(nt()== FN){
		read(FN);
		int n=0;

		do{	Vb();
			n++;
		}while(nt() == L_PAREN || nt() == ID);

		read(PERIOD);
		E();

		bt(LAMBDA,"lambda",n+1);
	}
	else
		Ew();
}

struct tree *ast(){
	scan();

	E();

	return(top->member);
}

//support fn

int stackno(struct t_stack *x){
	int i=0;
	struct t_stack *temp;
	temp = x;
	if(temp->member !=NULL) i++;

	while(temp->next !=NULL){
		i++;
		temp = temp->next;

	}

	return(i);
}

void dots(int n){
	while(n>0){
		printf(".");
		n--;
	}
}

void print_tree(struct tree *ptree){
	if(ptree->type ==ID)
		printf("<ID:%s> \n",ptree->content);
	else if(ptree->type ==INT)
		printf("<INT:%s> \n",ptree->content);
	else if(ptree->type == STR)
		printf("<STR:%s> \n",ptree->content);
	else if(ptree->type == TRUE)
		printf("<true> \n");
	else if(ptree->type == FALSE)
		printf("<false> \n");
	else if(ptree->type == XNIL){
		printf("<nil> \n");
	}
	else if(ptree->type == DUMMY){
		printf("<dummy> \n");
	}
	else if(ptree->type == YSTAR){
		printf("<Y*> \n");
	}
	else
		printf("%s \n",ptree->content);

}
void traverse_tree(struct tree *ptree, int level){
	dots(level);
	print_tree(ptree);

	if(ptree->child != NULL) traverse_tree(ptree->child,level+1);
	if(ptree->sibling != NULL) traverse_tree(ptree->sibling,level);

}




