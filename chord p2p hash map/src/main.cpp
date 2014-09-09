#include "Helper.hpp"
#include "Topology.hpp"


int main(int argc, char const *argv[])
{
	string line, command, ipv4_str, file_name;
	int begin, end;
	
	Topology topology;


	while(getline(cin,line) && line[0] != '#' && line.length() != 0){
		begin = line.find("(");
		command = line.substr(0,begin);
		// cout<<"command--> "<<command<<endl;

		if (command == "addNode"){
			ipv4_str = find_ipv4_str(line);
			// cout<<"\tipv4_str= "<<ipv4_str<<endl;

			if (topology.addNode(ipv4_str) == 0)
				cout<<"Success\n";

		}
		else if(command == "removeNode"){
			ipv4_str = find_ipv4_str(line);
			// cout<<"\tipv4_str= "<<ipv4_str<<endl;
			if (topology.removeNode(ipv4_str) == 0)
				cout<<"Success\n";

		}
		else if(command == "insertItem"){
			ipv4_str = find_ipv4_str(line);
			file_name = find_file_name(line);
			// cout<<"\tipv4_str= "<<ipv4_str<<endl;			
			// cout<<"\tfile name= "<<file_name<<endl;

			if (topology.insertItem(ipv4_str,file_name) == 0)
				cout<<"Success\n";

		}
		else if(command == "deleteItem"){
			ipv4_str = find_ipv4_str(line);
			file_name = find_file_name(line);
			// cout<<"\tipv4_str= "<<ipv4_str<<endl;			
			// cout<<"\tfile name= "<<file_name<<endl;

			if (topology.deleteItem(ipv4_str,file_name) == 0)
				cout<<"Success\n";

		}
		else if(command == "find"){
			ipv4_str = find_ipv4_str(line);
			file_name = find_file_name(line);
			// cout<<"\tipv4_str= "<<ipv4_str<<endl;			
			// cout<<"\tfile name= "<<file_name<<endl;

			list<string> res;
			res = topology.find(ipv4_str,file_name,res,false);
			cout<<"\t"<<file_name<<" hashed to "<<hex<<hashString(file_name)<<endl;
			print_res_list(res);

		}
		else{
			cout<<"Wrong command!! Exit now. Bye!"<<endl;
			exit(-1);
		}

	}
	return 0;
}