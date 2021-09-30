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

#include "fsm.h"

using namespace std;

namespace FSM 
{

	bool compareByTimes(const stateINFO &a, const stateINFO &b)
	{
    	return a.appearTimes > b.appearTimes;
	}

// Implemetation of class Table
    Table::Table()
	{
		std::cout<<"you need Table(int* list, int nstate, int nsymbol, int s, std::vector<int> acceptVec)\n"
		<<" or call\n"
		<<"static Table* readFromFile(const char* tableFileName, const char* acceptFileName," 
			"const int startState, const MappingRule* ruleUsed)\n"
		<<" to construct a Table\n";
		exit(1);
		// mTableList = NULL;
		// mNumState = 0;
		// mNumSymbol = 0;
		// mStartState = 0;
		// mAcceptStates = NULL;
	} 

	Table::Table(int* list, int nstate, int nsymbol, int s, std::vector<int> acceptVec)
	{
		mTableList = list;
		mNumState = nstate;
		mNumSymbol = nsymbol;
		mStartState = s;

		mAcceptStates = new bool [nstate]();
		for (std::vector<int>::iterator it = acceptVec.begin() ; it != acceptVec.end(); it++)
			//mAcceptStates.insert(*it);
			mAcceptStates[*it] = true;

	}

	Table::~Table()
	{
		if (mTableList != NULL)
			delete []mTableList;

		//mAcceptStates.clear();
		if (mAcceptStates != NULL)
			delete []mAcceptStates;

		mNumState = 0;
		mNumSymbol = 0;
		mStartState = 0;
	}

	Table* Table::readFromFile(const char* tableFile, const char* acceptFile, 
			const int s, const MappingRule* ruleset)
	{	
		int* list_  __attribute__ ((aligned (32)));
		int nstate_;
		int nsymbol_;

		// Loading the accept states from **acceptFile**
		vector<int> acceptVec;
		ifstream in_ac;
		in_ac.open(acceptFile);
		if (in_ac.is_open())
		{
			while(in_ac)
			{
				int temp_ac;
				in_ac >> temp_ac;
				acceptVec.push_back(temp_ac);
			}
			in_ac.close();
		}
		else
		{
			cout << "Fail to open Accept file " << acceptFile << endl;
			return NULL;
		}

		// Loading the transition table from the **tableFile**
		int MAXSYMBOL = ruleset->ruleSize();
		vector<int> vecTable;
		ifstream in_table;
		in_table.open(tableFile);
		if (in_table.is_open())
		{
			string line;
    		while(!in_table.eof())
    		{
        		getline(in_table,line);
        		if(in_table.fail())
            		break;
        		if (line.size() > 2)
        		{
        			int currentLineNum = 0;
        			stringstream stream(line);
        			while(stream && currentLineNum != MAXSYMBOL)
        			{
        				int temp_n;
        				stream >> temp_n;
        				currentLineNum++;
        				// No need to store Accept State 
        				// if (find (acceptVec.begin(), acceptVec.end(), temp_n) != acceptVec.end())
        				// 	temp_n = temp_n | 0XF0000000;
        				// else
        				// 	temp_n = temp_n & 0X0FFFFFFF;
        				vecTable.push_back(temp_n);
    				}
    				if (currentLineNum != MAXSYMBOL)
    				{
    					cout << "Number of Symbol does not match with current setting \n";
    					return NULL;
    				}
        		}
    		}
		}
		else
		{
			cout << "Fail to open Table file " << tableFile << endl;
			return NULL;
		}

		list_ = new int [(int)vecTable.size()];
		nsymbol_ = MAXSYMBOL;
		nstate_ = ((int)vecTable.size()) / nsymbol_;

		for (int i = 0 ; i < vecTable.size(); i++)
			list_[i] = vecTable[i];

	    Table *object = new Table(list_, nstate_, nsymbol_, s, acceptVec);
	    return object;		
	}

	int* Table::getTable() const
	{
		return mTableList;
	}

	int Table::getNumState() const
	{
		return mNumState;
	}
	int Table::getNumSymbol() const
	{
		return mNumSymbol;
	}

	int Table::getStartState() const
	{
		return mStartState;
	}

	bool Table::isAccept(int stateID) const
	{
		//return (mAcceptStates.count(stateID));
		return mAcceptStates[stateID];
	}	

	void Table::printTable() const
	{
		// Print Table in two dimentions
		for (int i = 0; i < mNumState * mNumSymbol; i++)
		{
			cout << mTableList[i] << " ";
			if ((i+1) % mNumSymbol == 0)
				cout << endl;
		}
		cout << endl;
		cout << "#State " << this->getNumState() << ", #Symbol " << this->getNumSymbol() << endl;
		
		// Print All Accept States
		cout << "Accept States include: " << endl;
		for (int j = 0; j < this->getNumState(); j++)
			if (this->isAccept(j))
				cout << j << " ";
		cout << endl;
	}
// Implemetation of class Predictor
	Predictor::Predictor()
	{
		mStoredStates = NULL;
		mFreq = NULL;
		mPrintAll = false;
		mSlot = MAXSLOT;
	}

	Predictor::~Predictor()
	{
		if (mStoredStates != NULL)
			delete []mStoredStates;
		if (mFreq != NULL)
			delete []mFreq;
	}

	void Predictor::printSpec() const
	{
		cout << "Printing the Speculation Results ... " << endl;
		cout << mStoredStates[0] << endl;
		for (int i = 1; i < mNumChk; i++)
		{
			for (int j = 0; j < mSlot; j++)
				if (mPrintAll == true)
					printf("%d (%f) ; ", mStoredStates[i*mSlot+j], mFreq[i*mSlot+j]);
				else
					cout << mStoredStates[i*mSlot+j] << " ";
				
			cout << endl;
		}
	}

	void Predictor::setPrintAll(bool p)
	{
		mPrintAll = p;
	}

	int Predictor::getNumSlots() const
	{
		return mSlot;
	}

	int* Predictor::getAllPreds()
	{
		return mStoredStates;
	}

	int Predictor::getPredState(int tid, int rank)
	{
		// TODO
		if(rank<mSlot)
			return mStoredStates[tid * mSlot + rank];
		else
			return -1;
	}

	double* Predictor::getFreq()
	{
		return mFreq;
	}

	void Predictor::setNumSlots(int nSlots)
	{
		mSlot = nSlots;
	}

	void Predictor::startSpec(Table* objTable, Input* objInput, int numBlk, int numThd, int slot)
	{
		mCurTab = objTable;
		mCurIn = objInput;
		mNumChk = numBlk*numThd;
		mNumBlk = numBlk;
		mNumThd = numThd;

		if ( (slot > objTable->getNumState()) || slot > MAXSLOT)
		{
			if (objTable->getNumState() < MAXSLOT)
				this->setNumSlots(objTable->getNumState());		
			else
				this->setNumSlots(MAXSLOT);
		}
		else
			this->setNumSlots(slot);	


		mStoredStates = new int [mSlot * mNumChk]();
		mFreq = new double [mSlot * mNumChk]();

		for (int i=0;i<numBlk;i++)
		{
			for(int j=0;j<numThd;j++)
				spec(i,j);
		}

	}

	void Predictor::spec(int bidx, int tidx)
	{
		int tid=bidx*mNumThd+tidx;
		if (tidx == 0)
		{
			mStoredStates[tid*mSlot] = mCurTab->getStartState();
			mFreq[tid*mSlot] = 1.0;
			for (int useless = 1; useless < mSlot; useless++)
			{
				mStoredStates[tid*mSlot+useless] = mCurTab->getStartState();
				mFreq[tid*mSlot+useless] = 0.0;
			}
			return;
		}

		int numState = mCurTab->getNumState();
		int numSymbol = mCurTab->getNumSymbol();
		int* transTable = mCurTab->getTable();
		uint8_t* inputs = mCurIn->getHostPointer();

		long lookBackStartIndex = (mCurIn->getLength())/mNumChk*tid - LOOKBACK;

		long i, j;	// Loop iteration counter
		int* predict_states;
		int* times_counter;
		predict_states = new int [numState];
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

		stateINFO* allStates;
		allStates = new stateINFO [numState];
		for (i = 0; i < numState; i++)
		{
			allStates[i].state = i;
			allStates[i].appearTimes = times_counter[i];	
		}
		sort(allStates, allStates+numState, compareByTimes);

		// uint8_t f_char = inputs[lookBackStartIndex];
		// uint8_t s_char = inputs[lookBackStartIndex+1];
		// cout<<"tid:"<<tid<<", first_char:"<<f_char<<", sec_char:"<<s_char<<endl;
		for (i = 0; i < mSlot; i++)
		{
			mStoredStates[tid * mSlot + i] = allStates[i].state;
			mFreq[tid * mSlot + i] = allStates[i].appearTimes*1.0/numState;
			// cout<<"comb_idx:"<<f_char*numSymbol+s_char<<", rank:"<<i<<", state:"<<allStates[i].state<<", freq:"<<(allStates[i].appearTimes*1.0/numState)<<endl;
		}

		delete []allStates;
		delete []times_counter;
		delete []predict_states;
	}

// Implemetation of class Verify
	Verify::Verify()
	{
		mGroundTruth = NULL;
	}

	Verify::~Verify()
	{
		delete []mGroundTruth;
	}
	
	void Verify::run_CPU(Table* objTable, Input* objInput, int numChunks)
	{
		mCk = numChunks;
		mGroundTruth = new int [mCk]();

		int nSymbol = objTable->getNumSymbol();
		int state_ = objTable->getStartState();
		int* tableList = objTable->getTable();	

		long chunkLength = (objInput->getLength()/mCk);
		long length = chunkLength * mCk;
		uint8_t* inputs_ = objInput->getHostPointer();

		for (long i = 0; i < length; i++)
		{
			uint8_t data = inputs_[i];
			state_ = tableList[state_ * nSymbol + data];
			if ((i+1) % chunkLength == 0)
			{
				int curChunk = (i+1)/chunkLength - 1;
				mGroundTruth[curChunk] = state_;
			}
		}
	}

	void Verify::run_CPU_multiInput(Table* objTable, Input* objInput, int numChunks, int numInputStream)
	{
		mCk = numChunks;
		mGroundTruth = new int [mCk]();

		int nSymbol = objTable->getNumSymbol();
		int startState = objTable->getStartState();
		int state_ = startState;
		int* tableList = objTable->getTable();	

		long chunkLength = (objInput->getLength()/mCk);
		long length = chunkLength * mCk;
		uint8_t* inputs_ = objInput->getHostPointer();

		long lenInputStream = length/numInputStream;

		for (long i = 0; i < length; i++)
		{
			uint8_t data = inputs_[i];
			if(i%lenInputStream==0)
			{
				state_ = startState;
			}
			state_ = tableList[state_ * nSymbol + data];
			if ((i+1) % chunkLength == 0)
			{
				int curChunk = (i+1)/chunkLength - 1;
				mGroundTruth[curChunk] = state_;
			}
		}
	}

	bool Verify::correct(int* results)
	{
		bool allVerified = true;
		for (int i = 0; i < mCk; i++)
			if (mGroundTruth[i] != results[i])
			{
				printf("Wrong at %d correct vs wrong: (%d, %d)\n", i, mGroundTruth[i], results[i]);
				allVerified = false;
			}

		return allVerified;

	}


}	// end of namespace FSM