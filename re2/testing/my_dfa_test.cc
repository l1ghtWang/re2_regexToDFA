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


// Test that the DFA gets the right result even if it runs
// out of memory during a search.  The regular expression
// 0[01]{n}$ matches a binary string of 0s and 1s only if
// the (n+1)th-to-last character is a 0.  Matching this in
// a single forward pass (as done by the DFA) requires
// keeping one bit for each of the last n+1 characters
// (whether each was a 0), or 2^(n+1) possible states.
// If we run this regexp to search in a string that contains
// every possible n-character binary string as a substring,
// then it will have to run through at least 2^n states.
// States are big data structures -- certainly more than 1 byte --
// so if the DFA can search correctly while staying within a
// 2^n byte limit, it must be handling out-of-memory conditions
// gracefully.

std::string readFileIntoString(const std::string& path) {
    std::ifstream input_file(path);
    if (!input_file.is_open()) {
        std::cerr << "Could not open the file - '" << path << "'" << std::endl;
        exit(EXIT_FAILURE);
    }
    return std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
}

TEST(SingleThreaded, SearchDFA) {
  // CHECK(RE2::FullMatch("4a", "[0-9]"));
  // CHECK(RE2::FullMatch("4a", "[0-9]"));

  char tmp [] = " (\\x2D .{1,20}\\x07(LAN|PROXY|MODEM|MODEM BUSY|UNKNOWN)\\x07Win)|(#-START-#([A-Za-z0-9+\\x2f]{4})*([A-Za-z0-9+\\x2f]{2}==|[A-Za-z0-9+\\x2f]{3}=)?#-END-#)|(%([01]|2([056A]|E%2E)|3[ACEF]|5C))|(%.*%)";
  // std::string tmp = ".{1,20}";
  // std::string tmp = "a{0,3}";
  std::cout<<"regex:"<<tmp<<std::endl;
  RE2 pattern(tmp);
  std::string contents = readFileIntoString("/home/wygzero/re2/ANMLZoo_regex/snort/snort_input.pcap");  


  // std::string contents = "baab";
  // std::cout<<"input:"<<contents<<std::endl;
  StringPiece input(contents);
  std::cout<<"lenght of contents:"<<contents.length()<<std::endl;

  // if(RE2::FullMatch(input, tmp))
  // {
  //   std::cout<<"FUllMatch pass\n";
  // }
  // else
  // {
  //   std::cout<<"FUllMatch failed\n";
  // }

  int matchCount = 0;
  while (RE2::FindAndConsume(&input, pattern)) {
    matchCount+=1;
  }
  std::cout<<"number of match:"<<matchCount<<std::endl;
}

// Helper function: searches for match, which should match,
// and no_match, which should not.
static void DoSearch(Prog* prog, const StringPiece& match,
                     const StringPiece& no_match) {
  for (int i = 0; i < 2; i++) {
    bool matched = false;
    bool failed = false;
    matched = prog->SearchDFA(match, StringPiece(), Prog::kUnanchored,
                              Prog::kFirstMatch, NULL, &failed, NULL);
    ASSERT_FALSE(failed);
    ASSERT_TRUE(matched);
    matched = prog->SearchDFA(no_match, StringPiece(), Prog::kUnanchored,
                              Prog::kFirstMatch, NULL, &failed, NULL);
    ASSERT_FALSE(failed);
    ASSERT_FALSE(matched);
  }
}


struct ReverseTest {
  const char* regexp;
  const char* text;
  bool match;
};

// Test that reverse DFA handles anchored/unanchored correctly.
// It's in the DFA interface but not used by RE2.
ReverseTest reverse_tests[] = {
  { "\\A(a|b)", "abc", true },
  { "(a|b)\\z", "cba", true },
  { "\\A(a|b)", "cba", false },
  { "(a|b)\\z", "abc", false },
};


struct CallbackTest {
  const char* regexp;
  const char* dump;
};

// Test that DFA::BuildAllStates() builds the expected DFA states
// and issues the expected callbacks. These test cases reflect the
// very compact encoding of the callbacks, but that also makes them
// very difficult to understand, so let's work through "\\Aa\\z".
// There are three slots per DFA state because the bytemap has two
// equivalence classes and there is a third slot for kByteEndText:
//   0: all bytes that are not 'a'
//   1: the byte 'a'
//   2: kByteEndText
// -1 means that there is no transition from that DFA state to any
// other DFA state for that slot. The valid transitions are thus:
//   state 0 --slot 1--> state 1
//   state 1 --slot 2--> state 2
// The double brackets indicate that state 2 is a matching state.
// Putting it together, this means that the DFA must consume the
// byte 'a' and then hit end of text. Q.E.D.
CallbackTest callback_tests[] = {
  { "\\Aa\\z", "[-1,1,-1] [-1,-1,2] [[-1,-1,-1]]" },
  { "\\Aab\\z", "[-1,1,-1,-1] [-1,-1,2,-1] [-1,-1,-1,3] [[-1,-1,-1,-1]]" },
  { "\\Aa*b\\z", "[-1,0,1,-1] [-1,-1,-1,2] [[-1,-1,-1,-1]]" },
  { "\\Aa+b\\z", "[-1,1,-1,-1] [-1,1,2,-1] [-1,-1,-1,3] [[-1,-1,-1,-1]]" },
  { "\\Aa?b\\z", "[-1,1,2,-1] [-1,-1,2,-1] [-1,-1,-1,3] [[-1,-1,-1,-1]]" },
  { "\\Aa\\C*\\z", "[-1,1,-1] [1,1,2] [[-1,-1,-1]]" },
  { "\\Aa\\C*", "[-1,1,-1] [2,2,3] [[2,2,2]] [[-1,-1,-1]]" },
  { "a\\C*", "[0,1,-1] [2,2,3] [[2,2,2]] [[-1,-1,-1]]" },
  { "\\C*", "[1,2] [[1,1]] [[-1,-1]]" },
  { "a", "[0,1,-1] [2,2,2] [[-1,-1,-1]]"} ,
};

}  // namespace re2
