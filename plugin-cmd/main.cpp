#include <plugin.h>
#include <util.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/epoll.h>

#include <sstream>
#include <vector>
#include <string>

using namespace std;
sighandler_t oldact;

static void sigchld(int) { }

int
execute_and_get_output(vector<string> args, string &output) {
    
    size_t buf_len = 0;
    for (int i = 0; i < args.size(); ++ i) {
        buf_len += args[i].length() + 1;
    }
    char **argv = (char **)malloc(sizeof(char *) * (args.size() + 1));
    char *buf = (char *)malloc(buf_len);
    char *cur = buf;
    for (int i = 0; i < args.size(); ++ i) {
        memcpy(cur, args[i].c_str(), args[i].length());
        argv[i] = cur;
        cur += args[i].length();
        *cur = 0; ++ cur;
    }
    argv[args.size()] = NULL;
    
    int pfd[2];
    int err;
    pid_t c;
    // c = 0;
    
    // struct sigaction oldsig;
    // struct sigaction sigact;
    // sigact.sa_handler = sigchld;
    // sigemptyset (&sigact.sa_mask);
    // sigact.sa_flags = 0;
    // sigact.sa_restorer = NULL;
    
    // sigaction(SIGCHLD, &sigact, &oldsig);
    oldact = signal(SIGCHLD, sigchld);
        
    if ((err = pipe2(pfd, O_NONBLOCK)) < 0) {
        goto onerr;
    }

    
    c = fork();
    if (c == 0)
    {
        int fdnull = open("/dev/null", O_WRONLY);
        if (dup2(fdnull, STDIN_FILENO) < 0)
            _exit(-1);
        if (dup2(pfd[1], STDOUT_FILENO) < 0)
            _exit(-1);
        if (dup2(fdnull, STDERR_FILENO) < 0)
            _exit(-1);
        close(pfd[0]); close(pfd[1]); close(fdnull);
        execvp(argv[0], argv);
        _exit(-1);
    } else {
        close(pfd[1]);
        
        int ep = epoll_create(1);
        struct epoll_event eev;
        eev.events = EPOLLIN | EPOLLERR | EPOLLET;
        epoll_ctl(ep, EPOLL_CTL_ADD, pfd[0], NULL);
        
        ssize_t s;
        char buf[1024];
        ostringstream ss;
        while (true) {
            epoll_wait(ep, &eev, 1, -1);
            while (true) {
                s = read(pfd[0], buf, 1024);
                // fprintf(stderr, "got %d\n", s);
                if (s == -EAGAIN) break;
                else if (s == 0) goto succ;
                else if (s < 0) {
                    err = s;
                    goto onerr;
                }
                ss.write(buf, s);
            }
        }
      succ:
        output = ss.str();
        close(pfd[0]);
    }

  onerr:

    // sigaction(SIGCHLD, &oldsig, &sigact);
    signal(SIGCHLD, oldact);
    free(argv);
    free(buf);
    return err;
}

webdeskit_host_interface_t host;
webdeskit_plugin_interface_s plugin =
{
    NULL,
};

extern char **environ;

static JSValueRef
js_cb_cmd_output(JSContextRef context,
                 JSObjectRef function,
                 JSObjectRef self,
                 size_t argc,
                 const JSValueRef argv[],
                 JSValueRef* exception)
{
    if (argc != 2)
        return JSValueMakeNull(context);
    
    int len = JSValueToNumber(context, argv[0], NULL);
    JSObjectRef arr = JSValueToObject(context, argv[1], NULL);

    if (len == 0) return JSValueMakeNull(context);
    vector<string> args;

    int i;
    for (i = 0; i < len; ++ i)
    {
        JSValueRef cur = JSObjectGetPropertyAtIndex(context, arr, i, NULL);
        string str;
        copy_from_jsstring_object(context, cur, str);
        JSValueUnprotect(context, cur);
        
        args.push_back(str);
    }

    string output;
    if (execute_and_get_output(args, output)) {
        return JSValueMakeNull(context);
    }

    return jsstring_object_from_string(context, output);
}

extern "C" {
    
    webdeskit_plugin_interface_t
    init(webdeskit_host_interface_t host_interface)
    {
        host = host_interface;
        
        host->register_native_method("CmdOutput", js_cb_cmd_output);
        
        return &plugin;
    }
}
