#ifndef COMPONENT_H
#define COMPONENT_H


typedef struct      //O componente é representado por uma estrutura de dados que armazenará todas as informações sobre ele
{
  char name[30];      //nome do componente
  char type;          //tipo de componente (capacitor, transistor, etc)
  char equation[20];  //equação para relação entrada/saída (somente para componentes não-lineares)
  double value;       //valor do componente (para componentes lineares)
  int ident;          //ordem do componente (apenas para contagem)
  int n1;          //número do nó ligado ao positivo (quando node3=-1 e node4=-1) ou coletor do TJB ou dreno do MOSFET
  int n2;          //número do nó ligado ao negativo (quando node3=-1 e node4=-1) ou base do TJB ou gate do MOSFET
  int n3;          //número do nó ligado ao positivo da saída para fontes ctrl, emissor do TJB ou source do MOSFET. valor -1 para componentes de 2 pinos
  int n4;          //número do nó ligado ao negativo da saída para fontes ctrl. valor -1 para componentes de 2 ou 3 pinos
  bool G2=false;
  int control;
  double RHS[6]={0,0,0,0,0,0};
  double stamp[7][6];
}component;
#endif
