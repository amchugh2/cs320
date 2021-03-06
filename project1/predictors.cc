#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>

using namespace std;

// Each function will return the number of correct predictions that it
// makes and the total of number branches in the correct format

// Each input will be the line of the trace, with format 
// *PC* *OUTCOME* *BRANCH TARGET*

// Return CORRECT: NUMBER OF CORRECT PREDICTIONS FROM EACH BRANCH
// Predictor #1: Always Taken
// Read in entire file as input, because line doesn't matter each prediction will be same
string always_taken(vector<pair<unsigned long long, string>> branches){
	int correct = 0;
	int total = 0;
	unsigned long long addr;
	string behavior;
	for(pair<unsigned long long, string> branch: branches){
		addr = branch.first;
		behavior = branch.second;
		if(behavior == "T"){
			correct = correct + 1;
		}
	total = total + 1;
	}
	string str = to_string(correct) + "," + to_string(total) + ";";
	return str;
}

// Predictor #2: Never Taken
// Same concept as AT - > Do not need to calculate anything because always NT
string never_taken(vector<pair<unsigned long long, string>> branches){
	int correct = 0;
	int total = 0;
	unsigned long long addr;
	string behavior;

	for(pair<unsigned long long, string> branch : branches){
		addr = branch.first;
		behavior = branch.second;
		if(behavior == "NT"){
			correct++;
		}
	total++;
	}
	string str = to_string(correct) + "," + to_string(total) + ";";
	return str;
}

// Predictor #3: Bimodal Predictor with Single Bit of History
// Must hard-code in table size -> will be passed on as parameter
// Assume initial state of all prediction counters is Taken ("T")
// input: test to compare with, unsigned long PC, size of table
string bimodal_single_bit(int table_size, vector<pair<unsigned long long, string>> branches){
	// Open file for reading 
	// Initialize vars
	int correct = 0;
	int total = 0;
	int index;
	string prediction;
	int last_bits;
	unsigned long long addr;
	string behavior;

	string table[table_size];
	for(int i = 0; i < table_size; i++){
		table[i] = "T";
	}

	// get index
	for(pair<unsigned long long, string> branch : branches){
		addr = branch.first;
		behavior = branch.second;		
		last_bits = (addr & ((1 << ((int)log2(table_size))) - 1));
		if(behavior == "T"){
			if(table[last_bits % table_size] == "T"){
				correct++;
			}
			// only going here
			if(table[last_bits % table_size] == "NT"){
				table[last_bits % table_size] = "T";
			}
		}
		else if(behavior == "NT"){
			if(table[last_bits % table_size] == "NT"){
			//	cout << "got to NT correct" << endl;
				correct++;
			}
			if(table[last_bits % table_size] == "T"){
			//	cout << "got to NT incorrect" << endl;
				table[last_bits % table_size] = "NT";
			}
		}
		total++;
	}
	string str = to_string(correct) + "," + to_string(total) + ";";
	return str;
}
// Predictor 4: Binomial Double Bit
string bimodal_double_bit(int table_size, vector<pair<unsigned long long, string>> branches){
	// Open file for reading 
	// Initialize vars
	unsigned long long addr;
	string behavior;
	int correct = 0;
	int total = 0;
	int index;
	int prediction;
	int last_bits;

	int table[table_size];
	for(int i = 0; i < table_size; i++){
		table[i] = 3;
	}

	// get index
	for(pair<unsigned long long, string> branch : branches){
		addr = branch.first;
		behavior = branch.second;
		last_bits = (addr & ((1 << ((int)log2(table_size))) - 1));
		// strongly or weakly not taken
		if(behavior == "T"){
			if(table[last_bits % table_size] == 0 || table[last_bits % table_size] == 1){
				table[last_bits % table_size]++;
			}
			else{
				table[last_bits % table_size] = 3;
				correct++;
			}
		}
		else if(behavior == "NT"){
			if(table[last_bits % table_size] == 3 || table[last_bits % table_size] == 2){
				table[last_bits % table_size]--;
			}
			else{
				table[last_bits % table_size] = 0;
				correct++;
			}
		}
		total++;
	}
	string str = to_string(correct) + "," + to_string(total) + ";";
	return str;
}

// Predictor 5: Gshare -> PC XOR'd with GHR to generate index. Fix table size at 2048. 
string gshare(unsigned int GHL, vector<pair<unsigned long long, string>> branches){
	// initialize variables
	int correct = 0;
	int total = 0;
	// fixed table size at 2048
	int table_size = 2048;
	int table[table_size];
	// initialize to strongly taken
	for(int i = 0; i < table_size; i++){
		table[i] = 3;
	}

	unsigned long long addr;
	string behavior;
	int prediction;
	int last_bits;
	unsigned int GHR = 0;

	for(pair<unsigned long long, string> branch : branches){
		addr = branch.first;
		behavior = branch.second;
		int index = ((GHR ^ addr) % table_size);
		if(behavior == "T"){
			if(table[index] == 0 || table[index] == 1){
				table[index]++;
			}
			else{
				table[index] = 3;
				correct++;
			}

			GHR = (((1 << (GHL)) - 1)) & ((GHR << 1) + 1);
		}
		else if(behavior == "NT"){
			if(table[index] == 3 || table[index] == 2){
				table[index]--;
			}
			else{
				table[index] = 0;
				correct++;
			}
			GHR = (((1 <<(GHL)) - 1)) & (GHR << 1);
		}
	total++;
	}
	string str = to_string(correct) + "," + to_string(total) + ";";
	return str;
}

int tournament(vector<pair<unsigned long long, string>> branches){
	//initialize tables
	int table_size = 2048;
	int bimodal[table_size];
	int gshare[table_size];
	int selector[table_size];
	for(int i = 0; i < table_size; i++){
		selector[i] = 0; // strongly not taken
		bimodal[i] = 3; // strongly not taken
		gshare[i] = 3; // strongly not taken
	}
	
	// initialize variables
	int correct = 0;
	// for gshare
	int GHR = 0;
	int GHL = 11;
	string behavior;
	unsigned long long addr;
	bool bimodal_correct = false;
	bool gshare_correct = false;
	int gshare_index;
	int pc_index;

	// get input
	for(pair<unsigned long long, string> branch : branches){
		addr = branch.first;
		behavior = branch.second;
		// to be used later
		bool bimodal_correct = false;
		bool gshare_correct = false;
		//get last 11 bits of address
		unsigned int pc_index = (addr & ((1 << GHL) - 1)) % table_size;
		//get index to use for gshare table 
		unsigned int gshare_index = (pc_index ^ GHR) % table_size;
	
		// LOGIC:
		// 1. CHECK EACH TABLE
		// 	A. CHECK BEHAVIOR
		// 		I. CORRECT PREDICTION: SET TO 0 (STRONGLY NOT TAKEN) / 3 (STRONGLY TAKEN)
		// 		II. INCORRECT PREDICTION: INCREMENT (NOT TAKEN) / DECREMENT (TAKEN)
		// 2. TAKE CARE OF EACH SELECTOR TABLE
		
		// BIMODAL TABLE
		if (bimodal[pc_index] == 3 || bimodal[pc_index] == 2) {
			if (behavior == "T") { // correct
				bimodal[pc_index] = 3;
				bimodal_correct = true;
			}
			else if(behavior == "NT"){ //incorrect
				bimodal[pc_index] -=1;
			}
		}
		else if(bimodal[pc_index] == 0 || bimodal[pc_index] == 1){
			if (behavior == "T") { // incorrect
				bimodal[pc_index] +=1;
			}
			else if(behavior == "NT"){ // correct
				bimodal[pc_index] = 0;
				bimodal_correct = true;
			}

		}

		// GSHARE TABLE
		if (gshare[gshare_index] == 3 || gshare[gshare_index] == 2) {
			if (behavior == "T") { // correct
				(gshare[gshare_index]) = 3;
				gshare_correct = true;
				GHR = ((GHR << 1) | 1) & ((1 << (GHL)) - 1);

			}
			else if(behavior == "NT") { // incorrect
				(gshare[gshare_index])-=1;
				GHR = (GHR << 1) & ((1 << (GHL)) - 1);

			}
		}
		else if(gshare[gshare_index] == 0 || gshare[gshare_index] == 1) {
			if(behavior == "T"){ // incorrect
				gshare[gshare_index] +=1;
				GHR = ((GHR << 1) | 1) & ((1 << (GHL)) - 1);
			}
			else if (behavior == "NT") { // correct
				(gshare[gshare_index]) = 0;
				gshare_correct = true;
				GHR = (GHR << 1) & ((1 << (GHL)) - 1);

			}
		}

		// SELECTOR TABLE LOGIC
		// 1. IF BOTH CORRECT: INCREMENT NUM CORRECT & DO NOTHING (CONTINUE)
		// 2. IF NEITHER CORRECT: DO NOTHING
		// 3. IF PREFER PREDICTOR
		// 	A. PREDICTION TRUE: SET TO 0 (GSHARE) OR 3 (BIMODAL)
		// 	B. PREDICTION FALSE: INCREMENT (GSHARE WRONG) / DECREMENT (BIMODAL WRONG)
		// if both are same: do nothing
		if(gshare_correct && bimodal_correct){
			correct++;
			continue;
		}
		if(!gshare_correct && !bimodal_correct){
			continue;
		}

		if (selector[pc_index] == 3 || selector[pc_index] == 2) {
			if (bimodal_correct == true) {
				selector[pc_index] = 3;
				correct++;
			}
			else {
				(selector[pc_index])-=1;
			}
		}
		else {
			if (gshare_correct == true) {
				selector[pc_index] = 0;
				correct++;
			}
			else {
				selector[pc_index] +=1;
			}

		}

	}
	return correct;
}

// 7. Branch Target Buffer
// BTB integrated with Bimodal Predictor. Size = 512 entries
// Initialized with single bit of history to "T"
// BTB indexed using the PC of the branch isn
// if prediction = taken -> target is read
// if actual behavior = taken -> BTB is updated with the correct branch target
// use PC to index into BTB
// if PC prediction != actual branch, misprediction -> write correct into table
string BTB(char* file){
	// initialize variables
	unsigned long long addr;
	unsigned long long target;
	string behavior;
	string line;
	int correct = 0;
	int total = 0;
	int table_size = 512;
	int last_bits = 0;
	int index = 0;

	// initialize tables
	int bimodal[table_size];
	unsigned long long BTB[table_size];
	for(int i = 0; i < table_size; i++){
		bimodal[i] = 1;
		BTB[i] = 0;
	}
	// read input
	ifstream infile(file);
	while(getline(infile,line)){
		stringstream s(line);
		s >> std::hex >> addr >> behavior >> std::hex >> target;
			last_bits = (addr & ((1 << ((int)log2(table_size))) - 1));
			index = last_bits % table_size;
			if(bimodal[index] == 1){ // taken
				total+=1;
				if(BTB[index] == target){
					correct+=1;
				}
				if(behavior == "T"){
					BTB[index] = target;
				}
				else{
					bimodal[index] = 0;
				}
			}
			else if(bimodal[index] == 0){ // not taken
				if(behavior == "T"){ 
					bimodal[index] = 1;
					BTB[index] = target;
				}
			}
		}
	string str = to_string(correct) + "," + to_string(total) + ";";
	return str;
}
 

int main(int argc, char *argv[]){
	// get total number of lines for tournament predictor
	int total = 0;
	unsigned long long addr;
	string behavior, line;
	unsigned long long target;
	vector<pair<unsigned long long, string>> branches;

	ifstream infile(argv[1]);

	while(getline(infile,line)){
		stringstream s(line);
		++total;
		s >> std::hex >> addr >> behavior >> std::hex >> target;
		branches.push_back(make_pair(addr, behavior));
	}

	ofstream file;
	file.open(argv[2]);

	// Always Taken
	file << always_taken(branches) << endl;
	// Never Taken
	file << never_taken(branches) << endl;

	// Bimodal Single Bit
	file << bimodal_single_bit(16, branches) << " ";
	file << bimodal_single_bit(32, branches) << " ";
	file << bimodal_single_bit(128, branches) << " ";
	file << bimodal_single_bit(256, branches) << " ";
	file << bimodal_single_bit(512, branches) << " ";
	file << bimodal_single_bit(1024, branches) << " ";
	file << bimodal_single_bit(2048, branches) << endl;
	
		
	// Bimodal Double Bit
	file << bimodal_double_bit(16, branches) << " ";
	file << bimodal_double_bit(32, branches) << " ";
	file << bimodal_double_bit(128, branches) << " ";
	file << bimodal_double_bit(256, branches) << " ";
	file << bimodal_double_bit(512, branches) << " ";
	file << bimodal_double_bit(1024, branches) << " ";
	file << bimodal_double_bit(2048, branches) << endl;
	
	// GShare
	file << gshare(3, branches) << " ";
	file << gshare(4, branches) << " ";
	file << gshare(5, branches) << " ";
	file << gshare(6, branches) << " ";
	file << gshare(7, branches) << " ";
	file << gshare(8, branches) << " ";
	file << gshare(9, branches) << " ";
	file << gshare(10, branches) << " ";
	file << gshare(11, branches) << endl;
	
	// Tournament
	file << tournament(branches) << "," << total << ";" << endl;
	
	file << BTB(argv[1]) << endl;
	file.close();
	return 0;
}
