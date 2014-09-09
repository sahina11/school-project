
void fill_token(char *start, char *here, int type,struct token *t){
	memset(t->content,'\0',MAX_CONTENT_LEN); //clean content
	strncpy(t->content,start,here-start);
	t->type = type;
}

void fill_line(FILE *fp){
	memset(line,'\0',MAX_LINE_LEN); //null line

	if((fgets(line,MAX_LINE_LEN,fp)) != NULL){
		start = line; //reset start and here ptr to the beginning of line
		here = line;
		if(print_list) printf("%s",line);
	}
	else{
		fill_token(start,here,0,t);//create a ending token to finish
	}
}

int isop(char test){
	static char operators[]="\"+-*<>&.@/:=~|$!#%^_[]{}`?";

	return( strchr(operators,test) != NULL);
}

int ispunc(char test){
	static char punc[]="();,";
	return( strchr(punc,test) != NULL);
}

void op_token(struct token *t){
	if(strcmp(".",t->content) == 0) t->type = PERIOD;
	else if(strcmp("&",t->content) == 0) t->type = AND;
	else if(strcmp("*",t->content) == 0) t->type = TIMES;
	else if(strcmp("+",t->content) == 0) t->type = PLUS;
	else if(strcmp("-",t->content) == 0) t->type = MINUS;
	else if(strcmp("/",t->content) == 0) t->type = DIVIDE;
	else if(strcmp("@",t->content) == 0) t->type = ATX;
	else if(strcmp("|",t->content) == 0) t->type = BAR;
	else if(strcmp("=",t->content) == 0) t->type = EQUAL;
	else if(strcmp("->",t->content) == 0) t->type = COND;
	else if(strcmp("**",t->content) == 0) t->type = EXPO;
	else if(strcmp(">=",t->content) == 0) t->type = GE;
	else if(strcmp("<=",t->content) == 0) t->type = LE;
}

void kw_token(struct token *t){
	if (strcmp("in",t->content) == 0) t->type = IN;
	else if(strcmp("gr",t->content) == 0) t->type = GR;
	else if(strcmp("ge",t->content) == 0) t->type = GE;
	else if(strcmp("ls",t->content) == 0) t->type = LS;
	else if(strcmp("le",t->content) == 0) t->type = LE;
	else if(strcmp("eq",t->content) == 0) t->type = EQ;
	else if(strcmp("ne",t->content) == 0) t->type = NE;
	else if(strcmp("or",t->content) == 0) t->type = OR;
	else if(strcmp("fn",t->content) == 0) t->type = FN;

	else if(strcmp("let",t->content) == 0) t->type = LET;
	else if(strcmp("not",t->content) == 0) t->type = NOT;
	else if(strcmp("neg",t->content) == 0) t->type = NEG;
	else if(strcmp("nil",t->content) == 0) t->type = XNIL;
	else if(strcmp("and",t->content) == 0) t->type = AND2;
	else if(strcmp("rec",t->content) == 0) t->type = REC;
	else if(strcmp("aug",t->content) == 0) t->type = AUG;

	else if(strcmp("true",t->content) == 0) t->type = TRUE;

	else if(strcmp("where",t->content) == 0) t->type = WHERE;
	else if(strcmp("false",t->content) == 0) t->type = FALSE;
	else if(strcmp("dummy",t->content) == 0) t->type = DUMMY;

	else if(strcmp("within",t->content) == 0) t->type = WITHIN;
}

void pre_scan(FILE *fp){
	/**
	 * start ==10 -> point to NL
	 * start == 0-> two cases:
	 * 				1.start pointer just initialized
	 * 				2.it is already EOF, start point to NULL
	 * but both two cases has to fill line
	 */
	if(*start == 10 ||*start == 0){
		fill_line(fp);
	}
}

void scan(void){
	int state = S_S;

	pre_scan(fp);
	while(state != END_S){
		switch (state){
		case S_S:{
			if(isalpha(*start)){ //ID
				state = ID_S;
				here++;
			}
			else if(isdigit(*start)){ //INT
				state = INT_S;
				here++;
			}
			else if(*start == 39){ //STR
				state = STR_S;
				here++;
			}
			else if(ispunc(*start)){ //PUNC
				state = PUNC_S;
				here++;
			}
			else if(isspace(*start)){ //SPACE
				state = SPACE_S;
				here++;
			}
			else if(isop(*start)){
				state = OP_S;
				here++;
			}
			break;
		}
		case ID_S:{
			if(isalnum(*here) || *here == 95) //'_'
				here++;
			else
				state = ID_F;

			break;
		}//end of ID_S

		case ID_F:{
			fill_token(start,here,ID,t);
			start = here;
			kw_token(t);
			state = END_S;
			break;
		}//end of ID_F

		case INT_S:{
			if(isdigit(*here))
				here++;
			else
				state = INT_F;
			break;
		}

		case INT_F:{
			fill_token(start,here,INT,t);
			start=here;
			state = END_S;
			break;
		}

		case STR_S:{
			if(*here ==92){ // '\'
				switch (*(here +1)){
				case 't':
				case 'n':
				case 92: // '\'
				case 39:{ // ''''
					here+=2;
					break;
				}
				}
			}

			if(ispunc(*here)) here++;
			if(isspace(*here)) here++;
			if(isalnum(*here) || isop(*here)) here++;

			if(*here == 39){
				here++;
				state = STR_F;
			}
			break;
		}

		case STR_F:{
			fill_token(start,here,STR,t);
			start=here;
			state = END_S;
			break;
		}

		case PUNC_S:{
			switch (*start){
			case '(':{
				fill_token(start,here,L_PAREN,t);
				start = here; break;
			}
			case ')':{
				fill_token(start,here,R_PAREN,t);
				start = here; break;
			}
			case ';':{
				fill_token(start,here,SEMI,t);
				start = here; break;
			}
			case ',':{
				fill_token(start,here,COMMA,t);
				start = here; break;
			}
			}//end of switch
			state = END_S;
			break;
		}

		case SPACE_S:{
			while (isspace(*here))
				here++;
			start = here;
			pre_scan(fp);
			state = S_S;
			break;
		}

		case OP_S:{
			//printf("%d\t%d\n",*start,*here);
			//printf("%s\n",start);

			if(*start == 42 && *here == 42){ // **
				here++;
				fill_token(start,here,EXPO,t);
				start = here;
				op_token(t);
				state = END_S;
			}
			else if(*start == 45 && *here == 62){ // ->
				here++;
				fill_token(start,here,COND,t);
				start = here;
				op_token(t);
				state = END_S;
			}
			else if(*start == 62 && *here == 61){ // >=
				here++;
				fill_token(start,here,GE,t);
				start = here;
				op_token(t);
				state = END_S;
			}
			else if(*start == 60 && *here == 61){ //<=
				here++;
				fill_token(start,here,LE,t);
				start = here;
				op_token(t);
				state = END_S;
			}
			else if(*start == 47 && *here == 47){ //comment
				here++;
				state = COMMENT_S;
			}
			else{
				fill_token(start,here,OP,t);
				start=here;
				op_token(t);
				state =END_S;
			}
			break;
		}
		case COMMENT_S:{
			while (*here != 10)
				here++;

			here++;
			start = here;
			pre_scan(fp);
			state = S_S;
			break;
		}
		}//end of switch
	}//end of while
}//end of scan()

