
#include "FluentData.h"
#include <iostream>
#include <fstream>
#include "ErrorLog.h"

using namespace std;

void FluentData::analyze(string filepath)
{
	//日志类对象
	ErrorLog log;

	const char * filepath_c = filepath.c_str();

	string filetype = filepath.substr(filepath.length() - 3, 3);

	if (!filetype.compare("msh"))
	{


#define NULL 0
#define INNODE1  1
#define INNODE2  2
#define INNODE3  3
#define INFACE1  4
#define INFACE2  5
#define INFACE3  6
		unsigned int bufnodenum = 0;
		unsigned int buffacenum = 0;

		int flag = NULL;
		char c;
		int leftkh = 0;
		int rightkh = 0;
		fstream inputfile(filepath.data());
		if (!inputfile.is_open())
		{
			log.ERROR_log("模型文件打开失败，请检查模型地址");
			return;
		}

		while (inputfile >> c)
		{
			if (c == '(')
			{
				++leftkh;
			}
			else if (c == ')')
			{
				++rightkh;
				if (rightkh == leftkh)
				{
					leftkh = rightkh = 0;

				}
				continue;
			}
			else
				continue;

			if (leftkh == 1)
			{
				int id;
				inputfile >> dec >> id;
				if (id == 0)
				{
					continue;
				}
				else if (id == 10)
				{
					flag = INNODE1;
					continue;
				}
				else if (id == 13)
				{
					flag = INFACE1;
					continue;
				}
			}
			else if (leftkh == 2)
			{
				if (flag == INNODE1)
				{
					flag = INNODE2;
					int id;
					unsigned int first_index, last_index;
					int type, ND;
					inputfile >> dec >> id;
					inputfile >> hex >> first_index >> last_index >> type >> ND;
					if (id == 0)
					{
						nodenum = last_index - first_index + 1;
						continue;
					}
					else
					{
						bufnodenum = last_index - first_index + 1;
						continue;
					}
				}
				else if (flag == INFACE1)
				{
					flag = INFACE2;
					int id;
					unsigned int first_index, last_index;
					int bc_type, face_type;
					inputfile >> dec >> id;
					inputfile >> hex >> first_index >> last_index >> bc_type >> face_type;
					if (id == 0)
					{
						facenum = last_index - first_index + 1;
						continue;
					}
					else
					{
						buffacenum = last_index - first_index + 1;
						continue;
					}

				}
			}
			else if (leftkh == 3)
			{
				if (flag == INNODE2)
				{
					flag = INNODE3;
					for (unsigned int i = 0; i < bufnodenum; i++)
					{
						TYPE x, y, z;
						inputfile >> x >> y >> z;
						if (x < minX)
						{
							minX = x;
						}
						else if (x > maxX)
						{
							maxX = x;
						}
						if (y < minY)
						{
							minY = y;
						}
						else if (y > maxY)
						{
							maxY = y;
						}
						if (z < minZ)
						{
							minZ = z;
						}
						else if (z > maxZ)
						{
							maxZ = z;
						}
						nodes.push_back(x);
						nodes.push_back(y);
						nodes.push_back(z);

						//王康数据
						Element bufElement;
						bufElement.point[0] = x;
						bufElement.point[1] = y;
						bufElement.point[2] = z;
						bufElement.PointsIndex = i;
						wkElement.push_back(bufElement);
					}
					continue;
				}
				if (flag == INFACE2)
				{
					flag = INFACE3;
					for (unsigned int i = 0; i < buffacenum; i++)
					{
						unsigned int x, y, z, a, b;
						inputfile >> hex >> x >> y >> z >> a >> b;
						faces.push_back(x - 1);
						faces.push_back(y - 1);
						faces.push_back(z - 1);

						//王康数据
						Triangle bufFaces;
						bufFaces.Points[0] = x - 1;
						bufFaces.Points[1] = y - 1;
						bufFaces.Points[2] = z - 1;
						bufFaces.TriangleIndex = i;
						wkFaces.push_back(bufFaces);
					}
					continue;
				}

			}
		}
		m_isLoaded = true;
	}

	else if (!filetype.compare("dat"))
	{
		FILE *fp = fopen(filepath.c_str(), "r");
		if (fp == NULL)
		{
			log.ERROR_log("打开模型文件失败，请检查模型地址");
			return;
		}
		fscanf(fp, "%d", &nodenum);
		float pointx;
		float pointy;
		float pointz;
		int index;

		int num = nodenum;

		while (num)
		{
			fscanf(fp, "%d", &index);
			fscanf(fp, "%f", &pointx);
			fscanf(fp, "%f", &pointy);//20210905姬梓遇，对换y,z坐标   20210922,又改回来了
			fscanf(fp, "%f", &pointz);//20210905姬梓遇，对换y,z坐标
			pointx = (pointx);
			pointy = pointy;
			pointz = pointz;
			if (pointx < minX)
			{
				minX = pointx;
			}
			else if (pointx > maxX)
			{
				maxX = pointx;
			}
			if (pointy < minY)
			{
				minY = pointy;
			}
			else if (pointy > maxY)
			{
				maxY = pointy;
			}
			if (pointz < minZ)
			{
				minZ = pointz;
			}
			else if (pointz > maxZ)
			{
				maxZ = pointz;
			}
			Element bufElement;
			bufElement.point[0] = pointx;
			bufElement.point[1] = pointy;
			bufElement.point[2] = pointz;
			bufElement.PointsIndex = index - 1;
			wkElement.push_back(bufElement);

			nodes.push_back(pointx);
			nodes.push_back(pointy);
			nodes.push_back(pointz);

			num--;
		}

		fscanf(fp, "%d", &facenum);

		num = facenum;
		int faceOne;
		int faceTwo;
		int faceThree;
		while (num)
		{
			fscanf(fp, "%d", &index);
			fscanf(fp, "%d", &faceOne);
			fscanf(fp, "%d", &faceTwo);
			fscanf(fp, "%d", &faceThree);
			Triangle bufFaces;
			bufFaces.Points[0] = faceOne - 1;
			bufFaces.Points[1] = faceTwo - 1;
			bufFaces.Points[2] = faceThree - 1;
			bufFaces.TriangleIndex = index - 1;
			wkFaces.push_back(bufFaces);

			faces.push_back(faceOne - 1);
			faces.push_back(faceTwo - 1);
			faces.push_back(faceThree - 1);
			num--;
		}
		fclose(fp);
		m_isLoaded = true;
	}
	else
	{
		log.ERROR_log("打开模型文件失败，请检查模型地址");
		return;
	}

}

void FluentData::clear()
{
	minX = 999999999;
	minY = 999999999;
	minZ = 999999999;
	maxX = -999999999;
	maxY = -999999999;
	maxZ = -999999999;
	faces.clear();
	nodes.clear();
	wkElement.clear();
	wkFaces.clear();

	m_isLoaded = false;
}

void FluentData::analyze_wave(string filepath)
{
	FILE *fp = fopen(filepath.c_str(), "r");
	float x, y;

	int num = 48000;

	for (int i = 0; i < num; i++)
	{
		fscanf(fp, "%f", &x);
		fscanf(fp, "%f", &y);
		x = (x);
		y = (y);

		XX.push_back(x);

		if (_isnan(y))
		{
			y = 0;
		}
		YY.push_back(y);

		if (x > maxXX)
		{
			maxXX = x;
		}
		else if (x < minXX)
		{
			minXX = x;
		}
		if (y > maxYY)
		{
			maxYY = y;
		}
		else if (y < minYY)
		{
			minYY = y;
		}
	}
	fclose(fp);
}

void FluentData::waveclear()
{
	XX.clear();
	YY.clear();
	minX = 999999999;
	minY = 999999999;
	maxX = -999999999;
	maxY = -999999999;

	wave_isLoaded = false;
}