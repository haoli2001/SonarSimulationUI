#ifndef FLUENTDATA_H
#define FLUENTDATA_H

#include <fstream>
#include <string>
#include <vector>

#include "commonstruct.h"

#define TYPE float



class FluentData
{
public:
	FluentData() :facenum(0), faces(), nodenum(0), nodes(), maxX(-999999999), maxY(-99999999), maxZ(-99999999), minX(99999999), minY(99999999), minZ(999999999), maxXX(-999999999), maxYY(-99999999), minXX(99999999), minYY(99999999), m_isLoaded(false), wave_isLoaded(false) { moduleFilePath.clear(); };
	FluentData(std::string filepath) { analyze(filepath); moduleFilePath = filepath; };
	~FluentData(){};

	void load(std::string filepath){ analyze(filepath); moduleFilePath = filepath;};//nodenum = 0; facenum = 0; nodes.clear(); faces.clear(); 
	void clear();
	bool isLoaded(){ return m_isLoaded; };

	const std::vector<TYPE>& getNodev(){ return nodes; };
	const std::vector<unsigned int>& getFacev(){ return faces; };
	unsigned int getNodeNum(){ return nodenum; };
	unsigned int getFaceNum(){ return facenum; };
	TYPE getMaxX(){ return maxX; };
	TYPE getMaxY(){ return maxY; };
	TYPE getMaxZ(){ return maxZ; };
	TYPE getMinX(){ return minX; };
	TYPE getMinY(){ return minY; };
	TYPE getMinZ(){ return minZ; };
	
	TYPE getMaxScale(){ float a = maxX - minX > maxY - minY ? maxX - minX : maxY - minY; return a > maxZ - minZ ? a : maxZ - minZ; };
	TYPE getMinScale(){ float a = maxX - minX < maxY - minY ? maxX - minX : maxY - minY; return a < maxZ - minZ ? a : maxZ - minZ; };

	std::string getModuleFilePath() { return moduleFilePath; };

	const std::vector<Element>& getNodesWK(){ return wkElement; };
	const std::vector<Triangle> & getFacesWK(){ return wkFaces; };

private:
	unsigned int facenum;
	std::vector<unsigned int> faces;
	unsigned int nodenum;
	std::vector<TYPE> nodes;

	std::vector<Element> wkElement;
	std::vector<Triangle> wkFaces;

	void analyze(std::string filepath);
	void analyze_wave(std::string filepath);

	TYPE maxX;
	TYPE maxY;
	TYPE maxZ;
	TYPE minX;
	TYPE minY;
	TYPE minZ;

	bool m_isLoaded;
	std::string moduleFilePath;

//²¨ÐÎ
public:
	void loadwave(std::string filepath){ analyze_wave(filepath); wave_isLoaded = true; };//nodenum = 0; facenum = 0; nodes.clear(); faces.clear(); 
	void waveclear();
	bool isWaveLoaded(){ return wave_isLoaded; };
	const std::vector<double>& getXX(){ return XX; };
	const std::vector<double>& getYY(){ return YY; };
	double getMaxXX(){ return maxXX; };
	double getMaxYY(){ return maxYY; };
	double getMinXX(){ return minXX; };
	double getMinYY(){ return minYY; };

private:
	std::vector<double> XX;
	std::vector<double> YY;
	double maxXX;
	double minXX;
	double maxYY;
	double minYY;
	bool wave_isLoaded;

};
#endif
