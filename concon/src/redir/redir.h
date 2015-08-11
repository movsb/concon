#ifndef __concon_redir_h__
#define __concon_redir_h__

#include <windows.h>
#include <cstdio>

namespace concon {
	class redir_t {
	public:
		redir_t();
		virtual ~redir_t();

	private:
		HANDLE _h_thread;
		HANDLE _h_event;
		DWORD _dw_thread_id;
		DWORD _dw_wait_interval_fg;
		DWORD _dw_wait_interval_bg;
		bool _b_fg;

	protected:
		HANDLE _h_stdin_write;
		HANDLE _h_stdout_read;
		HANDLE _h_child_process;

	protected:
		void close_handle(HANDLE& handle);
		bool spawn_child(LPCTSTR cmdline,
			HANDLE h_stdout,
			HANDLE h_stdin,
			HANDLE h_stderr,
			LPCTSTR working_directory);
		int redirect();

		static DWORD WINAPI thread_output(LPVOID ud);

	protected:
		virtual void write_stdout(LPCTSTR str);

	public:
		bool open(LPCTSTR cmdline, LPCTSTR working_directory = nullptr);
		bool close();
		bool print(LPCTSTR fmt, ...);
		void set_interval(DWORD fg, DWORD bg);
		void set_foreground(bool fg);
	};
}


#endif //!__concon_redir_h__
