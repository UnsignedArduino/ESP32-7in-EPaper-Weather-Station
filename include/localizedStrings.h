// #define FULL_NAMES

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
    "",     "January", "February",  "March",   "April",    "May",     "June",
    "July", "August",  "September", "October", "November", "December"};
#else
const char* MONTH_NAMES_EN[] = {"",    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
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

// CHINESE
const char* WEEKDAY_NAMES_CN[] = {"",       "星期一", "星期二", "星期三",
                                  "星期四", "星期五", "星期六", "星期日"};
#ifdef FULL_NAMES
const char* MONTH_NAMES_CN[] = {"",     "一月",   "二月",  "三月", "四月",
                                "五月", "六月",   "七月",  "八月", "九月",
                                "十月", "十一月", "十二月"};
#else
const char* MONTH_NAMES_CN[] = {"",     "1月",  "2月", "3月", "4月",
                                "5月",  "6月",  "7月", "8月", "9月",
                                "10月", "11月", "12月"};
#endif
#ifdef FULL_NAMES
const char* DAY_NAMES_CN[] = {
    "",         "一日",     "二日",     "三日",     "四日",     "五日",
    "六日",     "七日",     "八日",     "九日",     "十日",     "十一日",
    "十二日",   "十三日",   "十四日",   "十五日",   "十六日",   "十七日",
    "十八日",   "十九日",   "二十日",   "二十一日", "二十二日", "二十三日",
    "二十四日", "二十五日", "二十六日", "二十七日", "二十八日", "二十九日",
    "三十日",   "三十一日"};
#else
const char* DAY_NAMES_CN[] = {
    "",     "1日",  "2日",  "3日",  "4日",  "5日",  "6日",  "7日",
    "8日",  "9日",  "10日", "11日", "12日", "13日", "14日", "15日",
    "16日", "17日", "18日", "19日", "20日", "21日", "22日", "23日",
    "24日", "25日", "26日", "27日", "28日", "29日", "30日", "31日"};
#endif
const char* WEATHERS_CN[] = { // Google Translate sad
    "晴朗",   "基本晴朗", "部分晴朗", "阴天",     "雾",     "霜",     "小雨",
    "中雨",   "密集小雨", "小冻雨",   "密集冻雨", "小雨",   "中雨",   "大雨",
    "小冻雨", "大雪",     "中雪",     "大雪",     "雪粒",   "小阵雨", "中雨",
    "大雨",   "小雪",     "大雪",     "雷暴",     "小冰雹", "大冰雹", "未知"};
