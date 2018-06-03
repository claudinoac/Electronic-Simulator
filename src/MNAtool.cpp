#include "MNAtool.h"
#include <stdio.h>
#include <cstdlib>


double** MNAtool::stamp2MNA(int mnaSize , int numNodes, component *componente)
{
  int stampMAP[6];

  double** MNA = (double**)malloc(mnaSize * sizeof(double*));
  for (int index=0;index<mnaSize;++index)
{
    MNA[index] = (double*)malloc(mnaSize * sizeof(double*));
}

  for(int i=0;i<mnaSize;i++)
  {
    for(int j=0;j<mnaSize;j++)
    {
      MNA[i][j]=0;
    }

  }

  stampMAP[0]=componente->n1;
  stampMAP[1]=componente->n2;
  stampMAP[2]=componente->n3;
  stampMAP[3]=componente->n4;
  stampMAP[4]=componente->ident+numNodes;
  stampMAP[5]=componente->control+numNodes;

  for(int i=0;i<6;i++)
  {
    for(int j=0;j<6;j++)
    {
      if(componente->stamp[i][j]!=0)
      {
        MNA[stampMAP[i]][stampMAP[j]]=componente->stamp[i][j];
      }
    }
  }
  return MNA;
}

void MNAtool::stampGenerator(component *componente)
{
  for(int i=0;i<6;i++)
  {
    for(int j=0;j<6;j++)
    {
      componente->stamp[i][j]=0;
    }
  }
  switch(componente->type)
  {
    case 'R':
    {
      if(componente->G2)
      {
        componente->stamp[0][4]=1;
        componente->stamp[1][4]=-1;
        componente->stamp[4][0]=1;
        componente->stamp[4][1]=-1;
        componente->stamp[4][4]=-(componente->value);
      }
      else
      {
        componente->stamp[0][0]=1/(componente->value);
        componente->stamp[0][1]=-1/(componente->value);
        componente->stamp[1][0]=-1/(componente->value);
        componente->stamp[1][1]=1/(componente->value);
      }
      break;
    }
    case 'V':
    {
      componente->stamp[4][0]=1;
      componente->stamp[4][1]=-1;
      componente->stamp[0][4]=1;
      componente->stamp[1][4]=-1;
      componente->RHS[4]=componente->value;
      break;
    }
    case 'I':
    {
      if(componente->G2)
      {
        componente->stamp[0][4]=1;
        componente->stamp[1][4]=-1;
        componente->stamp[4][4]=1;
        componente->RHS[4]=componente->value;
      }
      else
      {
        componente->RHS[0]=componente->value;
        componente->RHS[1]=-(componente->value);
      }
      break;
    }
    case 'E':
    {
      componente->stamp[0][4]=1;
      componente->stamp[1][4]=-1;
      componente->stamp[4][0]=1;
      componente->stamp[4][1]=-1;
      componente->stamp[4][2]=-(componente->value);
      componente->stamp[4][3]=componente->value;
      break;
    }
    case 'F':
    {
      if(componente->G2)
      {
        componente->stamp[0][4]=1;
        componente->stamp[1][4]=-1;
        componente->stamp[4][4]=1;
        componente->stamp[4][5]=componente->value;
      }
      else
      {
        componente->stamp[0][5]=componente->value;
        componente->stamp[1][5]=-(componente->value);
      }
      break;
    }
    case 'G':
    {
      if(componente->G2)
      {
        componente->stamp[0][4]=1;
        componente->stamp[1][4]=-1;
        componente->stamp[4][2]=-(componente->value);
        componente->stamp[4][3]=componente->value;
        componente->stamp[4][4]=1;
      }
      else
      {
        componente->stamp[0][2]=componente->value;
        componente->stamp[0][3]=-(componente->value);
        componente->stamp[1][2]=-(componente->value);
        componente->stamp[1][3]=componente->value;
      }
      break;
    }
    case 'H':
    {
      componente->stamp[0][4]=1;
      componente->stamp[1][4]=1;
      componente->stamp[4][0]=1;
      componente->stamp[4][1]=1;
      componente->stamp[4][5]=componente->value;
      break;
    }
  }
}
