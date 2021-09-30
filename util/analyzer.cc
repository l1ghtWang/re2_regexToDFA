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
#include <unordered_set>
#include <iomanip>

#include "analyzer.h"

using namespace std;

namespace FSM 
{
	Analyzer::Analyzer()
	{
		mTranslationTable_OriginalToNew = new int [2000];
		mTranslationTable_NewToOriginal = new int [2000];
	}

	Analyzer::Analyzer(int numState)
	{
		mTranslationTable_OriginalToNew = new int [numState];
		mTranslationTable_NewToOriginal = new int [numState];
	}

	Analyzer::~Analyzer()
	{
		mOfflineResults.clear();
		delete mTranslationTable_OriginalToNew;
		delete mTranslationTable_NewToOriginal;
	}

	void Analyzer::offlineHotArea(Table* obj)
	{
		int i,j;
		int stateSize = obj->getNumState();
		int alphabetSize = obj->getNumSymbol();
		int* tableList = obj->getTable();

		for (i = 0; i < stateSize; i++)
		{
			stateINFO curInfo;
			curInfo.state = i;
			curInfo.appearTimes = 0;
			mOfflineResults.push_back(curInfo);
		}

		for (i = 0; i < stateSize*alphabetSize; i++)
		{	
			int curState = tableList[i];
			if (curState < 0 || curState >= stateSize)
				cout << "Something wrong on reading the table ... " << endl; 

			(mOfflineResults[curState]).appearTimes++;
		}
		sort(mOfflineResults.begin(), mOfflineResults.end(), compareByTimes);
	}

	//input sensitive hot area analyser
	void Analyzer::offlineHotArea_withInput(Table* objTable, Input* objIn, int* todo_state, int numChunks)
	{
		long i,j;
		int stateSize = objTable->getNumState();
		int alphabetSize = objTable->getNumSymbol();
		int state = objTable->getStartState();
		int* tableList = objTable->getTable();
		uint8_t* inputs = objIn->getHostPointer();
		long length = objIn->getLength();

		for (i = 0; i < stateSize; i++)
		{
			stateINFO curInfo;
			curInfo.state = i;
			curInfo.appearTimes = 0;
			mOfflineResults.push_back(curInfo);
		}
		long chunkLen = length/numChunks;
		for(j = 0; j < numChunks; j++)
		{
			long chunkStart = j*chunkLen;
			long chunkEnd = chunkStart + chunkLen;
			// long chunkEnd = chunkStart + length/100/numChunks;
			int state = todo_state[j];
			for(i = chunkStart; i < chunkEnd; i++)
			{
				uint8_t symbol = inputs[i];
				mOfflineResults[state].appearTimes += 1;
				state = tableList[state * alphabetSize + symbol];
			}
		}
		// for (i = 0; i < length; i++)
		// {
		// 	// if(i%numChunks==0)//first char of one chunk
		// 	// {
		// 	// 	int chunkIdx = i/numChunks;
		// 	// 	state=todo_state[chunkIdx];
		// 	// }
		// 	uint8_t symbol = inputs[i];
		// 	mOfflineResults[state].appearTimes += 1;

		// 	// cout<<"state:"<<state<<", appearTimes:"<<mOfflineResults[state].appearTimes<<endl;
		// 	state = tableList[state * alphabetSize + symbol];
		// }


		sort(mOfflineResults.begin(), mOfflineResults.end(), compareByTimes);
	}

	void Analyzer::printOfflineHot(int threshold)
	{
		long total = 0;
		for (auto m : mOfflineResults)
			total += m.appearTimes;
		int numState = mOfflineResults.size();

		int cutoff = (threshold < numState)? threshold : numState; 
		for (int i = 0; i < cutoff; i++)
		{
			stateINFO cur = mOfflineResults[i];
			cout << cur.state << " " << cur.appearTimes / (total * 1.0) << endl;
		}
		cout << endl;
	}

	void Analyzer::printHotAreaStat(int threshold)
	{
		long total = 0;
		for (auto m : mOfflineResults)
			total += m.appearTimes;
		int numState = mOfflineResults.size();

		int cutoff = (threshold < numState)? threshold : numState; 
		float sum = 0.0; 
		for (int i = 0; i < cutoff; i++)
		{
			stateINFO cur = mOfflineResults[i];
			sum += cur.appearTimes / (total * 1.0); 
		}
		cout<<"Coverage of "<<std::setw(2)<<threshold<<" hot states:"<<sum << endl;
	}

	void Analyzer::printHotAreaStat_result(int threshold, float &result_hotCoverage)
	{
		long total = 0;
		for (auto m : mOfflineResults)
			total += m.appearTimes;
		int numState = mOfflineResults.size();

		int cutoff = (threshold < numState)? threshold : numState; 
		float sum = 0.0; 
		for (int i = 0; i < cutoff; i++)
		{
			stateINFO cur = mOfflineResults[i];
			sum += cur.appearTimes / (total * 1.0); 
		}
		cout<<"Coverage of "<<std::setw(2)<<threshold<<" hot states:"<<sum << endl;
		result_hotCoverage=sum;
	}

	void Analyzer::loadHotArea(int threshold, Table* obj, 
		int* hotStates, int* hotTrans)
	{
		int alphabetSize = obj->getNumSymbol();
		int* tableList = obj->getTable();

		for (int i = 0; i < threshold; i++)
		{
			stateINFO cur = mOfflineResults[i];
			hotStates[i] = cur.state;
			for (int j = 0; j < alphabetSize; j++)
				hotTrans[i*alphabetSize+j] = tableList[(cur.state)*alphabetSize + j];
		}		
	}

	void Analyzer::onlineProfilingCheck(Table* obj, Input* objInput, 
		int start, int threshold)
	{
		int i, j;
		int numState = obj->getNumState();
		int numSymbol = obj->getNumSymbol();
		int* transTable = obj->getTable();
		uint8_t* inputs = objInput->getHostPointer();	
		long length = objInput->getLength();

		if (mOfflineResults.size() == 0)
			this->offlineHotArea(obj);

		int state = start;
		unordered_set<int> hot;
		int cutoff = (threshold < numState)? threshold : numState; 
		for (i = 0; i < cutoff; i++)
			hot.insert((mOfflineResults[i]).state);
		long missCount = 0;

		for (i = 0; i < length; i++)
		{
			if (hot.find(state) == hot.end())
				missCount++;
			state = transTable[state * numSymbol + inputs[i]];
		}	

		cout << "Miss Rate is " << missCount*1.0 / length << endl;
	}


	int Analyzer::hash(int state, int modulus, int scaleVar)
	{
		return (state*scaleVar)%modulus;
	}

	void Analyzer::printhotStatesAndhotTrans(Table* obj, int* hotStates, int* hotTrans, int threshold)
	{
		int alphabetSize = obj->getNumSymbol();
		for (int hashValue = 0; hashValue < threshold; hashValue++)
		{

			cout << "state:"<<hotStates[hashValue] << " first_tran:" << hotTrans[hashValue*alphabetSize]<<" second_tran:" << hotTrans[hashValue*alphabetSize+1] << endl;
		}
		cout << endl;
	}

	void Analyzer::loadHotArea_hash(Table* obj, 
		int* hotStates, int* hotTrans, int threshold, int scalevar)
	{
		int alphabetSize = obj->getNumSymbol();
		int* tableList = obj->getTable();

		long total = 0;
		for (auto m : mOfflineResults)
			total += m.appearTimes;

		int numItem_inHashTable=0;
		int curStateIdx=0;
		float sum=0.0;
		while(numItem_inHashTable<threshold && curStateIdx<mOfflineResults.size())
		{
			stateINFO cur = mOfflineResults[curStateIdx];
			curStateIdx+=1;
			int hashValue = hash(cur.state, threshold, scalevar);
			if(hotStates[hashValue]==-1)
			{
				sum += cur.appearTimes; 
				hotStates[hashValue]=cur.state;
				for (int j = 0; j < alphabetSize; j++)
				{
					hotTrans[hashValue*alphabetSize+j] = tableList[(cur.state)*alphabetSize + j];
				}
				numItem_inHashTable+=1;
			}
			
		}	
		cout<<"Coverage of "<<std::setw(2)<<threshold<<" hot states:"<<(sum/(total*1.0)) << endl;
	}

	void Analyzer::loadHotArea_hash_result(Table* obj, 
		int* hotStates, int* hotTrans, int threshold, int scalevar,float &result_hotCoverage)
	{
		int alphabetSize = obj->getNumSymbol();
		int* tableList = obj->getTable();

		long total = 0;
		for (auto m : mOfflineResults)
			total += m.appearTimes;

		int numItem_inHashTable=0;
		int curStateIdx=0;
		float sum=0.0;
		while(numItem_inHashTable<threshold && curStateIdx<mOfflineResults.size())
		{
			stateINFO cur = mOfflineResults[curStateIdx];
			curStateIdx+=1;
			int hashValue = hash(cur.state, threshold, scalevar);
			if(hotStates[hashValue]==-1)
			{
				sum += cur.appearTimes; 
				hotStates[hashValue]=cur.state;
				for (int j = 0; j < alphabetSize; j++)
				{
					hotTrans[hashValue*alphabetSize+j] = tableList[(cur.state)*alphabetSize + j];
				}
				numItem_inHashTable+=1;
			}
			
		}	
		cout<<"Coverage of "<<std::setw(2)<<threshold<<" hot states:"<<(sum/(total*1.0)) << endl;
		result_hotCoverage=(sum/(total*1.0));
	}
	void Analyzer::generateTranslationTable(int numState, int numSlot)
	{

		if(mOfflineResults.size() == numState)
		{
			int i = 0;
			vector<int> myVector;
			for (auto m : mOfflineResults)
			{
				if(i<numSlot)
				{
					mTranslationTable_NewToOriginal[i] = m.state;
					// cout<<"New State:"<<i<<", Original state:"<<m.state<<endl;
					mTranslationTable_OriginalToNew[m.state] = i;
				}
				else
				{
					myVector.push_back(m.state);
					mTranslationTable_NewToOriginal[i] = -1;
				}
				i++;
			}
			sort(myVector.begin(), myVector.end(), greater<int>());
			for(i=numSlot; i<numState;i++)
			{
				if(mTranslationTable_NewToOriginal[i] == -1)
				{
					int state = myVector.back();
					myVector.pop_back();
					mTranslationTable_NewToOriginal[i] = state;
					// cout<<"New State:"<<i<<", Original state:"<<state<<endl;
					mTranslationTable_OriginalToNew[state] = i;
				}
			}
		}
		else
		{
			printf("!!!numState not match!!!\n");
			exit(1);
		}	
	}

	void Analyzer::generateTranslationTable_old(int numState)
	{

		if(mOfflineResults.size() == numState)
		{
			int i = 0;
			for (auto m : mOfflineResults)
			{
				mTranslationTable_NewToOriginal[i] = m.state;
				mTranslationTable_OriginalToNew[m.state] = i;
				i++;
			}
		}
		else
		{
			printf("!!!numState not match!!!\n");
			exit(1);
		}	
	}


	void Analyzer::transform_Table(Table* obj, int* newTable, int* todo_message_h, int numThread)
	{
		int stateSize = obj->getNumState();
		int alphabetSize = obj->getNumSymbol();
		int* tableList = obj->getTable();
		int start = obj->getStartState();

		for(int newState=0; newState < stateSize; newState++)
		{
			int originalState = mTranslationTable_NewToOriginal[newState];
			for(int symbol=0; symbol<alphabetSize; symbol++)
			{
				newTable[newState*alphabetSize + symbol] = mTranslationTable_OriginalToNew[tableList[originalState*alphabetSize + symbol]];
			}
		}

		for(int i=0; i<numThread; i++)
		{
			todo_message_h[i] = mTranslationTable_OriginalToNew[todo_message_h[i]];
		}

	}

	void Analyzer::transform_result(Table* obj, int* finalTruth, int numChunk)
	{
		int stateSize = obj->getNumState();
		int alphabetSize = obj->getNumSymbol();

		for(int i=0; i<numChunk; i++)
		{
			finalTruth[i] = mTranslationTable_NewToOriginal[finalTruth[i]];
		}
	}

	void Analyzer::get_translate_table(Table* obj, int* translateTable_OriginalToNew)
	{
		int numStates = obj->getNumState();
		for(int i=0; i<numStates; i++)
		{
			translateTable_OriginalToNew[i]=mTranslationTable_OriginalToNew[i];
		}
	}

}	// end of namespace FSM