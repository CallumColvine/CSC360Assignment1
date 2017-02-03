extern "C"{
    #include <stdlib.h>
    #include <readline/readline.h>
    #include <readline/history.h>
    #include <unistd.h>
}
#include <cstdlib>
#include <sys/wait.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <vector>
#include <algorithm>
#include <memory>
#include "BgObject.h"

// List of background objects
static std::vector<BgObject> bgList;


int executeCommand(char** cmdList)
{
    std::cout << "Running command " << cmdList[0] << std::endl;
    for (int i = 0; i < 2; i++)
    {
        std::cout << cmdList[i] << std::endl;
    }

    int executeRes = execvp(cmdList[0], cmdList);
    if (executeRes < 0) {     
        printf("*** ERROR: exec failed\n");
        std::cout << strerror(errno) << std::endl;
        exit(1);
    }
    return 0;
}

void removeBgEntry(char* cmd, char** cmdListBg){
    char* token = std::strtok(cmd, " ");    
    int i = 0;
    while (token != NULL)
    {
        token = std::strtok(NULL, " ");
        cmdListBg[i] = token;
        i++;    
    }
    cmdListBg[i] = '\0';
}   

void nullTerm(char* unfinishedStr){
    char* p = unfinishedStr;
    for(uint i = 0; i < sizeof(unfinishedStr); i++){
        *p++ = *unfinishedStr++;        
    }
    *p++ = '\0';
}

void splitString(char* cmd, char** cmdList)
{
    char* token = std::strtok(cmd, " ");    
    int i = 0;
    while (token != NULL)
    {
        cmdList[i] = token;
        token = std::strtok(NULL, " ");
        i++;    
    }
    cmdList[i] = '\0';
}

int killBgProcess(std::vector<BgObject> bgList, char* toKill){
    int killIndex = atoi(toKill);
    std::cout << "killing process: " << killIndex << std::endl;
    pid_t pid = bgList[killIndex].getPid();
    kill(pid, SIGKILL);
    return killIndex;
}

// Returns -1 for exit
// Returns 1-7 for legit commands
// Returns 0 otherwise
int checkCommand(char* command)
{
    if (strcmp(command, "ls") == 0)
        return 1;
    else if (strcmp(command, "pwd") == 0)
        return 1;       
    else if (strcmp(command, "cd") == 0)
        return 2;
    else if (strcmp(command, "exit") == 0)
        return -1;
    else if (strcmp(command, "bg") == 0)
        return 3;
    else if (strcmp(command, "bglist") == 0)
        return 4;
    else if (strcmp(command, "bgkill") == 0)
        return 5;
    else if (strcmp(command, "stop") == 0)
        return 6;
    else if (strcmp(command, "start") == 0)
        return 7;
    else {
        // Was supposed to run other commands like cat, echo, etc...
        return 1;
    }
}

void printBgList(std::vector<BgObject> bgList){
    for (uint i = 0; i < bgList.size(); i++){
        std::cout << i << '[' << bgList[i].getStatus() << ']' 
            << ": " << bgList[i].getWorkingDir() << "\n";
    }
}

void cdCommand(char* path){
    chdir(path);
}

void cleanupKids(){
    pid_t childPid;
    pid_t curPid;
    int status;
    for (uint i = 0; i < bgList.size(); i++)
    {
        curPid = bgList[i].getPid();
        childPid = waitpid(curPid, &status, WNOHANG);
        if (childPid == -1){
            std::cout << "A child has finished: " << curPid << std::endl;
            bgList.erase(bgList.begin() + i);
        }
    }
}

int stopProcess(char* toStop){
    int stopIndex = atoi(toStop);
    char status = bgList[stopIndex].getStatus();
    if (status == 'S'){
        std::cout << "Error already stopped" << std::endl;
        return -1;
    }
    // Stop
    pid_t pid = bgList[stopIndex].getPid();
    kill(pid, SIGSTOP);
    bgList[stopIndex].setStatus('S');
    return 0;
}

int startProcess(char* toStart){
    int startIndex = atoi(toStart);
    char status = bgList[startIndex].getStatus();
    if (status == 'R'){
        std::cout << "Error already running" << std::endl;
        return -1;
    }
    // Start
    pid_t pid = bgList[startIndex].getPid();
    kill(pid, SIGCONT);
    bgList[startIndex].setStatus('R');
    return 0;
}


// Sorry main is so huge and gross, but I have another
// assignment I really need to do. CSC 305 is NOT easy :'(
int main ( void )
{
    // Reserve 5 since that's the max for this assignment
    bgList.reserve(5);
    int maxDirLength = 256;
    char* cwd;
    char store[maxDirLength + 1];
    cwd = getcwd(store, maxDirLength + 1 );
    char cwdBackup[256]; 
    // For the prompt line initialization
    strncpy(cwdBackup, cwd, sizeof(cwdBackup));
    char* cwdLine = strcat(cwd, "$ ");
    for (;;)
    {
        // Check if any child processes have finished
        cleanupKids();
        // Read and print input
        char *cmd = readline(cwdLine);
        printf ("Got: [%s]\n", cmd);
        // Used in case user calls 'bg ...'
        char cmdBackup[128];
        strncpy(cmdBackup, cmd, sizeof(cmdBackup));
        // max num of commands + 1 for null terminator
        char** cmdList = new char*[128 + 1];
        splitString(cmd, cmdList);
        // Check type of command
        int commandType = checkCommand(cmdList[0]);
        // If it is a verified Linux command
        if (commandType == 1){
            // Fork and execute command
            pid_t forkRes = fork();
            if (forkRes == 0)
            {
                executeCommand(cmdList);
            }
            else {
                int status;
                waitpid(forkRes, &status, 0);
            }
        }
        else if (commandType == 2){
            char dirChange[256] = {0};
            size_t cpySize = sizeof(dirChange);
            strncpy(dirChange, cmdList[1], cpySize);
            cdCommand(dirChange);
            cwd = getcwd(store, maxDirLength + 1 );
            cwdLine = strcat(cwd, "$ ");
        }

        else if (commandType == 3){
            char** cmdListBg = new char*[128 + 1];          
            pid_t forkRes = fork();
            if (forkRes == 0)
            {
                // Execute the bg command
                removeBgEntry(cmdBackup, cmdListBg);
                executeCommand(cmdListBg);
            }
            else {
                // Setting up list of current bg commands
                std::stringstream ss;
                ss << cwdBackup << "/" << cmdList[1];
                std::string bgDir = ss.str();
                BgObject temp;
                temp.setPid(forkRes);
                temp.setWorkingDir(bgDir);
                // Add to the list of background processes
                bgList.push_back(temp);
            }           
        }
        else if (commandType == 4){
            printBgList(bgList);
        }
        else if (commandType == 5){
            int killIndex = killBgProcess(bgList, cmdList[1]);
            bgList.erase(bgList.begin() + killIndex);
        }
        else if (commandType == 6){
            stopProcess(cmdList[1]);
        }
        else if (commandType == 7){
            startProcess(cmdList[1]);
        }
        else if (commandType == -1){
            exit(0);
        }
        free (cmd);
        free (cmdList);
    }   
}

