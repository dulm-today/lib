#ifndef __VADEF_H_INCLUDED__
#define __VADEF_H_INCLUDED__

#ifndef MACRO_VALUE
#define _MACRO_VALUE(x)	#x
#define MACRO_VALUE(x)	_MACRO_VALUE(x)
#endif

#ifndef va_copy
#if _MSC_VER < 1700
	#ifdef __va_copy
	#define va_copy(dest, src)		__va_copy((dest),(src))
	#else
	#pragma message( "Compiler version: " _MACRO_VALUE(_MSC_VER) " = " MACRO_VALUE(_MSC_VER) )
	#pragma message( "Warning: va_copy is need, VS2012 is the first version support it !" )
	#pragma message( "Warning: use \"#define va_copy(cp,ap) memcpy((&cp), (&ap), sizeof(va_list))\" instead !" )
	
	#define va_copy(cp, ap)		memcpy(&(cp), &(ap), sizeof(va_list))
	#endif
#endif
#endif

#endif //__VADEF_H_INCLUDED__