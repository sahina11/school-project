#include "Topology.hpp"


map<unsigned int,Node>::iterator Topology::find_successor(unsigned int k){
	if (graph.empty()){
		cout<<"Try to find_successor in an empty graph. Exit now."<<endl;
		exit(-1);
	}
	
	
	if (graph.find(k) == graph.end()){
		cout<<"This node is not in the graph. Cound't find_successor. Exit now."<<endl;
		exit(-1);
	}


	if (graph.size() ==1) return graph.begin();
	else{
		if (graph.upper_bound(k) == graph.end())
			return graph.begin();
		else
			return graph.upper_bound(k);
	}
}

map<unsigned int,Node>::iterator Topology::find_predecessor(unsigned int k){
	map<unsigned int,Node>::iterator it;

	if (graph.empty()){
		cout<<"Try to find_predecessor in an empty graph. Exit now."<<endl;
		exit(-1);
	}
	
	if (graph.find(k) == graph.end()){
		cout<<"This node is not in the graph. Cound't find_predecessor. Exit now."<<endl;
		exit(-1);
	}

	if (graph.size() ==1) return graph.begin();
	else{
		if (graph.find(k) == graph.begin()){
			it = graph.end();
			return --it;
		}
		else{
			it = graph.find(k);
			return --it;
		}
	}
}

map<unsigned int,Node>::iterator Topology::find_next_node(unsigned int k){

	if (graph.empty()){
		cout<<"Try to find_next_node in an empty graph. Exit now."<<endl;
		exit(-1);
	}

	
	if (graph.size() == 1) return graph.begin();
	else{
		if (graph.lower_bound(k) == graph.end())
			return graph.begin();
		else return graph.lower_bound(k);
	}
}

int Topology::get_files_from_succesor(unsigned int k){
	unsigned int predecessor = Topology::find_predecessor(k)->first;
	unsigned int successor = Topology::find_successor(k)->first;

	list< pair<string,unsigned int> > file_list;
	//file list comes from successor
	file_list = graph[successor].get_file_entry_list(predecessor,k);

	if (file_list.empty()){ //nothing to be add and delete
		return 0;
	}
	else{
		for (list< pair<string,unsigned int> >::iterator it_list = file_list.begin();
			it_list != file_list.end(); it_list++){
			//add file into newnode
			graph[k].add_file_entry( it_list->first , it_list->second );
			//delete file in successor
			graph[successor].delete_file_entry( it_list->first , it_list->second );
		}
		return 0;
	}
}

int Topology::dump_files_to_successor(unsigned int k){
	unsigned int successor = Topology::find_successor(k)->first;
	list< pair<string,unsigned int> > file_list;

	//dump all your file entries
	file_list = graph[k].get_all_file_entry_list(k);

	if (file_list.empty()){ //nothing to be dumped
		return 0;
	}
	else{
		for (list< pair<string,unsigned int> >::iterator it_list = file_list.begin();
			it_list != file_list.end(); it_list++){
			//add files into successor
			graph[successor].add_file_entry( it_list->first , it_list->second );
		}
		return 0;
	}
}

void Topology::is_node_exist(string ipv4_str){
	unsigned int ipv4_int = ip_strToInt(ipv4_str);
	map<unsigned int,Node>::iterator it = graph.find(ipv4_int);
	
	if ( it == graph.end()){
		cout<<ipv4_str<<" not in the graph."<<endl;
	}
	else{
		it->second.display();
	}
}

void Topology::display(void){
	if (graph.empty()){
		cout<<"Empty topology."<<endl;
	}
	else{
		cout<<"Topology: ";
		for(map<unsigned int,Node>::iterator it=graph.begin(); 
			it != graph.end(); it++){
			cout<<"--"<<ip_intToStr(it->first);
		}
		cout<<endl;
	}
}

int Topology::addNode(string ipv4_str){
	unsigned int newNode_int = ip_strToInt(ipv4_str);

	if(graph.find(newNode_int) != graph.end()){
		cout<<"Node of this IP has already existed in the graph. Can't addNode. Exit now."<<endl;
		exit(-1);
	}

	//1.create finger table
	//2.update other's finger table
	//3.ask file table from successor and update successor's file table

	unsigned int index,value;
	
	Node new_node;
	graph[newNode_int] = new_node; //add in first to help find_next_node working later

	//1.create finger table
	for (int j = 0; j<32;j++){
		index = (newNode_int + pow(2,j) > 0xffffffff) ?
				(newNode_int + pow(2,j) - 0xffffffff -1): //see the graph
				(newNode_int + pow(2,j));
		value =  find_next_node(index)->first;
		graph[newNode_int].add_finger_entry(index,value);
	}
	//no need to do 2,3, if this is the first one.
	
	if (graph.size() > 1){ //means you are not alone, 2,3 to be done
		//2.update other's finger table
		for(map<unsigned int,Node>::iterator it_graph = graph.begin();
			it_graph != graph.end(); it_graph++){

			if(it_graph->first != newNode_int){ //don't have to update newly created node's finger table
				unsigned int predecessor_of_newNode = find_predecessor(newNode_int)->first;
				(it_graph->second).update_finger_table(predecessor_of_newNode,newNode_int,newNode_int);
			}	
		}
		
		//3.ask file table entries from successor and update successor's file table
		Topology::get_files_from_succesor(newNode_int);
	}
	
	return 0;
}

int Topology::removeNode(string ipv4_str){
	unsigned int remove_node = ip_strToInt(ipv4_str);

	if(graph.find(remove_node) == graph.end()){
		cout<<"Node of this IP didn't exist in the graph. Can't removeNode. Exit now."<<endl;
		exit(-1);
	}


	unsigned int index,value; //for updating index table

	if (graph.size() == 1){ //if you are the only one, why bother?
		graph.clear();
		return 0;
	}
	else{
		//1. update others's finger table
		for(map<unsigned int,Node>::iterator it_graph = graph.begin();
			it_graph != graph.end(); it_graph++){

			if(it_graph->first != remove_node){ //don't worry the node is going to be removed
				unsigned int predecessor = find_predecessor(remove_node)->first;
				unsigned int successor = find_successor(remove_node)->first;
				(it_graph->second).update_finger_table(predecessor,remove_node,successor);
			}
		}
		//2. dump all you file table to your successor
		Topology::dump_files_to_successor(remove_node);
		

		//finished all info transfer, remove the node
		graph.erase(remove_node);

		return 0;
	}
}

int Topology::insertItem(string ipv4_str, string file_name){
	unsigned int myip = ip_strToInt(ipv4_str);
	unsigned int file_hash = hashString(file_name);

	unsigned int place_to_store_fileIndex = Topology::find_next_node(file_hash)->first;

	graph[place_to_store_fileIndex].add_file_entry(file_name,myip);

	return 0;
}

int Topology::deleteItem(string ipv4_str, string file_name){
	unsigned int myip = ip_strToInt(ipv4_str);
	unsigned int file_hash = hashString(file_name);

	unsigned int place_to_store_fileIndex = Topology::find_next_node(file_hash)->first;

	graph[place_to_store_fileIndex].delete_file_entry(file_name, myip);
	return 0;
}

list<string> Topology::find(string ipv4_str, string file_name, list<string> node_sequece, bool suc){
	list<string> new_sequece;
	new_sequece.swap(node_sequece);
	
	if (suc){ //for stupid (successor) annotation
		string empty;
		empty.append(ipv4_str);
		empty.append(" (successor)");
		new_sequece.push_back(empty);
	}
	else{
		new_sequece.push_back(ipv4_str);
	} 
		

	unsigned int myip = ip_strToInt(ipv4_str);
	unsigned int file_hash = hashString(file_name);

	if (graph.find(myip) == graph.end()){ //this node is not in the topology
		unsigned int successor = find_next_node(myip)->first;
		string successor_str = ip_intToStr(successor);

		return Topology::find(successor_str, file_name, new_sequece,true);
	}
	else{
		unsigned int loolup_result = graph[myip].finger_lookup(file_hash);
		new_sequece.push_back(ip_intToStr(loolup_result));

		if (myip == loolup_result){
			if( graph[myip].is_file_exist(file_name)){
				new_sequece.push_back("Success");
				return new_sequece;
			}
			else{
				new_sequece.push_back("failed");
				return new_sequece;
			}
		}
		else{
			return Topology::find(ip_intToStr(loolup_result), file_name, new_sequece,false);
		}
	}
}

