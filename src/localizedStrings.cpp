#include "localizedStrings.h"

// ENGLISH
#ifdef FULL_NAMES
const char* WEEKDAY_NAMES_EN[] = {"",        "Sunday",    "Monday",
                                  "Tuesday", "Wednesday", "Thursday",
                                  "Friday",  "Saturday"};
#else
const char* WEEKDAY_NAMES_EN[] = {"",    "Sun", "Mon", "Tue",
                                  "Wed", "Thu", "Fri", "Sat"};
#endif
#ifdef FULL_NAMES
const char* MONTH_NAMES_EN[] = {
  "",         "January ",  "February ", "March ",  "April ",
  "May ",     "June ",     "July ",     "August ", "September ",
  "October ", "November ", "December "};
#else
const char* MONTH_NAMES_EN[] = {"",     "Jan ", "Feb ", "Mar ", "Apr ",
                                "May ", "Jun ", "Jul ", "Aug ", "Sep ",
                                "Oct ", "Nov ", "Dec "};
#endif
const char* DAY_NAMES_EN[] = {"",   "1",  "2",  "3",  "4",  "5",  "6",  "7",
                              "8",  "9",  "10", "11", "12", "13", "14", "15",
                              "16", "17", "18", "19", "20", "21", "22", "23",
                              "24", "25", "26", "27", "28", "29", "30", "31"};
const char* WEATHERS_EN[] = {"Clear sky",
                             "Mainly clear",
                             "Partly clear",
                             "Overcast",
                             "Fog",
                             "Frost",
                             "Light drizzle",
                             "Moderate drizzle",
                             "Dense drizzle",
                             "Light freezing drizzle",
                             "Dense freezing drizzle",
                             "Slight rain",
                             "Moderate rain",
                             "Heavy rain",
                             "Light freezing rain",
                             "Heavy freezing rain",
                             "Slight snow fall",
                             "Moderate snow fall",
                             "Heavy snow fall",
                             "Snow grains",
                             "Slight rain showers",
                             "Moderate rain showers",
                             "Violent rain showers",
                             "Slight snow showers",
                             "Heavy snow showers",
                             "Thunderstorms",
                             "Slight hailing thunderstorms",
                             "Heavy hailing thunderstorms",
                             "Unknown"};

// SIMPLIFIED CHINESE
const char* WEEKDAY_NAMES_CN_SIMP[] = {"",       "星期日", "星期一", "星期二",
                                       "星期三", "星期四", "星期五", "星期六"};
#ifdef FULL_NAMES
const char* MONTH_NAMES_CN_SIMP[] = {"",     "一月",   "二月",  "三月", "四月",
                                     "五月", "六月",   "七月",  "八月", "九月",
                                     "十月", "十一月", "十二月"};
#else
const char* MONTH_NAMES_CN_SIMP[] = {"",     "1月",  "2月", "3月", "4月",
                                     "5月",  "6月",  "7月", "8月", "9月",
                                     "10月", "11月", "12月"};
#endif
#ifdef FULL_NAMES
const char* DAY_NAMES_CN_SIMP[] = {
  "",         "一日",     "二日",     "三日",     "四日",     "五日",
  "六日",     "七日",     "八日",     "九日",     "十日",     "十一日",
  "十二日",   "十三日",   "十四日",   "十五日",   "十六日",   "十七日",
  "十八日",   "十九日",   "二十日",   "二十一日", "二十二日", "二十三日",
  "二十四日", "二十五日", "二十六日", "二十七日", "二十八日", "二十九日",
  "三十日",   "三十一日"};
#else
const char* DAY_NAMES_CN_SIMP[] = {
  "",     "1日",  "2日",  "3日",  "4日",  "5日",  "6日",  "7日",
  "8日",  "9日",  "10日", "11日", "12日", "13日", "14日", "15日",
  "16日", "17日", "18日", "19日", "20日", "21日", "22日", "23日",
  "24日", "25日", "26日", "27日", "28日", "29日", "30日", "31日"};
#endif
const char* WEATHERS_CN_SIMP[] = {
  "晴朗",         // Clear sky
  "大部份天晴",   // Mainly clear
  "部份天晴",     // Partly clear
  "多云",         // Overcast
  "雾",           // Fog
  "霜",           // Frost
  "毛毛细雨",     // Light drizzle
  "细雨",         // Moderate drizzle
  "中细雨",       // Dense drizzle
  "冰冻毛毛细雨", // Light freezing drizzle
  "冰冻细雨",     // Dense freezing drizzle
  "小雨",         // Slight rain
  "中雨",         // Moderate rain
  "大雨",         // Heavy rain
  "冰冻小雨",     // Light freezing rain
  "冰冻大雨",     // Heavy freezing rain
  "小雪",         // Slight snow fall
  "中雪",         // Moderate snow fall
  "大雪",         // Heavy snow fall
  "雪粒",         // Snow grains
  "小雨",         // Slight rain showers
  "中雨",         // Moderate rain showers
  "大雨",         // Violent rain showers
  "细雨雪",       // Slight snow showers
  "大雨雪",       // Heavy snow showers
  "雷雨",         // Thunderstorms
  "细冰雹",       // Slight hailing thunderstorms
  "大冰雹",       // Heavy hailing thunderstorms
  "未知"          // Unknown
};

// TRADITIONAL CHINESE
const char** WEEKDAY_NAMES_CN_TRAD = WEEKDAY_NAMES_CN_SIMP;
#ifdef FULL_NAMES
const char** MONTH_NAMES_CN_TRAD = WEEKDAY_NAMES_CN_SIMP;
#else
const char** MONTH_NAMES_CN_TRAD = MONTH_NAMES_CN_SIMP;
#endif
#ifdef FULL_NAMES
const char** DAY_NAMES_CN_TRAD = MONTH_NAMES_CN_SIMP;
#else
const char** DAY_NAMES_CN_TRAD = DAY_NAMES_CN_SIMP;
#endif
const char* WEATHERS_CN_TRAD[] = {
  "晴朗",         // Clear sky
  "大部份天晴",   // Mainly clear
  "部份天晴",     // Partly clear
  "多雲",         // Overcast
  "霧",           // Fog
  "霜",           // Frost
  "毛毛细雨",     // Light drizzle
  "细雨",         // Moderate drizzle
  "中细雨",       // Dense drizzle
  "冰冻毛毛细雨", // Light freezing drizzle
  "冰冻细雨",     // Dense freezing drizzle
  "小雨",         // Slight rain
  "中雨",         // Moderate rain
  "大雨",         // Heavy rain
  "冰冻小雨",     // Light freezing rain
  "冰冻大雨",     // Heavy freezing rain
  "小雪",         // Slight snow fall
  "中雪",         // Moderate snow fall
  "大雪",         // Heavy snow fall
  "雪粒",         // Snow grains
  "小雨",         // Slight rain showers
  "中雨",         // Moderate rain showers
  "大雨",         // Violent rain showers
  "细雨雪",       // Slight snow showers
  "大雨雪",       // Heavy snow showers
  "雷雨",         // Thunderstorms
  "细冰雹",       // Slight hailing thunderstorms
  "大冰雹",       // Heavy hailing thunderstorms
  "未知"          // Unknown
};
