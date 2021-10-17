#include <iostream>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <string>
#include <string.h>
#include <stdlib.h>

#define pb(x) push_back(x)

using namespace std;

// todo -> 處理好指令帶參數的狀況 e.g. grep noop, parse 沒處理好



vector<string> parse (string cmd) {
	/*
	stringstream ss(cmd);
	string s;
	vector<string> res;
	while (ss >> s) {
		res.pb(s);
	}
	return res;
	*/
	vector<string> res;
	string delim = "|";
	char *c = strdup(cmd.c_str());
	char *tok = strtok(c, delim.c_str());

	while (tok) {
		res.pb(string(tok));
		tok = strtok(NULL, delim.c_str());
	}
	for (int i = 0; i < res.size(); i++) {
		res[i].erase(0, res[i].find_first_not_of(" "));
		res[i].erase(res[i].find_last_not_of(" ") + 1);
		cout << res[i] << '\n';
	}
	return res;
}

int flag;
void sigHandler(int signo) {
	pid_t pid;
	int stat;
	cout << "sig proc\n";
	while((pid = waitpid(-1, &stat, WNOHANG) > 0 ))
		cout << pid << "is exited\n";
	flag = 0;
	return;
}

int main () {
	flag = 1;
	// signal(SIGCHLD, sigHandler);
	while (1) {
		cout << "% ";
		string input;
		getline(cin, input);
		vector<string> list = parse(input);
		vector<string> cmd;
	
		for (int i = 0; i < list.size(); i++) {
			if ( (strncmp(list[i].c_str(), "|", 1) == 0 ) || (strncmp(list[i].c_str(), "!", 1) == 0) ) {
				continue;
			}
			cmd.pb(list[i]);
		}
		
		int cmdlen = cmd.size();
		int pipeSz = cmdlen - 1;

		// open pipe for process
		int pipefd[2*pipeSz];
		for (int i = 0; i < pipeSz; i++) {
			if ( pipe(pipefd + i*2) ) {
				cerr << "pipe error\n";
				return -1;
			}
		}
		cout << cmd[0] << '\n';
		if (cmd[0] == "exit") {
			//cout << "\n" ;
			return 0;
		} 

		

		for (int i = 0; i < cmd.size(); i++) {
			int pid = fork();
			if (pid == 0) {
				if (cmd.size() != 1) { // only needed when two process gonna communicate
					if (i == 0) { // first process
						dup2(pipefd[1], STDOUT_FILENO);
					} else if (i == cmd.size()-1) { // last process
						dup2(pipefd[2*i-2], STDIN_FILENO);
					} else {
						dup2(pipefd[2*i-2], STDIN_FILENO);
						dup2(pipefd[2*i+1], STDOUT_FILENO);
					}
					for (int i = 0; i < 2*pipeSz; i++) { // close all pipe
						close(pipefd[i]);
					}
				}
				char* c = strdup(cmd[i].c_str());
				char* tok = strtok(c, " ");
				char* argv[256];
				int cnt = 0;
				while (tok) {
					argv[cnt++] = tok;
					tok = strtok(NULL, " ");
				}
				argv[cnt] = NULL;
						
				if ( !strncmp(argv[0], "printenv", 8) ) {
					if (cnt < 2) {
						cerr << "Command error\n";
						return -1;
					}
					cout << getenv(argv[1]) << '\n';
					return 0;
				} else if ( !strncmp(argv[0], "setenv", 6) ){
					if (cnt < 3) {
						cerr << "Command error\n";
						return -1;
					}

					setenv(argv[1], argv[2], 1);
					return 0;
				} else {
					if (execvp(argv[0], argv) < 0 ) {
						cerr << "Unknown command: [" << argv[0] << "].\n";
						return -1;
					}
				}
			
			} 
		}
	
		for (int i = 0; i < 2*pipeSz; i++) {
			close(pipefd[i]);
		}
		int status;
		for (int i = 0; i < cmdlen; i++) {
			wait(&status);
		}
		

	}


}
