#  开发框架头文件路径。
PUBINCL = -I/root/project/public

#  开发框架cpp文件，这里直接包含进来，没有采用链接库，是为了方便调试。
PUBCPP = /root/project/public/_public.cpp

# 编译参数。
CFLAGS = -g

all:crtsurfdata1

crtsurfdata1:crtsurfdata1.cpp
        g++ $(CFLAGS) -o crtsurfdata1 crtsurfdata1.cpp $(PUBINCL) $(PUBCPP) -lm -lc
        cp crtsurfdata1 ../bin/.

clean:
        rm crtsurfdata1
