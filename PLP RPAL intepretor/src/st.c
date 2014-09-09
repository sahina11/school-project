struct tree *create_tree_node(int type){
	struct tree *x =(struct tree *)malloc(sizeof(struct tree));
	x->child =NULL;
	x->sibling = NULL;
	x->type = type;
	strcpy(x->content, st_node_name[type -51]);

	return x;
}

void copy_tree_node(struct tree *src, struct tree *dest){
	dest->type = src->type;
	strcpy(dest->content, src->content);
	dest->child = src->child;
	dest->sibling = src->sibling;
}


//remember to NULL god damn siblings.......
int transform(struct tree *here){
	struct tree *temp, *temp1, *temp2, *temp3, *temp4, *temp5, *temp6;

	
	switch (here->type){
		case LET:{
		if(here->child->type != EQUAL) break;

		here->type = GAMMA;
		strcpy(here->content,"gamma");

		here->child->type = LAMBDA;
		strcpy(here->child->content, "lambda");

		temp = here->child->child->sibling;
		here->child->child->sibling = here->child->sibling;
		here->child->sibling = temp;

		if (debug) printf("Finished LET\n");
		return 1;	
		}

		case WHERE:{
		if(here->child->sibling->type != EQUAL)	 break;

		/*whenever you copy the child, remeber its sibling may get 
		into your way, many bugs and segmentation fault come from here
		but you can't NULL it's sibling too soon. == damn

		you better copy the addresss which will be NULLize later first*/

		temp = here->child->sibling; // record here
		temp1 = here->child;
		temp1->sibling = NULL; // cut here, if the sequence is wrong, sementation fault may come

		here->child= temp;
		here->child->sibling = temp1;

		here->type = GAMMA;
		strcpy(here->content,"gamma");

		here->child->type = LAMBDA;
		strcpy(here->child->content, "lambda");

		temp = here->child->child->sibling;
		here->child->child->sibling = here->child->sibling;
		here->child->sibling = temp;

		if (debug) printf("Finished WHERE\n");
		return 1;
		}

		case REC:{
			if(here->child->type != EQUAL) break;

			temp = here->child->child->sibling;
			temp1 = here->child->child;
			temp1->sibling = NULL;
			temp2=malloc(sizeof(struct tree));
			copy_tree_node(temp1,temp2);

			here->type = EQUAL;
			strcpy(here->content,"=");

			here->child = temp1;
			here->child->sibling = create_tree_node(GAMMA);
			here->child->sibling->child = create_tree_node(YSTAR);
			here->child->sibling->child->sibling = create_tree_node(LAMBDA);
			here->child->sibling->child->sibling->child = temp2;
			here->child->sibling->child->sibling->child->sibling =temp;


			if (debug) printf("Finished REC\n");
			return 1;
		}

		case ATX:{
			temp = here->child->sibling->sibling;
			temp1= here->child->sibling;
			temp1->sibling=NULL;
			temp2 = here->child;
			temp2->sibling = NULL;

			here->type = GAMMA;
			strcpy(here->content,"gamma");

			here->child = create_tree_node(GAMMA);
			here->child->sibling = temp;

			here->child->child=temp1;
			here->child->child->sibling = temp2;

			return 1;
		}

		case WITHIN:{
			if(here->child->type != EQUAL || here->child->sibling->type !=EQUAL)
				break;
			temp = here->child->sibling->child->sibling; //E2
			temp1 = here->child->child->sibling; //E1
			temp2 = here->child->sibling->child; //X2
			temp2->sibling =NULL;
			temp3 = here->child->child; //X1
			temp3->sibling = NULL;

			here->type = EQUAL;
			strcpy(here->content,"=");

			here->child = temp2;
			here->child->sibling = create_tree_node(GAMMA);
			here->child->sibling->child = create_tree_node(LAMBDA);
			here->child->sibling->child->sibling = temp1;
			here->child->sibling->child->child = temp3;
			here->child->sibling->child->child->sibling = temp;

			if (debug) printf("Finished WITHING\n");
			return 1;
		}

		case AND2:{

			// temp = here->child;
			// while(temp !=NULL){
			// 	if(temp->type != EQUAL)
			// 		break;
			// 	else
			// 		temp = temp->sibling;
			// }

			temp1 = create_tree_node(COMMA);
			temp3=temp1;
			temp2 = create_tree_node(TAU);
			temp4= temp2;
			temp = here->child;

			if(temp != NULL){
				temp2->child = temp->child->sibling;
				temp1->child = temp->child;
				temp1->child->sibling = NULL;

				temp = temp->sibling;
				temp1 = temp1->child;
				temp2 = temp2->child; 
			}

			while(temp != NULL){
				temp2->sibling = temp->child->sibling;
				temp1->sibling = temp->child;
				temp1->sibling->sibling =NULL;

				temp1 = temp1->sibling;
				temp2 = temp2->sibling;
				temp= temp->sibling;
			}

			here->type = EQUAL;
			strcpy(here->content,"=");

			here->child = temp3;
			here->child->sibling = temp4;

			here->child->sibling->sibling = NULL; //cut off other siblings

			if (debug) printf("Finished AND\n");
		
			return 1;
		}

		case FCN_FORM:{
			temp = here->child->sibling; // point to first var
			while(temp->sibling != NULL){ //after this, temp will be fn's body
				temp = temp->sibling;
			}

			temp1 = here->child->sibling;

			temp2 = create_tree_node(LAMBDA);
			temp2->child = temp1;

			temp1= temp1->sibling;
			temp3= temp2->child; //

			while(temp1 != temp){
				temp3->sibling= create_tree_node(LAMBDA);
				temp3->sibling->child = temp1;

				temp3=temp3->sibling->child;
				temp1=temp1->sibling;
			}

			temp3->sibling=temp;

			here->type = EQUAL;
			strcpy(here->content,"=");

			here->child->sibling = temp2;
			here->child->sibling->sibling=NULL;

			if (debug) printf("Finished FCN_FORM\n");
			return 1;
		}
	}// end of switch 

	return 0;
}

struct tree *st(struct tree *ast){
	struct tree *here, *temp;
	here= ast;
	temp=here->child;
	
	if(temp != NULL){
		while(temp!= NULL){
			st(temp);
			temp=temp->sibling;
		}	

		transform(here);	
	}

	return here;
}












