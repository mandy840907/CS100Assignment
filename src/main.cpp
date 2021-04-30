// #include "../header/base.hpp"
#include "../header/command.hpp"
#include "../header/connector.hpp"
#include <vector>
#include <string>
#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
    if (argc == 1) // normal using
    while(true)
    {
        /* User's input */
        cout << "$ ";
        string input = "";
        
        if(argc == 1) 
        {
            getline(cin, input);
        }
        else if (argc > 1)
        {
            vector<string> allArgv(argv+1, argv + argc);
            input += "exit;";
            cout << endl;
            for(int i = 0; i < allArgv.size(); i++)
            {
                input += " " + allArgv[i];
            }
            input += "; exit";
        }
       
        /* exit */
        if(input == "exit")
        {
            break;
        }

        /* Add ; to the end when there is no ; */
        if(input[input.size() - 1] != ';' || input[input.size() - 1] - 1 != ';')
        {
            input += ';';
        }

        /* Send the input into command and parse it */
        Command* commands = new Command(input);
        bool parentheses = commands->evenParentheses();
        if(!parentheses) 
        {
            cout << "Error: Uneven parentheses" << endl;
        }
        else
        {
            commands->parseString();
            commands->convertArg();
            int i = commands->execute();
            if(i == -1) 
            {
                break;
            }
        }
    }    
}