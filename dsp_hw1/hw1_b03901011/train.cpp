#include "hmm.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>

using namespace std;

#define T 50
#define N 6
#define S 10000

int main(int argc, char **argv)//[./train][iteration][model_init.txt][seq_model_01.txt][model_01.txt]
{
  HMM iniModel;
  int iteration = atoi(argv[1]);
  char  *iniName = argv[2], *seqName = argv[3], *outName = argv[4];
	loadHMM( &iniModel, iniName );
  double alpha[T][N],beta[T][N], gamma[N][T], epsilon[T-1][N][N];
  double accGamma[N], accEpsilon[N][N], observGamma[N][N],firstGamma[N], lastGamma[N];

  //
  vector<string> seq;
  string line;
  ifstream fs;
  fs.open(seqName);
  while(getline(fs,line))
    seq.push_back(line);
  fs.close();
  //
  for(; iteration > 0; iteration-- ){

    for(int i = 0; i < N; i++){
      accGamma[i] = 0.0;
      firstGamma[i] = 0.0;
      lastGamma[i] = 0.0;
      for(int j = 0; j < N; j++){
        accEpsilon[i][j] = 0.0;
        observGamma[i][j] = 0.0;
      }
    }

    for(int lineNum = 0; lineNum < S; lineNum++){
      string line = seq[lineNum];

      //calculate alpha
      for(int i = 0; i < N; i++)
        alpha[0][i] = iniModel.initial[i] * iniModel.observation[line[0]-'A'][i];
      for(int t = 1; t < T; t++){
        for(int j = 0; j < N; j++){
          double tmp_sum = 0;
          for(int i = 0; i < N; i++)
            tmp_sum += alpha[t-1][i] * iniModel.transition[i][j];
          alpha[t][j] = tmp_sum * iniModel.observation[line[t]-'A'][j];
        }
      }

      //calculate beta
      for(int i = 0; i < N; i++)
          beta[T-1][i] = 1;
      for(int t = T-1; t > 0; t--)
        for(int i = 0; i < N; i++){
          double tmp_sum = 0;
          for(int j = 0; j < N; j++)
            tmp_sum += iniModel.transition[i][j] * iniModel.observation[line[t]-'A'][j] * beta[t][j];
          beta[t-1][i] = tmp_sum;
        }

      //calculate gamma
      for(int t = 0; t < T; t++){
        double tmp_sum = 0;
        for(int i = 0; i < N; i++)
          tmp_sum += alpha[t][i]*beta[t][i];
        for(int i = 0; i < N; i++){
          gamma[i][t] = alpha[t][i]*beta[t][i] / tmp_sum;
          if(t == T-1) lastGamma[i] += gamma[i][t];
          else accGamma[i] += gamma[i][t];//accumulate gamma
          observGamma[line[t]-'A'][i] += gamma[i][t];
        }
      }

      //calculate epsilon
      double p[N][N];
      for(int t = 0; t < T-1; t++){
        double tmp_sum = 0;
        for(int i = 0; i < N; i++){
          for(int j = 0; j < N; j++){
            p[i][j] = alpha[t][i]*iniModel.transition[i][j]*iniModel.observation[line[t+1]-'A'][j]*beta[t+1][j];
            tmp_sum += p[i][j];
          }
        }
        for(int i = 0; i < N; i++)
          for(int j = 0; j < N; j++){
            epsilon[t][i][j] = p[i][j]/tmp_sum;
            accEpsilon[i][j]+= epsilon[t][i][j];//accumulate epsilon
          }
      }
      //accumulate firstGamma
      for(int i = 0; i < N; i++)
        firstGamma[i] += gamma[i][0];
    }//end of one sample

    for(int i = 0; i < N; i++){
      iniModel.initial[i] = firstGamma[i]/S;
      for(int j = 0; j < N; j++)
        iniModel.transition[i][j] = accEpsilon[i][j]/accGamma[i];
      for(int k = 0; k < N; k++)
        iniModel.observation[k][i] = observGamma[k][i]/(accGamma[i]+lastGamma[i]);
    }

  }//end of iteration

  //HMM *result;
  strcpy(iniModel.model_name,argv[4]);
  FILE *fp = fopen(argv[4],"w");
  dumpHMM(fp,&iniModel);
  fclose(fp);

  return 0;
}
