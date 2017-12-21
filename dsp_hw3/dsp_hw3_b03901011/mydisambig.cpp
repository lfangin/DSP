#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "File.h"
#include "Vocab.h"
#include "Ngram.h"
#include "Prob.h"
using namespace std;

vector<vector<string> > ZtoC;

double getBigramProb(const char *w1, const char *w2, Ngram& lm, Vocab& voc);
double getTrigramProb(const char *w1, const char *w2, const char *w3, Ngram& lm, Vocab& voc);
double getUnigramProb(const char *w1, Ngram& lm, Vocab& voc);
void String2Vec(vector<string>& vec, string line)
{
  size_t pos = 0;
  string tmp;
  while(true){
    pos = line.find_first_not_of(" ",pos);
    if(pos!=string::npos){
      tmp = line.substr(pos,2);
      vec.push_back(tmp);
      pos += 2;
    }
    else break;
  }
}

int mapIndex(string str)
{
  for(int zhuyin = 0; zhuyin < ZtoC.size(); zhuyin++)//identify zhuyin
    if(str == ZtoC[zhuyin][0]) return zhuyin;
  return -1;
}

int main(int argc , char *argv[]){ // ./mydisambig seg_*.txt ZhuYin-Big5.map bigram.lm 2
  //read bigram
    int ngram_order = atoi(argv[4]);
    Vocab voc;
    Ngram lm( voc, ngram_order );
    {
        File lmFile( argv[3], "r" );
        lm.read(lmFile);
        lmFile.close();
    }
  //read map
  string line;
  ifstream map;
  map.open(argv[2]);
  while(getline(map,line)){
    vector<string> tmp;
    String2Vec(tmp,line);
    ZtoC.push_back(tmp);
  }
  map.close();

  // readfile
  ifstream fs;
  fs.open(argv[1]);
  while(getline(fs,line)){
    double prob[256][1024] = {0.0};
    vector<int> pos;
    vector<string> tmp;
    String2Vec(tmp,line);
    int trace[256][1024] = {-1};

    int Id = mapIndex(tmp[0]);
    pos.push_back(Id);
    double maxProb = -100; // for the comparison in first word
    double uniProb ;

    for(int j = 1; j < ZtoC[Id].size(); j++){
      uniProb = getUnigramProb(ZtoC[Id][j].c_str(),lm,voc);
      prob[0][j-1] = uniProb; // prob[][j-1], j is the position in ZtoC
      if(prob[0][j-1] == LogP_Zero) prob[0][j-1] = ( -100 );
    }
    int preId = Id;

    for(size_t i = 1; i < tmp.size(); i++){ // iterate words in line, except the first word
      Id = mapIndex(tmp[i]);
      pos.push_back(Id);
        for(int j = 1; j < ZtoC[Id].size(); j++){ //iterate condidate words in the ith word
          double uniProb = getUnigramProb(ZtoC[Id][j].c_str(),lm,voc);
          double maxProb = -100;
          for(int k = 1; k < ZtoC[preId].size(); k++){ // iterate candidate words in previous word
            double biProb = getBigramProb(ZtoC[preId][k].c_str(),ZtoC[Id][j].c_str(),lm,voc);
            if(biProb == LogP_Zero && uniProb ==LogP_Zero) biProb = -100;
            biProb += prob[i-1][k-1];
            if(biProb > maxProb){
              maxProb = biProb;
              trace[i][j] = k;
            }
          }
          prob[i][j-1] = (maxProb);
        }
      preId = Id;
    }

    // Viterbi, last word
    double max = -100;
    int j = 0, lastResult = -1;
    for(int j = 1; j < ZtoC[preId].size(); j++ ){
      if(prob[tmp.size()-1][j-1] > max){
        max = prob[tmp.size()-1][j-1];
        lastResult = j;
      }
    }
    // Viterbi, recursive
    int result[256] = {-1};
    result[tmp.size()-1] = lastResult;
    for(int i = tmp.size()-2; i >= 0; i--){
      result[i] = trace[i+1][result[i+1]];
    }
    //output
    cout << "<s> " ;
    for(int i = 0; i < tmp.size(); i++)
      cout << ZtoC[pos[i]][result[i]] << " ";
    cout << "</s>" << '\n';
  }
  fs.close();
  return 0;
}

// Get unigram
double getUnigramProb(const char *w1, Ngram& lm, Vocab& voc)
{
  VocabIndex wid1 = voc.getIndex(w1);
  if(wid1 == Vocab_None)  //OOV
    wid1 = voc.getIndex(Vocab_Unknown);
  VocabIndex context[] = { Vocab_None };
  return lm.wordProb(wid1, context);
}


// Get P(W2 | W1) -- bigram
double getBigramProb(const char *w1, const char *w2, Ngram& lm, Vocab& voc)
{
    VocabIndex wid1 = voc.getIndex(w1);
    VocabIndex wid2 = voc.getIndex(w2);

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);
    if(wid2 == Vocab_None)  //OOV
        wid2 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { wid1, Vocab_None };
    return lm.wordProb( wid2, context);
}

// Get P(W3 | W1, W2) -- trigram
double getTrigramProb(const char *w1, const char *w2, const char *w3,  Ngram& lm, Vocab& voc)
{
    VocabIndex wid1 = voc.getIndex(w1);
    VocabIndex wid2 = voc.getIndex(w2);
    VocabIndex wid3 = voc.getIndex(w3);

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);
    if(wid2 == Vocab_None)  //OOV
        wid2 = voc.getIndex(Vocab_Unknown);
    if(wid3 == Vocab_None)  //OOV
        wid3 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { wid2, wid1, Vocab_None };
    return lm.wordProb( wid3, context );
}
