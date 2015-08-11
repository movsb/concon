#include <windows.h>

#include <redir/redir.h>

/*int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)*/
int main()
{
	concon::redir_t redir;
	redir.open("cmd", "c:\\");
	
	char buf[1024];
	while(gets(buf)){
		redir.print("%s\n", buf);
	}

	return 0;
}
