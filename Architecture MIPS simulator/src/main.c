#include "MIPSsim.c"
#include "fn_unit.c"

int main(int argc, char const *argv[])
{
	if(argc != 2){
		printf("Useage: %s inputfile.txt\n",argv[0]);
		exit(-1);
	}

	input_file =fopen(argv[1],"r");
	output_file =fopen("simulation.txt","w");
	
	import(input_file);
	ini_context();

	int i;
	for(i=1;i<32;i++){
		clone();
		fetch_decode(i);
		issue();
		alu1();
		alu2();
		memory();
		wb();
		cout_state(i,output_file);
	}

	return 0;
}