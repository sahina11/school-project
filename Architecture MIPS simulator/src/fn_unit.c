#include "MIPSsim.h"

//remember,every fn unit reads from clone queues


int ck_hazard (int array_no){ 
	/*return 1 =pass the check, 0 for fail
	usually, array_no will be cpre_issue->q[] subscript no.
	why I wanna use array_no is for the fn to be used by bracch ins in IF*/
	
	int i;
	switch (array_no){
		case 0:{
			for(i=0;i<32;i++){ 
				if(c0[i] == READ && working[i]==WRITE){ //RAW
					return 0;
				}
				if(c0[i]== WRITE && working[i]==WRITE){ //WAW
					return 0;
				}
				//no WAR for the first test
			}
			break;
		}
		case 1:{
			for(i=0;i<32;i++){ 
				if(c1[i] == READ && (c0[i] == WRITE || working[i]==WRITE)){ //RAW
					return 0;
				}
				if(c1[i] == WRITE && (c0[i] == WRITE || working[i]==WRITE)){//WAW
					return 0;
				}
				if(c1[i] == WRITE && c0[i] == READ){//WAR
					return 0;
				}
			}
			break;
		}
		case 2:{
			for(i=0;i<32;i++){ 
				if(c2[i] == READ  && (c1[i] == WRITE || c0[i] == WRITE || working[i]==WRITE)){ //RAW
					return 0;
				}
				if(c2[i] == WRITE && (c1[i] == WRITE || c0[i] == WRITE || working[i]==WRITE)){//WAW
					return 0;
				}
				if(c2[i] == WRITE && (c1[i] == READ || c0[i] == READ)){//WAR
					return 0;
				}
			}
			break;
		}
		case 3:{
			for(i=0;i<32;i++){ 
				if(c3[i] == READ  && (c2[i] == WRITE || c1[i] == WRITE || c0[i] == WRITE || working[i]==WRITE)){ //RAW
					return 0;
				}
				if(c3[i] == WRITE && (c2[i] == WRITE || c1[i] == WRITE || c0[i] == WRITE || working[i]==WRITE)){//WAW
					return 0;
				}
				if(c3[i] == WRITE && (c2[i] == READ || c1[i] == READ || c0[i] == READ)){//WAR
					return 0;
				}
			}
			break;			
		}
	}//end of swtich
	return 1; //pass the check
}


int ck_ls_case (int array_no){ //1 for pass = good to go, 0 for fail
	//here the array_no is the same as q[] subscript
	int i;
	struct decoded_ins *ins;
	int opcode;

	if(cpre_issue->q[array_no]){
	ins= cpre_issue->q[array_no];//for currently checking ins
	opcode= get_opcode(ins->binary);		
	}
	else{
		printf("Error: in fn \"ck_ls_case()\", deref NULL ptr\n");
		return 0;
	}
	
	if(opcode ==22 || opcode ==23){  //if the ins is LW or SW
		for(i=0;i<array_no;i++){ //check previos ins if any SW exist
			if(get_opcode(cpre_issue->q[i]->binary) == 22)
				return 0;
		}

		return 1;
	}
	else{
		return 1;
	}
}

void update_working_array(int array_no){
	int i;
	if(array_no ==0){
		for(i=0;i<32;i++){
			if(c0[i]==WRITE){
				working[i]=WRITE;
			}
		}
	}
	else if(array_no ==1){
		for(i=0;i<32;i++){
			if(c1[i]==WRITE){
				working[i]=WRITE;
			}
		}
	}
	else if(array_no ==2){
		for(i=0;i<32;i++){
			if(c2[i]==WRITE){
				working[i]=WRITE;
			}
		}
	}
	else if(array_no ==3){
		for(i=0;i<32;i++){
			if(c3[i]==WRITE){
				working[i]=WRITE;
			}
		}
	}
}


int issue(void){
	//printf("in issue\n");
	struct decoded_ins *cand_ls=NULL;
	struct decoded_ins *cand_else=NULL;
	int issue_count=0;
	int i;
	clean_array();
	/*in the loop is ckecking and making candidates period*/

	for(i=0;i<4;i++){
		//printf("round: %d\n",i );
		struct decoded_ins *cking_ins;
		cking_ins = cpre_issue->q[i];
		
		//check breaking conditon for exiting examining for loop
		if(cking_ins ==NULL) //end of cpre_issue queue
			break;
		if(cand_ls!= NULL && cand_else!= NULL)//full cand
			break;


		ins2array(cking_ins,i);

		if(ck_hazard(i) && ck_ls_case(i)){ //both passed
			//printf("all pass\n");
			if(get_opcode(cking_ins->binary)== 22 || get_opcode(cking_ins->binary)==23){
				if(cand_ls == NULL){
					cand_ls=cking_ins;
					update_working_array(i);
				}
				else
					continue;
			}
			else{
				if(cand_else == NULL){
					cand_else=cking_ins;
					update_working_array(i);
				}
				else
					continue;
			}
		}
	}//end of for loop

	if(cand_ls!=NULL && cpre_alu1->count <2){
		enq_prealu(cand_ls,pre_alu1);
		deq_preissue(cand_ls);
		issue_count++;
	}
	if(cand_else!=NULL && cpre_alu2->count <2){
		enq_prealu(cand_else,pre_alu2 );
		deq_preissue(cand_else);
		issue_count++;
	}
	return issue_count;
}

int execute_branch(struct decoded_ins *branch){ //return 1 for success, 0 failure
	clean_array();
	int i;
	for(i=0;i<cpre_issue->count;i++){
		ins2array(cpre_issue->q[i],i);
	}
	ins2array(branch,cpre_issue->count);

	if(ck_hazard(cpre_issue->count)){//if passed, execute it!
		int opcode;
		opcode = get_opcode(branch->binary);
		switch (opcode){
			case 18:{ //BEQ
				if(reg[branch->rt] == reg[branch->rs]){
					pc = pc +4 +branch->imd * 4;
					executed_ins = branch;
					waiting_ins =NULL;//afte exec, this should be NULL again
					return 1;;
				}
				else{
					pc+=4;
					executed_ins = branch;
					waiting_ins =NULL;
					return 1;;
				}
			}
			case 19:{ //BLTZ
				if(reg[branch->rs] < 0){
					pc = pc +4 +branch->imd * 4;
					executed_ins = branch;
					waiting_ins =NULL;
					return 1;;
				}
				else{
					pc+=4;
					executed_ins = branch;
					waiting_ins =NULL;
					return 1;;
				}
			}
			case 20:{ //BGTZ
				if(reg[branch->rs] > 0){
					pc = pc +4 +branch->imd * 4;
					executed_ins = branch;
					waiting_ins =NULL;
					return 1;;
				}
				else{
					pc+=4;
					executed_ins = branch;
					waiting_ins =NULL;
					return 1;;
				}
			}					
		}//end of switch
	}
	return 0;
}

int alu1(); //pre declaration for break to use
int alu2();
int memory();
int wb();

int fetch_decode(int bk_cycle){
	//printf("in fetch_decode\n");
	executed_ins = NULL;
	int free_slot;
	int fetch_count=0;
	free_slot= 4- cpre_issue->count;
	int i;
	struct decoded_ins *new_ins;

	for(i=0;i<2;i++){
		if(free_slot ==0){
			return fetch_count;
		}

		if(waiting_ins !=NULL){ //pending branch ins
			execute_branch(waiting_ins);
			return fetch_count;
		}

		new_ins = cr_decoded_ins(mem[adr2sub(pc)]);

		int opcode;
		opcode = get_opcode(new_ins->binary);
		//handle special instructions

		switch (opcode){
			case 27:{ //NOP
				pc +=4;
				executed_ins = new_ins;
				fetch_count++;
				return fetch_count;
			}
			case 16:{ //JUMP
				pc = new_ins->imd26 * 4;
				executed_ins = new_ins;
				fetch_count++;
				return fetch_count;
			}
			case 17:{ //JR
				pc = reg[new_ins->rs];
				executed_ins = new_ins;
				fetch_count++;
				return fetch_count;
			}
			case 21:{ //BREAK
				executed_ins = new_ins;
				issue();
				alu1();
				alu2();
				memory();
				wb();
				cout_state(bk_cycle,output_file); ///this cycle I decided manually
				exit(0);
			}
			case 18: case 19: case 20:{
				if(execute_branch(new_ins)){ //if succeed, return
					fetch_count++;
					return fetch_count;
				}else{ //if failed, put in waiting_ins, return
					waiting_ins= new_ins;
					fetch_count++;
					return fetch_count;
				}
			}
		}//end of special case switch

		enq_preissue(new_ins);
		fetch_count++;
		free_slot--;
		pc+=4;
	}//end of for loop
	return fetch_count;
}


int alu1(void){ //return execute binary, 0 for nothing
	//printf("in alu1\n");
	struct decoded_ins *ins;
	ins = cpre_alu1->q[0];
	if(ins == NULL){//empty queue, nullize output
		pre_mem=NULL;
		return 0;
	}
	else{
		ins->exec_address = reg[ins->rs] + ins->imd;
		deq_prealu(pre_alu1);
		pre_mem = ins;
		return ins->exec_address;
	}
}

int memory(void){ //0 for no execution or failure
	//printf("in memory\n");
	struct decoded_ins *ins;
	ins = cpre_mem;
	if(ins == NULL){
		post_mem=NULL;
		return 0;
	}
	else{
		int opcode;
		opcode =get_opcode(ins->binary);
		if(opcode == 22){ //SW
			mem[adr2sub(ins->exec_address)] = reg[ins->rt];
			return ins->binary;
		}
		else if(opcode == 23){ //LW
			ins->exec_value = mem[adr2sub(ins->exec_address)];
			post_mem = ins;
			return ins->binary;
		}
	}
	return 0;
}

int execute_alu2(struct decoded_ins *ins){//return value or -1 for failure
	int value=0;
	int opcode;
	opcode = get_opcode(ins->binary);
	switch (opcode){
		case 24: //SLL
			value = (unsigned int)reg[ins->rt] << ins->shamt;
			break;
		case 25: //SRL
			value = (unsigned int)reg[ins->rt] >> ins->shamt;
			break;
		case 26: //SRA
			value = reg[ins->rt] >> ins->shamt;
			break;
		case 48: //ADD
			value =reg[ins->rs]+ reg[ins->rt];
			break;
		case 49: //SUB
			value =reg[ins->rs]- reg[ins->rt];
			break;
		case 50: //MUL
			value =reg[ins->rs]* reg[ins->rt];
			break;
		case 51: //AND
			value =reg[ins->rs]& reg[ins->rt];
			break;
		case 52: //OR
			value =reg[ins->rs]| reg[ins->rt];
			break;
		case 53: //XOR
			value =reg[ins->rs]^ reg[ins->rt];
			break;
		case 54: //NOR
			value = ~(reg[ins->rs] | reg[ins->rt]);
			break;
		case 55:{ //SLT
			if(reg[ins->rs] < reg[ins->rt])
				value=1;
			else
				value=0;
			break;
		}
		case 56: //ADDI
			value= reg[ins->rs]+ ins->imd;
			break;
		case 57: //ANDI
			value =reg[ins->rs]& ins->imd ;
			break;
		case 58: //ORI
			value =reg[ins->rs]| ins->imd ;
			break;
		case 59: //XORI
			value =reg[ins->rs]^ ins->imd ;
			break;
		default:
			return -1;
	}//end of switch
	
	return value;	
}

int alu2(void){
	//printf("in alu2\n");
	struct decoded_ins *ins;
	ins = cpre_alu2->q[0];
	if(ins == NULL){
		post_alu2=NULL;
		return 0;
	}
	else{
		if((ins->exec_value = execute_alu2(ins)) == -1) /*	execute return -1 when failure*/
			return 0;
		deq_prealu(pre_alu2);
		post_alu2 = ins;
		return ins->exec_value;
	}
}

int wb(void){
	//printf("in wb\n");
	int wb_count=0;
	if(cpost_mem != NULL){
		reg[cpost_mem->rt] = cpost_mem->exec_value;
		working[cpost_mem->rt]=0;
		wb_count++;
	}

	if(cpost_alu2 != NULL){
		if(cpost_alu2->type == 'r'){
			reg[cpost_alu2->rd] = cpost_alu2->exec_value;
			working[cpost_alu2->rd]=0;
			wb_count++;
		}
		else if(cpost_alu2->type == 'i'){
			reg[cpost_alu2->rt] = cpost_alu2->exec_value;
			working[cpost_alu2->rt]=0;
			wb_count++;
		}
	}

	return wb_count;
}







