#ifndef __MEMALLOC_H__
#define __MEMALLOC_H__


#ifdef _DEBUG
	#define Malloc(x)			__dbg_alloc_(x,__FILE__,__LINE__)
	#define Free(x)				__dbg_free_(x)
	#define MallocPrint			__dbg_print_
	#define BlockInit(x)		__dbg_binit(x,__FILE__,__LINE__)
	#define BlockDelete(h)		__dbg_bdelete(h,__FILE__,__LINE__)
	#define BlockMalloc(h,x)	__dbg_balloc(h,x,__FILE__,__LINE__)
	#define BlockFree(h,x)		__dbg_bfree(h,x,__FILE__,__LINE__)
#else
	#define Malloc(x)			__alloc(x)
	#define Free(x)				__free(x)
	#define BlockInit(x)		__binit(x)
	#define BlockDelete(h)		__bdelete(h)
	#define BlockMalloc(h,x)	__balloc(h,x)
	#define BlockFree(h,x)		__bfree(h,x)
#endif

#ifdef __cplusplus
extern "C"{
#endif

#ifdef _DEBUG

void* 	__dbg_alloc_( unsigned int n, const char* file, unsigned int line );
void 	__dbg_free_( void* p );
void	__dbg_print_();

void*	__dbg_binit( unsigned int n, const char* file, unsigned int line );
void	__dbg_bdelete( void* h );
void*	__dbg_balloc( void* h, unsigned int n );
void	__dbg_bfree( void* h, void* p );

#else

void* __alloc( unsigned int n );
void  __free( void* p );

void*	__binit( unsigned int n );
void	__bdelete( void* h );
void*	__balloc( void* h, void* p);
void	__bfree( void* h, void* p);

#endif


#ifdef __cplusplus
}
#endif

#endif // __MEMALLOC_H__