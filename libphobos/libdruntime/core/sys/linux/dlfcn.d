/**
 * D header file for GNU/Linux
 *
 * $(LINK2 http://sourceware.org/git/?p=glibc.git;a=blob;f=dlfcn/dlfcn.h, glibc dlfcn/dlfcn.h)
 */
module core.sys.linux.dlfcn;

version (linux):
extern (C):
nothrow:
@nogc:

public import core.sys.posix.dlfcn;
import core.sys.linux.config;

// <bits/dlfcn.h>
version (X86)
{
    // http://sourceware.org/git/?p=glibc.git;a=blob;f=bits/dlfcn.h
    // enum RTLD_LAZY = 0x00001; // POSIX
    // enum RTLD_NOW = 0x00002; // POSIX
    enum RTLD_BINDING_MASK = 0x3;
    enum RTLD_NOLOAD = 0x00004;
    enum RTLD_DEEPBIND = 0x00008;

    // enum RTLD_GLOBAL = 0x00100; // POSIX
    // enum RTLD_LOCAL = 0; // POSIX
    enum RTLD_NODELETE = 0x01000;

    static if (__USE_GNU)
    {
        RT DL_CALL_FCT(RT, Args...)(RT function(Args) fctp, auto ref Args args)
        {
            _dl_mcount_wrapper_check(cast(void*)fctp);
            return fctp(args);
        }

        void _dl_mcount_wrapper_check(void* __selfpc);
    }
}
else version (X86_64)
{
    // http://sourceware.org/git/?p=glibc.git;a=blob;f=bits/dlfcn.h
    // enum RTLD_LAZY = 0x00001; // POSIX
    // enum RTLD_NOW = 0x00002; // POSIX
    enum RTLD_BINDING_MASK = 0x3;
    enum RTLD_NOLOAD = 0x00004;
    enum RTLD_DEEPBIND = 0x00008;

    // enum RTLD_GLOBAL = 0x00100; // POSIX
    // enum RTLD_LOCAL = 0; // POSIX
    enum RTLD_NODELETE = 0x01000;

    static if (__USE_GNU)
    {
        RT DL_CALL_FCT(RT, Args...)(RT function(Args) fctp, auto ref Args args)
        {
            _dl_mcount_wrapper_check(cast(void*)fctp);
            return fctp(args);
        }

        void _dl_mcount_wrapper_check(void* __selfpc);
    }
}
else version (MIPS32)
{
    // http://sourceware.org/git/?p=glibc.git;a=blob;f=ports/sysdeps/mips/bits/dlfcn.h
    // enum RTLD_LAZY = 0x0001; // POSIX
    // enum RTLD_NOW = 0x0002; // POSIX
    enum RTLD_BINDING_MASK = 0x3;
    enum RTLD_NOLOAD = 0x00008;
    enum RTLD_DEEPBIND = 0x00010;

    // enum RTLD_GLOBAL = 0x0004; // POSIX
    // enum RTLD_LOCAL = 0; // POSIX
    enum RTLD_NODELETE = 0x01000;

    static if (__USE_GNU)
    {
        RT DL_CALL_FCT(RT, Args...)(RT function(Args) fctp, auto ref Args args)
        {
            _dl_mcount_wrapper_check(cast(void*)fctp);
            return fctp(args);
        }

        void _dl_mcount_wrapper_check(void* __selfpc);
    }
}
else version (MIPS64)
{
    // http://sourceware.org/git/?p=glibc.git;a=blob;f=ports/sysdeps/mips/bits/dlfcn.h
    // enum RTLD_LAZY = 0x0001; // POSIX
    // enum RTLD_NOW = 0x0002; // POSIX
    enum RTLD_BINDING_MASK = 0x3;
    enum RTLD_NOLOAD = 0x00008;
    enum RTLD_DEEPBIND = 0x00010;

    // enum RTLD_GLOBAL = 0x0004; // POSIX
    // enum RTLD_LOCAL = 0; // POSIX
    enum RTLD_NODELETE = 0x01000;

    static if (__USE_GNU)
    {
        RT DL_CALL_FCT(RT, Args...)(RT function(Args) fctp, auto ref Args args)
        {
            _dl_mcount_wrapper_check(cast(void*)fctp);
            return fctp(args);
        }

        void _dl_mcount_wrapper_check(void* __selfpc);
    }
}
else version (PPC)
{
    // http://sourceware.org/git/?p=glibc.git;a=blob;f=bits/dlfcn.h
    // enum RTLD_LAZY = 0x0001; // POSIX
    // enum RTLD_NOW = 0x0002; // POSIX
    enum RTLD_BINDING_MASK = 0x3;
    enum RTLD_NOLOAD = 0x00004;
    enum RTLD_DEEPBIND = 0x00008;

    // enum RTLD_GLOBAL = 0x00100; // POSIX
    // enum RTLD_LOCAL = 0; // POSIX
    enum RTLD_NODELETE = 0x01000;

    static if (__USE_GNU)
    {
        RT DL_CALL_FCT(RT, Args...)(RT function(Args) fctp, auto ref Args args)
        {
            _dl_mcount_wrapper_check(cast(void*)fctp);
            return fctp(args);
        }

        void _dl_mcount_wrapper_check(void* __selfpc);
    }
}
else version (PPC64)
{
    // http://sourceware.org/git/?p=glibc.git;a=blob;f=bits/dlfcn.h
    // enum RTLD_LAZY = 0x0001; // POSIX
    // enum RTLD_NOW = 0x0002; // POSIX
    enum RTLD_BINDING_MASK = 0x3;
    enum RTLD_NOLOAD = 0x00004;
    enum RTLD_DEEPBIND = 0x00008;

    // enum RTLD_GLOBAL = 0x00100; // POSIX
    // enum RTLD_LOCAL = 0; // POSIX
    enum RTLD_NODELETE = 0x01000;

    static if (__USE_GNU)
    {
        RT DL_CALL_FCT(RT, Args...)(RT function(Args) fctp, auto ref Args args)
        {
            _dl_mcount_wrapper_check(cast(void*)fctp);
            return fctp(args);
        }

        void _dl_mcount_wrapper_check(void* __selfpc);
    }
}
else version (ARM)
{
    // http://sourceware.org/git/?p=glibc.git;a=blob;f=bits/dlfcn.h
    // enum RTLD_LAZY = 0x0001; // POSIX
    // enum RTLD_NOW = 0x0002; // POSIX
    enum RTLD_BINDING_MASK = 0x3;
    enum RTLD_NOLOAD = 0x00004;
    enum RTLD_DEEPBIND = 0x00008;

    // enum RTLD_GLOBAL = 0x00100; // POSIX
    // enum RTLD_LOCAL = 0; // POSIX
    enum RTLD_NODELETE = 0x01000;

    static if (__USE_GNU)
    {
        RT DL_CALL_FCT(RT, Args...)(RT function(Args) fctp, auto ref Args args)
        {
            _dl_mcount_wrapper_check(cast(void*)fctp);
            return fctp(args);
        }

        void _dl_mcount_wrapper_check(void* __selfpc);
    }
}
else version (AArch64)
{
    // http://sourceware.org/git/?p=glibc.git;a=blob;f=bits/dlfcn.h
    // enum RTLD_LAZY = 0x0001; // POSIX
    // enum RTLD_NOW = 0x0002; // POSIX
    enum RTLD_BINDING_MASK = 0x3;
    enum RTLD_NOLOAD = 0x00004;
    enum RTLD_DEEPBIND = 0x00008;

    // enum RTLD_GLOBAL = 0x00100; // POSIX
    // enum RTLD_LOCAL = 0; // POSIX
    enum RTLD_NODELETE = 0x01000;

    static if (__USE_GNU)
    {
        RT DL_CALL_FCT(RT, Args...)(RT function(Args) fctp, auto ref Args args)
        {
            _dl_mcount_wrapper_check(cast(void*)fctp);
            return fctp(args);
        }

        void _dl_mcount_wrapper_check(void* __selfpc);
    }
}
else version (SPARC64)
{
    // http://sourceware.org/git/?p=glibc.git;a=blob;f=bits/dlfcn.h
    // enum RTLD_LAZY = 0x0001; // POSIX
    // enum RTLD_NOW = 0x0002; // POSIX
    enum RTLD_BINDING_MASK = 0x3;
    enum RTLD_NOLOAD = 0x00004;
    enum RTLD_DEEPBIND = 0x00008;

    // enum RTLD_GLOBAL = 0x00100; // POSIX
    // enum RTLD_LOCAL = 0; // POSIX
    enum RTLD_NODELETE = 0x01000;

    static if (__USE_GNU)
    {
        RT DL_CALL_FCT(RT, Args...)(RT function(Args) fctp, auto ref Args args)
        {
            _dl_mcount_wrapper_check(cast(void*)fctp);
            return fctp(args);
        }

        void _dl_mcount_wrapper_check(void* __selfpc);
    }
}
else version (SystemZ)
{
    // http://sourceware.org/git/?p=glibc.git;a=blob;f=bits/dlfcn.h
    // enum RTLD_LAZY = 0x0001; // POSIX
    // enum RTLD_NOW = 0x0002; // POSIX
    enum RTLD_BINDING_MASK = 0x3;
    enum RTLD_NOLOAD = 0x00004;
    enum RTLD_DEEPBIND = 0x00008;

    // enum RTLD_GLOBAL = 0x00100; // POSIX
    // enum RTLD_LOCAL = 0; // POSIX
    enum RTLD_NODELETE = 0x01000;

    static if (__USE_GNU)
    {
        RT DL_CALL_FCT(RT, Args...)(RT function(Args) fctp, auto ref Args args)
        {
            _dl_mcount_wrapper_check(cast(void*)fctp);
            return fctp(args);
        }

        void _dl_mcount_wrapper_check(void* __selfpc);
    }
}
else
    static assert(0, "unimplemented");

// <bits/dlfcn.h>

static if (__USE_GNU)
{
    enum RTLD_NEXT = cast(void *)-1L;
    enum RTLD_DEFAULT = cast(void *)0;
    alias c_long Lmid_t;
    enum LM_ID_BASE = 0;
    enum LM_ID_NEWLM = -1;
}

// void* dlopen(in char* __file, int __mode); // POSIX
// int dlclose(void* __handle); // POSIX
// void* dlsym(void* __handle, in char* __name); // POSIX

static if (__USE_GNU)
{
    void* dlmopen(Lmid_t __nsid, in char* __file, int __mode);
    void* dlvsym(void* __handle, in char* __name, in char* __version);
}

// char* dlerror(); // POSIX

static if (__USE_GNU)
{
    struct Dl_info
    {
        const(char)* dli_fname;
        void* dli_fbase;
        const(char)* dli_sname;
        void* dli_saddr;
    }

    int dladdr(in void* __address, Dl_info* __info);
    int dladdr1(void* __address, Dl_info* __info, void** __extra_info, int __flags);

    enum
    {
        RTLD_DL_SYMENT = 1,
        RTLD_DL_LINKMAP = 2,
    }

    int dlinfo(void* __handle, int __request, void* __arg);

    enum
    {
        RTLD_DI_LMID = 1,
        RTLD_DI_LINKMAP = 2,
        RTLD_DI_CONFIGADDR = 3,
        RTLD_DI_SERINFO = 4,
        RTLD_DI_SERINFOSIZE = 5,
        RTLD_DI_ORIGIN = 6,
        RTLD_DI_PROFILENAME = 7,
        RTLD_DI_PROFILEOUT = 8,
        RTLD_DI_TLS_MODID = 9,
        RTLD_DI_TLS_DATA = 10,
        RTLD_DI_MAX = 10,
    }

    struct Dl_serpath
    {
        char* dls_name;
        uint dls_flags;
    }

    struct Dl_serinfo
    {
        size_t dls_size;
        uint dls_cnt;
        Dl_serpath[1] dls_serpath;
    }
}
