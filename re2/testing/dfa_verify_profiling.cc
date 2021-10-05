#include <iostream>
#include <algorithm>
#include <chrono>

#include "util/fsm.h"
#include "util/input.h"
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

namespace re2 {

using namespace FSM;

TEST(SingleThreaded, BuildEntireDFA) {
	// char inputFile [] = "/home/wygzero/re2/ANMLZoo_regex/powerEN/poweren_10MB.input";
    // char tableFile [] = "/home/wygzero/re2/ANMLZoo_regex/powerEN/powerEN_DFA11_1216.table";
    // char acFile [] = "/home/wygzero/re2/ANMLZoo_regex/powerEN/powerEN_AC11.txt";
	// char inputFile [] = "/home/wygzero/re2/ANMLZoo_regex/snort/snort_TestInput2.in";
    // char tableFile [] = "/home/wygzero/re2/ANMLZoo_regex/snort/snort_DFA0.table";
    // char acFile [] = "/home/wygzero/re2/ANMLZoo_regex/snort/snort_AC0.txt";
    // char inputFile [] = "/home/wygzero/re2/ANMLZoo_regex/protomata/protomata_10MB.input";
    // char tableFile [] = "/home/share/Snort/SnortTable/FSM1069_31_256.table";
    // char acFile [] = "/home/share/Snort/SnortAccept/AC_1069.txt";
    // char tableFile [] = "/home/share/Snort/SnortTable/FSM4309_607_256.table";
    // char acFile [] = "/home/share/Snort/SnortAccept/AC_4309.txt";
    // char tableFile [] = "/home/share/Snort/SnortTable/FSM3096_49_256.table";
    // char acFile [] = "/home/share/Snort/SnortAccept/AC_3096.txt";
    // char tableFile [] = "/home/share/Snort/SnortTable/FSM2735_499_256.table";
    // char acFile [] = "/home/share/Snort/SnortAccept/AC_2735.txt";
    char tableFile [] = "/home/share/Snort/SnortTable/FSM3365_31_256.table";
    char acFile [] = "/home/share/Snort/SnortAccept/AC_3365.txt";
    
    char inputFile [] = "/home/share/networkpasckages/profiling_input1_1MB.in";

    int start = 0;
	std::string fn(inputFile);
    FSM::MappingRule* rules_ = FSM::MappingRule::defineMappingRule();
	Input* inputObj = Input::readFromFile(fn, rules_);
    long inputLength = inputObj->getLength(); 
    std::cout<<inputLength<<std::endl;


    Table* tableObj = Table::readFromFile(tableFile, acFile, start, rules_);
    // tableObj->printTable();

    uint8_t* inputPtr = inputObj->getHostPointer();
    int* tableList = tableObj->getTable();
    int state = start;
    int nSymbol = tableObj->getNumSymbol();
    int numState = tableObj->getNumState();
    std::cout<<"numState:"<<numState<<std::endl;

    
    long acceptCount = 0;
    // for(long i = 0; i < inputLength; i++)
    // {
    //     uint8_t data = inputPtr[i];
    //     state = tableList[state * nSymbol + data];
    //     if(tableObj->isAccept(state))
    //         acceptCount+=1;
    // }
    // std::cout<<"acceptCount:"<<acceptCount<<std::endl;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    int state_groundTrue = start;
    int inputSegment = 10;
    long inputSegmentLen = inputLength/inputSegment;
    Predictor* predObj;
    predObj = new Predictor();
    int num_spec_state = 4;
    predObj->startSpec(tableObj, inputObj, 1, inputSegment, num_spec_state);
    num_spec_state = predObj->getNumSlots();
    int* specList_h = predObj->getAllPreds();
    int spec1_correctCount = 0;
    int spec4_correctCount = 0;
    int sumUniqueState_L100 = 0;
    int sumUniqueState_L1000 = 0;
    int sumUniqueState_L10000 = 0;
    for(int segmentIdx = 0; segmentIdx < inputSegment; segmentIdx++)
    {
        long startInputPos = segmentIdx*inputSegmentLen;
        long endInputPos = (segmentIdx+1)*inputSegmentLen-1;
        
        std::vector<int> execPath;
        for(int i = 0; i < numState; i++)
            execPath.push_back(i);

        // std::cout<<"inputSegment["<<segmentIdx<<"],#uniqueState(L=100),#uniqueState(L=1000),#uniqueState(L=10000)\n";
        for(int curInputPos = startInputPos; curInputPos <= endInputPos; curInputPos++)
        {
            int numPath = execPath.size();
            uint8_t data = inputPtr[curInputPos];
            state_groundTrue = tableList[state_groundTrue * nSymbol + data];
            for(int pathIdx = 0; pathIdx < numPath; pathIdx++)
            {
                int curState = execPath[pathIdx];
                curState =  tableList[curState * nSymbol + data];
                execPath[pathIdx] = curState;
            }

            //check number of matchs
            if(tableObj->isAccept(state_groundTrue))
                acceptCount+=1;
            //check speculative accuracy
            if(curInputPos==startInputPos && curInputPos!=0)
            {
                if(state_groundTrue==specList_h[segmentIdx*num_spec_state])
                {
                    spec1_correctCount+=1;
                    spec4_correctCount+=1;
                }
                else if(state_groundTrue==specList_h[segmentIdx*num_spec_state+1]||
                        state_groundTrue==specList_h[segmentIdx*num_spec_state+2]||
                        state_groundTrue==specList_h[segmentIdx*num_spec_state+3])
                {
                    spec4_correctCount+=1;
                }
            }

            int executionLen = curInputPos-startInputPos+1; 
            //check # unique state when 100 symbols executed 
            if(executionLen==100)
            {
                //remove reduntant paths
                set<int> s;
                unsigned size = execPath.size();
                for( unsigned i = 0; i < size; ++i ) 
                    s.insert( execPath[i] );
                execPath.assign( s.begin(), s.end() );
                // std::cout<<"inputSegment["<<segmentIdx<<"],"<<execPath.size();
                sumUniqueState_L100 += execPath.size();
            }
            //check # unique state when 100 symbols executed 
            if(executionLen==1000)
            {
                //remove reduntant paths
                set<int> s;
                unsigned size = execPath.size();
                for( unsigned i = 0; i < size; ++i ) 
                    s.insert( execPath[i] );
                execPath.assign( s.begin(), s.end() );
                // std::cout<<","<<execPath.size();
                sumUniqueState_L1000 += execPath.size();
            }
            if(executionLen==10000)
            {
                //remove reduntant paths
                set<int> s;
                unsigned size = execPath.size();
                for( unsigned i = 0; i < size; ++i ) 
                    s.insert( execPath[i] );
                execPath.assign( s.begin(), s.end() );
                // std::cout<<","<<execPath.size();
                sumUniqueState_L10000 += execPath.size();
            }
            // if(executionLen==1000000)
            // {
            //     //remove reduntant paths
            //     set<int> s;
            //     unsigned size = execPath.size();
            //     for( unsigned i = 0; i < size; ++i ) 
            //         s.insert( execPath[i] );
            //     execPath.assign( s.begin(), s.end() );
            //     std::cout<<","<<execPath.size();
            // }
        }
        // std::cout<<std::endl;
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()/1000.0 << "[sec]" << std::endl;

    std::cout<<"acceptCount:"<<acceptCount<<std::endl;

    float spec1_accuracyRate = 1.0*spec1_correctCount/(inputSegment-1);
    float spec4_accuracyRate = 1.0*spec4_correctCount/(inputSegment-1);
    std::cout<<"spec1_accuracyRate:"<<spec1_accuracyRate<<std::endl;
    std::cout<<"spec4_accuracyRate:"<<spec4_accuracyRate<<std::endl;

    float aveNumUniqueState_L100 = 1.0*sumUniqueState_L100/inputSegment;
    float aveNumUniqueState_L1000 = 1.0*sumUniqueState_L1000/inputSegment;
    float aveNumUniqueState_L10000 = 1.0*sumUniqueState_L10000/inputSegment;
    std::cout<<"aveNumUniqueState_L100:"<<aveNumUniqueState_L100<<std::endl;
    std::cout<<"aveNumUniqueState_L1000:"<<aveNumUniqueState_L1000<<std::endl;
    std::cout<<"aveNumUniqueState_L10000:"<<aveNumUniqueState_L10000<<std::endl;
    delete inputObj;
    delete tableObj;
    delete predObj;
}

}