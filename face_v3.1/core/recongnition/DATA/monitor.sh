#!/bin/bash
FilePath='/root/data/target.txt'

# 发送邮件
sendMail() {
        if [ $1 = '0' ];then
                echo "智能锁被正常开启" | mail -s "智能锁" 1049521741@qq.com
        elif [ $1 = '1' ];then
                echo "智能锁被stranger开启" | mail -s "智能锁" 1049521741@qq.com
        fi
}


# 监控文件
CheckFile() {
        flag_change=$(sed -n '1p' $FilePath | awk -F " " '{print $1}')
        flag_person=$(sed -n '1p' $FilePath | awk -F " " '{print $2}')
        echo $flag_change
        echo $flag_person
        if [[ $flag_person = '1' ]];then
                sendMail $flag_person
                echo "邮件已发送"
        fi
}

while true
"monitor.sh" 30L, 605C 已写入                                                       
[root@MyLinux data]# bash monitor.sh 
1
0
^[^C
[root@MyLinux data]# vim monitor.sh  
# 发送邮件
sendMail() {
        if [ $1 = '0' ];then
                echo "智能锁被正常开启" | mail -s "智能锁" 1049521741@qq.com
        elif [ $1 = '1' ];then
                echo "智能锁被stranger开启" | mail -s "智能锁" 1049521741@qq.com
        fi
}


# 监控文件
CheckFile() {
        flag_change=$(sed -n '1p' $FilePath | awk -F " " '{print $1}')
        flag_person=$(sed -n '1p' $FilePath | awk -F " " '{print $2}')
        if [[ $flag_change = '1' ]];then
                sendMail $flag_person