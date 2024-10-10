#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <algorithm>
#include <cmath>
using namespace std;

vector<string> TEXT_FILE_PATH;
// #define FADE_RATE  0.1
// #define NATION_WEIGHT 0.5
// #define STATE_WEIGHT 0.5
// #define SENATE_WEIGHT 0.5
// #define HOUSE_WEIGHT 0.5
// #define LEGISLATURE_WEIGHT 0.1
// #define HISTORY_WEIGHT 0.2
// #define THIS_YEAR 2024  // 今年是哪年
// #define MAX_DAYS 200    // 最大统计天数
// #define START_MONTH 7   // 基准月份
// #define START_DAY 15    // 基准日期 2024-07-15对应0
// #define SMOOTH_STRENGTH 0.1 //最终胜率平滑强度
// #define WIN_RATE_VOLATILITY 1.0 // 胜率波动率
// #define EXPOSURE_WEIGHT 1.0
// #define SMOOTH_THRESHOLD_1 0.55
// #define SMOOTH_THRESHOLD_2 0.6
// #define MAX_NEWS 200000 //最多有多少篇新闻
// #define MAX_EXPOSURE_SOURCEFILE 161
// const string TARGET_STATE = "MI"; // 现在要计算的州名
// const string SOURCE_DIR = ".\\0930csv\\";
// const string OUTPUT_DIR = ".\\0930csv\\";
// const string EXPOSURE_SOURCE_DIR = "C:\\Users\\NM-on\\OneDrive\\2024election\\0929exposure\\";
const int MAX_NEWS = 200000;
const int MAX_DAYS = 200;
const string CONFIG_FILE = "config.txt";

float FADE_RATE = 0.1;
float NATION_WEIGHT = 0.5;
float STATE_WEIGHT = 0.5;
float SENATE_WEIGHT = 0.5;
float HOUSE_WEIGHT = 0.5;
float LEGISLATURE_WEIGHT = 0.1;
float HISTORY_WEIGHT = 0.2;
int THIS_YEAR = 2024;
int START_MONTH = 7;
int START_DAY = 15;
float SMOOTH_STRENGTH = 0.1;
float WIN_RATE_VOLATILITY = 1.0;
float EXPOSURE_WEIGHT = 1.0;
float SMOOTH_THRESHOLD_1 = 0.55;
float SMOOTH_THRESHOLD_2 = 0.6;
int MAX_EXPOSURE_SOURCEFILE = 161;
string TARGET_STATE = "MI"; // 现在要计算的州名
string SOURCE_DIR = ".\\0930csv\\";
string OUTPUT_DIR = ".\\0930csv\\";
string EFFECTIVE_ARTICLE_FILE = "effectiveArticle.txt";

int effectiveRows;
int allRows;
int redRepeat;
int blueRepeat;
int redNonRepeat;
int blueNonRepeat;

class CsvRow;
vector<CsvRow> sentimentData;   //保存从sentiment读出的源数据


bool effectiveArticle[MAX_NEWS];
float redExposureState[MAX_DAYS]; //红色热度 按天算
float blueExposureState[MAX_DAYS];    //蓝色热度 按天算
float redExposureNation[MAX_DAYS]; //红色热度 按天算
float blueExposureNation[MAX_DAYS];    //蓝色热度 按天算

int dateNewsNumState[MAX_DAYS];
int dateNewsNumNation[MAX_DAYS];
int redDateNewsNumStateNotNeutral[MAX_DAYS];
int redDateNewsNumNationNotNeutral[MAX_DAYS];
int blueDateNewsNumStateNotNeutral[MAX_DAYS];
int blueDateNewsNumNationNotNeutral[MAX_DAYS];

float redFavorState[MAX_DAYS];    //红色好感度 按天算
float redFavorNation[MAX_DAYS];    //红色好感度 按天算
float blueFavorState[MAX_DAYS];   //蓝色好感度 按天算
float blueFavorNation[MAX_DAYS];   //蓝色好感度 按天算

float redScore[MAX_DAYS];    //红色分数 按天算
float redScore_with_exposure[MAX_DAYS];    //红色分数 按天算
float blueScore[MAX_DAYS];   //蓝色分数 按天算
float blueScore_with_exposure[MAX_DAYS];   //蓝色分数 按天算
//map<string, int> sourceNewsNum[MAX_DAYS]; //每天各个媒体的新闻总量，比如sourceNewsNum[30]["fox"]就是第30天fox的新闻总数

float redWinRate[MAX_DAYS];
float redWinRate_with_exposure[MAX_DAYS];
float blueWinRate[MAX_DAYS];
float blueWinRate_with_exposure[MAX_DAYS];

float redWinRate_H[MAX_DAYS];
float redWinRate_H_with_exposure[MAX_DAYS];
float redWinRate_L[MAX_DAYS];
float redWinRate_L_with_exposure[MAX_DAYS];
float blueWinRate_H[MAX_DAYS];
float blueWinRate_H_with_exposure[MAX_DAYS];
float blueWinRate_L[MAX_DAYS];
float blueWinRate_L_with_exposure[MAX_DAYS];

int stateNewsHaveRedSentNum[MAX_NEWS];
int nationNewsHaveRedSentNum[MAX_NEWS];
int stateNewsHaveBlueSentNum[MAX_NEWS];
int nationNewsHaveBlueSentNum[MAX_NEWS];

float forChecking[MAX_DAYS];

int redSenate, blueSenate, redHouse, blueHouse;
float redLegiRatio,blueLegiRatio;



string redStrings[10] = {"Trump", "Presimpt Trump", "Former President Trump", "The Donald", "MAGA leader", "Republican","GOP","Conservatives","The right","Trump supporters"};
string blueStrings[9] = {"Harris", "Vice President Harris", "Former Senator Harris", "first female VP","Democrat", "Dems", "Liberals", "The left", "Progressives"};

std::string trim(const std::string& str) {
    size_t first = 0;
    size_t last = str.size() - 1;
    // 手动找到第一个非空白字符
    while (first < str.size() && (str[first] == ' ' || str[first] == '\t' || str[first] == '\n' || str[first] == '\r')) {
        ++first;
    }
    if (first == str.size()) {
        return ""; // 如果全是空白字符，返回空字符串
    }
    // 手动找到最后一个非空白字符
    while (last > first && (str[last] == ' ' || str[last] == '\t' || str[last] == '\n' || str[last] == '\r')) {
        --last;
    }
    return str.substr(first, last - first + 1);
}

void readConfig(const std::string& filename) {
    std::map<std::string, std::string> configMap;
    std::ifstream configFile(filename);
    
    if (!configFile.is_open()) {
        std::cerr << "无法打开配置文件: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(configFile, line)) {
        // 去除行首和行尾的空白字符
        line = trim(line);

        // 跳过空行或注释行（以#或//开头的行）
        if (line.empty() || line[0] == '#' || line.substr(0, 2) == "//") {
            continue;
        }

        // 检查行内的注释（去掉行尾的注释部分）
        size_t commentPos = line.find('#');
        if (commentPos == std::string::npos) {
            commentPos = line.find("//");
        }
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        std::istringstream lineStream(line);
        std::string key, value;
        
        // 解析出键值对，假设格式是 key=value
        if (std::getline(lineStream, key, '=') && std::getline(lineStream, value)) {
            // 去除键和值的前后空白
            key = trim(key);
            value = trim(value);
            configMap[key] = value;
        }
    }

    configFile.close();
    //return configMap;

    FADE_RATE = stof(configMap["FADE_RATE"]);
    NATION_WEIGHT = stof(configMap["NATION_WEIGHT"]);
    STATE_WEIGHT = stof(configMap["STATE_WEIGHT"]);
    SENATE_WEIGHT = stof(configMap["SENATE_WEIGHT"]);
    HOUSE_WEIGHT = stof(configMap["HOUSE_WEIGHT"]);
    LEGISLATURE_WEIGHT = stof(configMap["LEGISLATURE_WEIGHT"]);
    HISTORY_WEIGHT = stof(configMap["HISTORY_WEIGHT"]);
    THIS_YEAR = stoi(configMap["THIS_YEAR"]);
    START_MONTH = stoi(configMap["START_MONTH"]);
    START_DAY = stoi(configMap["START_DAY"]);
    SMOOTH_STRENGTH = stof(configMap["SMOOTH_STRENGTH"]);
    WIN_RATE_VOLATILITY = stof(configMap["WIN_RATE_VOLATILITY"]);
    EXPOSURE_WEIGHT = stof(configMap["EXPOSURE_WEIGHT"]);
    SMOOTH_THRESHOLD_1 = stof(configMap["SMOOTH_THRESHOLD_1"]);
    SMOOTH_THRESHOLD_2 = stof(configMap["SMOOTH_THRESHOLD_2"]);
    MAX_EXPOSURE_SOURCEFILE = stoi(configMap["MAX_EXPOSURE_SOURCEFILE"]);
    TARGET_STATE = configMap["TARGET_STATE"];
    EFFECTIVE_ARTICLE_FILE = configMap["EFFECTIVE_ARTICLE_FILE"];
    OUTPUT_DIR = configMap["OUTPUT_DIR"];
    SOURCE_DIR = configMap["SOURCE_DIR"];
}

void readEffectiveArticles() {
    std::ifstream file(EFFECTIVE_ARTICLE_FILE);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string word;
        while (iss >> word) {
            effectiveArticle[stoi(word)] = 1;
        }
    }
}

int Date2Int(const string& date);
int countOccurrences(const string& str, const string& sub) {
    if (sub.empty()) return 0; // 如果子字符串为空，直接返回0

    int count = 0;
    size_t pos = str.find(sub);

    while (pos != string::npos) {
        ++count;
        pos = str.find(sub, pos + sub.length());
    }

    return count;
}

bool inRed(string & s)
{
    for(int i=0;i<10;i++)
    {
        if(s.find(redStrings[i])!=string::npos)
        {
            return 1;
        }
    }
    return 0;
}

bool inBlue(string & s)
{
    for(int i=0;i<9;i++)
    {
        if(s.find(blueStrings[i])!=string::npos)
        {
            return 1;
        }
    }
    return 0;
}

bool GoodSent(string & s)
{
    return inRed(s) ^ inBlue(s);
}

// 定义一个类来表示CSV的一行数据
class CsvRow {
public:
    string state;
    string color;
    string source;
    string media_type;
    string date;
    string sen;
    float negative;
    float neutral;
    float positive;
    int rank;
    int dateInt;
    int keyWordCount = 0;
    int candNameCount = 0;
    int newsId = -1;
    

    // 构造函数接受列名和行数据，然后根据列名将数据解析到相应的成员变量
    CsvRow(const vector<string>& rowValues) {
        if (rowValues.size() != 11 && rowValues.size() != 10) {
            cerr << "ERROR: 10 elements are expected, but " << rowValues.size() << " elements got." << endl;
            // for(int i=0;i<rowValues.size();i++)
            //     cout<<rowValues[i]<<endl;;
            return;
        }
        if(rowValues.size()==11)    //州级别的数据
        {
            state = rowValues[0];
            color = rowValues[1];
            source = rowValues[2];
            media_type = rowValues[3];
            date = rowValues[4];
            sen = rowValues[5];
            negative = stof(rowValues[6]);
            neutral = stof(rowValues[7]);
            positive = stof(rowValues[8]);
            rank = stoi(rowValues[9]);
            newsId = stoi(rowValues[10]);
            dateInt = Date2Int(date);    
        }
        else if(rowValues.size()==10)   //nation级别的数据
        {
            date = rowValues[0];
            color = rowValues[1];
            source = rowValues[2];
            media_type = rowValues[3];
            sen = rowValues[4];
            negative = stof(rowValues[5]);
            neutral = stof(rowValues[6]);
            positive = stof(rowValues[7]);
            rank = stoi(rowValues[8]);
            newsId = stoi(rowValues[9]);
            dateInt = Date2Int(date);  
        }
        else
        {
            cerr << "READING ERROR!" <<endl;
        }
    }
    
    void CalculateKeywordsNum()
    {
        
        for(int i=0;i<10;i++)
        {
            keyWordCount += countOccurrences(sen, redStrings[i]);
        }
        for(int i=0;i<9;i++)
        {
            keyWordCount += countOccurrences(sen, blueStrings[i]);
        }
        if(keyWordCount>1 && color == "Red")
            redRepeat++;
        else if(keyWordCount>1 && color == "Blue")
            blueRepeat++;
        else if(keyWordCount==1 && color == "Red")
            redNonRepeat++;
        else if(keyWordCount==1 && color == "Blue")
            blueNonRepeat++;
        candNameCount += countOccurrences(sen, "Trump");
        candNameCount += countOccurrences(sen, "Harris");
    }

    // 打印该行的数据
    void printRow() const {
        cout << "state: " << state << endl;
        cout << "color: " << color << endl;
        cout << "source: " << source << endl;
        cout << "media_type: " << media_type << endl;
        cout << "date: " << date << endl;
        cout << "sen: " << sen << endl;
        cout << "negative: " << negative << endl;
        cout << "neutral: " << neutral << endl;
        cout << "positive: " << positive << endl;
        cout << "rank: " << rank << endl;
        cout << "---" << endl;
    }

    bool operator<(const CsvRow& other) const {
        return date < other.date;
    }
};

// 读取txt文件，并将每行数据与列名关联
vector<CsvRow> readTxtFile(const string& filePath) {
    ifstream file(filePath);
    string line;
    vector<CsvRow> data;
    vector<string> row;

    if (!file.is_open()) {
        cerr << "无法打开文件: " << filePath << endl;
        return data;
    }

    bool isFirstRow = true;

    while (getline(file, line)) {
        if (line.empty()) {
            if (!row.empty()) {
                if (isFirstRow) {
                    // 第一行是列名，直接跳过
                    isFirstRow = false;
                } else {
                    CsvRow now(row);
                    allRows++;
                    if(effectiveArticle[now.newsId] && GoodSent(now.sen))
                    {
                        effectiveRows++;
                        data.push_back(now);  // 生成 CsvRow 对象
                    }
                }
                row.clear();
            }
        } else {
            row.push_back(line);
        }
    }

    // 处理最后一行（如果文件没有以空行结束）
    if (!row.empty()) {
        CsvRow now(row);
        allRows++;
        if(effectiveArticle[now.newsId] && GoodSent(now.sen))
        {
            effectiveRows++;
            data.push_back(now);  // 生成 CsvRow 对象
        }
    }

    file.close();
    return data;
}

// 将日期转化为第几天，基准日期 2024-07-15，其对应第0天
int Date2Int(const string& date) {
    // 基准日期 2024-07-15
    int baseYear = 2024;
    int baseMonth = START_MONTH;
    int baseDay = START_DAY;
    
    // 解析输入日期
    int year, month, day;
    sscanf(date.c_str(), "%d-%d-%d", &year, &month, &day);

    // 将月份和日期转换为总天数
    static const int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    auto getDaysSinceYearStart = [](int y, int m, int d) {
        int days = d - 1; // 当前月的天数，减去1因为1号是起点
        while (--m > 0) {
            days += daysInMonth[m-1];
        }
        // 考虑闰年
        if (m >= 2 && ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))) {
            days++;
        }
        return days;
    };

    // 计算两日期之间的天数差
    int baseDays = getDaysSinceYearStart(baseYear, baseMonth, baseDay);
    int inputDays = getDaysSinceYearStart(year, month, day);

    // 计算自2024年一月的总天数
    int totalBaseDays = baseDays + 365 * (baseYear - 1); 
    int totalInputDays = inputDays + 365 * (year - 1); 

    return totalInputDays - totalBaseDays;
}

// 反过来将第几天转化为对应日期
string Int2Date(int days) {
    // 基准日期 2024-07-15
    int baseYear = 2024;
    int baseMonth = 7;
    int baseDay = 15;

    // 将月份和日期转换为总天数
    static const int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    
    auto isLeapYear = [](int year) {
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    };
    
    // 计算新的日期
    int year = baseYear;
    int month = baseMonth;
    int day = baseDay;

    // 添加天数
    while (days != 0) {
        int currentMonthDays = daysInMonth[month - 1];
        // 考虑闰年
        if (month == 2 && isLeapYear(year)) {
            currentMonthDays++;
        }
        int remainingDaysThisMonth = currentMonthDays - day;
        
        if (days > remainingDaysThisMonth) {
            // 向下一个月推进
            days -= (remainingDaysThisMonth + 1);
            day = 1;
            if (++month > 12) {
                month = 1;
                year++;
            }
        } else if (days < -day) {
            // 向前一个月推进
            days += day;
            if (--month < 1) {
                month = 12;
                year--;
            }
            day = daysInMonth[month - 1];
            if (month == 2 && isLeapYear(year)) {
                day++;
            }
        } else {
            // 当前月内调整天数
            day += days;
            days = 0;
        }
    }

    // 格式化日期字符串
    ostringstream dateStream;
    dateStream << year << "-" 
               << (month < 10 ? "0" : "") << month << "-"
               << (day < 10 ? "0" : "") << day;
    return dateStream.str();
}





void CalculateExposure()
{
    for(int i=0;i<sentimentData.size();i++) //统计各篇新闻有多少条有效句子
    {
        CsvRow now = sentimentData[i];
        if(now.color=="Red")
        {
            if(now.media_type=="state") stateNewsHaveRedSentNum[now.newsId]++;
            else nationNewsHaveRedSentNum[now.newsId]++;
        }
        else
        {
            if(now.media_type=="state") stateNewsHaveBlueSentNum[now.newsId]++;
            else nationNewsHaveBlueSentNum[now.newsId]++;
        }
    }
    for(int i=0;i<sentimentData.size();i++) //开始算热度
    {
        CsvRow now = sentimentData[i];
        int id = now.newsId;
        int dateInt = sentimentData[i].dateInt;
        int thisNewsSentNum = stateNewsHaveRedSentNum[id] + nationNewsHaveRedSentNum[id] + stateNewsHaveBlueSentNum[id] + nationNewsHaveBlueSentNum[id];
        float todayExposure = 1.0/(float)(thisNewsSentNum);
        if(now.color=="Red")
        {
            if(now.media_type=="state")
            {
                redExposureState[dateInt] += todayExposure;   //当天热度无衰减
                for(int j=1;j<=14;j++)
                    redExposureState[dateInt+j] += todayExposure * exp(-FADE_RATE*j);   //热度逐渐衰减地延伸提供给后14天
            }
            else
            {
                redExposureNation[dateInt] += todayExposure;   //当天热度无衰减
                for(int j=1;j<=14;j++)
                    redExposureNation[dateInt+j] += todayExposure * exp(-FADE_RATE*j);   //热度逐渐衰减地延伸提供给后14天
            }
        }
        else
        {
            if(now.media_type=="state")
            {
                blueExposureState[dateInt] += todayExposure;   //当天热度无衰减
                for(int j=1;j<=14;j++)
                    blueExposureState[dateInt+j] += todayExposure * exp(-FADE_RATE*j);   //热度逐渐衰减地延伸提供给后14天
            }
            else
            {
                blueExposureNation[dateInt] += todayExposure;   //当天热度无衰减
                for(int j=1;j<=14;j++)
                    blueExposureNation[dateInt+j] += todayExposure * exp(-FADE_RATE*j);   //热度逐渐衰减地延伸提供给后14天
            }
        }
    }
    
    
}

//计算每天的总好感度
void CalculateFavor()
{
    for(int i=0;i<sentimentData.size();i++)
    {
        CsvRow now = sentimentData[i];
        if(now.rank==1) continue;
        if(now.color=="Red"&&now.media_type=="state") redDateNewsNumStateNotNeutral[now.dateInt]++;
        if(now.color=="Red"&&now.media_type=="nation") redDateNewsNumNationNotNeutral[now.dateInt]++;
        if(now.color=="Blue"&&now.media_type=="state") blueDateNewsNumStateNotNeutral[now.dateInt]++;
        if(now.color=="Blue"&&now.media_type=="nation") blueDateNewsNumNationNotNeutral[now.dateInt]++;
        
    }
    for(int i=0;i<sentimentData.size();i++)
    {
        CsvRow now = sentimentData[i];
        if(now.rank==1) continue;   //不统计中立数据
        if(now.color=="Red")    //共和党
        {
            if(now.media_type=="state")
            {
                redFavorState[now.dateInt]+=now.positive;
                redFavorState[now.dateInt]-=now.negative;
            }
                
            else
            {
                redFavorNation[now.dateInt]+=now.positive;
                redFavorNation[now.dateInt]-=now.negative;
            }
                
        }
        else    //民主党
        {
            if(now.media_type=="state")
            {
                blueFavorState[now.dateInt]+=now.positive;
                blueFavorState[now.dateInt]-=now.negative;
            }
                
            else
            {
                blueFavorNation[now.dateInt]+=now.positive;
                blueFavorNation[now.dateInt]-=now.negative;
            }
        }
    }
    for(int i=0;i<MAX_DAYS;i++)  //归一化处理，归到(-1,1)，然后线性转换到(0,1)，然后取ln转换到(0,1)
    {
        forChecking[i]=redFavorState[i]/=(float)redDateNewsNumStateNotNeutral[i];
        if(redDateNewsNumStateNotNeutral[i]!=0)
            redFavorState[i]/=(float)redDateNewsNumStateNotNeutral[i]; //归到(-1,1)
        else
            redFavorState[i]=0;
        redFavorState[i] = (redFavorState[i] + 1.0)/2.0;    //线性转换到(0,1)
        redFavorState[i] = log(1.0+redFavorState[i])/log(2.0); //取ln映射到(0,1)
        
        if(redDateNewsNumNationNotNeutral[i]!=0)
            redFavorNation[i]/=(float)redDateNewsNumNationNotNeutral[i];
        else
            redFavorNation[i]=0;
        redFavorNation[i] = (redFavorNation[i] + 1.0)/2.0;    //线性转换到(0,1)
        redFavorNation[i] = log(1.0+redFavorNation[i])/log(2.0); //取ln映射到(0,1)

        if(blueDateNewsNumStateNotNeutral[i]!=0)
            blueFavorState[i]/=(float)blueDateNewsNumStateNotNeutral[i];
        else
            blueFavorState[i] = 0;
        blueFavorState[i] = (blueFavorState[i] + 1.0)/2.0;    //线性转换到(0,1)
        blueFavorState[i] = log(1.0+blueFavorState[i])/log(2.0); //取ln映射到(0,1)

        if(blueDateNewsNumNationNotNeutral[i]!=0)
            blueFavorNation[i]/=(float)blueDateNewsNumNationNotNeutral[i];
        else
            blueFavorNation[i] = 0;
        blueFavorNation[i] = (blueFavorNation[i] + 1.0)/2.0;    //线性转换到(0,1)
        blueFavorNation[i] = log(1.0+blueFavorNation[i])/log(2.0); //取ln映射到(0,1)

    }
    
}

void CalculateScore()
{
    for(int i=0;i<MAX_DAYS;i++)
    {
        redScore_with_exposure[i] += (1.0 + redExposureState[i] * EXPOSURE_WEIGHT) * redFavorState[i] * STATE_WEIGHT + (1.0 + redExposureNation[i] * EXPOSURE_WEIGHT) * redFavorNation[i] * NATION_WEIGHT;
        blueScore_with_exposure[i] += (1.0 + blueExposureState[i] * EXPOSURE_WEIGHT) * blueFavorState[i] * STATE_WEIGHT + (1.0 + blueExposureState[i] * EXPOSURE_WEIGHT) * blueFavorNation[i] * NATION_WEIGHT;
    }
    for(int i=0;i<MAX_DAYS;i++)
    {
        redScore[i] += redFavorState[i] * STATE_WEIGHT + redFavorNation[i] * NATION_WEIGHT;
        blueScore[i] += blueFavorState[i] * STATE_WEIGHT + blueFavorNation[i] * NATION_WEIGHT;
    }
}

void CalculateWinRate()
{
    for(int i=0;i<MAX_DAYS;i++)
    {
        redWinRate[i] = redScore[i]/(redScore[i]+blueScore[i]);
        blueWinRate[i] = 1.0 - redWinRate[i];
        redWinRate_with_exposure[i] = redScore_with_exposure[i]/(redScore_with_exposure[i]+blueScore_with_exposure[i]);
        blueWinRate_with_exposure[i] = 1.0 - redWinRate_with_exposure[i];
    }
}

void CalculateLegislatureEffect()
{
    ifstream senateFile(SOURCE_DIR+"senate.txt");   //读取senate.txt
    string line;
    bool lineFound = false;
    bool DFound = false;
    bool RFound = false;
    if (senateFile.is_open()) {
        while (getline(senateFile, line)) {
            if (line == TARGET_STATE) {
                lineFound = true;
            } else if (lineFound) {
                if (line[0] == 'D'  && (!DFound)) {
                    blueSenate = std::stoi(line.substr(2));
                    DFound = 1;
                } else if (line[0] == 'R' && (!RFound)) {
                    redSenate = std::stoi(line.substr(2));
                    RFound = 1;
                    break;
                }
            }
        }
        senateFile.close();
    } else {
        std::cerr << "Unable to open file";
    }

    ifstream houseFile(SOURCE_DIR+"house.txt"); //读取house.txt
    lineFound = false;
    DFound = false;
    RFound = false;
    if (houseFile.is_open()) {
        while (getline(houseFile, line)) {
            if (line == TARGET_STATE) {
                lineFound = true;
            } else if (lineFound) {
                if (line[0] == 'D' && (!DFound)) {
                    blueHouse = std::stoi(line.substr(2));
                    DFound = 1;
                } else if (line[0] == 'R' && (!RFound)) {
                    redHouse = std::stoi(line.substr(2));
                    RFound = 1;
                    break;
                }
            }
        }
    }

    redLegiRatio += (float)(redHouse)/(float)(redHouse+blueHouse) * HOUSE_WEIGHT;
    redLegiRatio += (float)(redSenate)/(float)(redSenate+blueSenate) * SENATE_WEIGHT;
    blueLegiRatio = 1.0 - redLegiRatio;

    float redEffect = exp((redLegiRatio - 0.5) * LEGISLATURE_WEIGHT);
    float blueEffect = exp((blueLegiRatio - 0.5) * LEGISLATURE_WEIGHT);
    
    for(int i=0;i<MAX_DAYS;i++) //计算最高胜率和最低胜率
    {
        // redScore[i] *= redEffect;
        // blueScore[i] *= blueEffect;
        redWinRate_H[i] = redWinRate[i];
        redWinRate_L[i] = redWinRate[i];
        blueWinRate_H[i] = blueWinRate[i];
        blueWinRate_L[i] = blueWinRate[i];
        
        redWinRate_H_with_exposure[i] = redWinRate_with_exposure[i];
        redWinRate_L_with_exposure[i] = redWinRate_with_exposure[i];
        blueWinRate_H_with_exposure[i] = blueWinRate_with_exposure[i];
        blueWinRate_L_with_exposure[i] = blueWinRate_with_exposure[i];
        
        if(redEffect >= 1.0)
        {
            redWinRate_H[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY;
            if(redWinRate_H[i]>0.55)
                redWinRate_L[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY / 4.0;
            else if(redWinRate_H[i]>0.525)
                redWinRate_L[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY / 3.0;
            else
                redWinRate_L[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY / 2.0;
        }
        else
        {
            redWinRate_L[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY;
            if(redWinRate_L[i]<0.45)
                redWinRate_H[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY / 4.0;
            else if(redWinRate_L[i]<0.475)
                redWinRate_H[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY / 3.0;
            else
                redWinRate_H[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY / 2.0;
        }
        
        blueWinRate_L[i] = 1.0 - redWinRate_H[i];
        blueWinRate_H[i] = 1.0 - redWinRate_L[i];
        
        
        if(redEffect >= 1.0)
        {
            redWinRate_H_with_exposure[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY;
            if(redWinRate_H_with_exposure[i]>0.55)
                redWinRate_L_with_exposure[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY / 4.0;
            else if(redWinRate_H_with_exposure[i]>0.525)
                redWinRate_L_with_exposure[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY / 3.0;
            else
                redWinRate_L_with_exposure[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY / 2.0;
        }
        else
        {
            redWinRate_L_with_exposure[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY;
            if(redWinRate_L_with_exposure[i]<0.45)
                redWinRate_H_with_exposure[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY / 4.0;
            else if(redWinRate_L_with_exposure[i]<0.475)
                redWinRate_H_with_exposure[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY / 3.0;
            else
                redWinRate_H_with_exposure[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY / 2.0;
        }
        
        blueWinRate_L_with_exposure[i] = 1.0 - redWinRate_H_with_exposure[i];
        blueWinRate_H_with_exposure[i] = 1.0 - redWinRate_L_with_exposure[i];
        
    }
}

void CalculateHistoryEffect()
{
    float redEffect = 1.0;
    float blueEffect = 1.0;
    ifstream file(SOURCE_DIR+"history.txt");
    string line;
    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.substr(0, 2) == TARGET_STATE) {
                int year = stoi(line.substr(3, 4));
                if(line[8]=='R')
                    redEffect += exp(-(2024-year)/4) * HISTORY_WEIGHT;
                else
                    blueEffect += exp(-(2024-year)/4) * HISTORY_WEIGHT;
            }
        }
        file.close();
    } else {
        std::cerr << "Unable to open file";
    }
    for(int i=0;i<MAX_DAYS;i++)
    {
        //redScore[i] *= redEffect;
        //blueScore[i] *= blueEffect;
        
        if(redEffect >= 1.0)
        {
            redWinRate_H[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY;
            if(redWinRate_H[i]>0.55)
                redWinRate_L[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY / 4.0;
            else if(redWinRate_H[i]>0.525)
                redWinRate_L[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY / 3.0;
            else
                redWinRate_L[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY / 2.0;
        }
        else
        {
            redWinRate_L[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY;
            if(redWinRate_L[i]<0.45)
                redWinRate_H[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY / 4.0;
            else if(redWinRate_L[i]<0.475)
                redWinRate_H[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY / 3.0;
            else
                redWinRate_H[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY / 2.0;
        }

        blueWinRate_L[i] = 1.0 - redWinRate_H[i];
        blueWinRate_H[i] = 1.0 - redWinRate_L[i];


        
        if(redEffect >= 1.0)
        {
            redWinRate_H_with_exposure[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY;
            if(redWinRate_H_with_exposure[i]>0.55)
                redWinRate_L_with_exposure[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY / 4.0;
            else if(redWinRate_H_with_exposure[i]>0.525)
                redWinRate_L_with_exposure[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY / 3.0;
            else
                redWinRate_L_with_exposure[i] += (redEffect - 1.0) * WIN_RATE_VOLATILITY / 2.0;
        }
        else
        {
            redWinRate_L_with_exposure[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY;
            if(redWinRate_L_with_exposure[i]<0.45)
                redWinRate_H_with_exposure[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY / 4.0;
            else if(redWinRate_L_with_exposure[i]<0.475)
                redWinRate_H_with_exposure[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY / 3.0;
            else
                redWinRate_H_with_exposure[i] -= (1.0 - redEffect) * WIN_RATE_VOLATILITY / 2.0;
        }

        blueWinRate_L_with_exposure[i] = 1.0 - redWinRate_H_with_exposure[i];
        blueWinRate_H_with_exposure[i] = 1.0 - redWinRate_L_with_exposure[i];
    }
}

void SmoothWinRate()
{
    for(int i=0;i<MAX_DAYS;i++)
    {
        if(redWinRate[i]>=SMOOTH_THRESHOLD_1 && redWinRate[i]<SMOOTH_THRESHOLD_2)
        {
            redWinRate[i] = SMOOTH_THRESHOLD_1 + (redWinRate[i]-SMOOTH_THRESHOLD_1)* SMOOTH_STRENGTH * 2;    //0.65-0.85平滑强度为0.5级
            blueWinRate[i] = 1.0 - redWinRate[i];
        }
        else if(redWinRate[i]>=SMOOTH_THRESHOLD_2)
        {
            redWinRate[i] = SMOOTH_THRESHOLD_1 + 0.2 * SMOOTH_STRENGTH * 2 + (redWinRate[i]-SMOOTH_THRESHOLD_2)* SMOOTH_STRENGTH;    //0.65-0.85平滑强度为0.5级，0.85-1.00平滑强度为1级
            blueWinRate[i] = 1.0 - redWinRate[i];
        }
        else if(blueWinRate[i]>=SMOOTH_THRESHOLD_1 && blueWinRate[i]<SMOOTH_THRESHOLD_2)
        {
            blueWinRate[i] = SMOOTH_THRESHOLD_1 + (blueWinRate[i]-SMOOTH_THRESHOLD_1)* SMOOTH_STRENGTH * 2;
            redWinRate[i] = 1.0 - blueWinRate[i];
        }
        else if(blueWinRate[i]>=SMOOTH_THRESHOLD_2)
        {
            blueWinRate[i] = SMOOTH_THRESHOLD_1 + 0.2 * SMOOTH_STRENGTH * 2 + (blueWinRate[i]-SMOOTH_THRESHOLD_2)* SMOOTH_STRENGTH;
            redWinRate[i] = 1.0 - blueWinRate[i];
        }

        
        if(redWinRate_with_exposure[i]>=SMOOTH_THRESHOLD_1 && redWinRate_with_exposure[i]<SMOOTH_THRESHOLD_2)
        {
            redWinRate_with_exposure[i] = SMOOTH_THRESHOLD_1 + (redWinRate_with_exposure[i]-SMOOTH_THRESHOLD_1)* SMOOTH_STRENGTH * 2;    //0.65-0.85平滑强度为0.5级
            blueWinRate_with_exposure[i] = 1.0 - redWinRate_with_exposure[i];
        }
        else if(redWinRate_with_exposure[i]>=SMOOTH_THRESHOLD_2)
        {
            redWinRate_with_exposure[i] = SMOOTH_THRESHOLD_1 + 0.2 * SMOOTH_STRENGTH * 2 + (redWinRate_with_exposure[i]-SMOOTH_THRESHOLD_2)* SMOOTH_STRENGTH;    //0.65-0.85平滑强度为0.5级，0.85-1.00平滑强度为1级
            blueWinRate_with_exposure[i] = 1.0 - redWinRate_with_exposure[i];
        }
        else if(blueWinRate_with_exposure[i]>=SMOOTH_THRESHOLD_1 && blueWinRate_with_exposure[i]<SMOOTH_THRESHOLD_2)
        {
            blueWinRate_with_exposure[i] = SMOOTH_THRESHOLD_1 + (blueWinRate_with_exposure[i]-SMOOTH_THRESHOLD_1)* SMOOTH_STRENGTH * 2;
            redWinRate_with_exposure[i] = 1.0 - blueWinRate_with_exposure[i];
        }
        else if(blueWinRate_with_exposure[i]>=SMOOTH_THRESHOLD_2)
        {
            blueWinRate_with_exposure[i] = SMOOTH_THRESHOLD_1 + 0.2 * SMOOTH_STRENGTH * 2 + (blueWinRate_with_exposure[i]-SMOOTH_THRESHOLD_2)* SMOOTH_STRENGTH;
            redWinRate_with_exposure[i] = 1.0 - blueWinRate_with_exposure[i];
        }
    }
}


void outputExposure()
{
    ofstream outputFileExposure(OUTPUT_DIR+"output_"+TARGET_STATE+"_exposure.csv");
    if(outputFileExposure.is_open())
    {
        outputFileExposure<<"Date,redExposure,blueExposure\n";
        for(int i=0;i<MAX_DAYS;i++)
        {
            outputFileExposure<<Int2Date(i)<<","<<redExposureState[i]*STATE_WEIGHT+redExposureNation[i]*NATION_WEIGHT<<","<<blueExposureState[i]*STATE_WEIGHT+blueExposureNation[i]*NATION_WEIGHT<<"\n";
        }
    }
}






int main() {
    readConfig(CONFIG_FILE);
    readEffectiveArticles();
    
    for(int i=1;i<=9;i++)
        TEXT_FILE_PATH.push_back(SOURCE_DIR+"res_2024-08-0"+to_string(i)+".txt");
    for(int i=10;i<=31;i++)
        TEXT_FILE_PATH.push_back(SOURCE_DIR+"res_2024-08-"+to_string(i)+".txt");
    for(int i=1;i<=9;i++)
        TEXT_FILE_PATH.push_back(SOURCE_DIR+"res_2024-09-0"+to_string(i)+".txt");
    for(int i=11;i<=19;i++)
        TEXT_FILE_PATH.push_back(SOURCE_DIR+"res_2024-09-"+to_string(i)+".txt");
    TEXT_FILE_PATH.push_back(SOURCE_DIR+"res_"+TARGET_STATE+".txt");
    
    for(int i=0;i<TEXT_FILE_PATH.size();i++)
    {
        vector<CsvRow> now = readTxtFile(TEXT_FILE_PATH[i]);
        sentimentData.insert(sentimentData.end(), now.begin(), now.end());
    }
    cout<<TARGET_STATE<<":"<<endl;
    cout<<"redRepeat:"<<redRepeat<<endl;
    cout<<"blueRepeat:"<<blueRepeat<<endl;
    cout<<"redNonRepeat:"<<redNonRepeat<<endl;
    cout<<"blueNonRepeat:"<<blueNonRepeat<<endl;

    CalculateExposure();
    outputExposure();
    CalculateFavor();
    CalculateScore();
    CalculateWinRate();
    SmoothWinRate();
    CalculateLegislatureEffect();
    CalculateHistoryEffect();
    SmoothWinRate();
    
    
    cout<<"effectiveRows:"<<effectiveRows<<endl;
    cout<<"allRows:"<<allRows<<endl;
    
    ofstream outputFile(OUTPUT_DIR+"output_"+TARGET_STATE+"_without_exposure.csv");
    if(outputFile.is_open())
    {
        outputFile<<"Date,redWinRate,blueWinRate\n";
        for(int i=0;i<MAX_DAYS;i++)
        {
            outputFile<<Int2Date(i)<<","<<redWinRate[i]<<","<<blueWinRate[i]<<"\n";
        }
    }
    
    ofstream outputFile_with_exposure_with_HL(OUTPUT_DIR+"output_"+TARGET_STATE+"__with_exposure_with_HL.csv");
    if(outputFile_with_exposure_with_HL.is_open())
    {
        outputFile_with_exposure_with_HL<<"Date,redWinRateH,redWinRateL,blueWinRateH,blueWinRateL\n";
        for(int i=0;i<MAX_DAYS;i++)
        {
            outputFile_with_exposure_with_HL<<Int2Date(i)<<","<<redWinRate_H_with_exposure[i]<<","<<redWinRate_L_with_exposure[i]<<","<<blueWinRate_H_with_exposure[i]<<","<<blueWinRate_L_with_exposure[i]<<"\n";
        }
    }
    
    ofstream outputFileWithHL(OUTPUT_DIR+"output_"+TARGET_STATE+"_without_exposure_with_HL.csv");
    if(outputFileWithHL.is_open())
    {
        outputFileWithHL<<"Date,redWinRateH,redWinRateL,blueWinRateH,blueWinRateL\n";
        for(int i=0;i<MAX_DAYS;i++)
        {
            outputFileWithHL<<Int2Date(i)<<","<<redWinRate_H[i]<<","<<redWinRate_L[i]<<","<<blueWinRate_H[i]<<","<<blueWinRate_L[i]<<"\n";
        }
    }

    

    // ofstream outputFileKeywordsNum(OUTPUT_DIR+"output_"+TARGET_STATE+"_keywordsNum.csv");
    // if(outputFileKeywordsNum.is_open())
    // {
    //     outputFileKeywordsNum<<"Date,redKeywordsNum,blueKeywordsNum\n";
    //     for(int i=0;i<MAX_DAYS;i++)
    //     {
    //         int redKeywordsNum=0;
    //         int blueKeywordsNum=0;
    //         for(int j=0;j<sentimentData.size();j++)
    //         {
    //             if(sentimentData[j].color=="Red" && sentimentData[j].dateInt==i) redKeywordsNum+=sentimentData[j].keyWordCount;
    //             else if(sentimentData[j].dateInt==i) blueKeywordsNum+=sentimentData[j].keyWordCount;
    //         }
    //         outputFileKeywordsNum<<Int2Date(i)<<","<<redKeywordsNum<<","<<blueKeywordsNum<<"\n";
    //     }
    // }

    ofstream outputFileCandNameNum(OUTPUT_DIR+"output_"+TARGET_STATE+"_candNameNum.csv");
    if(outputFileCandNameNum.is_open())
    {
        outputFileCandNameNum<<"Date,redCandNameNum,blueCandNameNum\n";
        for(int i=0;i<MAX_DAYS;i++)
        {
            int redCandNameNum=0;
            int blueCandNameNum=0;
            for(int j=0;j<sentimentData.size();j++)
            {
                if(sentimentData[j].color=="Red" && sentimentData[j].dateInt==i) redCandNameNum+=sentimentData[j].candNameCount;
                else if(sentimentData[j].dateInt==i) blueCandNameNum+=sentimentData[j].candNameCount;
            }
            outputFileCandNameNum<<Int2Date(i)<<","<<redCandNameNum<<","<<blueCandNameNum<<"\n";
        }
    }



    for(int i=0;i<70;i++)
    {
        //cout<<Int2Date(i)<<": ";
        
        //cout<<"redFavorState:"<<redFavorState[i]<<"  blueFavorState:"<<blueFavorState[i]<<endl;
        //cout<<"redScore:"<<redScore[i]<<"  blueScore:"<<blueScore[i]<<endl;
        // cout<<"forChecking:"<<forChecking[i]<<" ";
        //cout<<"redWinRate:"<<redWinRate[i]<<"  blueWinRate:"<<blueWinRate[i]<<endl;
        //cout<<"redExposureState:"<<redExposureState[i]<<"  blueExposureState:"<<blueExposureState[i]<<endl;
        // cout<<"redNewsNumStateNotNeutral:"<<redDateNewsNumStateNotNeutral[i]<<"  blueNewsNumStateNotNeutral:"<<blueDateNewsNumStateNotNeutral[i]<<endl;
    }

    return 0;
}
