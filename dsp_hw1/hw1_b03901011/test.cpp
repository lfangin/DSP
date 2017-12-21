#include "hmm.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <iomanip>


using namespace std;
#define N 6
#define T 50

int main(int argc, char **argv)//./test  modellist.txt  testing_data.txt  result.txt
{
  char *list = argv[1], *testData = argv[2], *result = argv[3];
  HMM models[5];
  load_models(list,models,5);

  vector<string> seq,ans;
  string line;
  ifstream fs,answer;
  fs.open(testData);
  while(getline(fs,line))
    seq.push_back(line);
  fs.close();

  string testAns = "testing_answer.txt";
  answer.open(testAns.c_str());
  while(getline(answer,line)){
    ans.push_back(line);
  }
  answer.close();

  //string f = "forward_ans.txt";
  FILE *fp = fopen(result,"w");
  //FILE *fp2 = fopen(f.c_str(),"w");

  double maxProb[5], delta[T][N], max,tmp_sum, tmp, acc = 0, acc2 = 0;
  double alpha[T][N],forward[5]={0};
  int S = seq.size();

  for(int s = 0; s < S; s++){
    line = seq[s];
    for(int i = 0 ; i < 5; i++) forward[i] = 0;
    for(int num_model = 0; num_model < 5; num_model++){
      for(int i = 0; i < N; i++){
        delta[0][i] = models[num_model].initial[i]*models[num_model].observation[line[0]-'A'][i];
        //alpha
        //alpha[0][i] = models[num_model].initial[i] * models[num_model].observation[line[0]-'A'][i];
      }
      for(int t = 1; t < T; t++){
        for(int j = 0; j < N; j++){
          max = 0;
          //tmp_sum = 0;
          for(int i = 0; i < N; i++){
            tmp = delta[t-1][i]*models[num_model].transition[i][j];
            if( tmp > max) max = tmp;
            //alpha
            //tmp_sum += alpha[t-1][i] * models[num_model].transition[i][j];
          }
          delta[t][j] = max*models[num_model].observation[line[t]-'A'][j];
          //alpha
          //alpha[t][j] = tmp_sum * models[num_model].observation[line[t]-'A'][j];
        }
      }
      max = 0;
      for(int i = 0; i < N; i++){
        //forward[num_model]+=alpha[T-1][i];
        if(delta[T-1][i] > max) max = delta[T-1][i];
      }
      maxProb[num_model] = max;

    }

    max = 0;
    int m = 0,n = 0;
    double tmp = 0;
    for(int i = 0; i < 5; i++){
      if(maxProb[i] > max){
        max = maxProb[i];
        m = i;
      }
      //if(forward[i] > tmp){
        //tmp = forward[i];
        //n = i;
      //}
    }

  fprintf(fp, "%s %e\n",models[m].model_name, max);
  if(ans[s] == models[m].model_name) acc++;
  //fprintf(fp2, "%s %e\n",models[n].model_name, tmp);
  //if(ans[s] == models[n].model_name) acc2++;
  }

  fclose(fp);
  //fclose(fp2);
  //output acc
  ofstream out;
  string a = "acc.txt";
  out.open(a.c_str());
  out << acc/S << endl;
  //out << acc2/S << endl;
  out.close();

  return 0;
}
