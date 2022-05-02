// /*
// * @Author: Junqiao Qiu
// * @Last Modified: Junqiao Qiu, 04/22/2020
// * @Brief: The file defines the mapping rules, Inputs and DFAs (the key component is the 
// 	transition table).
// */

// #ifndef DEFINITIONS_H
// #define DEFINITIONS_H

// #include <stdlib.h>
// #include <stdio.h>
// #include <unordered_map>
// #include <unordered_set>
// #include <vector>
// #include "input.h"

// namespace FSM 
// {
// 	// @Brief The class @MappingRule defines the mapping rule about transforming 
// 	// the possible appear char-type input characters into an integer.   	
// 	// const int ASCII256 = 256;	// The current rule range
// 	// class MappingRule
// 	// {
// 	// public:
// 	// 	MappingRule();
// 	// 	MappingRule(int* rules, int size);
// 	// 	~MappingRule();
		
// 	// 	// Transform the char type character to int type
// 	// 	int char2Int(unsigned char character) const;
// 	// 	void printRules() const;
// 	// 	int ruleSize() const;

// 	// 	// @Brief Provide @ruleKey to denote the rules aim to set, 
// 	// 	// which is current supported to be dna/div/protn/evenodd. And the 
// 	// 	// other cases will be consider as default one, i.e., ASCII256. 
// 	// 	static MappingRule* defineMappingRule();
// 	// 	static MappingRule* defineMappingRule(char* ruleKey);
// 	// private:
// 	// 	// @Brief character mapping rules, i.e., rules about transforming 
// 	// 	// char-type character into int-type integer.
// 	// 	int* mRules;
// 	// 	// the number of real effective (i.e., possible appear) characters
// 	// 	int mSize;
// 	// };

// 	// // @Brief The class @Input is used to store the necessary input contents 
// 	// // where DFAs execute on
// 	// class  Input
// 	// {
// 	// public:
// 	// 	Input();
// 	// 	Input(int* inputsPointer, long inputSize);	
// 	// 	Input(unsigned char* inputsPointer, long inputSize);	
// 	// 	~Input();

// 	// 	// @brief Provide @inputFileName to access the target inputs, 
// 	// 	// and @ruleUsed to denote the possible appear characters and 
// 	// 	// their mapping rule, then return an @Input type object  
// 	// 	static Input* readFromFile(const char* inputFileName, 
// 	// 		const MappingRule* ruleUsed, int type);

// 	// 	static Input* readFromFile_fabricate1200B_basedOn10M(const char* inputFileName, 
// 	// 		const MappingRule* ruleUsed, int type);
			
// 	// 	static Input* readFromFile_fabricate_94X_largerInput(const char* inputFileName, 
// 	// 		const MappingRule* ruleUsed, int type);

// 	// 	static Input* randomInput(const long size, const MappingRule* ruleUsed, int type);		
		
// 	// 	int* getPointer_INT() const;
// 	// 	unsigned char* getPointer_CHAR() const;
// 	// 	long getLength() const;
// 	// 	int getCharacter_INT(int index) const;
// 	// 	unsigned char getCharacter_CHAR(int index) const;
// 	// 	void printCurInputType() const;
// 	// 	void setLength(long ml);

// 	// private:
// 	// 	// the pointer to the executing inputs, it should follow aligned (32)
// 	// 	int mCharacterType; // 0: unknown, 1: int, 2: char
// 	// 	int* mPointer_INT __attribute__ ((aligned (32)));
// 	// 	unsigned char* mPointer_CHAR __attribute__ ((aligned (32)));
// 	// 	long mSize;
// 	// };

// 	// @Brief The class @Table is used to describe the transition table in DFA
// 	class Table
// 	{
// 	public:
// 		Table();
// 		Table(int* transTable, int nState, int nSymbol, int startState, std::vector<int> acceptVec);
// 		~Table();

// 		// @brief Providing @tableFileName to access the given transition table, 
// 		// with using @acceptFileName to mark the accept state in the table.
// 		// And applying required start state @startState while also define the mapping rule @ruleUsed, 
// 		// then return an @Table-type object  
// 		static Table* readFromFile(const char* tableFileName, const char* acceptFileName, 
// 			const int startState, const MappingRule* ruleUsed);

// 		int* getTable() const;
// 		int getNumState() const;
// 		int getNumSymbol() const;
// 		int getStartState() const;
// 		void printTable() const;
// 		bool isAccept(int stateID) const;

// 		void transpose(); // Transpose the row-major Table

// 	private:
// 		int* mTableList __attribute__ ((aligned (32)));
// 		int mNumState;
// 		int mNumSymbol;
// 		int mStartState;
// 		//std::unordered_set<int> mAcceptStates;
// 		bool* mAcceptStates;
// 	};


// }	// end of namespace FSM

// #endif // DEFINITIONS_H
