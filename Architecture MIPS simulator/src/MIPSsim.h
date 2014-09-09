#ifndef MIPSSIM_H
#define MIPSSIM_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define INS_SIZE 32
#define MEM_SIZE 64
#define SCB_SIZE 200 //how many entries in scoreboard == how many cycle you could have 

#define READ 1 //for labeling hazard detection array
#define WRITE 2

//register and memory array
int reg[32]={0};
int mem[MEM_SIZE]={0};

//IO pointers
FILE *input_file;
FILE *output_file;

int pc=256; //program counter

struct decoded_ins{
	int binary;
	int opcode;
	char *name;
	int type;
	int rs, rt,rd,shamt,imd,imd26;

	int exec_address; //to store value after execution
	int exec_value;
};


//6 real queues
struct pre_issue{
	struct decoded_ins *q[4];
	int count;
}*pre_issue;

struct pre_alu{
	struct decoded_ins *q[2];
	int count;
}*pre_alu1,*pre_alu2;

struct decoded_ins *pre_mem;
struct decoded_ins *post_alu2;
struct decoded_ins *post_mem;

//6 clone queue
/*I don't need to clone reg[], because when executing 
fn units, WB is the last one to do, so every unit 
will read the old reg[] value.*/

struct pre_issue *cpre_issue;
struct pre_alu *cpre_alu1,*cpre_alu2;
struct decoded_ins *cpre_mem, *cpost_alu2, *cpost_mem;


static char *fn_name[]={
	"J","JR","BEQ","BLTZ","BGTZ","BREAK","SW","LW",
	"SLL","SRL","SRA","NOP","ADD","SUB","MUL","AND",
	"OR","XOR","NOR","SLT","ADDI","ANDI","ORI","XORI"
};

//array for hazard detection

int working[32]={0};
int c0[32]={0};
int c1[32]={0};
int c2[32]={0};
int c3[32]={0};


//IF:
struct decoded_ins *waiting_ins;
struct decoded_ins *executed_ins;

/*************fn_unit.c*****************/
int ck_hazard (int array_no);
int ck_ls_case (int array_no);
void update_working_array(int array_no);
int issue(void);


/*************MIPSsim.c*****************/
int stoi(char *s); //convert 01010 string to integer
void import(FILE *input_file); //import into memory
int adr2sub(int address); //transform address to arrary subscript
int sub2add (int subscript);
int locate_data(void);

//for parser
int get_opcode(int binary);
int get_rs(int binary);
int get_rt(int binary);
int get_rd(int binary);
int get_shamt(int binary);
int get_imd(int binary);
int get_imd26(int binary);
int get_type(int binary);
char *get_fnname(int binary);
struct decoded_ins *cr_decoded_ins(int binary);


//queue
/*en_q and de_q will not happen at the same time 
becasue exections of fn unit are serialized.*/

int enq_preissue (struct decoded_ins *ins);
int find_clean(struct decoded_ins *ins);
int deq_preissue (struct decoded_ins *ins);
int enq_prealu(struct decoded_ins *ins, struct pre_alu *pre_alu);
int deq_prealu (struct pre_alu *pre_alu);
void clone(void);

//hazard detection
void i2a(struct decoded_ins *ins, int (*a)[]);
void ins2array(struct decoded_ins *ins, int array_no);
void clean_array(void);



//context building
void ini_queue(void);

//IO fn
void cout_ins (struct decoded_ins *ins , FILE *fout);
void cout_state(int cycle, FILE *fout);

//support fn
void ini_context(void);
void t_show_preissue(struct pre_issue *pre_issue);
void t_show_prealu(struct pre_alu *pre_alu);
struct decoded_ins *t_cr_dummycell(void);
void t_cout_ins (struct decoded_ins *p);
void t_cout_array(int a[]);



#endif