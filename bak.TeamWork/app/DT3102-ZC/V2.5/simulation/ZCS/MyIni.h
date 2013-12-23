#ifndef MYINI_H
#define MYINI_H

#include <string>
#include <vector>
#include <map>
using namespace std;
class MyIni
{
public:
	MyIni(void);
	MyIni(char *_iniFilePath);
	~MyIni(void);

	//获取所有的section名
	void getAllSections(vector<string>& vecSections);
	//通过section名，该section下所有的key-value
	void getKeyValueBySection(const char* sectionName,map<string,string>& kv);
	
	//判断是否存在指定的section
	bool isSectionExists(const char* sectionName);
	//判断是否存在指定的key
	bool isKeyExists(const char* sectionName,const char* keyName);

	//获取section下某个key的值
	string getValueString(const char* sectionName,const char* keyName);
	//获取section下某个key的值，如果该section或key不存在，返回false
	bool getValueInt(const char* sectionName,const char* keyName,int &result);
	bool getValueFloat(const char* sectionName,const char* keyName,float &result);
	//写入字符值
	void writeKeyValue(const char* sectionName,const char* keyName,const char* value);
	//void writeFloatValue(const char* sectionName,const char* keyName,const float value);
	//删除section
	void deleteSection(const char* sectionName);
	//删除section中的某个键
	void deleteKey(const char* sectionName,const char* keyName);

	//获取section的个数
	int getSectionCount();
	//获取该section下key-value的个数
	int getKeyValueCount(const char* sectionName);

private:
	//保存ini文件的绝对路径
	char iniFilePath[260];
	//申请缓冲区的默认长度
	static int _bufLen;
	//private 供getValueString和getKeyValueBySection调用
	void myGetPrivateProfileString(const char*sectionName,const char*keyName,char **buf,int bufLen);
	//private //private 供getSectionCount和getAllSections调用
	void myGetPrivateProfileSectionNames(char** buf,int bufLen);
};

#else
#endif
