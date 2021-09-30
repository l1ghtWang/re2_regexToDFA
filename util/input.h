#ifndef INPUT_H_
#define INPUT_H_

#include <stdlib.h>
#include <stdio.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <set>

using namespace std;

namespace FSM 
{
	// @Brief The class @MappingRule defines the mapping rule about transforming 
	// the possible appear char-type input characters into an integer.   	
	const int ASCII256 = 256;	// The current rule range
	class MappingRule
	{
	public:
		MappingRule();
		MappingRule(uint8_t* rules, int size);
		~MappingRule();
		
		// Transform the char type character to int type
		uint8_t char2Int(unsigned char character) const;
		void printRules() const;
		int ruleSize() const;

		// @Brief Provide @ruleKey to denote the rules aim to set, 
		// which is current supported to be dna/div/protn/evenodd. And the 
		// other cases will be consider as default one, i.e., ASCII256. 
		static MappingRule* defineMappingRule();
		static MappingRule* defineMappingRule(char* ruleKey);
	private:
		// @Brief character mapping rules, i.e., rules about transforming 
		// char-type character into int-type integer.
		uint8_t* mRules;
		// the number of real effective (i.e., possible appear) characters
		int mAlphabet;
	};

	// @Brief The class @Input is used to store the necessary input contents 
	// where DFAs execute on
	class  Input
	{
	public:
		Input();
		Input(uint8_t* inputsPointer, long inputSize);	
		~Input();

		// @Brief Provide @inputFileName to access the target inputs, 
		// and @ruleUsed to denote the possible appear characters and 
		// their mapping rule, then return an @Input type object  
		static Input* readFromFile(string inputFileName, 
			const MappingRule* ruleUsed);
		static Input* readFromFile(string inputFileName);		
		static Input* randomInput(const long size, const MappingRule* ruleUsed);		
		
		uint8_t* getHostPointer() const;
		long getLength() const;
		uint8_t getCharacter(int index) const;

		void setLength(long setL);

	private:
		// the pointer to the executing inputs, it should follow aligned (32)
		uint8_t* mHostData;
		long mLength;
	};

}	// end of namespace FSM

#endif // INPUT_H_