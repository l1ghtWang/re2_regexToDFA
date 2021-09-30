#ifndef ANALYZER_H_
#define ANALYZER_H_

#include <string>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>

#include "input.h"
#include "fsm.h"


using namespace std;

namespace FSM 
{
	class Analyzer
	{
	public:
		Analyzer();
		Analyzer(int numState);
		~Analyzer();

		void offlineHotArea(Table* obj);
		void offlineHotArea_withInput(Table* objTable, Input* objIn, int* todo_state, int numChunks);
		void onlineProfilingCheck(Table* obj, Input* objInput, 
			int start, int threshold);
		void printOfflineHot(int threshold);
		void printHotAreaStat(int threshold);
		void printHotAreaStat_result(int threshold, float &result_hotCoverage);
		void loadHotArea(int threshold, Table* obj, int* hotStates, int* hotTrans);
		int hash(int state, int scaleVar, int modulus);
		void loadHotArea_hash(Table* obj,
			int* hotStates, int* hotTrans, int threshold, int scalevar);
		void loadHotArea_hash_result(Table* obj, 
			int* hotStates, int* hotTrans, int threshold, int scalevar,float &result_hotCoverage);
		void printhotStatesAndhotTrans(Table* obj, int* hotStates, int* hotTrans, int threshold);
		void generateTranslationTable(int numState, int numSlot);
		void generateTranslationTable_old(int numState);
		void transform_Table(Table* obj, int* newTable, int* todo_message_h, int numThread);
		void transform_result(Table* obj, int* finalTruth, int numChunk);
		void get_translate_table(Table* obj, int* translateTable);

	private:
		vector<stateINFO> mOfflineResults;
		unordered_map<int, int> mhashMap;
		int* mTranslationTable_OriginalToNew;
		int* mTranslationTable_NewToOriginal;
	};

}	// end of namespace FSM
#endif // ANALYZER_H_