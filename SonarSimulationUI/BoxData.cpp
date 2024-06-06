#include "BoxData.h"

void BoxData::load(void* tree, int num)
{
	KD_Node_V *tree2 = (KD_Node_V*)tree;
	//for (int i = 0; i < num; i++)
	//{
	//	tree2[i].LeftIndex == -1 ? tree2[i].LeftChild = nullptr : tree2[i].LeftChild = &tree2[tree2[i].LeftIndex];
	//	tree2[i].RightIndex == -1 ? tree2[i].RightChild = nullptr : tree2[i].RightChild = &tree2[tree2[i].RightIndex];
	//	tree2[i].RopeIndex[0] == -1 ? tree2[i].rope[0] = nullptr : tree2[i].rope[0] = &tree2[tree2[i].RopeIndex[0]];
	//	tree2[i].RopeIndex[1] == -1 ? tree2[i].rope[1] = nullptr : tree2[i].rope[1] = &tree2[tree2[i].RopeIndex[1]];
	//	tree2[i].RopeIndex[2] == -1 ? tree2[i].rope[2] = nullptr : tree2[i].rope[2] = &tree2[tree2[i].RopeIndex[2]];
	//	tree2[i].RopeIndex[3] == -1 ? tree2[i].rope[3] = nullptr : tree2[i].rope[3] = &tree2[tree2[i].RopeIndex[3]];
	//	tree2[i].RopeIndex[4] == -1 ? tree2[i].rope[4] = nullptr : tree2[i].rope[4] = &tree2[tree2[i].RopeIndex[4]];
	//	tree2[i].RopeIndex[5] == -1 ? tree2[i].rope[5] = nullptr : tree2[i].rope[5] = &tree2[tree2[i].RopeIndex[5]];
	//}


	treerootnode = tree2;

	CreateData(num);

	m_isLoaded = true;
}

void BoxData::alanlyze(std::string filepath)
{
	std::fstream file(filepath);
	float x1, y1, z1, x2, y2, z2;
	while (file >> x1 >> y1 >> z1 >> x2 >> y2 >> z2)
	{
		pushdata(x1, y1, z1, x1, y2, z1);
		pushdata(x1, y1, z1, x2, y1, z1);
		pushdata(x1, y2, z1, x2, y2, z1);
		pushdata(x2, y1, z1, x2, y2, z1);

		pushdata(x1, y1, z1, x1, y1, z2);
		pushdata(x2, y1, z1, x2, y1, z2);
		pushdata(x2, y2, z1, x2, y2, z2);
		pushdata(x1, y2, z1, x1, y2, z2);

		pushdata(x1, y1, z2, x2, y1, z2);
		pushdata(x1, y2, z2, x2, y2, z2);
		pushdata(x2, y1, z2, x2, y2, z2);
		pushdata(x1, y1, z2, x1, y2, z2);



	}
	boxnum = data.size() / (24 * 3);
};

void BoxData::pushdata(float x, float y, float z, float a, float b, float c)
{
	data.push_back(x);
	data.push_back(y);
	data.push_back(z);
	data.push_back(a);
	data.push_back(b);
	data.push_back(c);
}

void BoxData::pushleaf(float x, float y, float z, float a, float b, float c)
{
	leafdata.push_back(x);
	leafdata.push_back(y);
	leafdata.push_back(z);
	leafdata.push_back(a);
	leafdata.push_back(b);
	leafdata.push_back(c);
}

void BoxData::CreateData(int num)
{
	float x1, y1, z1, x2, y2, z2;
	for (int i = 0; i < num; i++)
	{
		x1 = treerootnode[i].box.bmin[0];
		y1 = treerootnode[i].box.bmin[1];
		z1 = treerootnode[i].box.bmin[2];
		x2 = treerootnode[i].box.bmax[0];
		y2 = treerootnode[i].box.bmax[1];
		z2 = treerootnode[i].box.bmax[2];
		pushdata(x1, y1, z1, x1, y2, z1);
		pushdata(x1, y1, z1, x2, y1, z1);
		pushdata(x1, y2, z1, x2, y2, z1);
		pushdata(x2, y1, z1, x2, y2, z1);

		pushdata(x1, y1, z1, x1, y1, z2);
		pushdata(x2, y1, z1, x2, y1, z2);
		pushdata(x2, y2, z1, x2, y2, z2);
		pushdata(x1, y2, z1, x1, y2, z2);

		pushdata(x1, y1, z2, x2, y1, z2);
		pushdata(x1, y2, z2, x2, y2, z2);
		pushdata(x2, y1, z2, x2, y2, z2);
		pushdata(x1, y1, z2, x1, y2, z2);

		if (treerootnode[i].IsLeaf)
		{
			pushleaf(x1, y1, z1, x1, y2, z1);
			pushleaf(x1, y1, z1, x2, y1, z1);
			pushleaf(x1, y2, z1, x2, y2, z1);
			pushleaf(x2, y1, z1, x2, y2, z1);

			pushleaf(x1, y1, z1, x1, y1, z2);
			pushleaf(x2, y1, z1, x2, y1, z2);
			pushleaf(x2, y2, z1, x2, y2, z2);
			pushleaf(x1, y2, z1, x1, y2, z2);

			pushleaf(x1, y1, z2, x2, y1, z2);
			pushleaf(x1, y2, z2, x2, y2, z2);
			pushleaf(x2, y1, z2, x2, y2, z2);
			pushleaf(x1, y1, z2, x1, y2, z2);
		}

	}
}

void BoxData::clear()
{
	data.clear();
	leafdata.clear();
	m_isLoaded = false;
	boxnum = 0;
}
