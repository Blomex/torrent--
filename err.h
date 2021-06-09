#ifndef PROJECT_ERR_H
#define PROJECT_ERR_H

/*prints error when system functions fails, then exits program*/
extern void syserr(const char *fmt, ...);

/*prints error and exits program*/
extern void fatal(const char *fmt, ...);

#endif //PROJECT_ERR_H
