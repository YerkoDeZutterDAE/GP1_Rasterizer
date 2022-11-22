//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"

#include <iostream>

using namespace dae;

//#define mainRender
//#define BresenhamActive
#define TriStrip

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-20.f });

	m_pTexture = Texture::LoadFromFile("Resources/uv_grid_2.png");
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	delete m_pTexture;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);



#ifdef mainRender

	//Define Tri - Vertices in NDC space
	std::vector<Vertex> vertices_ndc
	{
		{{0.f, .5f, 1.f}, colors::White},
		{{.5f, -.5f, 1.f}, colors::White},
		{{ - .5f, -.5f, 1.f}, colors::White}
	};

	std::vector<Vertex> worldVertices{ vertices_ndc };
	std::vector<Vertex> vieuwVertices{};

	VertexTransformationFunction(worldVertices, vieuwVertices);

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			//Vector2 screenPix{px,py};

			float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			ColorRGB finalColor{ gradient, gradient, gradient };

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

#else
	Render_W1_Part1(); //Rasterizer Stage Only
	//Render_W1_Part2(); //Projection Stage (Camera)
	//Render_W1_Part3(); //BarCentric Coordimates
	//Render_W1_Part4(); //Depth Buffer
	//Render_W1_Part5(); //Boundingbox Optimization

#endif // mainRender

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	//Todo > W1 Projection Stage

	//float perspectiveDevide{};

	for (auto v : vertices_in)
	{
		Vertex viewVertex{};
		Vertex projectedVertex{};

		v.position = m_Camera.GetLookatVector(v.position);

		projectedVertex.position.x = v.position.x / v.position.z;
		projectedVertex.position.y = v.position.y / v.position.z;
		projectedVertex.position.z = v.position.z;

		float aspec{ (float(m_Width) / float(m_Height)) };

		projectedVertex.position.x = projectedVertex.position.x / (aspec * m_Camera.fov);
		projectedVertex.position.y = projectedVertex.position.y / m_Camera.fov;

		//-----------------------------------------------------------

		projectedVertex.color = v.color;

		projectedVertex.position.x = projectedVertex.position.x * m_Width + m_Width / 2;
		projectedVertex.position.y = projectedVertex.position.y * m_Height + m_Height / 2;

		projectedVertex.uv = v.uv;

		vertices_out.emplace_back(projectedVertex);
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

void dae::Renderer::Render_W1_Part1()
{
	/*{ {0.f, .5f, 1.f}, colors::White},
	{ {.5f, -.5f, 1.f}, colors::White },
	{ { -.5f, -.5f, 1.f}, colors::White }*/
	//Define Tri - Vertices in NDC space
	
	//-----------------

	//std::vector<Vertex> vertices_ndc
	//{
	//	{{0.f, 2.f, 0.f}, colors::Red},
	//	{{1.f, 0.f, 0.f}, colors::Red},
	//	{{ -1.f, 0.f, 0.f}, colors::Red}
	//};

#ifdef TriStrip

	std::vector<Mesh> Meshes = {
		{
			{
				//vertexes
				{
					{-3.f, 3.f, -2.f},
					{1},
					{0.f,0.f}
				},
				{
					{-0.f, 3.f, -2.f},
					{1},
					{0.5f,0.f}
				},
				{
					{3.f, 3.f, -2.f},
					{1},
					{1.f,0.f}
				},
				{
					{-3.f, 0.f, -2.f},
					{1},
					{0.f,0.5f}
				},
				{
					{0.f, 0.f, -2.f},
					{1},
					{0.5f,0.5f}
				},
				{
					{3.f, 0.f, -2.f},
					{1},
					{1.f,0.5f}
				},
				{
					{-3.f, -3.f, -2.f},
					{1},
					{0.f,1.f}
				},
				{
					{0.f, -3.f, -2.f},
					{1},
					{0.5f,1.f}
				},
				{
					{3.f, -3.f, -2.f},
					{1},
					{1.f,1.f}
				},
			},
			{
				3,0,4,1,5,2,
				2,6,
				6,3,7,4,8,5
			},
			PrimitiveTopology::TriangleStrip
		}
	};

#else


	std::vector<Mesh> Meshes = {
		{
			{
				//vertexes
				{
					{-3.f, 3.f, -2.f},
					{1},
					{0.f,0.f}
				},
				{
					{-0.f, 3.f, -2.f},
					{1},
					{0.5f,0.f}
				},
				{
					{3.f, 3.f, -2.f},
					{1},
					{1.f,0.f}
				},
				{
					{-3.f, 0.f, -2.f},
					{1},
					{0.f,0.5f}
				},
				{
					{0.f, 0.f, -2.f},
					{1},
					{0.5f,0.5f}
				},
				{
					{3.f, 0.f, -2.f},
					{1},
					{1.f,0.5f}
				},
				{
					{-3.f, -3.f, -2.f},
					{1},
					{0.f,1.f}
				},
				{
					{0.f, -3.f, -2.f},
					{1},
					{0.5f,1.f}
				},
				{
					{3.f, -3.f, -2.f},
					{1},
					{1.f,1.f}
				},
			},
			{
				3,0,1, 1,4,3, 4,1,2, /**/ 2,5,4, 6,3,4, 4,7,6, /**/ 7,4,5, 5,8,7
			},
			PrimitiveTopology::TriangleStrip
		}
	};


#endif // TriStrip

	std::vector<std::vector<Vertex>> Triangles
	{
		{
			{{0.f, 4.f, -2.f}, colors::Red},
			{{3.f, -2.f, -2.f}, colors::Green},
			{{ -3.f, -2.f, -2.f}, colors::Blue}
		},
		{
			{{0.f, 2.f, 0.f}, colors::Red},
			{{1.5f, -1.f, 0.f}, colors::Red},
			{{ -1.5f, -1.f, 0.f}, colors::Red}
		}
	};

	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	int pixelCount{m_Width*m_Height};
	std::fill_n(m_pDepthBufferPixels, pixelCount, FLT_MAX);

	std::vector<uint32_t> indeces = Meshes[0].indices;
	
	//for (auto& vertices_ndc : Triangles)

	int idx0{}, idx1{}, idx2{};

	//std::vector<Vertex> worldVertices{ Meshes[0].vertices };
	std::vector<Vertex> vieuwVertices0{};

	VertexTransformationFunction(Meshes[0].vertices, vieuwVertices0);
#ifdef TriStrip

	for (int i = 0; i < indeces.size() - 2; i++)
	{

		idx0 = i;

		if (!(i & 1))
		{
			idx1 = i + 1;
			idx2 = i + 2;
		}
		else
		{
			idx1 = i + 2;
			idx2 = i + 1;
		}
	//}

	//for (int idx = 0; idx < indeces.size() - 1; idx += 3)
	//{

#else

	for (int idx = 0; idx < indeces.size() - 1; idx += 3)
	{
		idx0 = i + 0;
		idx1 = i + 1;
		idx2 = i + 2;
#endif // TriStrip
		
		//std::vector<Vertex> vertices_ndc
		//{
		//	{Meshes[0].vertices[indeces[idx0]]},
		//	{Meshes[0].vertices[indeces[idx1]]},
		//	{Meshes[0].vertices[indeces[idx2]]}
		//};

		//std::vector<Vertex> worldVertices{ vertices_ndc };
		//std::vector<Vertex> vieuwVertices{};

		//VertexTransformationFunction(worldVertices, vieuwVertices);

		std::vector<Vertex> vieuwVertices
		{
			{vieuwVertices0[indeces[idx0]]},
			{vieuwVertices0[indeces[idx1]]},
			{vieuwVertices0[indeces[idx2]]}
		};

		std::vector<Vertex> vieuwVertices2
		{
			{vieuwVertices0[indeces[i + 0]]},
			{vieuwVertices0[indeces[i + 1]]},
			{vieuwVertices0[indeces[i + 2]]}
		};

		std::vector<Vector2> tri
		{
		Vector2{vieuwVertices[0].position.x, vieuwVertices[0].position.y},
		Vector2{vieuwVertices[1].position.x, vieuwVertices[1].position.y},
		Vector2{vieuwVertices[2].position.x, vieuwVertices[2].position.y},
		};

#ifdef BresenhamActive


		std::vector<std::vector<Vector2>> edgePixels
		{
			{},
			{},
			{}
		};

		std::vector<int> usedIndex{};

		std::vector<Vector2> yPixels{};

		//std::vector<ColorRGB[2]> edgeColors{};
		//ColorRGB pEdgeColors{};
		std::vector<std::vector<dae::ColorRGB>> edgeColors{};

		yPixels.resize(m_Height);
		edgeColors.resize(m_Height);

		//std::cout << "ok?" << std::endl;

		//std::vector<Vector2> markedPix{};

		Utils::Bresenham(tri[1], tri[0], vertices_ndc[1].color, vertices_ndc[0].color, yPixels, edgeColors);
		Utils::Bresenham(tri[2], tri[1], vertices_ndc[2].color, vertices_ndc[1].color, yPixels, edgeColors);
		Utils::Bresenham(tri[0], tri[2], vertices_ndc[0].color, vertices_ndc[2].color, yPixels, edgeColors);

#else

		std::vector<Vector2> TriToPix
		{
			{},
			{},
			{}
		};

		std::vector<Vector2> Edges
		{
			{},
			{},
			{}
		};

		std::vector<float> triCross
		{
			{},
			{},
			{}
		};

		std::vector<float> weight
		{
			{},
			{},
			{}
		};

		std::vector<float> inerPolat
		{
			{},
			{},
			{}
		};

		std::vector<Vector2> UV
		{
			{},
			{},
			{}
		};



		Edges[0] = { tri[1] - tri[0] };
		Edges[1] = { tri[2] - tri[1] };
		Edges[2] = { tri[0] - tri[2] };


		float triArea{ Vector2::Cross(Edges[0], Edges[1]) };

		Vector2 BoundBoxMin = {Vector2::min(tri[0], Vector2::min(tri[1],tri[2]))};
		Vector2 BoundBoxMax = {Vector2::max(tri[0], Vector2::max(tri[1],tri[2]))};

		if (0 >= BoundBoxMin.x || 0 >= BoundBoxMin.y || m_Width - 1 <= BoundBoxMin.x || m_Height - 1 <= BoundBoxMin.y)
			continue;

#endif // BresenhamActive


		//RENDER LOGIC
		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				Vector2 screenPix{ float(px),float(py) };

				float gradient{};
				//Vector3 point{ m_Camera.forward + m_Camera.origin + screenPix };
				//std::cout << vieuwVertices[0].position.x << " - " << vieuwVertices[1].position.x << " - " << vieuwVertices[2].position.x << std::endl;
#ifdef BresenhamActive

				ColorRGB finalColor{  };

				if (yPixels[py].x == 0)
				{
					continue;
				}

				if (yPixels[py].x <= px && yPixels[py].y >= px)
				{
					float d = yPixels[py].y - yPixels[py].x;
					finalColor = { (((d - (px - yPixels[py].x)) / d) * edgeColors[py][0]) + (((px - yPixels[py].x) / d) * edgeColors[py][1]) };
					//finalColor = { pixColor };
				}
				else
				{
					continue;
				}

				int pIdx{ px + py * m_Width };

				if (m_pDepthBufferPixels[pIdx] < 0)
					continue;

				m_pDepthBufferPixels[pIdx] = 0;

#else

				if (px < BoundBoxMin.x || px > BoundBoxMax.x || py < BoundBoxMin.y || py > BoundBoxMax.y)
					continue;

				TriToPix[0] = { screenPix - tri[0] };
				TriToPix[1] = { screenPix - tri[1] };
				TriToPix[2] = { screenPix - tri[2] };

				triCross[0] = { Vector2::Cross(Edges[0], TriToPix[0]) };
				triCross[1] = { Vector2::Cross(Edges[1], TriToPix[1]) };
				triCross[2] = { Vector2::Cross(Edges[2], TriToPix[2]) };

				//Texture

				if ((triCross[0] > 0 || triCross[1] >= 0 || triCross[2] >= 0))
					continue;

				weight[0] = { triCross[1] / triArea };
				weight[1] = { triCross[2] / triArea };
				weight[2] = { triCross[0] / triArea };

				//inerPolat[0] = { 1 / (vieuwVertices[0].position.z) * weight[0] };
				//inerPolat[1] = { 1 / (vieuwVertices[1].position.z) * weight[1] };
				//inerPolat[2] = { 1 / (vieuwVertices[2].position.z) * weight[2] };

				inerPolat[0] = { weight[0] / (vieuwVertices[0].position.z) };
				inerPolat[1] = { weight[1] / (vieuwVertices[1].position.z) };
				inerPolat[2] = { weight[2] / (vieuwVertices[2].position.z) };

				float inerPolatWeight = { 1.f / (inerPolat[0] + inerPolat[1] + inerPolat[2]) };

				int pIdx{ px + py * m_Width };

				if (m_pDepthBufferPixels[pIdx] < inerPolatWeight)
					continue;

				m_pDepthBufferPixels[pIdx] = inerPolatWeight;

				//-------------------------

				//UV[0] = { weight[0] * (vieuwVertices[0].uv / vieuwVertices0[indeces[ i + 0 ]].position.z) };
				//UV[1] = { weight[1] * (vieuwVertices[1].uv / vieuwVertices0[indeces[ i + 1 ]].position.z) };
				//UV[2] = { weight[2] * (vieuwVertices[2].uv / vieuwVertices0[indeces[ i + 2 ]].position.z) };

				UV[0] = { weight[0] * (vieuwVertices[0].uv / vieuwVertices2[0].position.z) };
				UV[1] = { weight[1] * (vieuwVertices[1].uv / vieuwVertices2[1].position.z) };
				UV[2] = { weight[2] * (vieuwVertices[2].uv / vieuwVertices2[2].position.z) };

				Vector2 fullUV = { (UV[0] + UV[1] + UV[2]) * inerPolatWeight };

				ColorRGB finalColor
				{
					m_pTexture->Sample(fullUV)
				};

				//-------------------------

				//ColorRGB finalColor{ vertices_ndc[0].color * weight[0] + vertices_ndc[1].color * weight[1] + vertices_ndc[2].color * weight[2] };

					//gradient = 1;
					//gradient += 1;
					//gradient /= 2;
					//ColorRGB finalColor{ gradient, gradient, gradient };

#endif // BresenhamActive

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}
}

void dae::Renderer::Render_W1_Part2()
{
}

void dae::Renderer::Render_W1_Part3()
{
}

void dae::Renderer::Render_W1_Part4()
{
}

void dae::Renderer::Render_W1_Part5()
{
}
