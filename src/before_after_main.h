#ifndef __BEFORE_AFTER_MAIN_H__
#define __BEFORE_AFTER_MAIN_H__

#if defined(_MSC_VER)
	#ifndef _CRTALLOC
		#define _CRTALLOC(x)		__declspec(allocate(x))
	#endif

	typedef int  (*_PIFV)(void);
	typedef void (*_PVFV)(void);

	#if 0
	#define RUN_XI(seq, func, ...)	_CRTALLOC(".CRT$XI" #seq)  _PIFV _run_xi_##seq##func[] = {func, __VA_ARGS__}
	#define RUN_XC(seq, func, ...)	_CRTALLOC(".CRT$XC" #seq)  _PVFV _run_xc_##seq##func[] = {func, __VA_ARGS__}
	#define RUN_XP(seq, func, ...)  _CRTALLOC(".CRT$XP" #seq)  _PVFV _run_xp_##seq##func[] = {func, __VA_ARGS__}
	#define RUN_XT(seq, func, ...)  _CRTALLOC(".CRT$XT" #seq)  _PVFV _run_xp_##seq##func[] = {func, __VA_ARGS__}
	#else
	#define RUN_X(seg, seq, func, ...)	\
		\#pragma data_seg(".CRT$X" #seg #seq)\
			_PVFV _run_xi_##seg##_##seq##_##func[] = {func, __VA_ARGS__};\
		\#pragma data_seg()

	#define RUN_XI(seq, func, ...)	RUN_X(I, seq, func, __VA_ARGS__)
	#define RUN_XC(seq, func, ...)	RUN_X(C, seq, func, __VA_ARGS__)
	#define RUN_XP(seq, func, ...)	RUN_X(P, seq, func, __VA_ARGS__)
	#define RUN_XT(seq, func, ...)  RUN_X(T, seq, func, __VA_ARGS__)
	#endif

	#define RUN_BEFORE_MAIN(func, ...)			RUN_XC(U, func, __VA_ARGS__)
	#define RUN_AFTER_MAIN(func, ...)			RUN_XP(U, func, __VA_ARGS__)
	#define RUN_BEFORE_AFTER_MAIN(func, ...)	RUN_BEFORE_MAIN(func, __VA_ARGS__);\
												RUN_AFTER_MAIN(func, __VA_ARGS__)

#elif defined(__GNUC__)
	#define RUN_BEFORE_MAIN(func)				void func(void)	__attribute__((constructor))
	#define RUN_AFTER_MAIN(func)				void func(void) __attribute__((destructor))
	#define RUN_BEFORE_AFTER_MAIN(func)			void func(void) __attribute__((constructor destructor))
#else
	#pragma message( "Warning: Compiler no support, some code is not run!" )
	#define RUN_BEFORE_MAIN(func, ...)				((void)0)	
	#define RUN_AFTER_MAIN(func, ...)				((void)0)
	#define RUN_BEFORE_AFTER_MAIN(func, ...)		((void)0)
#endif



#endif //__BEFORE_AFTER_MAIN_H__