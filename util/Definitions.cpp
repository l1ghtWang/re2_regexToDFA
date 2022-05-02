// #include <iostream>

// // mmap system
// #include <sys/mman.h>
// #include <fcntl.h>
// #include <sys/stat.h>
// #include <sys/types.h>
// #include <unistd.h>

// #include <stdio.h>
// #include <stdlib.h>
// #include <fstream>
// #include <cstring>
// #include <string>
// #include <cctype>
// #include <sstream>
// #include <vector>
// #include <stdexcept>
// #include <map>
// #include <algorithm> 
// #include <iterator> 
// #include <time.h>
// #include <unordered_map>

// #include "Definitions.h"

// using namespace std;

// namespace FSM
// {

// /* ---------class MappingRule------------ */

// 	MappingRule::MappingRule()
// 	{
// 		mRules = new int [ASCII256];
// 		mSize = ASCII256;		
// 		for (int i = 0; i<ASCII256; i++)
// 			mRules[i] = i;
// 	}

// 	MappingRule::MappingRule(int* rules, int size)
// 	{
// 		mRules = new int [ASCII256];
// 		mSize = size;
// 		for (int i = 0; i<ASCII256; i++)
// 			mRules[i] = rules[i];
// 	}

// 	MappingRule::~MappingRule()
// 	{
// 		delete []mRules;
// 	}

// 	MappingRule* MappingRule::defineMappingRule()
// 	{
// 		MappingRule* obj = new MappingRule();
// 		return obj;
// 	}

// 	MappingRule* MappingRule::defineMappingRule(char* ruleKey)
// 	{
// 		// Transform teh input parameter *ruleKey* into low cases 
// 		char* ruleKeyLow;
// 		ruleKeyLow = new char [strlen(ruleKey)];
// 		for (int i = 0; i < strlen(ruleKey); ++i)
// 	    	ruleKeyLow[i] = tolower(ruleKey[i]);

// 		// Currently allow size of 256
// 		int* tempRules;
// 		int tempSize;

// 		// Initial tempRules as 0 
// 		tempRules = new int [ASCII256]();

// 		if (ruleKeyLow == std::string("dna"))
// 		{
// 			tempSize = 4;
// 			tempRules['A'] = 0;
// 			tempRules['T'] = 1;
// 			tempRules['G'] = 2;
// 			tempRules['C'] = 3;
// 		}
// 		else if (ruleKeyLow == std::string("protn") )
// 		{
// 			tempSize = 20;
// 			tempRules['A'] = 0;
// 			tempRules['C'] = 1;
// 			tempRules['D'] = 2;
// 			tempRules['E'] = 3;
// 			tempRules['F'] = 4;
// 			tempRules['G'] = 5;
// 			tempRules['H'] = 6;
// 			tempRules['I'] = 7;
// 			tempRules['K'] = 8;
// 			tempRules['L'] = 9;
// 			tempRules['M'] = 10;
// 			tempRules['N'] = 11;
// 			tempRules['P'] = 12;
// 			tempRules['Q'] = 13;
// 			tempRules['R'] = 14;
// 			tempRules['S'] = 15;
// 			tempRules['T'] = 16;
// 			tempRules['V'] = 17;
// 			tempRules['W'] = 18;
// 			tempRules['Y'] = 19;
// 		}
// 		else if (ruleKeyLow == std::string("evenodd") )
// 		{
// 			tempSize = 4;
// 			tempRules['a'] = 0;
// 			tempRules['b'] = 1;
// 			tempRules['c'] = 2;
// 			tempRules['d'] = 3;			
// 		}
// 		else if (ruleKeyLow == std::string("div") )
// 		{
// 			tempSize = 2;
// 			tempRules['0'] = 0;
// 			tempRules['1'] = 1;			
// 		}
// 		else if (ruleKeyLow == std::string("comment") )
// 		{
// 			tempSize = 3;
// 			tempRules['/'] = 0;
// 			tempRules['*'] = 1;
// 			tempRules['X'] = 2;
// 		}
// 		else
// 		{
// 			tempSize = ASCII256;
// 			for (int i = 0; i<tempSize; i++)
// 			{
// 				tempRules[i] = i;
// 			}
// 		}

// 		MappingRule* obj = new MappingRule(tempRules, tempSize);
// 		return obj;
// 	}

// 	int MappingRule::char2Int(unsigned char character) const
// 	{
// 		int temp;
// 		if (character < 0 || character >= ASCII256)
// 			temp = 0;
// 		else
// 			temp = mRules[character];
// 		return temp;
// 	}

// 	int MappingRule::ruleSize() const
// 	{
// 		return mSize;
// 	}

// 	void MappingRule::printRules() const
// 	{
// 		int it;
// 		for (it = 0; it < ASCII256; it++)
// 			cout << (char)it << " : " << mRules[it] << endl;
// 		cout << endl;
// 	}


// /* ---------class Input------------ */

// 	Input::Input()
// 	{
// 		mCharacterType = 0; 
// 		mPointer_INT = NULL;
// 		mPointer_CHAR = NULL;
// 		mSize = 0;
// 	}

// 	Input::Input(int* inputsPointer, long inputSize)
// 	{
// 		mCharacterType = 1; 
// 		mPointer_INT = inputsPointer;
// 		mPointer_CHAR = NULL;
// 		mSize = inputSize;
// 	}

// 	Input::Input(unsigned char* inputsPointer, long inputSize)
// 	{
// 		mCharacterType = 2; 
// 		mPointer_INT = NULL;
// 		mPointer_CHAR = inputsPointer;
// 		mSize = inputSize;
// 	}

// 	Input::~Input()
// 	{
// 		if (mPointer_INT != NULL)
// 			delete []mPointer_INT;
// 		if (mPointer_CHAR != NULL)
// 			delete []mPointer_CHAR;		
// 		mSize = 0;
// 	}

// 	Input* Input::readFromFile(const char* inputFileName, const MappingRule* ruleUsed, int type)
// 	{
// 		int* _inputs_INT __attribute__ ((aligned (32)));
// 		unsigned char* _inputs_CHAR __attribute__ ((aligned (32)));

// 		long length_;
// 		unsigned char* inputs_Char;

// 		// Open the input file
// 		struct stat infile_sb;
// 		int fdSrc = open(inputFileName, O_RDONLY);
// 	    if (fdSrc == -1 || fstat(fdSrc, &infile_sb) == -1 )
// 	    {
// 	        cout << "Error: cannot open " << inputFileName << " for processing. Skipped." << endl;
// 	        return NULL;
// 	    }	
// 	    length_ = infile_sb.st_size;

// 	    // Based on the char input, making them to be input
// 	    if (length_ == 0)
// 	    {
// 	    	cout << "Nothing In the Input File ... " << endl;
// 	    	return NULL;
// 	    }
// 	    else
// 	    {
// 	    	inputs_Char = (unsigned char *) mmap(NULL, length_, PROT_READ, MAP_PRIVATE, fdSrc, 0);
// 	        if (inputs_Char == MAP_FAILED)
// 	        {
// 	            if (errno ==  ENOMEM)
// 	                cout << "Error:  mmap of " << inputFileName << " failed: out of memory\n";
// 	            else
// 	                cout << "Error: mmap of " << inputFileName << " failed with errno " << errno << ". Skipped.\n";
// 	            return NULL;
// 	        }	    	

// 	        if (type == 2)
// 	        {
// 			    _inputs_CHAR = new unsigned char [length_];
// 			    for (int l = 0; l < length_; l++)
// 			    	_inputs_CHAR[l] = (unsigned char)ruleUsed->char2Int(inputs_Char[l]);
// 			}
// 	        else
// 	        {
// 			    _inputs_INT = new int [length_];
// 			    for (int l = 0; l < length_; l++)
// 			    	_inputs_INT[l] = ruleUsed->char2Int(inputs_Char[l]);	        	
// 	        }

// 		    // mmap and input file close
// 	    	munmap((void *)inputs_Char, length_);
// 	    	close(fdSrc);
// 	    }

// 	    if (type == 2)
// 	    {
// 		    Input *object_char = new Input(_inputs_CHAR, length_);
// 		    return object_char;
// 	    }
// 	    else
// 	    {
// 		    Input *object_int = new Input(_inputs_INT, length_);
// 		    return object_int;	    	
// 	    }
// 	}

// 	Input* Input::readFromFile_fabricate1200B_basedOn10M(const char* inputFileName, const MappingRule* ruleUsed, int type)
// 	{
// 		int* _inputs_INT __attribute__ ((aligned (32)));
// 		unsigned char* _inputs_CHAR __attribute__ ((aligned (32)));

// 		long length_;
// 		unsigned char* inputs_Char;

// 		// Open the input file
// 		struct stat infile_sb;
// 		int fdSrc = open(inputFileName, O_RDONLY);
// 	    if (fdSrc == -1 || fstat(fdSrc, &infile_sb) == -1 )
// 	    {
// 	        cout << "Error: cannot open " << inputFileName << " for processing. Skipped." << endl;
// 	        return NULL;
// 	    }	
// 	    length_ = infile_sb.st_size;

// 	    // Based on the char input, making them to be input
// 	    if (length_ == 0)
// 	    {
// 	    	cout << "Nothing In the Input File ... " << endl;
// 	    	return NULL;
// 	    }
// 	    else
// 	    {
// 	    	inputs_Char = (unsigned char *) mmap(NULL, length_, PROT_READ, MAP_PRIVATE, fdSrc, 0);
// 	        if (inputs_Char == MAP_FAILED)
// 	        {
// 	            if (errno ==  ENOMEM)
// 	                cout << "Error:  mmap of " << inputFileName << " failed: out of memory\n";
// 	            else
// 	                cout << "Error: mmap of " << inputFileName << " failed with errno " << errno << ". Skipped.\n";
// 	            return NULL;
// 	        }	    	

// 			int extendFactor = 120;
// 	        if (type == 2)
// 	        {
// 			    _inputs_CHAR = new unsigned char [length_ * extendFactor];
// 				long inputLoc = 0;
// 				for (int randCount = 0; randCount < (extendFactor-1); randCount++)
// 				{
// 					long randStartPoint = rand()%length_;
// 					long readCound = 0;
// 					while (readCound < length_)
// 					{
// 						if(randStartPoint == length_)
// 							randStartPoint = 0;
// 			    		_inputs_CHAR[inputLoc] = (unsigned char)ruleUsed->char2Int(inputs_Char[randStartPoint]);
// 						inputLoc += 1;
// 						readCound += 1;
// 					}
// 				}
			    
// 			}
// 	        else
// 	        {

// 				_inputs_INT = new int [length_ * extendFactor];
// 				long inputLoc = 0;
// 				for (int randCount = 0; randCount < (extendFactor-1); randCount++)
// 				{
// 					long randStartPoint = rand()%length_;
// 					long readCound = 0;
// 					while (readCound < length_)
// 					{
// 						if(randStartPoint == length_)
// 							randStartPoint = 0;
// 			    		_inputs_INT[inputLoc] = (unsigned char)ruleUsed->char2Int(inputs_Char[randStartPoint]);
// 						inputLoc += 1;
// 						readCound += 1;
// 					}
// 				}	        	
// 	        }

// 		    // mmap and input file close
// 	    	munmap((void *)inputs_Char, length_);
// 	    	close(fdSrc);

// 			length_ = length_ * extendFactor;
// 	    }

// 	    if (type == 2)
// 	    {
// 		    Input *object_char = new Input(_inputs_CHAR, length_);
// 		    return object_char;
// 	    }
// 	    else
// 	    {
// 		    Input *object_int = new Input(_inputs_INT, length_);
// 		    return object_int;	    	
// 	    }
// 	}

// 	Input* Input::readFromFile_fabricate_94X_largerInput(const char* inputFileName, const MappingRule* ruleUsed, int type)
// 	{
// 		int* _inputs_INT __attribute__ ((aligned (32)));
// 		unsigned char* _inputs_CHAR __attribute__ ((aligned (32)));

// 		long length_;
// 		unsigned char* inputs_Char;

// 		// Open the input file
// 		struct stat infile_sb;
// 		int fdSrc = open(inputFileName, O_RDONLY);
// 	    if (fdSrc == -1 || fstat(fdSrc, &infile_sb) == -1 )
// 	    {
// 	        cout << "Error: cannot open " << inputFileName << " for processing. Skipped." << endl;
// 	        return NULL;
// 	    }	
// 	    length_ = infile_sb.st_size;

// 	    // Based on the char input, making them to be input
// 	    if (length_ == 0)
// 	    {
// 	    	cout << "Nothing In the Input File ... " << endl;
// 	    	return NULL;
// 	    }
// 	    else
// 	    {
// 	    	inputs_Char = (unsigned char *) mmap(NULL, length_, PROT_READ, MAP_PRIVATE, fdSrc, 0);
// 	        if (inputs_Char == MAP_FAILED)
// 	        {
// 	            if (errno ==  ENOMEM)
// 	                cout << "Error:  mmap of " << inputFileName << " failed: out of memory\n";
// 	            else
// 	                cout << "Error: mmap of " << inputFileName << " failed with errno " << errno << ". Skipped.\n";
// 	            return NULL;
// 	        }	    	

// 			int extendFactor = 94;
// 	        if (type == 2)
// 	        {
// 			    _inputs_CHAR = new unsigned char [length_ * extendFactor];
// 				long inputLoc = 0;
// 				for (int randCount = 0; randCount < (extendFactor-1); randCount++)
// 				{
// 					long randStartPoint = rand()%length_;
// 					long readCound = 0;
// 					while (readCound < length_)
// 					{
// 						if(randStartPoint == length_)
// 							randStartPoint = 0;
// 			    		_inputs_CHAR[inputLoc] = (unsigned char)ruleUsed->char2Int(inputs_Char[randStartPoint]);
// 						inputLoc += 1;
// 						readCound += 1;
// 					}
// 				}
			    
// 			}
// 	        else
// 	        {

// 				_inputs_INT = new int [length_ * extendFactor];
// 				long inputLoc = 0;
// 				for (int randCount = 0; randCount < (extendFactor-1); randCount++)
// 				{
// 					long randStartPoint = rand()%length_;
// 					long readCound = 0;
// 					while (readCound < length_)
// 					{
// 						if(randStartPoint == length_)
// 							randStartPoint = 0;
// 			    		_inputs_INT[inputLoc] = (unsigned char)ruleUsed->char2Int(inputs_Char[randStartPoint]);
// 						inputLoc += 1;
// 						readCound += 1;
// 					}
// 				}	        	
// 	        }

// 		    // mmap and input file close
// 	    	munmap((void *)inputs_Char, length_);
// 	    	close(fdSrc);

// 			length_ = length_ * extendFactor;
// 	    }

// 	    if (type == 2)
// 	    {
// 		    Input *object_char = new Input(_inputs_CHAR, length_);
// 		    return object_char;
// 	    }
// 	    else
// 	    {
// 		    Input *object_int = new Input(_inputs_INT, length_);
// 		    return object_int;	    	
// 	    }
// 	}

// 	Input* Input::randomInput(const long size, const MappingRule* ruleUsed, int type)
// 	{
// 		srand((time(NULL)));
// 		if (type == 2)
// 		{
// 			unsigned char* _inputs_char __attribute__ ((aligned (32)));
// 			_inputs_char = new unsigned char [size];
// 			for (long i = 0; i < size; i++)
// 				_inputs_char[i] = (unsigned char) (rand() % (ruleUsed->ruleSize()));

// 		    Input *object_char = new Input(_inputs_char, size);
// 		    return object_char;
// 		}
// 		else
// 		{
// 			int* inputs_ __attribute__ ((aligned (32)));
// 			inputs_ = new int [size];

// 			for (long i = 0; i < size; i++)
// 				inputs_[i] = rand() % (ruleUsed->ruleSize());

// 		    Input *object = new Input(inputs_, size);
// 		    return object;			
// 		}
// 	}	

// 	int* Input::getPointer_INT() const
// 	{
// 		return mPointer_INT;
// 	}

// 	int Input::getCharacter_INT(int index) const 
// 	{
// 		if (index < 0 || index > mSize)
// 			return 0;
// 		else
// 			return mPointer_INT[index];
// 	}

// 	unsigned char* Input::getPointer_CHAR() const
// 	{
// 		return mPointer_CHAR;
// 	}

// 	unsigned char Input::getCharacter_CHAR(int index) const 
// 	{
// 		if (index < 0 || index > mSize)
// 			return 0;
// 		else
// 			return mPointer_CHAR[index];
// 	}

// 	long Input::getLength() const
// 	{
// 		return mSize;
// 	}

// 	void Input::setLength(long ml)
// 	{
// 		mSize = ml;
// 	}

// 	void Input::printCurInputType() const
// 	{
// 		if (mCharacterType == 1)
// 			printf("INT ARRAY !\n");
// 		else if (mCharacterType == 2)
// 			printf("CHAR ARRAY !\n");
// 		else
// 			printf("Unknown, but we keep INT ARRAY !\n");
// 	}

// /* ---------class Table------------ */

// 	Table::Table()
// 	{
// 		mTableList = NULL;
// 		mNumState = 0;
// 		mNumSymbol = 0;
// 		mStartState = 0;
// 		mAcceptStates = NULL;
// 	} 

// 	Table::Table(int* list, int nstate, int nsymbol, int s, std::vector<int> acceptVec)
// 	{
// 		mTableList = list;
// 		mNumState = nstate;
// 		mNumSymbol = nsymbol;
// 		mStartState = s;

// 		mAcceptStates = new bool [nstate]();
// 		for (std::vector<int>::iterator it = acceptVec.begin() ; it != acceptVec.end(); it++)
// 			//mAcceptStates.insert(*it);
// 			mAcceptStates[*it] = true;

// 	}

// 	Table::~Table()
// 	{
// 		if (mTableList != NULL)
// 			delete []mTableList;

// 		//mAcceptStates.clear();
// 		if (mAcceptStates != NULL)
// 			delete []mAcceptStates;

// 		mNumState = 0;
// 		mNumSymbol = 0;
// 		mStartState = 0;
// 	}

// 	Table* Table::readFromFile(const char* tableFile, const char* acceptFile, 
// 			const int s, const MappingRule* ruleset)
// 	{	
// 		int* list_  __attribute__ ((aligned (32)));
// 		int nstate_;
// 		int nsymbol_;

// 		// Loading the accept states from **acceptFile**
// 		vector<int> acceptVec;
// 		ifstream in_ac;
// 		in_ac.open(acceptFile);
// 		if (in_ac.is_open())
// 		{
// 			while(in_ac)
// 			{
// 				int temp_ac;
// 				in_ac >> temp_ac;
// 				acceptVec.push_back(temp_ac);
// 			}
// 			in_ac.close();
// 		}
// 		else
// 		{
// 			cout << "Fail to open Accept file " << acceptFile << endl;
// 			return NULL;
// 		}

// 		// Loading the transition table from the **tableFile**
// 		int MAXSYMBOL = ruleset->ruleSize();
// 		vector<int> vecTable;
// 		ifstream in_table;
// 		in_table.open(tableFile);
// 		if (in_table.is_open())
// 		{
// 			string line;
//     		while(!in_table.eof())
//     		{
//         		getline(in_table,line);
//         		if(in_table.fail())
//             		break;
//         		if (line.size() > 2)
//         		{
//         			int currentLineNum = 0;
//         			stringstream stream(line);
//         			while(stream && currentLineNum != MAXSYMBOL)
//         			{
//         				int temp_n;
//         				stream >> temp_n;
//         				currentLineNum++;
//         				// No need to store Accept State 
//         				// if (find (acceptVec.begin(), acceptVec.end(), temp_n) != acceptVec.end())
//         				// 	temp_n = temp_n | 0XF0000000;
//         				// else
//         				// 	temp_n = temp_n & 0X0FFFFFFF;
//         				vecTable.push_back(temp_n);
//     				}
//     				if (currentLineNum != MAXSYMBOL)
//     				{
//     					cout << "Number of Symbol does not match with current setting \n";
//     					return NULL;
//     				}
//         		}
//     		}
// 		}
// 		else
// 		{
// 			cout << "Fail to open Table file " << tableFile << endl;
// 			return NULL;
// 		}

// 		list_ = new int [(int)vecTable.size()];
// 		nsymbol_ = MAXSYMBOL;
// 		nstate_ = ((int)vecTable.size()) / nsymbol_;

// 		for (int i = 0 ; i < vecTable.size(); i++)
// 			list_[i] = vecTable[i];

// 	    Table *object = new Table(list_, nstate_, nsymbol_, s, acceptVec);
// 	    return object;		
// 	}

// 	int* Table::getTable() const
// 	{
// 		return mTableList;
// 	}

// 	int Table::getNumState() const
// 	{
// 		return mNumState;
// 	}
// 	int Table::getNumSymbol() const
// 	{
// 		return mNumSymbol;
// 	}

// 	int Table::getStartState() const
// 	{
// 		return mStartState;
// 	}

// 	bool Table::isAccept(int stateID) const
// 	{
// 		//return (mAcceptStates.count(stateID));
// 		return mAcceptStates[stateID];
// 	}	

// 	void Table::printTable() const
// 	{
// 		// Print Table in two dimentions
// 		for (int i = 0; i < mNumState * mNumSymbol; i++)
// 		{
// 			cout << mTableList[i] << " ";
// 			if ((i+1) % mNumSymbol == 0)
// 				cout << endl;
// 		}
// 		cout << endl;
// 		cout << "#State " << this->getNumState() << ", #Symbol " << this->getNumSymbol() << endl;
		
// 		// Print All Accept States
// 		cout << "Accept States include: " << endl;
// 		for (int j = 0; j < this->getNumState(); j++)
// 			if (this->isAccept(j))
// 				cout << j << " ";
// 		cout << endl;
// 	}

// }	// End of namespace microspec
