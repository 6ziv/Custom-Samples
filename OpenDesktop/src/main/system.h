#ifndef SYSTEM_H
#define SYSTEM_H
[[noreturn]] inline void die();
[[noreturn]] void error(const char *message);
#define NOFAULTBEG try{
#define NOFAULTFIN }catch(...){qt_noop();}
#define NOFAULTMID NOFAULTFIN NOFAULTBEG
#define NOFAULT    NOFAULTMID
#endif // SYSTEM_H
