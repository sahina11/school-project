#include "MIPSsim.h"

int stoi(char *s){ //convert 01010 string to integer
	int sum =0;
	int i,p;

	if(*s == '1'){ //negative number
		for(i=INS_SIZE-1,p=0;i>=0;i--,p++){
		    if(*(s+i) == '0') //equal to revert 1 to 0 
		      sum+=pow(2,p);
		}
		sum++; //after revert, +1
		return -sum;
	}
	else {   //positive number
		for(i=INS_SIZE-1,p=0;i>=0;i--,p++){
    		if(*(s+i) == '1')
      		sum+=pow(2,p);
  		}
  		return sum;
	}
}

void import(FILE *input_file){ //(context) into memory
	//printf("in import\n");
	char *line=calloc(INS_SIZE+2,sizeof(char));
	int i;
	for (i = 0; fgets(line,INS_SIZE+2,input_file)!= NULL; i++) //don't know why +2 XD try and error's result
	{
		mem[i]=stoi(line);
	}
	free(line);
}

int adr2sub(int address){ //transform address to arrary subscript
	return (address - 256)/4;
}

void clone(void){ 
	//printf("in clone\n");
	//printf("clone cpre_issue\n");
	cpre_issue =calloc(1,sizeof(struct pre_issue));
	*cpre_issue = *pre_issue;

	//printf("clone cpre_alu1\n");
	cpre_alu1 = calloc(1,sizeof(struct pre_alu));
	*cpre_alu1 = *pre_alu1;

	//printf("clone cpre_alu2\n");
	cpre_alu2 = calloc(1,sizeof(struct pre_alu));
	*cpre_alu2 = *pre_alu2;

	//printf("clone cpre_mem\n");
	if(pre_mem==NULL){
		cpre_mem =NULL;
	}
	else{
		cpre_mem = calloc(1,sizeof(struct decoded_ins));
		*cpre_mem = *pre_mem;		
	}

	//printf("clone cpost_alu2\n");
	if(post_alu2==NULL){
		cpost_alu2 =NULL;
	}
	else{
		cpost_alu2 = calloc(1,sizeof(struct decoded_ins));
		*cpost_alu2 = *post_alu2;		
	}

	//printf("clone cpost_mem\n");
	if(post_mem==NULL){
		cpost_mem =NULL;
	}
	else{
		cpost_mem = calloc(1,sizeof(struct decoded_ins));
		*cpost_mem = *post_mem;		
	}
}


void ini_queue(void){
	pre_issue=calloc(1,sizeof(struct pre_issue));
	pre_alu1=calloc(1,sizeof(struct pre_alu));
	pre_alu2=calloc(1,sizeof(struct pre_alu));
	pre_mem=calloc(1,sizeof(struct decoded_ins));	
	post_alu2=calloc(1,sizeof(struct decoded_ins));	
	post_mem=calloc(1,sizeof(struct decoded_ins));
}

int enq_preissue (struct decoded_ins *ins){ //IF will do parse and malloc mem
	if(pre_issue->count ==4){
		printf("En_q Error: Full pre_issue buffer!\n");
		return -1;		
	}
	else{
		pre_issue->q[pre_issue->count]=ins;
		pre_issue->count++;
		return 1;
	}
}

int find_clean(struct decoded_ins *ins){
	int i;
	for(i=0;i<4;i++){
		if(pre_issue->q[i] == ins){
			pre_issue->q[i]=NULL ;
			return i;
		}
	}

	return -1;
}

int deq_preissue (struct decoded_ins *ins){ 
// deq is used to organize the queue, not to pop out certain ins 
// 1 success, -1 failure	
	int subscript;
	if((subscript=find_clean(ins))>=0){
		int i;
		for(i = subscript;i<3;i++){
			pre_issue->q[i]=pre_issue->q[i+1];
		}
		pre_issue->q[3]=NULL;
		pre_issue->count--;
		return 1;
	}
	else{
		printf("De_q Error: Instruction %x does not exitst in pre_issue beffer\n",ins->binary);
		return 0;
	}
}


int enq_prealu(struct decoded_ins *ins, struct pre_alu *pre_alu){
	if(pre_alu->count == 2){
		printf("En_q Error: Full pre_alu buffer!\n");
		return -1;
	}
	else{
		pre_alu->q[pre_alu->count]=ins;
		pre_alu->count++;
		return 1;
	}
}

int deq_prealu (struct pre_alu *pre_alu){ 	
	if(pre_alu->q[0] == NULL){
		printf("De_q Error: Nothing to de_q from pre_alu buffer\n");
		return -1;
	}
	else{
		pre_alu->q[0] = pre_alu->q[1];
		pre_alu->q[1] = NULL;
		pre_alu->count--;
		return 1;
	}
}


int get_opcode(int binary){
	int mask=0xfc000000;
	return(unsigned int)(binary&mask)>>26;
}

int get_rs(int binary){
	int mask=0x03e00000;
	return (unsigned int)(binary&mask)>>21;
}

int get_rt(int binary){
	int mask=0x001f0000;
	return (unsigned int)(binary&mask)>>16;
}

int get_rd(int binary){
	int mask=0x0000f800;
	return (unsigned int)(binary&mask)>>11;
}

int get_shamt(int binary){
	int mask=0x000003e0;
	return (unsigned int)(binary&mask)>>6;
}

int get_imd(int binary){
	int mask=0x0000ffff;
	return (unsigned int)(binary&mask);
}

int get_imd26(int binary){
	int mask=0x03ffffff;
	return (unsigned int)(binary&mask);
}

int get_type(int binary){ //-1 failure
	int opcode = get_opcode(binary);

	switch (opcode){
		case 17: case 21: case 24: case 25: case 26: case 27:
		case 48: case 49: case 50: case 51: case 52: case 53:
		case 54: case 55:{
			return 'r';
		}
		case 18: case 19: case 20: case 22: case 23: case 56:
		case 57: case 58: case 59:{
			return 'i';
		}
		case 16:{
			return 'j';
		}
	}

	return -1;
}

char *get_fnname(int binary){
	int opcode = get_opcode(binary);
	if(opcode<28)
		return fn_name[opcode-16];
	else if(opcode>47)
		return fn_name[opcode-36];
	else
		return NULL;
}

struct decoded_ins *cr_decoded_ins(int binary){
	// 1.calloc a space for decoded_ins ptr
	// 2.parse binary code into structure
	// 3.return the ptr				

	struct decoded_ins *ins;
	ins = calloc(1,sizeof(struct decoded_ins));

	ins->binary = binary;
	ins->opcode = get_opcode(binary);

	ins->name = get_fnname(binary);
	ins->type = get_type(binary);

	switch (ins->type){ //switch to decide following arguments
		case 'j':{
			ins->imd26 = get_imd26(binary);
			break;
		}
		case 'i':{
			ins->rs =get_rs(binary);
			ins->rt =get_rt(binary);
			ins->imd =get_imd(binary);
			break;
		}
		case 'r':{
			ins->rs =get_rs(binary);
			ins->rt =get_rt(binary);
			ins->rd =get_rd(binary);
			ins->shamt =get_shamt(binary);
			break;
		}
	}
	return ins;
}



void cout_ins (struct decoded_ins *ins , FILE *fout){
	if(ins == NULL){
		//fprintf(fout, "[NULL]");
	}
	else{
	switch(ins->opcode){
	case 16: //j
		fprintf(fout,"[%s #%d]"
				,ins->name
				,ins->imd26 * 4); //shift left 2 bits
		break;
	case 17: //JR
		fprintf(fout,"[%s R%d]"
				,ins->name
				,ins->rs);
		break;
	case 18: //BEQ
		fprintf(fout,"[%s R%d, R%d, #%d]"
				,ins->name
				,ins->rs
				,ins->rt
				,ins->imd *4); //shift left 2 bits
		break;
	case 19: //BLTZ
		fprintf(fout,"[%s R%d, #%d]"
				,ins->name
				,ins->rs
				,ins->imd *4); //shift left 2 bits
		break;
	case 20: //BGTZ
		fprintf(fout,"[%s R%d, #%d]"
				,ins->name
				,ins->rs
				,ins->imd *4); //shift left 2 bits
		break;
	case 21: //BREAK
		fprintf(fout,"[%s]"
				,ins->name);
		break;
	case 22: //SW
		fprintf(fout,"[%s R%d, %d(R%d)]"
				,ins->name
				,ins->rt
				,ins->imd
				,ins->rs);
		break;
	case 23: //LW
		fprintf(fout,"[%s R%d, %d(R%d)]"
				,ins->name
				,ins->rt
				,ins->imd
				,ins->rs);
		break;
	case 24: //SLL
		fprintf(fout,"[%s R%d, R%d, #%d]"
				,ins->name
				,ins->rd
				,ins->rt
				,ins->shamt);
		break;
	case 25: //SRL
		fprintf(fout,"[%s R%d, R%d, #%d]"
				,ins->name
				,ins->rd
				,ins->rt
				,ins->shamt);
		break;
	case 26: //SRA
		fprintf(fout,"[%s R%d, R%d, #%d]"
				,ins->name
				,ins->rd
				,ins->rt
				,ins->shamt);
		break;
	case 27: //NOP
		fprintf(fout,"[%s]"
				,ins->name);
		break;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55:
		//ADD SUB MUL AND OR XOR NOR SLT
		fprintf(fout,"[%s R%d, R%d, R%d]"
				,ins->name
				,ins->rd
				,ins->rs
				,ins->rt);
		break;
	case 56: case 57: case 58: case 59:
		//ADDI ANDI ORI XORI
		fprintf(fout,"[%s R%d, R%d, #%d]"
				,ins->name
				,ins->rt
				,ins->rs
				,ins->imd);
		break;
	}//end of swtich
	}//end of else
}

int sub2add (int subscript){
	return (subscript*4 +256);
}

int locate_data(void){ //return address, or -1 for no data at all
	int i;
	for(i=0; i<MEM_SIZE; i++){
		if(get_opcode(mem[i]) == 0 || get_opcode(mem[i]) == 63)
			return(sub2add(i));
	}
	return -1;
}

void cout_state(int cycle, FILE *fout){
	//printf("in cout state\n");
	int i;  //20 hyphens and a new line = =
	for(i=0;i<20;i++){
		fprintf(fout,"-");
	}
	fprintf(fout,"\n");

	fprintf(fout,"Cycle:%d\n\n",cycle);

	fprintf(fout,"IF Unit:\n");
	fprintf(fout,"\tWaiting Instruction:"); cout_ins(waiting_ins,fout);
	fprintf(fout,"\n");
	fprintf(fout,"\tExecuted Instruction:"); cout_ins(executed_ins,fout);
	fprintf(fout,"\n");

	fprintf(fout,"Pre-Issue Queue:\n");
	for(i=0;i<4;i++){
	fprintf(fout,"\tEntry %d: ",i);
	cout_ins(pre_issue->q[i],fout);	
	fprintf(fout,"\n");
	}
	
	fprintf(fout,"Pre-ALU1 Queue:\n");
	for(i=0;i<2;i++){
	fprintf(fout,"\tEntry %d: ",i);
	cout_ins(pre_alu1->q[i],fout);	
	fprintf(fout,"\n");
	}

	fprintf(fout,"Pre-MEM Queue:");
	cout_ins(pre_mem,fout);	
	fprintf(fout,"\n");
	
	fprintf(fout,"Post-MEM Queue:");
	cout_ins(post_mem,fout);	
	fprintf(fout,"\n");

	fprintf(fout,"Pre-ALU2 Queue:\n");
	for(i=0;i<2;i++){
	fprintf(fout,"\tEntry %d: ",i);
	cout_ins(pre_alu2->q[i],fout);	
	fprintf(fout,"\n");
	}

	fprintf(fout,"Post-ALU2 Queue:");
	cout_ins(post_alu2,fout);	
	fprintf(fout,"\n");
	fprintf(fout,"\n");

	fprintf(fout,"Registers\n");
	fprintf(fout,"R00:\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",reg[0],reg[1],reg[2],reg[3],reg[4],reg[5],reg[6],reg[7]);
	fprintf(fout,"R08:\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",reg[8],reg[9],reg[10],reg[11],reg[12],reg[13],reg[14],reg[15]);
	fprintf(fout,"R16:\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",reg[16],reg[17],reg[18],reg[19],reg[20],reg[21],reg[22],reg[23]);
	fprintf(fout,"R24:\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n\n",reg[24],reg[25],reg[26],reg[27],reg[28],reg[29],reg[30],reg[31]);

	int add_data = locate_data();
	int subscript = adr2sub(add_data);
	fprintf(fout,"Data\n");
	fprintf(fout, "%d:",add_data);
	for(i=0;i<8;i++){
		fprintf(fout, "\t%d",mem[subscript++] );
	}
	fprintf(fout,"\n");

	fprintf(fout, "%d:",add_data +32);
	for(i=0;i<8;i++){
		fprintf(fout, "\t%d",mem[subscript++] );
	}
	fprintf(fout,"\n");
}

void i2a(struct decoded_ins *ins, int (*a)[]){ 
	if(ins->type == 'r'){
		if(ins->opcode == 24 ||ins->opcode == 25 ||ins->opcode == 26){// SLL,SRL,SRA
			(*a)[ins->rt]= READ;
			(*a)[ins->rd]= WRITE;
		}
		else{
			(*a)[ins->rt]= READ;
			(*a)[ins->rs]= READ;
			(*a)[ins->rd]= WRITE;
		}
	}
	else if(ins->type =='i'){
		if(ins->opcode ==18){//BEQ
			(*a)[ins->rt]= READ;
			(*a)[ins->rs]= READ;			
		}
		else if(ins->opcode == 19 || ins->opcode == 20){//BLTZ BGTZ
			(*a)[ins->rs]= READ;
		}
		else if(ins->opcode == 22){ //SW , no write
			(*a)[ins->rt]= READ;
			(*a)[ins->rs]= READ;
		}
		else{
			(*a)[ins->rs]= READ;
			(*a)[ins->rt]= WRITE;
		}
	}
}

void ins2array(struct decoded_ins *ins, int array_no){
	if(array_no ==0)
		i2a(ins,&c0);
	else if(array_no ==1)
		i2a(ins,&c1);
	else if(array_no ==2)
		i2a(ins,&c2);
	else if(array_no ==3)
		i2a(ins,&c3);
}

void clean_array(void){
	int i;
	for(i=0;i<32;i++){
		c0[i]=0;
		c1[i]=0;
		c2[i]=0;
		c3[i]=0;
	}
}


// support function
void ini_context(void){
	ini_queue();
	
	waiting_ins=NULL;
	executed_ins=NULL;
}

void t_show_preissue(struct pre_issue *pre_issue){ //could be used to show cpre_issue
	int i;
	for (i = 0; i < 4; ++i)
	{
		if(pre_issue->q[i]){
			printf("pre_issue->q[%d]: ",i );
			cout_ins(pre_issue->q[i],stdout);
			printf("\n");
		}
		else
			printf("pre_issue->q[%d] == NULL\n",i );
	}
	printf("pre_issue count = %d\n\n",pre_issue->count );
}

void t_show_prealu(struct pre_alu *pre_alu){
	int i;
	for (i = 0; i < 2; ++i)
	{
		if(pre_alu->q[i]){
			printf("pre_alu->q[%d]: ",i );
			cout_ins(pre_alu->q[i],stdout);
			printf("\n");
		}
		else
			printf("pre_alu->q[%d] == NULL\n",i );
	}
	printf("pre_alu count = %d\n\n",pre_alu->count );
}

struct decoded_ins *t_cr_dummycell(void){
	struct decoded_ins *p=calloc(1,sizeof(struct decoded_ins));
	p->binary = mem[0];
	p->name ="keke";
	return p;
}

void t_cout_ins (struct decoded_ins *p){
	if(p ==NULL){
		printf("NULL ptr,check again!\n");
		return;
	}
	else{
	printf("binary == %x\n",p->binary);
	printf("name == %s\n",p->name);
	printf("type == %c\n",p->type);

	switch (p->type){ //switch to decide following arguments
		case 'j':{
			printf("imd26 == %x\n",p->imd26);
			return;
		}
		case 'i':{
			printf("rs == %d\n",p->rs);
			printf("rt == %d\n",p->rt);
			printf("imd == %d\n",p->imd);
			return;
		}
		case 'r':{
			printf("rs == %d\n",p->rs);
			printf("rt == %d\n",p->rt);
			printf("rd == %d\n",p->rd);
			printf("shamt == %d\n",p->shamt);
			return;
		}
	}
	}
}

void t_cout_array(int a[]){
	int i;
	for(i=0;i<32;i++){
		printf("a[%d]==%d\n",i,a[i] );
	}
}

