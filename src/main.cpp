#include "MNAtool.cpp"
#include "netlist_parser.cpp"



int main()
{
  netlist_parser parser;

  parser.begin();
  MNAtool mna;
  int mnaSize=parser.numNodes+parser.numComp;


  for(int i=0;i<parser.numComp;i++)
    {
      mna.stampGenerator(&parser.componentes[i]);
    }

    for(int k=0;k<parser.numComp;k++)
    {
      for(int i=0;i<6;i++)
      {
        for(int j=0;j<6;j++)
        {
        printf(" %.2lf",parser.componentes[k].stamp[i][j]);
        }
        printf("   %.2lf",parser.componentes[k].RHS[i]);
        printf("\n");
      }
      printf("\n\n");
    }

    double **MNA=mna.stamp2MNA(mnaSize,parser.numNodes, &parser.componentes[1]);
    for(int i=0;i<parser.numNodes+parser.numComp;i++)
    {
      for(int j=0;j<parser.numNodes+parser.numComp;j++)
      {

        printf(" %.2lf",MNA[i][j]);
        printf
      }
      printf("\n");
    }
}
