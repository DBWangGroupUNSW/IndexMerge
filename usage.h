#ifndef __usage_h__
#define __usage_h__


extern long __user_time_sec;
extern long __user_time_usec;

extern long __sys_time_sec;
extern long __sys_time_usec;

extern long __elapse_time_sec;
extern long __elapse_time_usec;

extern char __usage_information[1024];

void ResetUsage(void);

void ShowUsage(void);

#endif
