#ifndef FSM_H_
#define FSM_H_

#include <string>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include "input.h"


using namespace std;

namespace FSM 
{
	#define LOOKBACK 2 
	#define MAXSLOT 32

	// @Brief The structure is used to sort the predicted states
	struct stateINFO
	{
		int state;
		long appearTimes;
	};
	bool compareByTimes(const stateINFO &a, const stateINFO &b);

    
    // @Brief The class @Table is used to describe the transition table in DFA
	class Table
	{
	public:
		Table();
		Table(int* transTable, int nState, int nSymbol, int startState, std::vector<int> acceptVec);
		~Table();

		// @brief Providing @tableFileName to access the given transition table, 
		// with using @acceptFileName to mark the accept state in the table.
		// And applying required start state @startState while also define the mapping rule @ruleUsed, 
		// then return an @Table-type object  
		static Table* readFromFile(const char* tableFileName, const char* acceptFileName, 
			const int startState, const MappingRule* ruleUsed);

		int* getTable() const;
		int getNumState() const;
		int getNumSymbol() const;
		int getStartState() const;
		void printTable() const;
		bool isAccept(int stateID) const;

		void transpose(); // Transpose the row-major Table

	private:
		int* mTableList __attribute__ ((aligned (32)));
		int mNumState;
		int mNumSymbol;
		int mStartState;
		//std::unordered_set<int> mAcceptStates;
		bool* mAcceptStates;
	};


	// @Brief The class @Predictor provides prediction for FSM speculative parallelization.
	// Currently it enables a lightweight two-step lookback approach. 
	class Predictor
	{           
	public:
		Predictor();
		~Predictor();

		void printSpec() const;
		void setPrintAll(bool p);

		void startSpec(Table* objTable, Input* objInput, int numBlk, int numThd, int slot);

		int getPredState(int tid, int rank);
		int* getAllPreds();
		double* getFreq();

		int getNumSlots() const;
		void setNumSlots(int nSlots);

	private:
		void spec(int bidx, int tidx);

	private:
		bool mPrintAll;
		Table* mCurTab;
		Input* mCurIn;
		int mNumChk;
		int mNumBlk;
		int mNumThd;
		int mSlot;
		int* mStoredStates;
		double* mFreq;
	};

	class Verify
	{
	public:
		Verify();
		~Verify();

		void run_CPU(Table* objTable, Input* objInput, int numChunks);
		void run_CPU_multiInput(Table* objTable, Input* objInput, int numChunks, int numInputStream);
		bool correct(int* results);

	private:
		int* mGroundTruth;
		int mCk;
	};

}	// end of namespace FSM
#endif // FSM_H_