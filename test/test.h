#ifndef __TEST_H__
#define __TEST_H__

#define lambda(ret_type, args, body) ({ret_type func_name args body &func_name;})
#define $(body) lambda(void, (), body)

typedef void (*TestFunc)(gpointer data);
gboolean T_test(TestFunc f, gpointer data, int ms, int count, const char*);
gboolean T(TestFunc f, const char* test_name);

#define Test(body, name) T($(body), name)
#define RES_IN_KB(n) (n)
#define RES_IN_MB(n) (n * 1024)
#define RES_IN_GB(n) (RES_IN_MB(n) * 1024)

#endif
