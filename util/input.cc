#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <cstring>
#include <string>
#include <cctype>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <map>
#include <algorithm>  
#include <iterator>
#include <time.h>
#include <cassert>
#include <unordered_map>

#include "input.h"

using namespace std;

namespace FSM
{

/* ---------class MappingRule------------ */

	MappingRule::MappingRule()
	{
		mRules = new uint8_t [ASCII256];
		mAlphabet = ASCII256;		
		for (int i = 0; i<ASCII256; i++)
			mRules[i] = i;
	}

	MappingRule::MappingRule(uint8_t* rules, int size)
	{
		mRules = new uint8_t [ASCII256];
		mAlphabet = size;
		for (uint8_t i = 0; i<ASCII256; i++)
			mRules[i] = rules[i];
	}

	MappingRule::~MappingRule()
	{
		delete []mRules;
	}

	MappingRule* MappingRule::defineMappingRule()
	{
		MappingRule* obj = new MappingRule();
		return obj;
	}

	MappingRule* MappingRule::defineMappingRule(char* ruleKey)
	{
		// Transform the input parameter *ruleKey* into low cases 
		char* ruleKeyLow;
		ruleKeyLow = new char [strlen(ruleKey)];
		for (int i = 0; i < strlen(ruleKey); ++i)
	    	ruleKeyLow[i] = tolower(ruleKey[i]);

		// Currently allow size of 256
		uint8_t* tempRules;
		int tempSize;

		// Initial tempRules as 0
		tempRules = new uint8_t [ASCII256]();
		
		if (ruleKeyLow == std::string("dna"))
		{
			tempSize = 4;
			tempRules['A'] = 0;
			tempRules['T'] = 1;
			tempRules['G'] = 2;
			tempRules['C'] = 3;
		}
		else if (ruleKeyLow == std::string("protn") )
		{
			tempSize = 20;
			tempRules['A'] = 0;
			tempRules['C'] = 1;
			tempRules['D'] = 2;
			tempRules['E'] = 3;
			tempRules['F'] = 4;
			tempRules['G'] = 5;
			tempRules['H'] = 6;
			tempRules['I'] = 7;
			tempRules['K'] = 8;
			tempRules['L'] = 9;
			tempRules['M'] = 10;
			tempRules['N'] = 11;
			tempRules['P'] = 12;
			tempRules['Q'] = 13;
			tempRules['R'] = 14;
			tempRules['S'] = 15;
			tempRules['T'] = 16;
			tempRules['V'] = 17;
			tempRules['W'] = 18;
			tempRules['Y'] = 19;
		}
		else if (ruleKeyLow == std::string("evenodd") )
		{
			tempSize = 4;
			tempRules['a'] = 0;
			tempRules['b'] = 1;
			tempRules['c'] = 2;
			tempRules['d'] = 3;			
		}
		else if (ruleKeyLow == std::string("div") )
		{
			tempSize = 2;
			tempRules['0'] = 0;
			tempRules['1'] = 1;			
		}
		else if (ruleKeyLow == std::string("comment") )
		{
			tempSize = 3;
			tempRules['/'] = 0;
			tempRules['*'] = 1;
			tempRules['X'] = 2;
		}
		else
		{
			tempSize = ASCII256;
			for (int i = 0; i<tempSize; i++)
				tempRules[i] = i;
		}

		MappingRule* obj = new MappingRule(tempRules, tempSize);
		return obj;
	}

	uint8_t MappingRule::char2Int(unsigned char character) const
	{
		uint8_t temp;
		if (character < 0 || character >= ASCII256)
			temp = 0;
		else
			temp = mRules[character];
		return temp;
	}

	int MappingRule::ruleSize() const
	{
		return mAlphabet;
	}

	void MappingRule::printRules() const
	{
		int it;
		for (it = 0; it < ASCII256; it++)
			cout << (char)it << " : " << mRules[it] << endl;
		cout << endl;
	}


/* ---------class Input------------ */

	Input::Input()
	{
		mHostData = NULL;
		mLength = 0;
	}

	Input::Input(uint8_t* inputsPointer, long inputSize)
	{
		mHostData = inputsPointer;
		mLength = inputSize;
	}

	Input::~Input()
	{
		delete []mHostData;
		mLength = 0;
	}

	Input* Input::readFromFile(string inputFileName, const MappingRule* ruleUsed)
	{
		uint8_t* inputs_ __attribute__ ((aligned (32)));
		long length_;

    	// open the file:
    	ifstream file(inputFileName, std::ios::binary);
    	if(file.fail())
    	{
        	if(errno == ENOENT) 
        	{
            	cout<< " Error: no such input file." << endl;
            	exit(-1);
        	}
    	}

    	// get its size:
    	std::streampos fileSize;

    	file.seekg(0, std::ios::end);
    	fileSize = file.tellg();
    	file.seekg(0, ios::beg);

    	// Stop eating new lines in binary mode!!!
    	file.unsetf(std::ios::skipws);

    	// reserve capacity
    	std::vector<unsigned char> vec;
    	vec.reserve(fileSize);

    	// read the data:
    	vec.insert(vec.begin(),
               std::istream_iterator<unsigned char>(file),
               std::istream_iterator<unsigned char>());		

    	length_ = vec.size();
    	inputs_ = new uint8_t [length_]();
    	long counter;
    	if (ruleUsed->ruleSize() == 256)
    	{
    		for (counter = 0; counter < length_; counter++)
    			inputs_[counter] = vec[counter];
    	}
    	else
    	{
    		for (counter = 0; counter < length_; counter++)
    			inputs_[counter] = ruleUsed->char2Int(vec[counter]);		
    	}

	    Input *object = new Input(inputs_, length_);
	    return object;
	}

	Input* Input::readFromFile(string inputFileName)
	{
		uint8_t* inputs_ __attribute__ ((aligned (32)));
		long length_;

		MappingRule* ruleUsed = new MappingRule();

    	// open the file:
    	ifstream file(inputFileName, std::ios::binary);
    	if(file.fail())
    	{
        	if(errno == ENOENT) 
        	{
            	cout<< " Error: no such input file." << endl;
            	exit(-1);
        	}
    	}

    	// get its size:
    	std::streampos fileSize;

    	file.seekg(0, std::ios::end);
    	fileSize = file.tellg();
    	file.seekg(0, ios::beg);

    	// Stop eating new lines in binary mode!!!
    	file.unsetf(std::ios::skipws);

    	// reserve capacity
    	std::vector<unsigned char> vec;
    	vec.reserve(fileSize);

    	// read the data:
    	vec.insert(vec.begin(),
               std::istream_iterator<unsigned char>(file),
               std::istream_iterator<unsigned char>());		

    	length_ = vec.size();
    	inputs_ = new uint8_t [length_]();
    	long counter;
    	if (ruleUsed->ruleSize() == 256)
    	{
    		for (counter = 0; counter < length_; counter++)
    			inputs_[counter] = vec[counter];
    	}
    	else
    	{
    		for (counter = 0; counter < length_; counter++)
    			inputs_[counter] = ruleUsed->char2Int(vec[counter]);		
    	}

	    Input *object = new Input(inputs_, length_);
	    return object;
	}


	Input* Input::randomInput(const long size, const MappingRule* ruleUsed)
	{
		srand((time(NULL)));
		uint8_t* inputs_ __attribute__ ((aligned (32)));
		inputs_ = new uint8_t [size];

		for (long i = 0; i < size; i++)
			inputs_[i] = rand() % (ruleUsed->ruleSize());

	    Input *object = new Input(inputs_, size);
	    return object;
	}	

	uint8_t* Input::getHostPointer() const
	{
		return mHostData;
	}

	long Input::getLength() const
	{
		return mLength;
	}
	 
	void Input::setLength(long setL)
	{
		mLength = setL;
	}

	uint8_t Input::getCharacter(int index) const 
	{
		if (index < 0 || index > mLength)
			return 0;
		else
			return mHostData[index];
	}

}	// End of namespace microspec
