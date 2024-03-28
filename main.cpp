#include "./include/test.h"


int main(int argc,char** argv) {

//    test_log();
//    test_util();
//
//    test_env(argc,argv);
//
//    main_test_config(argc,argv);

//    test_thread(argc,argv);

    test_fiber_main_func(argc,argv);
    return 0;
}