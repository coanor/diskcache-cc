# examples fome https://stackoverflow.com/a/32168987/342348
TARGET = lib.a

SPDLOG_DIR=../spdlog
GTEST_DIR=../googletest-1.14.0/googletest
USER_DIR=.

CPPFLAGS += -isystem $(GTEST_DIR)/include -isystem $(SPDLOG_DIR)/include
CXXFLAGS += -g -Wall -Wextra -pthread -std=c++2a

GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

SPDLOG_HEADERS = $(SPDLOG_DIR)/include

GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

$(TARGET): open.o put.o
	@ar rcs $@ $^

open.o: open.cc
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

put.o: put.cc
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf *.o *.a *.gch $(TARGET)

t_open.o: $(USER_DIR)/t_open.cc $(GTEST_HEADERS)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(USER_DIR)/t_open.cc

t_put.o: $(USER_DIR)/t_put.cc $(GTEST_HEADERS)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(USER_DIR)/t_put.cc

T: t_open.o t_put.o gtest_main.a open.o put.o
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@

############################################################
# gtest related
############################################################

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

############################################################
# spdlog related(TODO)
############################################################
