#ifndef MNATOOL_H
#define MNATOOL_H
#include "component.h"



class MNAtool
{
public:


    double **stamp2MNA(int mnaSize,int numNodes, component *componente);
    void stampGenerator(component *componente);

};

#endif
