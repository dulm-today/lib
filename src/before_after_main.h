#ifndef __BEFORE_AFTER_MAIN_H__
#define __BEFORE_AFTER_MAIN_H__

/*
	VC/VC++

	run before main is implement by add function pointer to the section 
	".CRT$XIA ~ .CRT$XIZ" for C init or ".CRT$XCA ~ .CRT$XCZ" for C++ init.

	run after main has two way:
		1. add function pointer to the section ".CRT$XPA ~ .CRT$XPZ" or 
		".CRT$XTA ~ .CRT$XTZ". This is not worked when exec/dll linking to 
		CRT DLL, you should use 'atexit' instead.
		2. run the function 'atexit'.

	add something to section:
		1. #pragma section(".CRT$XIU", long, read)

		// .CRT$XIU must be defined in data_seg or section
		__declspec(allocate(".CRT$XIU")) type value[] = {};	
		
		2. #pragma data_seg(".CRT$XIU")
		   type value[] = {};
		   #pragma data_seg()
*/



#if defined(_MSC_VER)
	#ifndef _CRTALLOC
		#define _CRTALLOC(x)		__declspec(allocate(x))
	#endif

	#define _JOIN(x, y)		x##y

	typedef int  (*_PIFV)(void);
	typedef void (*_PVFV)(void);

	#pragma section(".CRT$XIU",long,read)
	#pragma section(".CRT$XCU",long,read)
	#pragma section(".CRT$XPU",long,read)
	#pragma section(".CRT$XTU",long,read)
	
	#define RUN_XIU(func, ...)		_CRTALLOC(".CRT$XIU")  _PVFV _run_xiu##_##func[] = {func, __VA_ARGS__}
	#define RUN_XCU(func, ...)		_CRTALLOC(".CRT$XCU")  _PVFV _run_xcu##_##func[] = {func, __VA_ARGS__}
	#define RUN_XPU(func, ...)		_CRTALLOC(".CRT$XPU")  _PVFV _run_xpu##_##func[] = {func, __VA_ARGS__}
	#define RUN_XTU(func, ...)  	_CRTALLOC(".CRT$XTU")  _PVFV _run_xtu##_##func[] = {func, __VA_ARGS__}

	#define RUN_BEFORE_MAIN(func, ...)			RUN_XCU(func, __VA_ARGS__)
	#define RUN_AFTER_MAIN(func, ...)			RUN_XPU(func, __VA_ARGS__)
	#define RUN_BEFORE_AFTER_MAIN(func, ...)	RUN_BEFORE_MAIN(func, __VA_ARGS__);\
												RUN_AFTER_MAIN(func, __VA_ARGS__)	
#elif defined(__GNUC__)
	#define RUN_BEFORE_MAIN(func)				void func(void)	__attribute__((constructor))
	#define RUN_AFTER_MAIN(func)				void func(void) __attribute__((destructor))
	#define RUN_BEFORE_AFTER_MAIN(func)			void func(void) __attribute__((constructor));\
												void func(void) __attribute__((destructor))
#else
	#pragma message( "Warning: Compiler no support, some code is not run!" )
	#define RUN_BEFORE_MAIN(func, ...)				((void)0)	
	#define RUN_AFTER_MAIN(func, ...)				((void)0)
	#define RUN_BEFORE_AFTER_MAIN(func, ...)		((void)0)
#endif



#endif //__BEFORE_AFTER_MAIN_H__