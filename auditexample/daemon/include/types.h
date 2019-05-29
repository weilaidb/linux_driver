#ifndef _AUDIT_TYPES_H
#define _AUDIT_TYPES_H

#include <sys/types.h>

typedef signed char s8;    /**<\brief 8-bit signed integer*/
typedef signed short s16;  /**<\brief 16-bit signed integer*/
typedef signed int s32;    /**<\brief 32-bit signed integer*/
typedef unsigned char u8;  /**<\brief 8-bit unsigned integer*/
typedef unsigned short u16;/**<\brief 16-bit unsigned integer*/
typedef unsigned int u32;  /**<\brief 32-bit unsigned integer*/

/**\brief stores information about a system call that will be logged*/
struct syscall_buf {
    u32 serial;                 /**<\brief serial number */
    u32 ts_sec;                 /**<\brief timestamp, number of seconds since January 1, 1970 */
    u32 ts_micro;               /**<\brief microseconds timestamp, complements ts_sec */
    u32 syscall;                /**<\brief the system call identifier */
    s32 status;                 /**<\brief the return status of the process */
    pid_t pid;                  /**<\brief Process ID */
    uid_t uid;                  /**<\brief Real user ID */
    uid_t euid;                 /**<\brief Effective User ID */
    s32 device;                 /**<\brief tty device */
    u8 comm[COMM_SIZE];         /**<\brief The name of the program executed by the process */
};

#endif                          /* _AUDIT_TYPES_H */


