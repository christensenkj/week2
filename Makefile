# Welcome to the world's worst Makefile.
CC := gcc

WARNINGS_ARE_ERRORS := -Wall -Wextra -Werror
COMPILER_OPTIMIZATIONS := -O3 -g
SO_FLAGS := -shared -fPIC -g 
CFLAGS := $(WARNINGS_ARE_ERRORS) $(COMPILER_OPTIMIZATIONS)

# Add any source files that you need to be compiled
# for your linked list here.
#
LINKED_LIST_SOURCE_FILES := linked_list.c
LINKED_LIST_OBJECT_FILES := linked_list.o

# Add any source files that you need to be compiled
# for your queue here.
#
QUEUE_SOURCE_FILES := queue.c $(LINKED_LIST_SOURCE_FILES)
QUEUE_OBJECT_FILES := queue.o $(LINKED_LIST_OBJECT_FILES)

# Functional testing support
#
FUNCTIONAL_TEST_SOURCE_FILES := linked_list_test_program.c
FUNCTIONAL_TEST_OBJECT_FILES := linked_list_test_program.o

# Set to 1 if on an ARM system.
#
COMPILE_ARM_PMU_CODE := 0

PERFORMANCE_TEST_SOURCE_FILES := queue_performance.c mmio.c
PERFORMANCE_TEST_OBJECT_FILES := queue_performance.o mmio.o

ifeq ($(COMPILE_ARM_PMU_CODE), 1)
	PERFORMANCE_TEST_SOURCE_FILES += arm_pmu.c
	PERFORMANCE_TEST_OBJECT_FILES += arm_pmu.c
	PERFORMANCE_TEST_COMPILER_DEFINES += -DCOMPILE_ARM_PMU_CODE
endif

# Specify what to test.
#
FUNCTIONAL_TEST_COMPILER_DEFINES := -DTEST_LINKED_LIST -DTEST_QUEUE

liblinked_list.so : $(LINKED_LIST_OBJECT_FILES)
	$(CC) $(CFLAGS) $(SO_FLAGS) $^ -o $@

libqueue.so : $(QUEUE_OBJECT_FILES)
	$(CC) $(CFLAGS) $(SO_FLAGS) $^ -o $@

linked_list_test_program: liblinked_list.so libqueue.so $(FUNCTIONAL_TEST_OBJECT_FILES)
	$(CC) -o $@ $(FUNCTIONAL_TEST_OBJECT_FILES) -L `pwd` -llinked_list -lqueue

queue_performance: $(PERFORMANCE_TEST_OBJECT_FILES) libqueue.so
	$(CC) -o $@ $(PERFORMANCE_TEST_OBJECT_FILES) $(PERFORMANCE_TEST_COMPILER_DEFINES) -L `pwd` -lqueue

run_functional_tests: linked_list_test_program
	LD_LIBRARY_PATH=`pwd`:$$LD_LIBRARY_PATH ./linked_list_test_program

run_functional_tests_gdb: linked_list_test_program
	LD_LIBRARY_PATH=`pwd`:$$LD_LIBRARY_PATH gdb ./linked_list_test_program

run_valgrind_tests: linked_list_test_program
	LD_LIBRARY_PATH=`pwd`:$$LD_LIBRARY_PATH valgrind ./linked_list_test_program

run_performance_tests: queue_performance
	LD_LIBRARY_PATH=`pwd`:$$LD_LIBRARY_PATH ./queue_performance

# Special case the Matrix Market I/O code
mmio.o : mmio.c
	$(CC) -c -o mmio.o $(CFLAGS) -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-unused-result $^

linked_list_test_program.o : linked_list_test_program.c
	$(CC) -c -o linked_list_test_program.o $(CFLAGS) $(FUNCTIONAL_TEST_COMPILER_DEFINES) $^

download_and_decompress_test_data:
	echo "Downloading and decompressing test data (2007 Wikipedia adjacency matrix)"
	echo "provided under license (CC-BY 4.0 license) from the SuiteSparse Matrix Collection"
	wget "https://suitesparse-collection-website.herokuapp.com/MM/Gleich/wikipedia-20070206.tar.gz"
	tar -xvf wikipedia-20070206.tar.gz

%.o : %.c
	$(CC) -c $(CFLAGS) $^ -o $@

clean:
	rm $(LINKED_LIST_OBJECT_FILES) $(QUEUE_OBJECT_FILES) $(FUNCTIONAL_TEST_OBJECT_FILES) $(PERFORMANCE_TEST_OBJECT_FILES) liblinked_list.so libqueue.so linked_list_test_program 
