// Submitter: bsmorton(Morton, Bradley)
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>                    //Biggest int: std::numeric_limits<int>::max()
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "array_priority_queue.hpp"
#include "array_set.hpp"
#include "array_map.hpp"


typedef ics::ArrayQueue<std::string>              CandidateQueue;
typedef ics::ArraySet<std::string>                CandidateSet;
typedef ics::ArrayMap<std::string,int>            CandidateTally;

typedef ics::ArrayMap<std::string,CandidateQueue> Preferences;
typedef ics::pair<std::string,CandidateQueue>     PreferencesEntry;
typedef ics::ArrayPriorityQueue<PreferencesEntry> PreferencesEntryPQ; //Must supply gt at construction

typedef ics::pair<std::string,int>                TallyEntry;
typedef ics::ArrayPriorityQueue<TallyEntry>       TallyEntryPQ;



//Read an open file stating voter preferences (each line is (a) a voter
//  followed by (b) all the candidates the voter would vote for, in
//  preference order (from most to least preferred candidate, separated
//  by semicolons), and return a Map of preferences: a Map whose keys are
//  voter names and whose values are a queue of candidate preferences.
Preferences read_voter_preferences(std::ifstream &file) {
    std::string line;
    Preferences p;
    while(std::getline(file,line)) {
        ics::ArrayQueue<std::string> temp;
        std::vector<std::string> spline=ics::split(line,";");
        for (int i=1; i<spline.size(); i++){
            temp.enqueue(spline[i]);
        }
        p.put(spline[0],temp);
    }
    return p;
}


//Print a label and all the entries in the preferences Map, in alphabetical
//  order according to the voter.
//Use a "->" to separate the voter name from the Queue of candidates.
void print_voter_preferences(const Preferences& preferences) {
    std::cout << "Voter -> queue[Preferences]" << std::endl;
    for(const ics::pair<std::string,ics::ArrayQueue<std::string>>& p : preferences){
        std::cout << "  " << p.first << " -> " << p.second << std::endl;
    }
}


//Print the message followed by all the entries in the CandidateTally, in
//  the order specified by has_higher_priority: i is printed before j, if
//  has_higher_priority(i,j) returns true: sometimes alphabetically by candidate,
//  other times by decreasing votes for the candidate.
//Use a "->" to separate the candidat name from the number of votes they
//  received.

bool alpha(const TallyEntry& a, const TallyEntry& b)
{return a.first < b.first;}

bool num(const TallyEntry& a, const TallyEntry& b)
{
    if(a.second==b.second){
        return a.first < b.first;
    }
    return a.second > b.second;
}

void print_tally(std::string message, const CandidateTally& tally, bool (*has_higher_priority)(const TallyEntry& i,const TallyEntry& j)) {
    std::cout << message << std::endl;
    ics::ArrayPriorityQueue<TallyEntry> q(has_higher_priority);
    q.enqueue_all(tally);
    for(TallyEntry& t: q){
        std::cout << "  " << t.first << " -> " << t.second << std::endl;
    }

}


//Return the CandidateTally: a Map of candidates (as keys) and the number of
//  votes they received, based on the unchanging Preferences (read from the
//  file) and the candidates who are currently still in the election (which changes).
//Every possible candidate should appear as a key in the resulting tally.
//Each voter should tally one vote: for their highest-ranked candidate who is
//  still in the the election.
CandidateTally evaluate_ballot(const Preferences& preferences, const CandidateSet& candidates) {
    CandidateTally ctal;
    for (std::string& s : candidates){

        ctal.put(s,0);
    }
    for (ics::pair<std::string,ics::ArrayQueue<std::string>>& p : preferences){
        ctal[p.second.dequeue()]+=1;
    }
    return ctal;
}


//Return the Set of candidates who are still in the election, based on the
//  tally of votes: compute the minimum number of votes and return a Set of
//  all candidates receiving more than that minimum; if all candidates
//  receive the same number of votes (that would be the minimum), the empty
//  Set is returned.
bool b2(const ics::pair<std::string,int>& a, const ics::pair<std::string,int>& b)
{return a.second > b.second;}


CandidateSet remaining_candidates(const CandidateTally& tally) {
    ics::ArrayPriorityQueue<ics::pair<std::string,int>,b2> pq;
    CandidateSet rc;
    pq.enqueue_all(tally);
    ics::pair<std::string,int> high;
    high=pq.dequeue();
    rc.insert(high.first);
    while(pq.empty()==0) {
        ics::pair<std::string, int> temp;
        temp = pq.dequeue();
        if (temp.second == high.second) {
            rc.insert(temp.first);
        }
    }
    return rc;
}






//Prompt the user for a file, create a voter preference Map, and print it.
//Determine the Set of all the candidates in the election, from this Map.
//Repeatedly evaluate the ballot based on the candidates (still) in the
//  election, printing the vote count (tally) two ways: with the candidates
//  (a) shown alphabetically increasing and (b) shown with the vote count
//  decreasing (candidates with equal vote counts are shown alphabetically
//  increasing); from this tally, compute which candidates remain in the
//  election: all candidates receiving more than the minimum number of votes;
//  continue this process until there are less than 2 candidates.
//Print the final result: there may 1 candidate left (the winner) or 0 left
//   (no winner).
bool b(const ics::pair<std::string,int>& a, const ics::pair<std::string,int>& b)
{return  a.second > b.second;}


int main() {
  try {
      std::string file;
      std::cout << "Enter a voter preferences file's name: ";
      std::getline(std::cin,file);
      //std::ifstream g1("votepref1.txt");
      std::ifstream g1(file);
      std::cout << std::endl;
      Preferences p=read_voter_preferences(g1);
      print_voter_preferences(p);
      ics::ArrayQueue<ics::pair<std::string,ics::ArrayQueue<std::string>>> temp;
      temp.enqueue_all(p);
      int length=temp.peek().second.size();
      CandidateSet cset;
      for(ics::pair<std::string,ics::ArrayQueue<std::string>>& j : p){
          if (cset.contains_all(j.second)==0){
              cset.insert_all(j.second);
          }
      }
      for(int i=0; i<length; i++){
          CandidateTally ctal=evaluate_ballot(p,cset);
          ics::ArrayPriorityQueue<ics::pair<std::string,int>,b> temp_q;
          temp_q.enqueue_all(ctal);
          int high=temp_q.peek().second;
          std::string temp_set="set[";
          for(std::string& str:cset){
              temp_set+=str+",";
          }
          temp_set[temp_set.size()-1]=']';
          std::string alpha_str=std::string("Vote count on ballot #")+std::to_string(i+1)+" with candidates (alphabetically ordered); remaining candidates = "+temp_set;
          print_tally(alpha_str,ctal,alpha);
          std::string num_str=std::string("Vote count on ballot #")+std::to_string(i+1)+" with candidates (numerically ordered); remaining candidates = "+temp_set;
          print_tally(num_str,ctal,num);
          for(std::string& s:cset){
              if(ctal[s]<high){
                  ctal.erase(s);
              }
          }
          cset.clear();
          for(ics::pair<std::string,int>& temp_pair:ctal){
              cset.insert(temp_pair.first);
          }
          if(ctal.size()<=1){
              break;
          }

      }
      std::string win;
      if(cset.size()!=1){
          win="NONE";
      }
      else{
          for (std::string& s2:cset){
              win=s2;
          }
      }

      std::cout << "Winner is " << win <<std::endl;


  } catch (ics::IcsError& e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
