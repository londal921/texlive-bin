/*************************************************************************
** Process.cpp                                                          **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2018 Martin Gieseking <martin.gieseking@uos.de>   **
**                                                                      **
** This program is free software; you can redistribute it and/or        **
** modify it under the terms of the GNU General Public License as       **
** published by the Free Software Foundation; either version 3 of       **
** the License, or (at your option) any later version.                  **
**                                                                      **
** This program is distributed in the hope that it will be useful, but  **
** WITHOUT ANY WARRANTY; without even the implied warranty of           **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         **
** GNU General Public License for more details.                         **
**                                                                      **
** You should have received a copy of the GNU General Public License    **
** along with this program; if not, see <http://www.gnu.org/licenses/>. **
*************************************************************************/

#ifdef _WIN32
	#include "windows.hpp"
#else
	#include <fcntl.h>
	#include <sys/wait.h>
	#include <signal.h>
	#include <unistd.h>
#endif

#include <cstdlib>
#include "FileSystem.hpp"
#include "Process.hpp"
#include "SignalHandler.hpp"

using namespace std;


/** Helper class that encapsulates the system-specific parts of
 *  running a subprocess and retrieving its terminal output. */
class Subprocess {
	public:
		enum class State {RUNNING, FINISHED, FAILED};

	public:
		Subprocess ();
		Subprocess (const Subprocess&) =delete;
		Subprocess (Subprocess&&) =delete;
		~Subprocess ();
		bool run (const string &cmd, string params);
		bool readFromPipe (string &out);
		State state ();

	private:
#ifdef _WIN32
		HANDLE _pipeReadHandle;   ///< handle of read end of pipe
		HANDLE _childProcHandle;  ///< handle of child process
#else
		int _readfd; ///< file descriptor of read end of pipe
		pid_t _pid;  ///< PID of the subprocess
#endif
};


Process::Process (const string &cmd, const string &paramstr)
	: _cmd(cmd), _paramstr(paramstr)
{
}


/** Runs the process and waits until it's finished.
 *  @param[out] out takes the output written to stdout by the executed subprocess
 *  @return true if process terminated properly
 *  @throw SignalException if CTRL-C was pressed during execution */
bool Process::run (string *out) {
	Subprocess subprocess;
	if (!subprocess.run(_cmd, _paramstr))
		return false;
	for (;;) {
		if (out)
			subprocess.readFromPipe(*out);
		Subprocess::State state = subprocess.state();
		if (state != Subprocess::State::RUNNING)
			return state == Subprocess::State::FINISHED;
		SignalHandler::instance().check();
	}
}


/** Runs the process in the given working directory and waits until it's finished.
 *  @param[in] dir working directory
 *  @param[out] out takes the output written to stdout by the executed process
 *  @return true if process terminated properly
 *  @throw SignalException if CTRL-C was pressed during execution */
bool Process::run (const string &dir, string *out) {
	bool ret = false;
	string cwd = FileSystem::getcwd();
	if (FileSystem::chdir(dir)) {
		ret = run(out);
		ret &= FileSystem::chdir(cwd);
	}
	return ret;
}

// system-specific stuff

#ifdef _WIN32

static inline void close_and_zero_handle (HANDLE &handle) {
	CloseHandle(handle);
	handle = NULL;
}


Subprocess::Subprocess() : _pipeReadHandle(NULL), _childProcHandle(NULL) {
}


Subprocess::~Subprocess () {
	if (_pipeReadHandle != NULL)
		CloseHandle(_pipeReadHandle);
	if (_childProcHandle != NULL) {
		TerminateProcess(_childProcHandle, 1);
		CloseHandle(_childProcHandle);
	}
}


/** Retrieves output generated by child process.
 *  @param[out] out read output is appended to this string
 *  @returns false on errors */
bool Subprocess::readFromPipe (string &out) {
	if (!_pipeReadHandle)
		return false;

	bool success=false;
	DWORD len;
	while (PeekNamedPipe(_pipeReadHandle, NULL, 0, NULL, &len, NULL) && len > 0) {  // prevent blocking
		char buf[4096];
		success = ReadFile(_pipeReadHandle, buf, sizeof(buf), &len, NULL);
		if (success && len > 0)
			out.append(buf, len);
	}
	return success;
}


/** Starts a child process.
 *  @param[in] cmd name of command to execute
 *  @param[in] paramstr parameters required by command
 *  @returns true if child process started properly */
bool Subprocess::run (const string &cmd, string paramstr) {
	SECURITY_ATTRIBUTES securityAttribs;
	ZeroMemory(&securityAttribs, sizeof(SECURITY_ATTRIBUTES));
	securityAttribs.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttribs.bInheritHandle = true;

	HANDLE pipeWriteHandle; // write end of pipe
	if (CreatePipe(&_pipeReadHandle, &pipeWriteHandle, &securityAttribs, 0) == ERROR_INVALID_HANDLE)
		return false;

	SetHandleInformation(_pipeReadHandle, HANDLE_FLAG_INHERIT, 0);
	HANDLE nullFile = CreateFile("NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &securityAttribs, OPEN_EXISTING, 0, NULL);
	bool success = false;
	if (nullFile != INVALID_HANDLE_VALUE) {
		STARTUPINFO startupInfo;
		ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
		startupInfo.cb = sizeof(STARTUPINFO);
		startupInfo.dwFlags = STARTF_USESTDHANDLES;
		startupInfo.hStdInput = nullFile;
		startupInfo.hStdOutput = pipeWriteHandle;
		startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);

		PROCESS_INFORMATION processInfo;
		ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

		string cmdline = cmd + " " + paramstr;
		// put subprocess in separate process group to prevent its termination in case of CTRL-C
		success = CreateProcess(NULL, (LPSTR)cmdline.c_str(), NULL, NULL, true, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &startupInfo, &processInfo);
		if (success) {
			_childProcHandle = processInfo.hProcess;
			CloseHandle(processInfo.hThread);
		}
		CloseHandle(nullFile);
	}
	CloseHandle(pipeWriteHandle);  // must be closed before reading from pipe to prevent blocking
	if (!success)
		close_and_zero_handle(_pipeReadHandle);
	return success;
}


/** Returns the current state of the child process. */
Subprocess::State Subprocess::state () {
	DWORD status;
	if (!GetExitCodeProcess(_childProcHandle, &status))
		return State::FAILED;
	if (status == STILL_ACTIVE)
		return State::RUNNING;
	close_and_zero_handle(_childProcHandle);
	return status == 0 ? State::FINISHED : State::FAILED;
}

#else  // !_WIN32

Subprocess::Subprocess () : _readfd(-1), _pid(-1) {
}


Subprocess::~Subprocess () {
	if (_readfd >= 0)
		close(_readfd);
	if (_pid > 0)
		kill(_pid, SIGKILL);
}


/** Retrieves output generated by child process.
 *  @param[out] out read output is appended to this string
 *  @returns false on errors */
bool Subprocess::readFromPipe (string &out) {
	if (_readfd < 0 || _pid < 0)
		return false;

	char buf[1024];
	ssize_t len;
	while ((len = read(_readfd, buf, sizeof(buf))) > 0)
		out.append(buf, len);
	if (len < 0) {
		close(_readfd);
		_readfd = -1;
	}
	return len >= 0;
}


/** Extracts whitespace-separated parameters from a string.
 *  @param[in,out] paramstr the parameter string
 *  @param[out] params vector holding pointers to the extracted parameters */
static void split_paramstr (string &paramstr, vector<const char*> &params) {
	size_t left=0, right=0;  // index of first and last character of current parameter
	char quote=0;            // current quote character, 0=none
	const size_t len = paramstr.length();
	while (left <= right && right < len) {
		while (left < len && isspace(paramstr[left]))
			++left;
		if (left < len && (paramstr[left] == '"' || paramstr[left] == '\''))
			quote = paramstr[left++];
		right = left;
		while (right < len && (quote || !isspace(paramstr[right]))) {
			if (quote && paramstr[right] == quote) {
				quote=0;
				break;
			}
			else
				++right;
		}
		if (right < len)
			paramstr[right]=0;
		if (left < len)
			params.push_back(&paramstr[left]);
		left = ++right;
	}
}


/** Starts a child process.
 *  @param[in] cmd name of command to execute
 *  @param[in] paramstr parameters required by command
 *  @returns true if child process started properly */
bool Subprocess::run (const string &cmd, string paramstr) {
	int pipefd[2];
	if (pipe(pipefd) < 0)
		return false;

	_pid = fork();
	if (_pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		return false;
	}
	if (_pid == 0) {   // child process
		dup2(pipefd[1], STDOUT_FILENO);  // redirect stdout to the pipe
		dup2(pipefd[1], STDERR_FILENO);  // redirect stderr to the pipe
		close(pipefd[0]);
		close(pipefd[1]);

		vector<const char*> params;
		params.push_back(cmd.c_str());
		split_paramstr(paramstr, params);
		params.push_back(nullptr); // trailing null pointer marks end of parameter list
		signal(SIGINT, SIG_IGN);   // child process is supposed to ignore ctrl-c events
		execvp(cmd.c_str(), const_cast<char* const*>(&params[0]));
		exit(1);
	}
	_readfd = pipefd[0];
	close(pipefd[1]);  // close write end of pipe
	return true;
}


/** Returns the current state of the child process. */
Subprocess::State Subprocess::state () {
	int status;
	pid_t wpid = waitpid(_pid, &status, WNOHANG);
	if (wpid == 0)
		return State::RUNNING;  // still running
	_pid = -1;
	if (wpid > 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return State::FINISHED;
	return State::FAILED;
}

#endif  // !_WIN32