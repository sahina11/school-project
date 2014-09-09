#ifndef TOPOLOGY_HPP
#define TOPOLOGY_HPP

#include "Node.hpp"

class Topology{
	public:
		//find successor, the node must be in the topology
		//k = converted ipv4
		map<unsigned int,Node>::iterator find_successor(unsigned int k); 
		
		map<unsigned int,Node>::iterator find_predecessor(unsigned int k); 
		
		//used to find a node for file index , or building finger table
		//k could be hashed file, or coverted ipv4 int
		map<unsigned int,Node>::iterator find_next_node(unsigned int k); 

		void is_node_exist(string ipv4_str); 

	 	
		//when you addnode into the topology, get some files from your successor
		//it will delete those file in successor, add them into newnode
		//k = newnode's int
		int get_files_from_succesor(unsigned int k); 

		//when removing node, dump all your file into successor
		//k = node to be removed
		int dump_files_to_successor(unsigned int k);


		//return 0 for success, -1 for fail
	 	int addNode(string ipv4_str); 
	 	
	 	//0, -1
	 	int removeNode(string ipv4_str);
	 	
	 	//insert file index into a node
	 	int insertItem(string ipv4_str, string file_name); 
	 	
	 	//delete file index, if the ip is not the one who insert the file, report error
	 	int deleteItem(string ipv4_str, string file_name); 
	 	
		list<string> find(string ipv4_str, string file_name, list<string> node_sequece,bool suc);


	 	void display(void); 

	 private:
	 	map<unsigned int,Node> graph;
	
};


#endif