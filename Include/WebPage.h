#ifndef __WebPage_H__
#define __WebPage_H__

#include "Configuration.h"
#include "RssReader.h"
// #include "SplitTool.h"
#include "SplitTool2.h"
#include <vector>
#include <string>
#include <map>

using std::vector;
using std::string;
using std::map;
class WebPage
{
public:
    WebPage()
    :_doc("")
    ,_docId(0)
    ,_docTitle("")
    ,_docUrl("")
    ,_docDescription("")
    ,_docContent("")
    ,_docSummary("")
    {

    }
    WebPage(const WebPage&) =default;
    WebPage(const RssItem&);
    WebPage(const string& doc);


    int getDocId()const{return _docId;}
    string getDocTitle()const{return _docTitle;}
    string getDocLink() const { return _docUrl; }
    string getDocDescription() const { return _docDescription; }
    string getDocContent() const { return _docContent; }
    string getDocSummary() const { return _docSummary; }
    string getDoc() const { return _doc; }
    const map<string, int>& getWordsMap()const{return _wordsMap;}

    void setDocId(int id) { _docId = id; }
    void setDoc();
    void setDoc(const string& doc);
    void setSummary();
    void setSummary(const string& doc);
    void setContent(const string& content);

    //生成停用词库
    void calWordsFreq(SplitTool& cuttor, const unordered_set<string> &stopWords); 
private:
    string _doc;//整篇文章

    int _docId;

    string _docTitle;//标题
    string _docUrl;//网址
    string _docDescription;//描述
    string _docContent;//内容
    string _docSummary;// 摘要
    map<string, int> _wordsMap;
};





#endif
