#include "../../Include/WebPageSearcher.h"
#include "../../lib/nlohmann/json.hpp"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <utility>

using namespace nlohmann;

    


WebPageSearcher::WebPageSearcher()
: _cuttor() 
{
    cout << "init WebPageSearcher" << endl;
    loadStopWords();
    loadOffsetLib();
    loadInvertIndexLib();
}

void WebPageSearcher::loadStopWords()
{
    cout << "getStopWords" << endl;
    const string enStopWordsPath = GetPath("EN_STOP");
    const string cnStopWordsPath = GetPath("CN_STOP");
    ifstream enIfs(enStopWordsPath, ifstream::in);
    ifstream cnIfs(cnStopWordsPath, ifstream::in);

    if (!enIfs.good()) {
        perror("getStopWords ifs");
        return;
    }

    if (!cnIfs.good()) {
        perror("getStopWords ifs");
        return;
    }

    string lines, word;

    while (getline(enIfs, lines)) {
        istringstream str(lines);
        str >> word;
        _stopWords.insert(word);
    }

    while (getline(cnIfs, lines)) {
        istringstream str(lines);
        str >> word;
        _stopWords.insert(word);
    }

    enIfs.close();
    cnIfs.close();
}

//加载OffsetLib的同时顺便加载网页库
void WebPageSearcher::loadOffsetLib() {
    cout << "loadOffsetLib" << endl;
    const string offsetPath = GetPath("WEB_OFFSET");
    const string ripePagesPath = GetPath("WEB_RIPEPAGE");
    ifstream offsetIfs(offsetPath, ifstream::in);
    ifstream ripePagesIfs(ripePagesPath, ifstream::in);

    if (!offsetIfs) {
        perror("loadOffsetLib failed!");
        return;
    }

    if (!ripePagesIfs) {
        perror("loadRipePages failed!");
        return;
    }
    cout << "打开" << endl;

    string lines;
    int docId, docOffset, docLength;
    char docBuf[65535] = {0};

    while (getline(offsetIfs, lines)) {
        bzero(docBuf, 65535);
        istringstream str(lines);
        str >> docId >> docOffset >> docLength;
        _offsetLib[docId] = make_pair(docOffset, docLength);
        ripePagesIfs.read(docBuf, docLength);
        _ripePagesLib.push_back(WebPage(string(docBuf)));
    }

    offsetIfs.close();
    ripePagesIfs.close();
}

void WebPageSearcher::loadInvertIndexLib() {
    cout << "loadInvertIndexLib" << endl;
    const string invertIndexPath = GetPath("WEB_INVIDX");
    ifstream invertIndexIfs(invertIndexPath, ifstream::in);

    if (!invertIndexIfs) {
        perror("loadInvertIndexLib failed!");
        return;
    }

    string lines, word;
    int docId;
    double weight;

    while (getline(invertIndexIfs, lines)) {
        istringstream str(lines);
        str >> word;

        while (str >> docId >> weight) {
            _invertIndexLib[word].insert(make_pair(docId, weight));
        }
    }

    invertIndexIfs.close();
}


string WebPageSearcher::serializationForNone()const
{
    json myJ;
    myJ["msgID"] = 404;
    myJ["msg"] = "未找到相关内容";
    return myJ.dump(4);
}
string WebPageSearcher::serialization(const vector<int>& sortedIds) const {
    json myJ;
    int pageNum = stoi(GetPath("PAGENUM"));
    myJ["msgID"] = 200;
    int count = 0;

    vector<map<string, string>> msg;

    for (auto& id : sortedIds) {
        if (++count > pageNum)
            break;
        auto curPage = _ripePagesLib[id];
        /* WebPage& curPage = _ripePagesLib[id]; */
        // cout << curPage.getDocContent() << endl;
        map<string, string> page;
        page["title"] = curPage.getDocTitle();
        page["url"] = curPage.getDocLink();
        page["summary"] = curPage.getDocSummary();
        cout << id << ": " << curPage.getDocSummary() << endl;
        msg.push_back(page);
    }

    myJ["msg"] = msg;
    return myJ.dump(4);
}

string WebPageSearcher::doQuery(const string& msg)
{
    cout << "doQuery" << endl;
    WebPage msgPage;//无参构造
    msgPage.setContent(msg);
    msgPage.calWordsFreq(_cuttor, _stopWords);
    auto msgVec = getVector(msgPage);
    for(auto& elem : msgVec)
    {
        cout << elem.first << endl;
    }
    string response;
    set<int> ids = getPageIds(msgPage);//获取关键词出现过的文章号

    if(ids.empty())//如果关键词没有在文章中出现过，可以直接返回结果
    {
        response = serializationForNone();
    }
    else
    {
        vector<int> sortedIds = getSortedIds(msgVec, ids);
        setSummarys(sortedIds, msgPage);
        response = serialization(sortedIds);
    }
    return response;
}

//获取每个关键词的权重
unordered_map<string, double> WebPageSearcher::getVector(const WebPage& page)
{
    unordered_map<string ,double> vec;
    auto& wordsMap = page.getWordsMap();//map<string,int>关键词，频率
    int WordNumInpage = 0;

    //计算总词数
    for(auto& wordPair : wordsMap)
    {
        WordNumInpage += wordPair.second;    
    }

    double sumWeight = 0.0;

    for(auto& wordPair : wordsMap)
    {
        double TF = static_cast<double>(wordPair.second) / WordNumInpage;//该词本doc中的频率
        //该词在多少篇文章中出现过的计数，+1是防止 出现0的情况，保证代码的健壮性
        int DF =_invertIndexLib[wordPair.first].size() + 1;
        //网页库文章数量计数，相当于将网页库计数    +1
        int N = _ripePagesLib.size()+1;
        double  IDF = 0.0;
        if(N != DF)
        {
            IDF = log2(static_cast<double>(N) / (DF+1));
        }
        double w = TF * IDF;
        sumWeight += w*w;
        vec.insert(make_pair(wordPair.first, w));
    }
    
    for(auto& wordPair :vec)
    {
        if(sumWeight == 0)
        {
            wordPair.second = 0;
        }

        wordPair.second /= sqrt(sumWeight);
    }
    return vec;
}


set<int> WebPageSearcher::getPageIds(const WebPage& page)
{
    auto& wordsMap = page.getWordsMap();
    vector<set<int>> idsAddr;

    for(auto& wordPair : wordsMap)
    {
        set<int> ids;
        for(auto& invertInvertPair : _invertIndexLib[wordPair.first])
        {
            ids.insert(invertInvertPair.first);
        }
        idsAddr.push_back(std::move(ids));//整个set移动
    }

    set<int> ids;

    for(auto& elem : idsAddr)
    {
        if(ids.empty())
        {
            ids = elem;
        }
        else
        { 
            set<int> temSet;
            std::set_intersection(elem.begin(), elem.end()
                                    ,ids.begin(), ids.end()
                                    ,std::inserter(temSet, temSet.begin()));
            swap(ids, temSet);
        }
    }
    return ids;
}
vector<int> WebPageSearcher::getSortedIds(const unordered_map<string, double>& vecX, const set<int>& ids) {
    std::multimap<double, int, greater<double>> sortCos;
    //遍历每篇相关的doc
    for (auto& id : ids) {
        unordered_map<string, double> vecY;
        //找到出现过的关键字在该篇doc中的权重，计为加入向量y的map
        for (auto& wordPair : vecX) {
            double weight = _invertIndexLib[wordPair.first][id];
            vecY.insert(make_pair(wordPair.first, weight));
        }

        double innerProduct = 0.0, xProduct = 0.0, yProduct = 0.0;
        //对每个关键词的权重
        for (auto& wordPair : vecX) {
            innerProduct += wordPair.second * vecY[wordPair.first];
            xProduct += wordPair.second * wordPair.second;
            yProduct += vecY[wordPair.first] * vecY[wordPair.first];
        }

        double cosValue = innerProduct / (sqrt(xProduct) * sqrt(yProduct));
        //加入比较队列，夹角越小，相似度越高
        sortCos.insert(make_pair(cosValue, id));
    }

    vector<int> result;

    for (auto& cosPair : sortCos) {
        result.push_back(cosPair.second);
    }

    return result;
}
//生成摘要
void WebPageSearcher::setSummarys(vector<int>& sortedIds, WebPage& page) {
    const size_t STEP = 40;
    auto& wordsMap = page.getWordsMap();

    for (auto& id : sortedIds) {
        const string title = _ripePagesLib[id].getDocTitle();
        const string content = _ripePagesLib[id].getDocContent();
        size_t first_pos = SIZE_MAX;

        bool isFoundInTitle = false;
        bool isFoundInContent = false;

        for (auto& wordPair : wordsMap) {  //在标题中找
            size_t pos = title.find(wordPair.first);
            if (pos != title.npos && pos < first_pos) {
                isFoundInTitle = true;
                break;
            }
        }

        for (auto& wordPair : wordsMap) {  //找这篇文章中最早出现相关词的位置
            size_t pos = content.find(wordPair.first);

            if (pos != content.npos && pos < first_pos) {
                first_pos = pos;
                isFoundInContent = true;
            }
        }

        if (first_pos == SIZE_MAX) {  //没能找到
            auto endIt = remove(sortedIds.begin(), sortedIds.end(), id);
            sortedIds.erase(endIt, sortedIds.end());
            /* _ripePagesLib[id].setSummary(); */
            continue;
        }

        if (!isFoundInContent && isFoundInTitle) {  //在内容中未发现，在标题中发现
            _ripePagesLib[id].setSummary();//摘要设置为默认摘要
            continue;
        } else if (!isFoundInTitle && !isFoundInContent) {  //在内容和标题中均未发现,删除这个ID
            auto endIt = remove(sortedIds.begin(), sortedIds.end(), id);
            sortedIds.erase(endIt, sortedIds.end());
            continue;
        }
        //否则修改摘要为找到的关键词的，就是标题和内容中都发现了，加上40个字符，从最早发现的词开始
        size_t first_to_end = content.substr(first_pos).size();
        size_t right_pos = first_pos + countBytes(content.substr(first_pos), first_to_end, STEP);

        vector<size_t> ppc = getPosPerCharactor(content, first_pos);
        size_t left_pos = ppc.size() >= STEP ? ppc[ppc.size() - STEP] : 0;

        // size_t left_pos = first_pos;

        string summary = "";

        if (left_pos != 0)
            summary += "... ";

        summary += content.substr(left_pos, right_pos - left_pos);

        if (right_pos < content.size())
            summary += " ...";

        _ripePagesLib[id].setSummary(summary);
    }
}