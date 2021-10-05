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

// #include "util/inputFileName.h"
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

extern std::string regexFileName;
std::string benchmarkPath = "/home/yugwang/re2_regexToDFA/ANMLZoo_regex/snort/snort_";
// std::string benchmarkPath = "/home/yugwang/re2_regexToDFA/ANMLZoo_regex/clamAV/clamAV_";
// std::string benchmarkPath = "/home/yugwang/re2_regexToDFA/ANMLZoo_regex/protomata/protomata_";
// std::string benchmarkPath = "/home/yugwang/re2_regexToDFA/ANMLZoo_regex/brill/brill_";

int dfaIdx = 0;
std::string regexFileName = benchmarkPath+"regex"+std::to_string(dfaIdx)+".regex";
std::string inputFileName(benchmarkPath+"TestInput2.in");
// std::string inputFileName(benchmarkPath+"Input_10MB.input");
// std::string inputFileName(benchmarkPath+"vasim_10MB.input");
// std::string inputFileName(benchmarkPath+"10MB.input");
// std::string inputFileName(benchmarkPath+"30k.input");
// std::string inputFileName(benchmarkPath+"brownCorpus.input");

std::string tableFileName;
std::string acFileName(benchmarkPath+"AC"+std::to_string(dfaIdx)+".txt");


TEST(SingleThreaded, BuildEntireDFA) {
  std::cout<<"open:"<<regexFileName<<std::endl;
  std::string s = readFileIntoString(regexFileName);

  Regexp* re = Regexp::Parse(s, Regexp::LikePerl|Regexp::Latin1, NULL);
  ASSERT_TRUE(re != NULL);

  Prog* prog = re->CompileToProg(256*1024*1024*1024);
  ASSERT_TRUE(prog != NULL);
  prog->BuildEntireDFA(Prog::kManyMatch, nullptr, benchmarkPath, dfaIdx, tableFileName);
//   prog->BuildEntireDFA(Prog::kFirstMatch, nullptr, benchmarkPath, dfaIdx, tableFileName);
  
  delete prog;
  
  re->Decref();
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

TEST(SingleThreaded, ProfilingDFA) {

    std::cout<<"tableFileName:"<<tableFileName<<std::endl;
    int start = 0;

    FSM::MappingRule* rules_ = FSM::MappingRule::defineMappingRule();
	Input* inputObj = Input::readFromFile(inputFileName, rules_);
    long inputLength = inputObj->getLength(); 
    std::cout<<inputLength<<std::endl;


    Table* tableObj = Table::readFromFile(tableFileName.c_str(), acFileName.c_str(), start, rules_);
    // tableObj->printTable();

    uint8_t* inputPtr = inputObj->getHostPointer();
    int* tableList = tableObj->getTable();
    int state = start;
    int nSymbol = tableObj->getNumSymbol();
    int numState = tableObj->getNumState();
    

    
    long acceptCount = 0;
    // for(long i = 0; i < inputLength; i++)
    // {
    //     uint8_t data = inputPtr[i];
    //     state = tableList[state * nSymbol + data];
    //     if(tableObj->isAccept(state))
    //         acceptCount+=1;
    // }
    // std::cout<<"acceptCount:"<<acceptCount<<std::endl;


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
    int sumUniqueState_L2 = 0;
    int sumUniqueState_L10 = 0;
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
            //check # unique state when 2 symbols executed 
            if(executionLen==2)
            {
                //remove reduntant paths
                set<int> s;
                unsigned size = execPath.size();
                for( unsigned i = 0; i < size; ++i ) 
                    s.insert( execPath[i] );
                execPath.assign( s.begin(), s.end() );
                // std::cout<<"inputSegment["<<segmentIdx<<"],"<<execPath.size();
                sumUniqueState_L2 += execPath.size();
            }
            //check # unique state when 10 symbols executed 
            if(executionLen==10)
            {
                //remove reduntant paths
                set<int> s;
                unsigned size = execPath.size();
                for( unsigned i = 0; i < size; ++i ) 
                    s.insert( execPath[i] );
                execPath.assign( s.begin(), s.end() );
                // std::cout<<"inputSegment["<<segmentIdx<<"],"<<execPath.size();
                sumUniqueState_L10 += execPath.size();
            }             
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

    std::cout<<"acceptCount:"<<acceptCount<<std::endl;

    float spec1_accuracyRate = 1.0*spec1_correctCount/(inputSegment-1);
    float spec4_accuracyRate = 1.0*spec4_correctCount/(inputSegment-1);
    std::cout<<"spec1_accuracyRate:"<<spec1_accuracyRate<<std::endl;
    std::cout<<"spec4_accuracyRate:"<<spec4_accuracyRate<<std::endl;

    float aveNumUniqueState_L2 = 1.0*sumUniqueState_L2/inputSegment;
    float aveNumUniqueState_L10 = 1.0*sumUniqueState_L10/inputSegment;
    float aveNumUniqueState_L100 = 1.0*sumUniqueState_L100/inputSegment;
    float aveNumUniqueState_L1000 = 1.0*sumUniqueState_L1000/inputSegment;
    float aveNumUniqueState_L10000 = 1.0*sumUniqueState_L10000/inputSegment;
    std::cout<<"aveNumUniqueState_L2:"<<aveNumUniqueState_L2<<std::endl;
    std::cout<<"aveNumUniqueState_L10:"<<aveNumUniqueState_L10<<std::endl;
    std::cout<<"aveNumUniqueState_L100:"<<aveNumUniqueState_L100<<std::endl;
    std::cout<<"aveNumUniqueState_L1000:"<<aveNumUniqueState_L1000<<std::endl;
    std::cout<<"aveNumUniqueState_L10000:"<<aveNumUniqueState_L10000<<std::endl;
    delete inputObj;
    delete tableObj;
    delete predObj;
}

}  // namespace re2
