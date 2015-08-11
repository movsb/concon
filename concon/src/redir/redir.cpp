#include "redir.h"

#include <process.h>

namespace concon {
redir_t::redir_t()
: _h_thread(nullptr)
, _h_event(nullptr)
, _dw_thread_id(0)
, _dw_wait_interval_fg(100)
, _dw_wait_interval_bg(1000)
, _b_fg(true)
, _h_stdin_write(nullptr)
, _h_stdout_read(nullptr)
, _h_child_process(nullptr)
{

}

redir_t::~redir_t()
{

}

void redir_t::close_handle(HANDLE& handle)
{
	::CloseHandle(handle);
	handle = NULL;
}

bool redir_t::spawn_child(LPCTSTR cmdline, HANDLE h_stdout, HANDLE h_stdin, HANDLE h_stderr, LPCTSTR working_directory)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si = {0};

	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.hStdInput = h_stdin;
	si.hStdOutput = h_stdout;
	si.hStdError = h_stderr;

	if(!::CreateProcess(nullptr, (LPTSTR)cmdline,
		nullptr, nullptr,
		TRUE,
		CREATE_NEW_CONSOLE,
		NULL,
		working_directory,
		&si, &pi))
	{
		return false;
	}

	_h_child_process = pi.hProcess;
	close_handle(pi.hThread);

	return true;
}

int redir_t::redirect()
{
	for(;;){
		DWORD dw_avail = 0;
		if(!::PeekNamedPipe(_h_stdout_read, nullptr, 0, 0, &dw_avail, nullptr))
			break;

		if(!dw_avail) return 1;

		char buff[10240];
		DWORD dw_read = 0;
		if(!::ReadFile(_h_stdout_read, buff, min(_countof(buff)-1, dw_avail), &dw_read, nullptr))
			break;

		buff[dw_read] = '\0';

		write_stdout(buff);
	}

	DWORD dw_err = ::GetLastError();
	if(dw_err == ERROR_BROKEN_PIPE || dw_err == ERROR_NO_DATA) {
		return 0;
	}

	return -1;
}

DWORD WINAPI redir_t::thread_output(LPVOID ud)
{
	int r;
	redir_t* that = reinterpret_cast<redir_t*>(ud);
	HANDLE handles[2] = {that->_h_child_process, that->_h_event};
	DWORD interval = that->_b_fg ? that->_dw_wait_interval_fg : that->_dw_wait_interval_bg;

	for(;;) {
		r = that->redirect();
		
		if(r < 0) break;

		DWORD obj_id = ::WaitForMultipleObjects(_countof(handles), handles, FALSE, interval);
		if(obj_id == WAIT_OBJECT_0 + 0) {
			r = that->redirect();
			if(r > 0) r = 0;
			break;
		}
		else if(obj_id == WAIT_OBJECT_0 + 1) {
			r = 1; 
			break;
		}
	}

	that->close();
	return r;

}

void redir_t::write_stdout(LPCTSTR str)
{
	::printf(str);
	//::MessageBox(nullptr, str, "", MB_OK);
}

bool redir_t::open(LPCTSTR cmdline, LPCTSTR working_directory)
{
	HANDLE h_stdout_read_tmp = nullptr;
	HANDLE h_stdout_write = nullptr, h_stderr_write = nullptr;
	HANDLE h_stdin_write_tmp = nullptr;
	HANDLE h_stdin_read = nullptr;

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = nullptr;
	sa.bInheritHandle = TRUE;

	try{
		if(!::CreatePipe(&h_stdout_read_tmp, &h_stdout_write, &sa, 0))
			throw "[error] CreatePipe failed.";

		if(!::DuplicateHandle(GetCurrentProcess(), h_stdout_write,
			GetCurrentProcess(), &h_stderr_write,
			0, TRUE, DUPLICATE_SAME_ACCESS))
		{
			throw "[error] DuplicateHandle failed.";
		}

		if(!::CreatePipe(&h_stdin_read, &h_stdin_write_tmp, &sa, 0))
			throw "[error] CreatePipe failed.";

		if(!::DuplicateHandle(GetCurrentProcess(), h_stdout_read_tmp,
			::GetCurrentProcess(), &_h_stdout_read,
			0, FALSE, DUPLICATE_SAME_ACCESS))
		{
			throw "[error] DuplicateHandle failed.";
		}

		if(!::DuplicateHandle(::GetCurrentProcess(), h_stdin_write_tmp,
			::GetCurrentProcess(), &_h_stdin_write,
			0, FALSE, DUPLICATE_SAME_ACCESS))
		{
			throw "[error] DuplicateHandle failed.";
		}

		close_handle(h_stdout_read_tmp);
		close_handle(h_stdin_write_tmp);

		if(!spawn_child(cmdline, h_stdout_write, h_stdin_read, h_stderr_write, working_directory))
			throw "[error] spawn_child failed.";

		close_handle(h_stdout_write);
		close_handle(h_stdin_read);
		close_handle(h_stderr_write);

		_h_event = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);
		_h_thread = ::CreateThread(nullptr, 0, thread_output, this, 0, &_dw_thread_id);

		if(!_h_event || !_h_thread)
			throw "[error] CreateEvent or CreateThread failed.";
	}
	catch(...) {
		throw;
	}

	return true;
}

bool redir_t::close()
{
	return false;
}

bool redir_t::print(LPCTSTR fmt, ...)
{
	char buf[2048];
	va_list va;
	va_start(va, fmt);
	int len = vsnprintf(buf, sizeof(buf), fmt, va);
	va_end(va);

	DWORD dwWritten;
	return !!::WriteFile(_h_stdin_write, buf, len, &dwWritten, NULL);
}

void redir_t::set_interval(DWORD fg, DWORD bg)
{
	_dw_wait_interval_fg = fg;
	_dw_wait_interval_bg = bg;
}

void redir_t::set_foreground(bool fg)
{
	_b_fg = fg;
}
}

