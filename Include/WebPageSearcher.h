#ifndef __WebPageSearcher_H__
#define __WebPageSearcher_H__

#include "WebPage.h"
#include "MultiBytesCharactorFunc.h"
#include <string>

using std::string;

class WebPageSearcher {
public:
    WebPageSearcher();
    string doQuery(const string& key);

private:
    void loadStopWords();
    //void loadRipePages();
    void loadOffsetLib();
    void loadInvertIndexLib();

    string serializationForNone() const;
    string serialization(const vector<int>&) const;
    unordered_map<string, double> getVector(const WebPage&);

    set<int> getPageIds(const WebPage&);
    vector<int> getSortedIds(const unordered_map<string, double>&, const set<int>&);
    void setSummarys(vector<int>&, WebPage&);

private:
    vector<WebPage> _ripePagesLib; 
    unordered_map<int, pair<int, int>> _offsetLib;
    unordered_map<string, unordered_map<int, double>> _invertIndexLib;//倒排索引库

    CppJieba _cuttor;
    unordered_set<string> _stopWords;
};





#endif
