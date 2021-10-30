//
// Created by jeremy on 2021/10/29.
//
# include <time.h>
#include <cstdlib>
#include <iostream>
#include <cstring>

class mytime {
    public:
        static bool chars_to_date(const char * str, time_t& time){
            //用来将格式化字符串进行类型检查并且转换成time_t格式返回
            //如果非法日期无法转化就会返回false
            int size = sizeof(struct tm);
            struct tm *timeinfo = (struct tm *)malloc(sizeof (struct tm));
            memset(timeinfo,0x00,sizeof(struct tm));
            char * pret = strptime(str,"%Y-%m-%d",timeinfo);// 对输入的日期字符串进行转换并且同时判断非法输入
            if (pret== NULL || *pret!='\0') {
                free(timeinfo);
                return false;// 非法日期格式输入（只判断了格式）
            }
            {
                //判断已经解析的日期是否是一个合法的日期，比如闰年什么的。
                if (nullptr == timeinfo)
                {
                    free(timeinfo);
                    return false;
                }

                if (timeinfo->tm_mon < 0 || timeinfo->tm_mon > 11
                    || timeinfo->tm_mday < 1 || timeinfo->tm_mday > 31
                    || timeinfo->tm_hour < 0 || timeinfo->tm_hour > 23
                    || timeinfo->tm_min < 0 || timeinfo->tm_min > 59
                    || timeinfo->tm_sec < 0 || timeinfo->tm_sec > 59)
                {
                    free(timeinfo);
                    return false;
                }

                if (4 == (timeinfo->tm_mon + 1) || 6 == (timeinfo->tm_mon + 1) || 9 == (timeinfo->tm_mon + 1) || 11 == (timeinfo->tm_mon + 1))    //小月有30天
                {
                    if (timeinfo->tm_mday > 30)
                    {
                        free(timeinfo);
                        return false;
                    }
                }

                if (2 == (timeinfo->tm_mon + 1))    //2月
                {
                    if ((0 == ((timeinfo->tm_year + 1900) % 400)) || ((0 == ((timeinfo->tm_year + 1900) % 4)) && (0 != ((timeinfo->tm_year + 1900) % 100))))
                    {
                        if (timeinfo->tm_mday > 29)    //闰年2月份有29天
                        {
                            free(timeinfo);
                            return false;
                        }
                    }
                    else    //平年2月份有28天
                    {
                        if (timeinfo->tm_mday > 28)
                        {
                            free(timeinfo);
                            return false;
                        }
                    }
                }
            }
            time = mktime(timeinfo);
            if(time==-1){
                free(timeinfo);
                return false;
            }
            free(timeinfo);
            return true;
        }
};