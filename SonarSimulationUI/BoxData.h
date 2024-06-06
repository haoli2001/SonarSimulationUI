#ifndef BOXDATA_H
#define BOXDATA_H
#include <fstream>
#include <vector>
#include <string>
#include <stdio.h>
#include "commonstruct.h"



class BoxData
{
public:
	BoxData():boxnum(0),treerootnode(NULL),m_isLoaded(false){};
	BoxData(std::string filepath){ alanlyze(filepath); };
	~BoxData(){};
	const std::vector<float>& getData(){ return data; };
	const std::vector<float>& getLeafData(){ return leafdata; };
    unsigned int getBoxnum(){ return data.size()/(24*3); };
    unsigned int getLeafBoxnum(){ return leafdata.size() / (24 * 3); };
	void load(void* tree, int num);
	void clear();

	bool isLoaded(){ return m_isLoaded; };
	

private:
	void alanlyze(std::string filepath);

	void pushdata(float x, float y, float z, float a, float b, float c);

	void pushleaf(float x, float y, float z, float a, float b, float c);

	void CreateData(int num);
 
private:
	std::vector<float> data;
	std::vector<float> leafdata;
	unsigned boxnum;
	KD_Node_V *treerootnode;
	bool m_isLoaded;
};

#endif
