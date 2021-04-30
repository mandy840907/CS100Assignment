#ifndef __COMMAND_HPP__
#define __COMMAND_HPP__

#include "base.hpp"
#include "connector.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h> 
#include <unistd.h>
#include <cstring>
#include <vector>
#include <stack>
#include <iostream>
#include <fstream>
using namespace std;

struct status
{
    bool connector;
    bool result;   
};

class Command : public Base
{
    protected:
        string commands;                    /* User's input */
        vector<string> commandList;         /* Separate commands by ; */
        vector<string> argList;             /* Separate command by ' ' */
        int* commandIndex;                  /* Check where to start the command in argList */
        int* commandEnd;                    /* Check the end of the command in argList */
    public:
        Command(string);
        string stringify();                 /* Get the input */
        void parseString();                 /* When read ;, save the command before the ; into vector<Base*> */
        void convertArg();                  /* Convert string to char array for later use  */
        int execute();                      /* Execute the command */
        int getListSize();                  /* Return the size of commandList */
        int grandchildExec(vector<string>); /* Execute grandchild process */
        bool evenParentheses();             /* Check uneven parentheses */
        void testCommand(vector<char*>);    /* Test command */
        status initS();                     /* Initial status */
        bool update(bool, bool, bool);      /* Update status */
};

/* Constructor */
Command::Command(string input)
{
    commands = input;
}

/* Get the input */
string Command::stringify()
{
    return commands;
}

/* When read ;, save the command before the ; into vector<Base*> */
void Command::parseString()
{
    string newCommand = "";
    bool quotation = false;
    for(int i = 0; i < commands.size(); i++)
    {
        if(commands[i] == '\"')
        {
            quotation = !quotation;
        }
        if(!quotation && commands[i] == '#')
        {
            newCommand += ' ';
            commandList.push_back(newCommand);
            break;
        }
        else if(commands[i] != ';' || (commands[i] == ';' && quotation))
        {
            newCommand += commands[i];
        }
        else
        {
            newCommand += ' ';
            commandList.push_back(newCommand);
            newCommand = "";
            i++;
        }
    }
    /* argList can announce it's size by following ways */
    // argList = (string*)malloc(commandList.size() * sizeof(string));
    // argList = new string[commandList.size()];

    /* Assign size to int* */
    commandIndex = new int[commandList.size()];
    commandEnd = new int[commandList.size()];
}

/* Convert string to char array for later use  */
void Command::convertArg()
{
    int execNUM = commandList.size();
    string newC = "";

    for(int i = 0; i < execNUM; i++)
    {
        commandIndex[i] = argList.size();
        for(int j = 0; j < commandList[i].size(); j++)
        {
            if(commandList[i][j] != ' ')
            {
                newC += commandList[i][j];
            }
            else
            {
                argList.push_back(newC);
                newC = "";
            }
        }
        commandEnd[i] = argList.size() - 1;
    }
}

/* Execute the command */
int Command::execute()
{
    int execNUM = commandList.size();
    int childpid = 100;
    int childstatus[execNUM];
    int childlist[execNUM];
    int selfIndex;

    for(int i = 0; i < execNUM; i++)
    {   
        if(childpid != 0) // parent
        {
            childpid = fork();
            childlist[i] = childpid;
        }
    }
    for(int i = 0; i < execNUM; i++)
    {   
        if(childlist[i] == 0)
        {
            selfIndex = i;
            break;
        }
    }
    if(childpid < 0) // Fail to fork 
    {
        perror("Fail to fork");
    }
    else if(childpid == 0) // Child process
    {
        int count =1;
        int end = commandEnd[selfIndex];
        int start = commandIndex[selfIndex];
        int grandchildStatus = 0;
        bool preConnector = true;
        bool connector;
        vector<string> localArg;
        bool quotation = false;
        bool currStatus = true;
        bool fileRead = false;

        stack<status> S;
        bool finalize = false;
        status curr = initS();

        for(int i = start; i < end+1; i++)
        {  
            if(argList[i] == "cat")
            {
                bool out_flag = false;
                ifstream fin;
                ofstream fout;
                fin.open(argList[i + 2]);
                if (argList.size()>3)
                {
                    fout.open(argList[i + 4]);
                    if(!fout)
                        cout << "Cannout output"<< endl;
                    else 
                    {
                        cout << "output file " << argList[i + 4] << endl;
                        out_flag = true;
                    }
                }

                if(!fin)
                {
                    cout << "Cannot open" << endl;
                }
                else
                {
                    char ch;
                    while(fin.peek() != EOF)
                    {
                        fin.get(ch);
                        cout << ch;
                        if(out_flag)
                        {
                            fout.put(ch);
                        }
                    }
                    fin.close();
                    if(out_flag) fout.close();
                }
                fileRead = true;
                break;
            }
            fileRead = false;
            if((argList[i][0] != '&' && argList[i][0] != '|') || quotation)
            {       
                string tempArg = "";
                for(int j = 0; j < argList[i].length(); j++)
                {
                    if(argList[i][j] == '(')
                    {
                        S.push(curr);
                        curr = initS();
                    }   
                    else if(argList[i][j] == ')')
                    {
                        finalize = true;
                    }
                    else
                    {
                        tempArg += argList[i][j];
                    }
                    if(argList[i][j] == '\"')
                    {
                        quotation = !quotation;
                    }
                }
                localArg.push_back(tempArg);
            }
            else 
            {
                if(S.empty() || !(S.top().result ^ S.top().connector))
                {
                    if(!(curr.result ^ curr.connector))
                    {  
                        if(localArg[0] == "exit") exit(3);
                        
                        grandchildStatus = grandchildExec(localArg);
                        curr.result = update(curr.result, curr.connector, !grandchildStatus);
                        
                    }
                    if(argList[i][0] == '&') 
                    {
                        curr.connector = true;
                    }
                    if(argList[i][0] == '|') 
                    {
                        curr.connector = false;
                    }
                }
                localArg.clear();
                if(finalize)
                {
                    finalize = false;
                    S.top().result = update(S.top().result, S.top().connector, curr.result);
                    if(argList[i][0] == '&') S.top().connector = true;
                    if(argList[i][0] == '|') S.top().connector = false;
                    curr = S.top();
                    S.pop();
                }
            }  
        }
        if(((S.empty() || !(S.top().result ^ S.top().connector)) && !(curr.result ^ curr.connector)) && !fileRead)
        {
            if(localArg[0] == "exit") exit(3);
            grandchildStatus = grandchildExec(localArg);
            curr.result = update(curr.result, curr.connector, !grandchildStatus);
        }
        exit(0);
    }
    else // Parent process, will wait for child process to run first
    {
        int flag = 0;
        int doExit = 0;
        while(flag != execNUM)
        {
            flag=0;
            for(int i = 0; i < execNUM; i++)
            {
                childpid = waitpid(childlist[i], &childstatus[i], 0);
                if(childpid < 0)
                {
                    perror("error");
                }
                if (childstatus[i] != 0) doExit = -1;
                flag += WIFEXITED(childstatus[i]); 
            }     
        }
        return doExit;
    }
    return 1;
}

/* Return the size of commandList */
int Command::getListSize()
{
    return commandList.size();
}

int Command::grandchildExec(vector<string> inputArg)
{
    vector<string> localArg;
    string tempArg = "";
    int grandchildStatus;
    for(int i = 0; i < inputArg.size(); i++)
    {
        for(int j = 0; j < inputArg[i].size(); j++)
        {
            if(inputArg[i][j] != '\"')
            {
                tempArg += inputArg[i][j];
            }
        }
        localArg.push_back(tempArg);
        tempArg = "";
    }
    int grandchild = fork();
    if(grandchild == 0) // grandchild process
    {
        if(localArg[0] == "[" || localArg[0] == "test")
        {
            vector<char*> cArg(localArg.size(),nullptr);
            for(int i = 0; i < localArg.size(); i++)
            {
                cArg[i] = strdup(localArg[i].c_str());
            }
            testCommand(cArg);
        }
        else
        {        
            int len = localArg.size() + 1;
            char** execArg = new char * [len];
            for (int j = 0; j < len - 1; j++)
            {
                execArg[j] = strdup(localArg[j].c_str());
            }
            execArg[len - 1] = NULL;
            if(execvp(execArg[0], execArg) < 0)
            {
                perror("error while executing");
                _exit(1);
            }
        }
    }
    else
    {
        do
        {
            int grandchildpid = waitpid(grandchild, &grandchildStatus, 0);
            if(grandchildpid < 0)
            {
                perror("error");
            }
        }while(!WIFEXITED(grandchildStatus) && !WIFSIGNALED(grandchildStatus));
        /* Debug */
        // cout << getpid() << " " << WEXITSTATUS(grandchildStatus) << endl; 
        return WEXITSTATUS(grandchildStatus);
    }
    return 10;
}

/* Check uneven parentheses */
bool Command::evenParentheses()
{
    int counter = 0;
    for(int i = 0; i < commands.length(); i++)
    {
        if(commands[i] == '(')
        {
            counter++;
        }
        else if(commands[i] == ')')
        {
            counter--;
        }
    }
    if(counter == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/* Test command */
void Command::testCommand(vector<char*> testcommand)
{
    char flag = 'e';
    if(strncmp(testcommand[0], "[", 1) == 0)
    {
        testcommand.erase(testcommand.end() - 1);
    }
    testcommand.erase(testcommand.begin());
    if(testcommand[0][1] == 'f' || testcommand[0][1] == 'd')
    {
        flag = testcommand[0][1];
    }
    struct stat buf;
    int i = stat(testcommand[1], &buf);
    if(flag == 'e')
    {
        if(S_ISDIR(buf.st_mode) || S_ISREG(buf.st_mode))
        {
            cout << "(TRUE)" << endl;
            exit(0);
        }
        else
        {
            cout << "(FALSE)" << endl;
            _exit(1);
        }
    }
    else if(flag == 'f')
    {
        if(S_ISREG(buf.st_mode))
        {
            cout << "(TRUE)" << endl;
            exit(0);
        }
        else
        {
            cout << "(FALSE)" << endl;
            _exit(1);
        }
    }
    else if(flag == 'd')
    {
        if(S_ISDIR(buf.st_mode))
        {
            cout << "(TRUE)" << endl;
            exit(0);
        }
        else
        {
            cout << "(FALSE)" << endl;
            _exit(1);
        }
    }
}

status Command::initS()
{
    status temp;
    temp.connector = true;
    temp.result = true;
    return temp;
}

bool Command::update(bool preResult, bool preConnector, bool currResult)
{
    if(preConnector) // &&
    {
        return preResult && currResult;
    }
    else // ||
    {
        return preResult || currResult;
    }
}


#endif // __COMMAND_HPP__