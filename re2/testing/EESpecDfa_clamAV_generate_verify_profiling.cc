// Copyright 2006-2008 The RE2 Authors.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <stdint.h>
#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <condition_variable>

// #include "util/inputFileName.h"
#include "util/fsm.h"
#include "util/input.h"
// #include "input.h"
// #include "util/Definitions.h"


#include "util/test.h"
#include "util/flags.h"
#include "util/logging.h"
#include "util/malloc_counter.h"
#include "util/strutil.h"
#include "re2/prog.h"
#include "re2/re2.h"
#include "re2/regexp.h"
#include "re2/testing/regexp_generator.h"
#include "re2/testing/string_generator.h"

#define INFI_CONV_LEN 10000000000

static const bool UsingMallocCounter = false;

DEFINE_FLAG(int, size, 8, "log2(number of DFA nodes)");
DEFINE_FLAG(int, repeat, 2, "Repetition count.");
DEFINE_FLAG(int, threads, 4, "number of threads");

namespace re2 {

static int state_cache_resets = 0;
static int search_failures = 0;

struct SetHooks {
  SetHooks() {
    hooks::SetDFAStateCacheResetHook([](const hooks::DFAStateCacheReset&) {
      ++state_cache_resets;
    });
    hooks::SetDFASearchFailureHook([](const hooks::DFASearchFailure&) {
      ++search_failures;
    });
  }
} set_hooks;

// Check that multithreaded access to DFA class works.

// Helper function: builds entire DFA for prog.
static void DoBuild(Prog* prog) {
  ASSERT_TRUE(prog->BuildEntireDFA(Prog::kFirstMatch, nullptr));
}

std::string readFileIntoString(const std::string& path) {
    std::ifstream input_file(path);
    if (!input_file.is_open()) {
        std::cerr << "Could not open the file - '" << path << "'" << std::endl;
        exit(EXIT_FAILURE);
    }
    return std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
}


struct CheckPoint_t
{
    int cur_state;
    long convergentLen;
};

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

	struct stateINFO
	{
		int state;
		long appearTimes;
	};
	bool compareByTimes(const stateINFO &a, const stateINFO &b)
	{
    	return a.appearTimes > b.appearTimes;
	}

	void spec_at(int numState, int numSymbol, std::vector<int>transTable, Input* objInput, long curIdx, std::vector<int> &ranked_states)
	{


		uint8_t* inputs = objInput->getHostPointer();

		long lookBackStartIndex = curIdx - LOOKBACK;

		long i, j;	// Loop iteration counter
		int* predict_states;
		int* times_counter;
		predict_states = new int [numState]();
		times_counter = new int [numState]();
		
		for (i=0; i < numState; i++)
			predict_states[i]=i;


		// Executing the look back and counting the number of appearance
		for(i=0; i < LOOKBACK; i++)
		{
			uint8_t symbol = inputs[lookBackStartIndex + i];
			for(j=0; j < numState; j++)
				predict_states[j] = ( transTable[predict_states[j] * numSymbol + symbol ] );
		}		

		// Counting the appearances
		for(i=0; i < numState; i++)
				times_counter[predict_states[i]]++;

		// Sorting the states based on frequency
		stateINFO* allStates;
		allStates = new stateINFO [numState];
		for (i = 0; i < numState; i++)
		{
			allStates[i].state = i;
			allStates[i].appearTimes = times_counter[i];	
		}
		sort(allStates, allStates+numState, compareByTimes);

		for (i = 0; i < numState; i++)
			ranked_states.push_back(allStates[i].state);

		delete []allStates;
		delete []times_counter;
		delete []predict_states;
	}	

    extern std::string regexFileName;
    std::string fsmType = "clamAV";
    std::string benchmarkPath = "/home/yugwang/re2_regexToDFA/ANMLZoo_regex/regex_EESpec/"+fsmType+"/";

    int dfa_baseIdx = 900;
    // std::string regexFileName = benchmarkPath+"regex"+std::to_string(dfaIdx)+".regex";
    std::string regexFileName = "/home/yugwang/re2_regexToDFA/ANMLZoo_regex/regex_EESpec/"+fsmType+"/"+fsmType+"_yug.regex";

    std::string inputFileName("/home/share/clamAV/clamAV_input_1200MB.in");

    std::string tableFileName;
    std::string acFileName;
    
	int num_10_8levelFSM = 0;
    int targetNum_10_8levelFSM = 20;
    int num_10_7levelFSM = 0;
    int targetNum_10_7levelFSM = 20;
    int num_10_6levelFSM = 0;
    int targetNum_10_6levelFSM = 20;
    int num_10_5levelFSM = 0;
    int targetNum_10_5levelFSM = 20;

    void writeTableAC_toFile(int DFA_index, int numState, int numSymbol, vector<int> table, vector<int> ac, float aveConvLength, long leastNumTableEntry_percent90TotalTransition, std::string combRegex)
    {
        tableFileName = benchmarkPath+fsmType+"_DFA"+to_string(DFA_index)+".table";
        cout<<"tableFileName: "<<tableFileName<<endl;
        acFileName = benchmarkPath+fsmType+"_AC"+to_string(DFA_index)+".txt";
		std::string reportName = benchmarkPath+fsmType+"_report"+to_string(DFA_index)+".txt";
        //write table to file
        std::ofstream tableFile;
        tableFile.open(tableFileName,ios::trunc);
        for(int i=0; i<numState; i++)
        {
            for(int j=0; j<numSymbol; j++)
            {
                // std::cout<<table[i*numSymbol+j]<<" ";
                tableFile<<to_string(table[i*numSymbol+j])+" ";
            }
            tableFile<<endl;
        }
        tableFile.close();
        std::ofstream acFile(acFileName, ios::trunc);
        for(int i=0; i<ac.size(); i++)
        {
            // std::cout<<ac[i]<<" ";
            acFile<<to_string(ac[i])+" ";
        }
        acFile.close();
		std::ofstream reportFile(reportName, ios::trunc);
		reportFile<<fsmType<<",";
		reportFile<<DFA_index<<",";
		reportFile<<numState<<",";
		reportFile<<leastNumTableEntry_percent90TotalTransition<<",";
		reportFile<<aveConvLength<<",";
		reportFile<<combRegex<<"\n";
		reportFile.close();
	
		cout<<fsmType<<",";
		cout<<DFA_index<<",";
		cout<<numState<<",";
		cout<<leastNumTableEntry_percent90TotalTransition<<",";
		cout<<aveConvLength<<",";
		cout<<combRegex<<"\n";
    }

void generateFSM_profiling_outputToFile(std::vector<std::string> regex_list, int num_profile_point, Input* inputs_){
	long length_ = inputs_->getLength();
	uint8_t* inputStream = inputs_->getHostPointer();
	
	std::string combRegex;
	for(int i=0; i<3; i++) {
		int idx = rand() % regex_list.size();
		combRegex += "("+regex_list[idx]+")|";
	}
	combRegex = combRegex.substr(0, combRegex.size()-1);
	std::cout << "combined regex: " << combRegex << std::endl;
	Regexp* re = Regexp::Parse(combRegex, Regexp::LikePerl|Regexp::Latin1, NULL);
	if(re == NULL)
		return;
	ASSERT_TRUE(re != NULL);

	int64_t memLimit = 256;
	memLimit = memLimit *1024*1024*1024;
	Prog* prog = re->CompileToProg(memLimit);

	std::vector<int> table;
	std::vector<int> ac;
	ASSERT_TRUE(prog != NULL);
	prog->BuildEntireDFA(Prog::kManyMatch, nullptr, table, ac);
	//   prog->BuildEntireDFA(Prog::kFirstMatch, nullptr, benchmarkPath, dfaIdx, tableFileName);
	
	int nSymbol_ = 256;
	int numStates = table.size()/256;
	std::cout<<"table.size(): "<<table.size()<<std::endl;
	std::cout<<"numStates: "<<numStates<<std::endl;
	if(numStates > 100000)
	{
		delete prog;
		re->Decref();
		return;
	}
	vector<long> rand_checkpoints;
	vector<CheckPoint_t> checkpoints_vec;
	
	int ground_true_state [num_profile_point];
	int correctCount = num_profile_point;
	for (int i = 0; i < num_profile_point; i++)
	{
		rand_checkpoints.push_back((long) (rand() % 10000)+10); 

	}
	sort(rand_checkpoints.begin(), rand_checkpoints.end());

	int state_ = 0;
	
	int cur_point = 0;

	long sumConvergenceLen = 0;
	long maxConvLen = 0;
	long minConvLen = INFI_CONV_LEN;
	vector<long> entryCounter(numStates*nSymbol_, 0);

	for (long i = 0; i < length_; i++)
	{

		if (cur_point < rand_checkpoints.size() && i == rand_checkpoints[cur_point])
		{
			vector<int> ranked_states;
			// Profiling the speculation accuracy
			spec_at(numStates, nSymbol_, table, inputs_, i, ranked_states);
			ground_true_state[cur_point] = state_;

			//add a checkpoint obj to cp_list
			if(state_ != ranked_states[0])
			{
				correctCount += -1;
				checkpoints_vec.push_back(CheckPoint_t());
				checkpoints_vec.back().convergentLen = 0;
				checkpoints_vec.back().cur_state = ranked_states[0];//get speculative state with highest possibility
			}

			cur_point++;
		}

		uint8_t data = inputStream[i];
		entryCounter[state_ * nSymbol_ + data] += 1;
		state_ = table[state_ * nSymbol_ + data];

		//all unconverged checkpoints transiting
		for(auto &checkpoint : checkpoints_vec)
		{
			checkpoint.cur_state = table[checkpoint.cur_state * nSymbol_ + data];
		}

		//check if checkpoint state converge to ground true state
		int index_checkpoint = 0;
		for(auto &checkpoint : checkpoints_vec)
		{
			if(state_ != checkpoint.cur_state)
			{
				checkpoint.convergentLen += 1;

				if(i == length_-1) // reach the end of input stream but not converge yet
				{
					sumConvergenceLen += INFI_CONV_LEN;
					if(maxConvLen < INFI_CONV_LEN)
						maxConvLen = INFI_CONV_LEN;
				}
			}
			else
			{
				sumConvergenceLen += checkpoint.convergentLen;
				if(maxConvLen < checkpoint.convergentLen)
					maxConvLen = checkpoint.convergentLen;
				if(minConvLen > checkpoint.convergentLen)
					minConvLen = checkpoint.convergentLen;
				checkpoints_vec.erase(checkpoints_vec.begin()+index_checkpoint);
				index_checkpoint -= 1;
			}
			index_checkpoint += 1;
		}
	}
	cout<<"correctCount: "<<correctCount<<endl;
	float averageConvergenceLen = ((num_profile_point-correctCount==0)? 0:1.0*sumConvergenceLen/(num_profile_point-correctCount));
	cout<<"average Recovery Len is "<< averageConvergenceLen <<endl;
	cout<<"Max Recovery Len is "<<maxConvLen<<endl;
	cout<<"Min Recovery Len is "<<minConvLen<<endl;

	sort(entryCounter.begin(), entryCounter.end(), greater<int>());
	long leastNumTableEntry_percent90TotalTransition = 0;
	long sumTemp = 0;
	auto it = entryCounter.begin();
	while( (sumTemp*1.0/length_) < 0.9)
	{
		sumTemp += *it;
		leastNumTableEntry_percent90TotalTransition++;
		++it;
	}
	cout<<"least Num Table Entry reaching 90% transition: "<<leastNumTableEntry_percent90TotalTransition<<endl;

	if(averageConvergenceLen > 50000000 && num_10_8levelFSM < targetNum_10_8levelFSM)
	{
		int dfa_index = dfa_baseIdx+num_10_8levelFSM;
		writeTableAC_toFile(dfa_index, numStates, nSymbol_, table, ac, averageConvergenceLen,leastNumTableEntry_percent90TotalTransition, combRegex);
		num_10_8levelFSM += 1;
		cout<<"num_10_8levelFSM: "<<num_10_8levelFSM<<endl;
	}
	else if(averageConvergenceLen <= 50000000 && averageConvergenceLen > 5000000 && num_10_7levelFSM < targetNum_10_7levelFSM)
	{
		
		int dfa_index = dfa_baseIdx+targetNum_10_8levelFSM+num_10_7levelFSM;
		writeTableAC_toFile(dfa_index, numStates, nSymbol_, table, ac, averageConvergenceLen,leastNumTableEntry_percent90TotalTransition, combRegex);
		num_10_7levelFSM += 1;
		cout<<"num_10_7levelFSM: "<<num_10_7levelFSM<<endl;
	}
	else if(averageConvergenceLen <= 5000000 && averageConvergenceLen > 500000 && num_10_6levelFSM < targetNum_10_6levelFSM)
	{
		int dfa_index = dfa_baseIdx+targetNum_10_8levelFSM+targetNum_10_7levelFSM+num_10_6levelFSM;
		writeTableAC_toFile(dfa_index, numStates, nSymbol_, table, ac, averageConvergenceLen,leastNumTableEntry_percent90TotalTransition, combRegex);
		num_10_6levelFSM += 1;
		cout<<"num_10_6levelFSM: "<<num_10_6levelFSM<<endl;
	}
	else if(averageConvergenceLen <= 500000 && averageConvergenceLen > 50000 && num_10_5levelFSM < num_10_5levelFSM)
	{
		int dfa_index = dfa_baseIdx+targetNum_10_8levelFSM+targetNum_10_7levelFSM+targetNum_10_6levelFSM+num_10_5levelFSM;
		writeTableAC_toFile(dfa_index, numStates, nSymbol_, table, ac, averageConvergenceLen,leastNumTableEntry_percent90TotalTransition, combRegex);      
		num_10_5levelFSM += 1;
		cout<<"num_10_5levelFSM: "<<num_10_5levelFSM<<endl;
	}
	// tableFileName = benchmarkPath+fsmType+"_DFA"+to_string(dfa_baseIdx+num_10_8levelFSM)+".table";
	// cout<<"tableFileName: "<<tableFileName<<endl;
	// delete prog;
	delete prog;
	re->Decref();
}


void generateFSM_profiling_outputToFile_wrapper(std::vector<std::string> regex_list, int num_profile_point, Input* inputs_)
{
    std::mutex m;
    std::condition_variable cv;
    int retValue;

    std::thread t([&cv, &regex_list, &num_profile_point, &inputs_]() 
    {
        generateFSM_profiling_outputToFile(regex_list, num_profile_point, inputs_);
        cv.notify_one();
    });

    t.detach();

    {
        std::unique_lock<std::mutex> l(m);
        if(cv.wait_for(l,std::chrono::seconds(60)) == std::cv_status::timeout) 
            throw std::runtime_error("Timeout");
    }
 
}


TEST(SingleThreaded, BuildEntireDFA) {
    srand(time(NULL));
  		//read from console
	cin>>dfa_baseIdx;
	cout<<dfa_baseIdx<<endl;
	
	int num_profile_point = 10;
    MappingRule* rules_ = MappingRule::defineMappingRule();
    Input* inputs_ = Input::readFromFile(inputFileName, rules_);
    uint8_t* inputStream = inputs_->getHostPointer();
    long length_ = inputs_->getLength();


    //read regex from file, and generate larger regex
    std::ifstream regex_file(regexFileName);
    std::vector<std::string> regex_list;
    while(!regex_file.eof()) {
        std::string regex;
        regex_file >> regex;
        regex_list.push_back(regex);
    }

	int whileCounter = 0;
	while( num_10_8levelFSM < targetNum_10_8levelFSM || num_10_7levelFSM < targetNum_10_7levelFSM || num_10_6levelFSM < targetNum_10_6levelFSM || num_10_5levelFSM < targetNum_10_5levelFSM)
	{
		whileCounter+=1;
		if(whileCounter > 100)
		{
			cout<<"whileCounter > 100"<<endl;
			break;
		}
		cout<<"whileCounter: "<<whileCounter<<endl;
		bool timedout = false;
		try {
			generateFSM_profiling_outputToFile_wrapper(regex_list, num_profile_point, inputs_);
		}
		catch(std::runtime_error& e) {
			std::cout << e.what() << std::endl;
			timedout = true;
		}
		catch (...)
		{}

		if(!timedout)
			std::cout << "Success" << std::endl;
	}
	
    delete inputs_;
    delete rules_;

}

// TEST(SingleThreaded, checkhDFA) {
//   // CHECK(RE2::FullMatch("4a", "[0-9]"));
//   // CHECK(RE2::FullMatch("4a", "[0-9]"));


//   std::string pattern_str = readFileIntoString(regexFileName);
//   std::string input_str = readFileIntoString(inputFileName);
  
//   re2::RE2::Options myOption;
//   myOption.set_perl_classes(true);
//   myOption.set_one_line(true);
//   StringPiece pattern_stringPiece(pattern_str);
//   RE2 pattern(pattern_stringPiece,myOption);
// //   Regexp* re = Regexp::Parse(pattern_str, Regexp::LikePerl, NULL);

//   // std::string contents = "aaabaaabaaa";
//   StringPiece input(input_str);

//   int matchCount = 0;
//   while (RE2::FindAndConsume(&input, pattern)) {
// //   while (RE2::FindAndConsume(&input, re)) {
//     matchCount+=1;
//   }
//   std::cout<<"number of match:"<<matchCount<<std::endl;
// }


using namespace FSM;

// TEST(SingleThreaded, ProfilingDFA) {

//     std::cout<<"tableFileName:"<<tableFileName<<std::endl;
//     int start = 0;

//     FSM::MappingRule* rules_ = FSM::MappingRule::defineMappingRule();
// 	Input* inputObj = Input::readFromFile(inputFileName, rules_);
//     long inputLength = inputObj->getLength(); 
//     std::cout<<inputLength<<std::endl;


//     Table* tableObj = Table::readFromFile(tableFileName.c_str(), acFileName.c_str(), start, rules_);
//     // tableObj->printTable();

//     uint8_t* inputPtr = inputObj->getHostPointer();
//     int* tableList = tableObj->getTable();
//     int state = start;
//     int nSymbol = tableObj->getNumSymbol();
//     int numState = tableObj->getNumState();
    

    
//     long acceptCount = 0;
//     // for(long i = 0; i < inputLength; i++)
//     // {
//     //     uint8_t data = inputPtr[i];
//     //     state = tableList[state * nSymbol + data];
//     //     if(tableObj->isAccept(state))
//     //         acceptCount+=1;
//     // }
//     // std::cout<<"acceptCount:"<<acceptCount<<std::endl;


//     int state_groundTrue = start;
//     int inputSegment = 10;
//     long inputSegmentLen = inputLength/inputSegment;
//     Predictor* predObj;
//     predObj = new Predictor();
//     int num_spec_state = 4;
//     predObj->startSpec(tableObj, inputObj, 1, inputSegment, num_spec_state);
//     num_spec_state = predObj->getNumSlots();
//     int* specList_h = predObj->getAllPreds();
//     int spec1_correctCount = 0;
//     int spec4_correctCount = 0;
//     int sumUniqueState_L2 = 0;
//     int sumUniqueState_L10 = 0;
//     int sumUniqueState_L100 = 0;
//     int sumUniqueState_L1000 = 0;
//     int sumUniqueState_L10000 = 0;
//     for(int segmentIdx = 0; segmentIdx < inputSegment; segmentIdx++)
//     {
//         long startInputPos = segmentIdx*inputSegmentLen;
//         long endInputPos = (segmentIdx+1)*inputSegmentLen-1;
        
//         std::vector<int> execPath;
//         for(int i = 0; i < numState; i++)
//             execPath.push_back(i);

//         // std::cout<<"inputSegment["<<segmentIdx<<"],#uniqueState(L=100),#uniqueState(L=1000),#uniqueState(L=10000)\n";
//         for(int curInputPos = startInputPos; curInputPos <= endInputPos; curInputPos++)
//         {
//             int numPath = execPath.size();
//             uint8_t data = inputPtr[curInputPos];
//             state_groundTrue = tableList[state_groundTrue * nSymbol + data];
//             for(int pathIdx = 0; pathIdx < numPath; pathIdx++)
//             {
//                 int curState = execPath[pathIdx];
//                 curState =  tableList[curState * nSymbol + data];
//                 execPath[pathIdx] = curState;
//             }

//             //check number of matchs
//             if(tableObj->isAccept(state_groundTrue))
//                 acceptCount+=1;
//             //check speculative accuracy
//             if(curInputPos==startInputPos && curInputPos!=0)
//             {
//                 if(state_groundTrue==specList_h[segmentIdx*num_spec_state])
//                 {
//                     spec1_correctCount+=1;
//                     spec4_correctCount+=1;
//                 }
//                 else if(state_groundTrue==specList_h[segmentIdx*num_spec_state+1]||
//                         state_groundTrue==specList_h[segmentIdx*num_spec_state+2]||
//                         state_groundTrue==specList_h[segmentIdx*num_spec_state+3])
//                 {
//                     spec4_correctCount+=1;
//                 }
//             }

//             int executionLen = curInputPos-startInputPos+1;
//             //check # unique state when 2 symbols executed 
//             if(executionLen==2)
//             {
//                 //remove reduntant paths
//                 set<int> s;
//                 unsigned size = execPath.size();
//                 for( unsigned i = 0; i < size; ++i ) 
//                     s.insert( execPath[i] );
//                 execPath.assign( s.begin(), s.end() );
//                 // std::cout<<"inputSegment["<<segmentIdx<<"],"<<execPath.size();
//                 sumUniqueState_L2 += execPath.size();
//             }
//             //check # unique state when 10 symbols executed 
//             if(executionLen==10)
//             {
//                 //remove reduntant paths
//                 set<int> s;
//                 unsigned size = execPath.size();
//                 for( unsigned i = 0; i < size; ++i ) 
//                     s.insert( execPath[i] );
//                 execPath.assign( s.begin(), s.end() );
//                 // std::cout<<"inputSegment["<<segmentIdx<<"],"<<execPath.size();
//                 sumUniqueState_L10 += execPath.size();
//             }             
//             //check # unique state when 100 symbols executed 
//             if(executionLen==100)
//             {
//                 //remove reduntant paths
//                 set<int> s;
//                 unsigned size = execPath.size();
//                 for( unsigned i = 0; i < size; ++i ) 
//                     s.insert( execPath[i] );
//                 execPath.assign( s.begin(), s.end() );
//                 // std::cout<<"inputSegment["<<segmentIdx<<"],"<<execPath.size();
//                 sumUniqueState_L100 += execPath.size();
//             }
//             //check # unique state when 100 symbols executed 
//             if(executionLen==1000)
//             {
//                 //remove reduntant paths
//                 set<int> s;
//                 unsigned size = execPath.size();
//                 for( unsigned i = 0; i < size; ++i ) 
//                     s.insert( execPath[i] );
//                 execPath.assign( s.begin(), s.end() );
//                 // std::cout<<","<<execPath.size();
//                 sumUniqueState_L1000 += execPath.size();
//             }
//             if(executionLen==10000)
//             {
//                 //remove reduntant paths
//                 set<int> s;
//                 unsigned size = execPath.size();
//                 for( unsigned i = 0; i < size; ++i ) 
//                     s.insert( execPath[i] );
//                 execPath.assign( s.begin(), s.end() );
//                 // std::cout<<","<<execPath.size();
//                 sumUniqueState_L10000 += execPath.size();
//             }
//             // if(executionLen==1000000)
//             // {
//             //     //remove reduntant paths
//             //     set<int> s;
//             //     unsigned size = execPath.size();
//             //     for( unsigned i = 0; i < size; ++i ) 
//             //         s.insert( execPath[i] );
//             //     execPath.assign( s.begin(), s.end() );
//             //     std::cout<<","<<execPath.size();
//             // }
//         }
//         // std::cout<<std::endl;
//     }

//     std::cout<<"acceptCount:"<<acceptCount<<std::endl;

//     float spec1_accuracyRate = 1.0*spec1_correctCount/(inputSegment-1);
//     float spec4_accuracyRate = 1.0*spec4_correctCount/(inputSegment-1);
//     std::cout<<"spec1_accuracyRate:"<<spec1_accuracyRate<<std::endl;
//     std::cout<<"spec4_accuracyRate:"<<spec4_accuracyRate<<std::endl;

//     float aveNumUniqueState_L2 = 1.0*sumUniqueState_L2/inputSegment;
//     float aveNumUniqueState_L10 = 1.0*sumUniqueState_L10/inputSegment;
//     float aveNumUniqueState_L100 = 1.0*sumUniqueState_L100/inputSegment;
//     float aveNumUniqueState_L1000 = 1.0*sumUniqueState_L1000/inputSegment;
//     float aveNumUniqueState_L10000 = 1.0*sumUniqueState_L10000/inputSegment;
//     std::cout<<"aveNumUniqueState_L2:"<<aveNumUniqueState_L2<<std::endl;
//     std::cout<<"aveNumUniqueState_L10:"<<aveNumUniqueState_L10<<std::endl;
//     std::cout<<"aveNumUniqueState_L100:"<<aveNumUniqueState_L100<<std::endl;
//     std::cout<<"aveNumUniqueState_L1000:"<<aveNumUniqueState_L1000<<std::endl;
//     std::cout<<"aveNumUniqueState_L10000:"<<aveNumUniqueState_L10000<<std::endl;
//     delete inputObj;
//     delete tableObj;
//     delete predObj;
// }

}  // namespace re2
