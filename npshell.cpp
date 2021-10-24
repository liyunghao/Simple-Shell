#include <iostream>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <map>

#define pb(x) push_back(x)

using namespace std;




vector<string> parse (string cmd) {
	vector<string> res;
	string delim = "|!>";
	char *c = strdup(cmd.c_str());
	char *tok = strtok(c, delim.c_str());

	while (tok) {
		res.pb(string(tok));
		tok = strtok(NULL, delim.c_str());
	}
	for (int i = 0; i < res.size(); i++) {
		res[i].erase(0, res[i].find_first_not_of(" "));
		res[i].erase(res[i].find_last_not_of(" ") + 1);
		//cout << res[i] << '\n';
	}
	return res;
}

void sigHandler(int signo) {
	pid_t pid;
	int stat;
	while((pid = waitpid(-1, &stat, WNOHANG) > 0 )) {
		//cout << "sig proc\n";
		//cout << pid << " is exited\n";
	}
	return;
}

int main () {
	signal(SIGCHLD, sigHandler);
	setenv("PATH", "bin:.", 1);
	int rip = 0;
	map<int, int[2]> mp;
	while (1) {
		string prompt = "% ";
		write(STDOUT_FILENO, "% ", 2);
		
		string input;
		getline(cin, input);
		if (!cin) {
			break;
		}
		if (input.empty())
			continue;
		//write(STDOUT_FILENO, input.c_str(), input.size());
		//write(STDOUT_FILENO, "\n", 2);
		rip++;
		vector<string> cmd = parse(input);
		int np = 0, exp = 0;
		int idx1 = input.find_last_of("|");
		int idx2 = input.find_last_of("!");
		int idx3 = input.find_last_of(">");
		string snp, sexp, filename;
		
		if (idx1 != -1) {
			snp = input.substr(idx1+1);	
		}
		if (idx2 != -1) {
			sexp = input.substr(idx2+1);	
		}
		if (idx3 != -1) {
			filename = cmd.back();
			cmd.pop_back();
		}

		if ( !snp.empty() && strncmp(snp.c_str(),  " ", 1) ) {
			np = stoi(snp);
		} else if ( !sexp.empty() && strncmp(sexp.c_str(), " ", 1) ) {
			exp = stoi(sexp);
		}

		//cout << filename << '\n';
		if (np) { 
			cmd.pop_back();
			if (mp.find(rip + np) == mp.end()) { //
				int fd[2];
				pipe(fd);
				mp[rip + np][0] = fd[0];
				mp[rip + np][1] = fd[1];
			}
		}
		if (exp) {
			cmd.pop_back();
			if (mp.find(rip + exp) == mp.end()) {
				int fd[2];
				pipe(fd);
				mp[rip + exp][0] = fd[0];
				mp[rip + exp][1] = fd[1];	
			}
		}

		//cout << "np: " << np << '\n';
		//cout << "exp: " << exp << '\n';
		//cout << "filename: " << filename << '\n';
		int cmdlen = cmd.size();
		int pipeSz = cmdlen - 1;
		// open pipe for process
		/*
		int pipefd[2*pipeSz];
		for (int i = 0; i < pipeSz; i++) {
			if ( pipe(pipefd + i*2) ) {
				perror("pipe error");
				return -1;
			}
		}
		*/

		if (cmd[0] == "exit") {
			return 0;
		} 
		int lastpid = 0;
		int *prev = nullptr, *cur = nullptr;
		for (int i = 0; i < cmd.size(); i++) {
			
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
				if ( getenv(argv[1]) != NULL) 
					cout << getenv(argv[1]) << '\n';
				continue;
			} else if ( !strncmp(argv[0], "setenv", 6) ){
				if (cnt < 3) {
					cerr << "Command error\n";
					continue;
				}
				if (setenv(argv[1], argv[2], 1) < 0) {
					cout << errno << '\n';
				}
				continue;
			} 
			
			if (i != cmd.size()-1) {
				cur = new int[2];
				while ( pipe(cur) );
			}
			
			int pid;

			while ( (pid = fork()) == -1) ;
			
			lastpid = pid;
				
			if (mp.find(rip) != mp.end())
				close(mp[rip][1]);
			if (pid == 0) {
				if (cmd.size() != 1) { // only needed when two process gonna communicate
					if (i == 0) { // first process
						
						if (mp.find(rip) != mp.end()) {
							dup2(mp[rip][0], STDIN_FILENO);
						}
						dup2(cur[1], STDOUT_FILENO);
						close(cur[0]);
						close(cur[1]);
					
					} else if (i == cmd.size()-1) { // last process
						dup2(prev[0], STDIN_FILENO);
						close(prev[0]);
						close(prev[1]);
						
						if (np) {
							dup2(mp[rip+np][1], STDOUT_FILENO);
						} else if (exp) {
							dup2(mp[rip+exp][1], STDOUT_FILENO);
							dup2(mp[rip+exp][1], STDERR_FILENO);
						} else if (filename != "") {
							int f;
							if (filename != "") {
								f = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
							}
							dup2(f, STDOUT_FILENO);
							close(f);
						}
					} else {
						dup2(prev[0], STDIN_FILENO);
						close(prev[0]);
						close(prev[1]);
						dup2(cur[1], STDOUT_FILENO);	
						close(cur[0]);
						close(cur[1]);

					}
				} else {
					if (mp.find(rip) != mp.end()) {
						dup2(mp[rip][0], STDIN_FILENO);
					}
					if (np) {
						dup2(mp[rip+np][1], STDOUT_FILENO);
					} else if (exp) {
						dup2(mp[rip+exp][1], STDOUT_FILENO);
						dup2(mp[rip+exp][1], STDERR_FILENO);
					} else if (filename != "") {	
						int f;
						if (filename != "") {
							f = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
						}	
						if ( dup2(f, STDOUT_FILENO)  == -1) {
							perror("dup2 error");
						}
						close(f);
					}

				}
				
				if (execvp(argv[0], argv) < 0 ) {
					// poss
					cerr << "Unknown command: [" << argv[0] << "].\n";
					return -1;
				}
			} else {
				if (prev) {
					close(prev[0]);
					close(prev[1]);
					delete[] prev;
				}
				prev = cur;
			}

		}
		
		
		int status;
		if ( np == 0 && exp == 0) {
			waitpid(lastpid, nullptr, WUNTRACED);
		}
		/*
		for (int i = 0; i < cmdlen; i++) {
			wait(&status);
		}
		*/
		if (mp.find(rip) != mp.end()) {
			close(mp[rip][1]);
			close(mp[rip][0]);
			mp.erase(rip);
		}
		

	}


}
