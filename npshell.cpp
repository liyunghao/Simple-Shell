#include <iostream>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <string>
#define pb(x) push_back(x)

using namespace std;

vector<string> parse (string cmd) {
	stringstream ss(cmd);
	string s;
	vector<string> res;
	while (ss >> s) {
		res.pb(s);
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

int main (int argc, char** argv) {
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
		for (int i = 0; i < cmdlen; i++) {
			if ( pipe(pipefd + i*2) ) {
				cerr << "pipe error\n";
				return -1;
			}
		}
		if (cmd[0] == "exit") {
			cout << "\n" ;
			return 0;
		}
		// multiple cmd
		if (cmd.size() > 1) {
			for (int i = 0; i < cmd.size(); i++) {

				
				int pid = fork();
				if (pid == 0) {
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
					char* arg[2] = { strdup(cmd[i].c_str()), NULL };
					if (execvp(cmd[i].c_str(), arg) < 0 ) {
						cerr << "exec error";
						return -1;
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
		} else {
			int pid = fork();
			if (pid == 0) {
				char* arg[2] = { strdup(cmd[0].c_str()), NULL };
				if (execvp(cmd[0].c_str(), arg) < 0 ) {
					cerr << "exec error";
					return -1;
				}	
			} else {
				int status;
				wait(&status);
			}
		}
		// cout << "\n";

	}


}
