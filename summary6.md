# linux系统
## shell脚本编写
bash和sh：sh为早起的脚本执行命令，bash可以被视为一个功能更加强大的sh。<br>
在脚本文件中第一行通常写为#!/bin/bash，注意仅能写在第一行，告诉系统该脚本又bash来解释执行。<br>
shell脚本的一些流程控制语句，if、for、while、case。<br>
makefile文件的编写。<br>
make执行编译指令的条件是根据目标文件和所依赖文件的最后修改日期进行判断的。<br>
make命令执行，make命令让系统寻找当前目录下makefile\Makefile文件，能够快速执行已经预先写好的编译命令。
## 用户身份与文件权限的基础概念
Linux用户分为管理员用户(root，uid为0)、系统用户(uid为1-999)、普通用户(uid>1000)。<br>
Linux同时引入了用户组的概念。<br>
文件的权限可以分为，读、写、执行。在Linux系统可以通过'll'命令快速查看，这三个权限依次被分为文件所有者、文件所属组、其他用户。<br>
Linux操作系统不仅提供这些简单的方式来保证文件权限在正确的方式被需求的用户所使用。如su、sudo命令。<br>
# 数据库基础
MySQL的数据类型较多，但是主要可以分为数字、字符和日期类型。<br>
MySQL的数据字典(INFORMATION_SCHEMA)，该数据库提供了访问元数据的方式。<br>
这位开发数据抽取子系统和数据入库子系统提供非常友好的帮助。<br>
关系型数据库。<br>
了解一些MySQL高可用方案的简单原理。<br>
MySQL的federated引擎基础概念。
# 项目进展
完成了数据入库子系统和数据抽取子系统。<br><br>
[leetcode](https://github.com/yzh123qwer/leetcode)
