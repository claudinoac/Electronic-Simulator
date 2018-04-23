/*
*   SPICE NETLIST PARSER
*   Autor: Alisson Claudino
*   Licença: GNU GPLv3
*   Propósito: Mapear componentes e nós de um circuito de um arquivo padrão SPICE para realizar simulações
*/

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
using namespace std;


//Essa struct deve ser reimplementada como LinkedList posteriormente
struct component      //O componente é representado por uma estrutura de dados que armazenará todas as informações sobre ele
{
  char name[30];      //nome do componente
  char type;          //tipo de componente (capacitor, transistor, etc)
  char equation[20];  //equação para relação entrada/saída (somente para componentes não-lineares)
  double value;       //valor do componente (para componentes lineares)
  int ident;          //ordem do componente (apenas para contagem)
  int node1;          //número do nó ligado ao positivo (quando node3=-1 e node4=-1) ou coletor do TJB ou dreno do MOSFET
  int node2;          //número do nó ligado ao negativo (quando node3=-1 e node4=-1) ou base do TJB ou gate do MOSFET
  int node3;          //número do nó ligado ao positivo da saída para fontes ctrl, emissor do TJB ou source do MOSFET. valor -1 para componentes de 2 pinos
  int node4;          //número do nó ligado ao negativo da saída para fontes ctrl. valor -1 para componentes de 2 ou 3 pinos
};

char comments[1000][1000];            //Capacidade de comentários de 1000 cada um com 1000 caracteres
char command[1000][50];               //Capacidade de armazenar 1000 comandos de 50 caracteres
char nodes[10000][50];                //Capacidade de armazenar 10000 nós de 50 caracteres para nome
int  numNodes=1;                      //Inicializa a variável que indica o número de nós no circuito
int numComp=0;                        //Inicializa a variável que conta o número de componentes
struct component componentes[10000];  //Inicaliza o array de struct que reunirá informações sobre cada componente

void showNodes(FILE *stdout)                              //Função para imprimir a lista de nós em um arquivo externo. Recebe um ponteiro pro arquivo de saída
{
  int i;                                                  //Variável auxiliar para varredura
  fprintf(stdout,"\n\n\tNODE LIST:\n\nIndex\tName\n\n");  //Primeira linha, cabeçalho
  for(i=0;i<numNodes;i++)                                 //Realiza uma varredura no array de nós, imprimindo os seus índices e nomes
  {
    fprintf(stdout,"#%d:\t %s\n",i,nodes[i]);
  }
}

void showCompInfo(FILE *stdout)                           //Função para imprimir a lista de componentes com informações sobre os mesmos
{
  fprintf(stdout,"\n\n\t\t\t\tCOMPONENT LIST\n\nIndex\tName\t\t\t\tConnections\t\t\t\tValue/Equation\n");  //Imprime cabeçalho no arquivo
  int i=0;
  for(i=0;i<numComp;i++)                                  //Loop para imprimir dados de cada componente, realizando uma varredura no array de struct
  {
    fprintf(stdout,"\n#%d:\t",i);                         //Imprime índice do Componente
    switch(componentes[i].type)                           //Classifica os componentes e imprime as informações de acordo com o tipo
    {
      case 'R':                                                 //Resistor
      {
        fprintf(stdout,"RES[%s]\t;\t",componentes[i].name);
        fprintf(stdout,"n+[%d]\t\t",componentes[i].node1);
        fprintf(stdout,"n-[%d]\t\t;\t",componentes[i].node2);
        fprintf(stdout,"Value=%g\n",componentes[i].value);
        break;
      }
      case 'L':                                                 //Indutor
      {
        fprintf(stdout,"IND[%s]\t;\t",componentes[i].name);
        fprintf(stdout,"n+[%d]\t\t",componentes[i].node1);
        fprintf(stdout,"n-[%d]\t\t;\t",componentes[i].node2);
        fprintf(stdout,"Value=%g\n",componentes[i].value);
        break;
      }
      case 'C':                                                //Capacitor
      {
        fprintf(stdout,"CAP[%s]\t;\t",componentes[i].name);
        fprintf(stdout,"n+[%d]\t\t",componentes[i].node1);
        fprintf(stdout,"n-[%d]\t\t;\t",componentes[i].node2);
        fprintf(stdout,"Value=%g\n",componentes[i].value);
        break;
      }
      case 'V':                                                //Fonte de tensão
      {
        fprintf(stdout,"VSR[%s]\t;\t",componentes[i].name);
        fprintf(stdout,"n+[%d]\t\t",componentes[i].node1);
        fprintf(stdout,"n-[%d]\t\t;\t",componentes[i].node2);
        fprintf(stdout,"Value=%g\n",componentes[i].value);
        break;
      }
      case 'I':                                                //Fonte de Corrente
      {
        fprintf(stdout,"ISR[%s]\t;\t",componentes[i].name);
        fprintf(stdout,"n+[%d]\t\t",componentes[i].node1);
        fprintf(stdout,"n-[%d]\t\t;\t",componentes[i].node2);
        fprintf(stdout,"Value=%g\n",componentes[i].value);
        break;
      }
      case 'E':                                                //Fonte de tensão dependente de tensão
      {
        fprintf(stdout,"VCVS[%s]\t;\t",componentes[i].name);
        fprintf(stdout,"n+[%d]\t",componentes[i].node1);
        fprintf(stdout,"n-[%d]\t",componentes[i].node2);
        fprintf(stdout,"nC+[%d]\t",componentes[i].node3);
        fprintf(stdout,"nC-[%d]\t;\t",componentes[i].node4);
        fprintf(stdout,"Value=%g\n",componentes[i].value);
        break;
      }
      case 'F':                                                //Fonte de corrente controlada por corrente
      {
        fprintf(stdout,"CCCS[%s]\t;\t",componentes[i].name);
        fprintf(stdout,"n+[%d]\t",componentes[i].node1);
        fprintf(stdout,"n-[%d]\t",componentes[i].node2);
        fprintf(stdout,"nC+[%d]\t",componentes[i].node3);
        fprintf(stdout,"nC-[%d]\t;\t",componentes[i].node4);
        fprintf(stdout,"Value=%g\n",componentes[i].value);
        break;
      }
      case 'G':                                               //Fonte de corrente controlada por tensão
      {
        fprintf(stdout,"VCCS[%s]\t;\t",componentes[i].name);
        fprintf(stdout,"n+[%d]\t",componentes[i].node1);
        fprintf(stdout,"n-[%d]\t",componentes[i].node2);
        fprintf(stdout,"nC+[%d]\t",componentes[i].node3);
        fprintf(stdout,"nC-[%d]\t;\t",componentes[i].node4);
        fprintf(stdout,"Value=%g\n",componentes[i].value);
        break;
      }
      case 'H':                                               //Fonte de tensão controlada por corrente
      {
        fprintf(stdout,"CCVS[%s]\t;\t",componentes[i].name);
        fprintf(stdout,"n-[%d]\t",componentes[i].node2);
        fprintf(stdout,"n+[%d]\t",componentes[i].node1);
        fprintf(stdout,"nC+[%d]\t",componentes[i].node3);
        fprintf(stdout,"nC-[%d]\t;\t",componentes[i].node4);
        fprintf(stdout,"Value=%g\n",componentes[i].value);
        break;
      }
      case 'D':                                               //Diodo
      {
        fprintf(stdout,"DIO[%s]\t;\t",componentes[i].name);
        fprintf(stdout,"n+[%d]\t\t",componentes[i].node1);
        fprintf(stdout,"n-[%d]\t\t;\t",componentes[i].node2);
        fprintf(stdout,"Equation=%s\n",componentes[i].equation);
        break;
      }
      case 'Q':                                                //Transistor de Junção Bipolar
      {
        fprintf(stdout,"BJT[%s]\t;\t",componentes[i].name);
        fprintf(stdout,"nC[%d]\t\t",componentes[i].node1);
        fprintf(stdout,"nB[%d]\t",componentes[i].node2);
        fprintf(stdout,"nE[%d]\t;\t",componentes[i].node3);
        fprintf(stdout,"Equation=%s\n",componentes[i].equation);
        break;
      }
      case 'M':                                               //Transistor MOSFET
      {
        fprintf(stdout,"MOS[%s]\t;\t",componentes[i].name);
        fprintf(stdout,"nG[%d]\t",componentes[i].node2);
        fprintf(stdout,"nD[%d]\t\t",componentes[i].node1);
        fprintf(stdout,"nS[%d]\t",componentes[i].node3);
        fprintf(stdout,"nSS[%d]\t;\t",componentes[i].node4);
        fprintf(stdout,"Equation=%s\n",componentes[i].equation);
        break;
      }
    }
  }
}

void parseValues(char *number, int cmpNum, char* cmpName)  //Função para traduzir os valores indiferente da notação e armazenar no componente pelo seu índice
{
  char auxStr[100]; //String auxiliar para armazenar a string de número
  char bufStr[100]; //String buffer para pegar partes separadas do número (antes e após o ponto, por exemplo)
  bool expFlag=false,dotFlag=false,mulFlag=false,negExpFlag=false; //Flags auxiliares para identificar a notação
  strcpy(auxStr,number);
  int aux1=0,aux2=0;
  double posD=0, preD=0,expo=0;
  double num=0;
  while(aux1!=strlen(auxStr))  //percorre toda a string valor
  {
    if(isdigit(auxStr[aux1]))  //verifica se o caractere é um número
    {
      bufStr[aux2]=auxStr[aux1];
      aux2++;
    }
    else if(auxStr[aux1]=='.') //encontrando o ponto, separa a parte inteira
    {
      bufStr[aux2+1]='\0';
      sscanf(bufStr, "%lf", &preD);
      aux2=0;
      memset(bufStr, 0, sizeof(bufStr));
      dotFlag=true;
    }

    else if(isalpha(auxStr[aux1])) //encontrando uma letra, verifica, separa a parte fracionária e aplica a multiplicação
    {
      if(auxStr[aux1]=='+' || auxStr[aux1]=='-');
      if(dotFlag)                           //Se achou o ponto
      {
        bufStr[aux2+1]='\0';
        sscanf(bufStr, "%lf", &posD);       //Então transfere o valor acumulado para a variável posD
        posD=posD/pow(10,aux2);             //Realiza a divisão para o valor após o ponto
        aux2=0;
        memset(bufStr, 0, sizeof(bufStr));  //Limpa o buffer
      }
      else
      {
        bufStr[aux2+1]='\0';
        sscanf(bufStr, "%lf", &preD);       //Se há valores após a virgula, finaliza o valor e passa para análise do multiplicador
        aux2=0;
        memset(bufStr, 0, sizeof(bufStr));
      }
      if(toupper(auxStr[aux1])=='E')  //Verifica se a notação é exponencial
      {
        expFlag=true;                   //Seta a flag indicando que há exponencial
        if(auxStr[aux1+1]=='-')
        {
          negExpFlag=true;
        }
      }
      else
      {
        mulFlag=true;                 //Se não é exponencial, então é notação multiplicador (k,m,n,M,G...)
        switch(tolower(auxStr[aux1])) //identifica o multiplicador
        {
          case 'f':
          {
            expo=-15;
            break;
          }
          case 'p':
          {
            expo=-12;
            break;
          }
          case 'u':
          {
            expo=-6;
            break;
          }
          case 'n':
          {
            expo=-9;
            break;
          }
          case 'm':
          {
            if(auxStr[aux1]=='M')
            {
              expo=6;
            }
            else
            {
              expo=-3;
            }
            break;
          }
          case 'k':
          {
            expo=3;
            break;
          }
          case 'g':
          {
            expo=9;
            break;
          }
          case 't':
          {
            expo=12;
            break;
          }
          default:                                          //Se a letra encontrada não for de um multiplicador, emite um erro.
          {
            printf("\nErro: Multiplicador do valor de %s Inválido!\nSetando exponente 0",cmpName);
            expo=0;
            break;
          }
        }
      }
    }
    aux1++;     //Aumenta a variável auxiliar e segue o loop
  }
  if(!dotFlag && !expFlag)  //se não há ponto nem expoente, separa somente a parte inteira
  {
    bufStr[aux2+1]='\0';
    sscanf(bufStr, "%lf", &preD);
  }
  if(dotFlag && (!mulFlag && !expFlag)) //se possui ponto, mas não possui exponente ou multiplicador, separa a parte fracionária
  {
    bufStr[aux2+1]='\0';
    sscanf(bufStr, "%lf", &posD);
    posD=posD/pow(10,aux2);
    expo=0;
  }
  if(expFlag) //se possui expoente, aplica-o (notação exponencial)
  {
    sscanf(bufStr, "%lf", &expo);
    if(negExpFlag)
    expo=-expo;
  }
  num=preD+posD; //soma as partes inteira e fracionária
  num=num*pow(10,expo); //aplica o multiplicador
  componentes[cmpNum].value=num;
  //printf("\n%lf",num);
}

int nodeVDD(char *nodename) //Verifica se o nó já existe, adiciona na lista se ainda não existir e retorna o número do nó
{

  bool exists=false;
  int aux=0,nodenumber=0;
  while(aux!=numNodes) //Percorre o array de nós
  {
    if(strcmp(nodes[aux],nodename)==0) //Compara o nome dado com os já existentes
    {
      exists=true;
      nodenumber=aux;

    }
    aux++;
  }

  if(!exists) //Caso o nó não exista, adiciona mais um índice e guarda no array
  {
    strcpy(nodes[numNodes],nodename);
    nodenumber=numNodes;
    numNodes++;
  }

  return nodenumber; //Retorna o número do nó
}

void nodeSet(int compnumber, int nodetype, char *nodename) //Associa os componentes aos seus nós respectivos
{
  switch(nodetype)
  {
    case 1: //nó positivo do componente (Coletor para TJB e Dreno para MOSFET)
    {
      componentes[compnumber].node1=nodeVDD(nodename); //Passa o número do nó para o componente
      break;
    }

    case 2: //nó negativo do componente (Base para TJB e Gate para MOSFET)
    {
      componentes[compnumber].node2=nodeVDD(nodename);
      break;
    }

    case 3: //nó positivo de saída para fontes controladas, Emissor para TJB e Source para MOSFET
    {
      componentes[compnumber].node3=nodeVDD(nodename);
      break;
    }

    case 4: //nó negativo de saída para fontes controladas
    {
      componentes[compnumber].node4=nodeVDD(nodename);
      break;
    }
  }
}

int main(void)          //Função principal
{

  int linecount = 0;        //Contador de linhas
  bool compFlag = false;    //Flag para indicar que um componente foi encontrado na linha
  nodes[0][0]='0';          //Inicializa o array de nós com o nó de referência
  nodes[0][1]='\0';
  char bufStr[100];
  int strcount=1;           //Contrador auxiliar para quebra de linha
  int wordcount=0;          //Contador auxiliar para palavras da linha
  int charcount=0;          //Contador auxiliar para quebra de linha
  char words[100][144],filename[100];   //Buffer auxiliar para quebra de linha
  char fstletter;           //Buffer auxiliar para identificação do componente
  bool wordflag = false;    //Buffer auxiliar para quebra de linha
  char line[1000];          //Buffer para leitura do arquivo
  FILE *spicearq,*stdout;   //Inicialização dos ponteiros para os arquivos de entrada e saída
  printf("\n\nInsira o nome do arquivo spice:");  //Abre um prompt para inserção do nome do arquivo de entrada
  scanf("%s",filename);
  spicearq= fopen(filename,"r");                  //Abre o arquivo de entrada como somente leitura
  stdout = fopen("result.spice","w");             //Abre o arquivo de saída como gravação
  while(spicearq==NULL)                              //Se o arquivo de entrada não existe, emite um erro e solicita novamente
  {
    printf("\n\nERRO na abertura do arquivo!\n\n");
    printf("\n\nInsira o nome do arquivo spice:");  //Abre um prompt para inserção do nome do arquivo de entrada
    scanf("%s",filename);
    spicearq= fopen(filename,"r");                  //Abre o arquivo de entrada como somente leitura
  }

  fgets(line,1000,spicearq);                      //Lê a primeira linha que é sempre somente comentários e aloca no array de comentários
  strcpy(comments[0],line);
  while(!feof(spicearq))                          //Loop de leitura e associação até o final do arquivo de entrada
  {
    compFlag=false;
    fflush(spicearq);
    fgets(line,1000,spicearq);                    //Lê nova linha e armazena no buffer
    fstletter=toupper(line[0]);
    //Esse conjunto if-else verifica se a linha é componente, comentário ou comando.
    if(fstletter == '*') //Verifica se a linha é do tipo comentário
    {
      strcpy(comments[0],line);
    }
    else if(fstletter == '.') //Verifica se a linha é do tipo comando
    {
      strcpy(command[0],line);
    }
    else if(fstletter == '\n')
    {
    }
    else if(fstletter=='R' || fstletter=='L' || fstletter=='C'|| fstletter=='V'||
    fstletter=='I' || fstletter=='D') //Testa se a linha é um componente de dois pinos
    {
      componentes[numComp].node3=-1; //Seta os pinos 3 e 4 como não existentes
      componentes[numComp].node4=-1;
      componentes[numComp].type=toupper(fstletter);
      compFlag=true;               //Define a linha como componente
    }
    else if(fstletter=='Q'|| fstletter=='M')  //Verifica se o componente é um transistor
    {
      componentes[numComp].node3=0;
      componentes[numComp].node4=-1;
      componentes[numComp].type=toupper(fstletter);
      compFlag=true;               //Define a linha como componente
    }
    else if(fstletter=='E' || fstletter=='F'|| fstletter=='G' || fstletter=='H')  //Verifica se o componente é uma fonte controlada
    {
      componentes[numComp].node3=0;
      componentes[numComp].node4=0;
      componentes[numComp].type=toupper(fstletter);
      compFlag=true;               //Define a linha como componente
    }
    else //Se a linha não for um componente, comentário ou comando, retorna erro de sintaxe
    {
      printf("\n\n Erro de Sintaxe: Componente, Comentário ou Comando Inválido!");
      return 0;
    }

    if(compFlag==true)    //Trabalha com a linha se ela for do tipo componente
    {
      while(line[strcount]!='\n') //Separa a linha em palavras até achar o final da linha
      {
        while(line[strcount]!=' ' && line[strcount]!='\t' && line[strcount]!='\n') //Coloca cada palavra em uma string diferente
        {
          words[wordcount][charcount]=line[strcount];
          charcount++;
          strcount++;
        }
        while(line[strcount]==' '|| line[strcount]=='\t')  //Continua varrendo a linha até encontrar um caractere diferente de espaço
        {
          strcount++;
        }
        words[wordcount][charcount+1]='\0';       //zera o contador da palavra e passa para a próxima ao encontrar um caractere de espaço
        charcount=0;

        switch(wordcount)       //Associa o componente com suas informações durante a varredura da linha que foi quebrada em palavras
        {
          case 0:
          {
            strcpy(componentes[numComp].name,words[wordcount]);   //Associa o nome ao componente
            break;
          }
          case 1:
          {
            nodeSet(numComp,wordcount,words[wordcount]);          //Associa o primeiro nó
            break;
          }
          case 2:
          {
            nodeSet(numComp,wordcount,words[wordcount]);          //Associa o segundo nó
            break;
          }
          case 3:                                                 //Associa o terceiro nó, mas antes faz uma verificação do componente
          {
            if(componentes[numComp].node3==0)
            nodeSet(numComp,3,words[wordcount]);
            else if(componentes[numComp].type=='D')                 //Verifica se é um diodo e caso seja, associa sua equação correspondente
            strcpy(componentes[numComp].equation,words[wordcount]);
            else
            parseValues(words[wordcount],numComp,componentes[numComp].name);               //Se não for um diodo, então certamente é um componente linear, então passa seu respectivo valor
            break;
          }
          case 4:
          {
            if(componentes[numComp].node4==0)                     //Verifica se é uma fonte controlada
            nodeSet(numComp,4,words[wordcount]);
            else
            strcpy(componentes[numComp].equation,words[wordcount]); //Se não for, certamente é um TJB ou MOSFET, então associa sua respectiva equação
            break;
          }
          case 5:
          {
            parseValues(words[wordcount],numComp,componentes[numComp].name);
            break;
          }
        }

        wordcount++;                                            //Aumenta o contador de palavra
        memset(words[wordcount], 0, sizeof(words[wordcount]));  //Limpa o buffer para nova varredura
      }
      strcount=1;     //Reinicializa a variável de contagem na string
      charcount=0;    //Reinicializa a variável de contagem de caracteres
      wordcount=0;    //Reinicializa a variável de contagem de palavras
      numComp++;      //Conclui a associação de componente e adiciona um na varável de contagem de componentes
    }
  }
  showCompInfo(stdout); //Chama a função que imprime a associação dos componentes no arquivo de saída através do seu ponteiro
  showNodes(stdout);    //Chama a função que imprime os nós no arquivo de saída através do seu ponteiro
  fclose(stdout);       //Fecha  o arquivo de saída
}
