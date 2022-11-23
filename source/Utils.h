#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"
#include <iostream>

#define DISABLE_OBJ

namespace dae
{
	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, bool flipAxisAndWinding = true)
		{
#ifdef DISABLE_OBJ

			//TODO: Enable the code below after uncommenting all the vertex attributes of DataTypes::Vertex
			// >> Comment/Remove '#define DISABLE_OBJ'
			assert(false && "OBJ PARSER not enabled! Check the comments in Utils::ParseOBJ");

#else

			std::ifstream file(filename);
			if (!file)
				return false;

			std::vector<Vector3> positions{};
			std::vector<Vector3> normals{};
			std::vector<Vector2> UVs{};

			vertices.clear();
			indices.clear();

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;

					positions.emplace_back(x, y, z);
				}
				else if (sCommand == "vt")
				{
					// Vertex TexCoord
					float u, v;
					file >> u >> v;
					UVs.emplace_back(u, 1 - v);
				}
				else if (sCommand == "vn")
				{
					// Vertex Normal
					float x, y, z;
					file >> x >> y >> z;

					normals.emplace_back(x, y, z);
				}
				else if (sCommand == "f")
				{
					//if a face is read:
					//construct the 3 vertices, add them to the vertex array
					//add three indices to the index array
					//add the material index as attibute to the attribute array
					//
					// Faces or triangles
					Vertex vertex{};
					size_t iPosition, iTexCoord, iNormal;

					uint32_t tempIndices[3];
					for (size_t iFace = 0; iFace < 3; iFace++)
					{
						// OBJ format uses 1-based arrays
						file >> iPosition;
						vertex.position = positions[iPosition - 1];

						if ('/' == file.peek())//is next in buffer ==  '/' ?
						{
							file.ignore();//read and ignore one element ('/')

							if ('/' != file.peek())
							{
								// Optional texture coordinate
								file >> iTexCoord;
								vertex.uv = UVs[iTexCoord - 1];
							}

							if ('/' == file.peek())
							{
								file.ignore();

								// Optional vertex normal
								file >> iNormal;
								vertex.normal = normals[iNormal - 1];
							}
						}

						vertices.push_back(vertex);
						tempIndices[iFace] = uint32_t(vertices.size()) - 1;
						//indices.push_back(uint32_t(vertices.size()) - 1);
					}

					indices.push_back(tempIndices[0]);
					if (flipAxisAndWinding) 
					{
						indices.push_back(tempIndices[2]);
						indices.push_back(tempIndices[1]);
					}
					else
					{
						indices.push_back(tempIndices[1]);
						indices.push_back(tempIndices[2]);
					}
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');
			}

			//Cheap Tangent Calculations
			for (uint32_t i = 0; i < indices.size(); i += 3)
			{
				uint32_t index0 = indices[i];
				uint32_t index1 = indices[size_t(i) + 1];
				uint32_t index2 = indices[size_t(i) + 2];

				const Vector3& p0 = vertices[index0].position;
				const Vector3& p1 = vertices[index1].position;
				const Vector3& p2 = vertices[index2].position;
				const Vector2& uv0 = vertices[index0].uv;
				const Vector2& uv1 = vertices[index1].uv;
				const Vector2& uv2 = vertices[index2].uv;

				const Vector3 edge0 = p1 - p0;
				const Vector3 edge1 = p2 - p0;
				const Vector2 diffX = Vector2(uv1.x - uv0.x, uv2.x - uv0.x);
				const Vector2 diffY = Vector2(uv1.y - uv0.y, uv2.y - uv0.y);
				float r = 1.f / Vector2::Cross(diffX, diffY);

				Vector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
				vertices[index0].tangent += tangent;
				vertices[index1].tangent += tangent;
				vertices[index2].tangent += tangent;
			}

			//Fix the tangents per vertex now because we accumulated
			for (auto& v : vertices)
			{
				v.tangent = Vector3::Reject(v.tangent, v.normal).Normalized();

				if(flipAxisAndWinding)
				{
					v.position.z *= -1.f;
					v.normal.z *= -1.f;
					v.tangent.z *= -1.f;
				}

			}

			return true;
#endif
		}
#pragma warning(pop)
		void Bresenham(const Vector2& p1, const Vector2& p2, const ColorRGB& color1, const ColorRGB& color2, std::vector<Vector2>& outputY, std::vector<std::vector<dae::ColorRGB>>& outputColors)
		{

			ColorRGB c1 = color1;
			ColorRGB c2 = color2;

			int x1 = static_cast<int>(p1.x);
			int x2 = static_cast<int>(p2.x);
			int y1 = static_cast<int>(p1.y);
			int y2 = static_cast<int>(p2.y);

			int dx = abs(x2 - x1);
			int dy = abs(y2 - y1);
			bool test{};

			if (!(dx > dy))
			{

				x1 = static_cast<int>(p1.y);
				x2 = static_cast<int>(p2.y);
				y1 = static_cast<int>(p1.x);
				y2 = static_cast<int>(p2.x);

				dx = abs(x2 - x1);
				dy = abs(y2 - y1);

				test = true;
			}

			float dFull{sqrt(float(dx * dx + dy * dy))};

			int pk = 2 * dy - dx;
			for (int i = 0; i <= dx; i++)
			{
				ColorRGB pixColor{ (((dFull - i) / dFull) * c1) + (((i) / dFull) * c2) };
				if (test)
				{
					if (!(x1 < 0 || x1 > outputY.size() - 1))
					{
						if (outputY[x1].x == 0)
							outputY[x1].x = 1000;
						if (!outputColors[x1].size())
							outputColors[x1].resize(2);

						if (std::min(int(outputY[x1].x), y1) == y1)
						{
							outputY[x1].x = (float)y1;
							outputColors[x1][0] = pixColor;
						}
						else if (std::max(int(outputY[x1].y), y1) == y1)
						{
							outputY[x1].y = (float)y1;
							outputColors[x1][1] = pixColor;
						}
					}
				}
				else
				{
					if (!(y1 < 0 || y1 > outputY.size() - 1))
					{
						if (outputY[y1].x == 0)
							outputY[y1].x = 1000;
						if (!outputColors[y1].size())
							outputColors[y1].resize(2);

						if (std::min(int(outputY[y1].x), x1) == x1)
						{
							outputY[y1].x = (float)x1;
							outputColors[y1][0] = pixColor;
						}
						else if (std::max(int(outputY[y1].y), x1) == x1)
						{
							outputY[y1].y = (float)x1;
							outputColors[y1][1] = pixColor;
						}
					}
				}

				x1 < x2 ? x1++ : x1--;
				if (pk < 0)
				{
					pk = pk + 2 * dy;
				}
				else
				{
					y1 < y2 ? y1++ : y1--;
					pk = pk + 2 * dy - 2 * dx;
				}
			}
		}
	}
}