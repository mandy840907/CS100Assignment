#ifndef __CONNECTOR_HPP__
#define __CONNECTOR_HPP__

#include "base.hpp"
#include "command.hpp"
#include <vector>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
using namespace std;

class Connector : public Base
{
    protected:
        bool state;
    public:
        Connector();  
        void execute();
        bool getState();
};

/* Constructor */
Connector::Connector()
{

}

void Connector::execute()
{
 
}

bool Connector::getState()
{
    return state;
}

#endif // __CONNECTOR_HPP__