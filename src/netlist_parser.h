#ifndef NETLIST_PARSER_H
#define NETLIST_PARSER_H

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "component.h"

#define btoa(x) ((x)?"true":"false")

class netlist_parser
{
  public:
  //Essa struct deve ser reimplementada como LinkedList posteriormente

    char comments[1000][1000];            //Capacidade de comentários de 1000 cada um com 1000 caracteres
    char command[1000][50];               //Capacidade de armazenar 1000 comandos de 50 caracteres
    char nodes[10000][50];                //Capacidade de armazenar 10000 nós de 50 caracteres para nome
    int  numNodes=1;                      //Inicializa a variável que indica o número de nós no circuito
    int numComp=0;                        //Inicializa a variável que conta o número de componentes
    component componentes[10000];  //Inicaliza o array de struct que reunirá informações sobre cada componente
    netlist_parser();
    bool begin();
    void showNodes(FILE *stdout);
    void showCompInfo(FILE *stdout);
    void parseValues(char *number, int cmpNum, char* cmpName);
    int nodeVDD(char *nodename);
    void nodeSet(int compnumber, int nodetype, char *nodename);
  private:
};
#endif
