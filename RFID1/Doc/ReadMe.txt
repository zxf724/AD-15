一、工程说明
    1、时间：    2016年09月02日
    2、开发环境：IAR for STM8 V2.20.1
    3、标准库：  V2.2.0
    4、工程版本：V1.0.0
    5、目标芯片：stm8l15x系列芯片

二、更新日志：
    2016-09-02 V1.0.0:初始版本

三、卡信息数据块（默认使用块2）描述
        字节：   0        1  2            3  4                  5                   6  7
    代表意思： 卡类型  用户编号(ID)   普通卡充值次数     管理卡读取刷卡机编号     刷卡次数

    管理卡读取刷卡机（数据块4、5）
        字节：   0          1     2  3       4     5  6  ···
              刷卡机数     编号   次数      编号   次数  ···