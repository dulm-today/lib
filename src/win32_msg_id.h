#ifndef __WIN32_MSG_ID_H__
#define __WIN32_MSG_ID_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _UNICODE
const wchar_t* win32_msg_id(int id);
#else
const char* win32_msg_id(int id);
#endif


#ifdef __cplusplus
}
#endif

#endif //__WIN32_MSG_ID_H__