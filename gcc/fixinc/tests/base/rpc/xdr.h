/*  DO NOT EDIT THIS FILE.

    It has been auto-edited by fixincludes from:

	"fixinc/tests/inc/rpc/xdr.h"

    This had to be done to correct non-standard usages in the
    original, manufacturer supplied header file.  */



#if defined( RPC_XDR_LVALUE_CAST_A_CHECK )
#define IXDR_GET_LONG(buf) ((long)IXDR_GET_U_INT32(buf))
#endif  /* RPC_XDR_LVALUE_CAST_A_CHECK */


#if defined( RPC_XDR_LVALUE_CAST_B_CHECK )
#define IXDR_PUT_LONG(buf, v) ((long)IXDR_PUT_INT32(buf, (long)(v)))
#endif  /* RPC_XDR_LVALUE_CAST_B_CHECK */


#if defined( STRUCT_FILE_CHECK )
struct __file_s;
extern void xdrstdio_create( struct __file_s* );
#endif  /* STRUCT_FILE_CHECK */
