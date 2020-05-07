#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include <core/vec.h>
#include <core/mat.h>

#include <GL/gl3w.h>            // Initialize with gl3wInit()
#include <GLFW/glfw3.h>

class Mesh
{
	struct Vertex {
		// position
		vecf3 Position;
		// Parameter coordinates
		vecf3 ParaCoord;
		// normal
		vecf3 Normal;
		// texCoord
		vecf2 TexCoord;
		// TBN
		matf3 TBN;
	};

	struct TextureInfo {
		unsigned id;
		std::string type;
		std::string path;
	};

public:
	Mesh();
	Mesh(std::string path);	// ��ʼ����ʱ�����Ĭ�ϲ�����������������ϵ�µ�ŷʽ����
	~Mesh();

	void LoadObj(std::string path);
	void Transform(vecf3 center, matf3 S, matf3 R);	//�任�������ParaCoord��

	void LoadMesh();				// ��OpenGL�м�����������
	//void ParaReset();
	void Draw(GLuint programID);	// ����ɫ������

private:
	std::vector<std::string> SplitString(const std::string& s, const std::string& spliter);

public:
	/*  Mesh Data  */
	std::vector<Vertex> vertices;	//��
	std::vector<unsigned int> indices;	//��
	std::vector<TextureInfo> textureInfos;	//��ͼ

	/*  Render data  */
	unsigned int VAO = 0;
	unsigned int VBO = 0, EBO = 0;

private:
	std::vector<vecf3> positions;
	std::vector<vecf3> normals;
	std::vector<vecf2> texcoords;
};

inline Mesh::Mesh()
{
}

inline Mesh::Mesh(std::string path)
{
	LoadObj(path);
	LoadMesh();
}

inline Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

inline void Mesh::LoadObj(std::string path)
{
	std::ifstream objfile(path);
	if (!objfile) {
		std::cout << "ERROR::Mesh::LoadObj:" << std::endl
			<< "\t" << "open file (" << path << ") fail" << std::endl;
	}

	while (!objfile.eof())
	{
		std::string line;
		std::getline(objfile, line);
		if (line.empty() || line == "\n")
			continue;

		std::stringstream ss(line);
		std::string type;
		ss >> type;
		switch (type[0])
		{
		case 'v': {
			if (type.size() > 1)
			{
				switch (type[1])
				{
				case 'n': {
					vecf3 pos;
					ss >> pos[0] >> pos[1] >> pos[2];
					normals.push_back(pos);
				}
						break;
				case 't': {
					vecf2 pos;
					ss >> pos[0] >> pos[1];
					texcoords.push_back(pos);
				}
						break;
				default:
					break;
				}
			}
			else
			{
				vecf3 pos;
				ss >> pos[0];
				ss >> pos[1];
				ss >> pos[2];
				positions.push_back(pos);
			}
		}
				break;
		case 'f': {
			for (int i = 0; i < 3; i++)
			{
				std::string temp;
				ss >> temp;
				auto splitresult = SplitString(temp, "/");
				indices.push_back(std::stoi(splitresult[0]) - 1);	// EBO�еĶ���������0��ʼ
			}
		}
				break;
		default:
			break;
		}
	}

	for (int i = 0; i < positions.size(); i++)
	{
		Vertex v;
		v.Position = positions[i];
		v.ParaCoord = positions[i];				// ������������ϵ��ŷ�������ʼ����������
		vertices.push_back(v);
	}
	if (normals.size() == vertices.size())
	{
		for (int i = 0; i < vertices.size(); i++)
		{
			vertices[i].Normal = normals[i];
		}
	}
	if (texcoords.size() == vertices.size())
	{
		for (int i = 0; i < vertices.size(); i++)
		{
			vertices[i].TexCoord = texcoords[i];
		}
	}
}

inline void Mesh::Transform(vecf3 center, matf3 S, matf3 R)
{
	for (int i = 0; i < vertices.size(); i++)
	{
		vertices[i].ParaCoord = matf3::dot(S, R).dot(vertices[i].Position) + center;
	}
}

inline std::vector<std::string> Mesh::SplitString(const std::string& s, const std::string& spliter)
{
	std::string::size_type pos1, pos2;
	std::vector<std::string> v;
	pos2 = s.find(spliter);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + spliter.size();
		pos2 = s.find(spliter, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
	return v;
}

inline void Mesh::LoadMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);

	// vecf3��������������ƺ���һ��ָ�룬��̫����ֱ�ӽ�vertices���鴫��buffer����
	std::vector<float> vertice_data;
	for (size_t i = 0; i < vertices.size(); i++)
	{
		// TODO:���Ҫ��������Ϣ�����������ӣ�ע�⻹Ҫ�޸ĺ����glVertexAttribPointer
		for (size_t j = 0; j < 3; j++)
			vertice_data.push_back(vertices[i].ParaCoord[j]);
	}

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertice_data.size() * sizeof(float), &vertice_data[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// ����Ҫ�����ݴ���OpenGL������׷�ӣ�
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,									// 0->paracoord,��������ɫ����������ȫ�ֲ�������
		3,									// 3��float�ĳ���
		GL_FLOAT,
		GL_FALSE,
		3 * sizeof(float),					// ����
		(void*)0	// ��ʼλ��
	);

	// Unavailable
	/*glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,					// 1->normal
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, Normal)
	);*/

	/*glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,					// 2->texcoord
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, TexCoord)
	);*/
	glBindVertexArray(0);
}

inline void Mesh::Draw(GLuint programID)
{
	// TODO:����������ͼ
	// TODO:�����������


	// draw
	glUseProgram(programID);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}