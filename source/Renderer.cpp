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
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
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

	for (auto& v : vertices_in)
	{
		Vertex viewVertex{};
		Vertex projectedVertex{};

		viewVertex = v;

		viewVertex.position = m_Camera.GetLookatVector(viewVertex.position);

		projectedVertex.position.x = viewVertex.position.x / viewVertex.position.z;
		projectedVertex.position.y = viewVertex.position.y / viewVertex.position.z;
		projectedVertex.position.z = viewVertex.position.z;

		float aspec{ (float(m_Width) / float(m_Height)) };

		projectedVertex.position.x = projectedVertex.position.x / (aspec * m_Camera.fov);
		projectedVertex.position.y = projectedVertex.position.y / m_Camera.fov;

		projectedVertex.color = v.color;

		projectedVertex.position.x = projectedVertex.position.x * m_Width + m_Width / 2;
		projectedVertex.position.y = projectedVertex.position.y * m_Height + m_Height / 2;

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

	std::vector<std::vector<Vertex>> Triangles
	{
		{
			{{0.f, 4.f, 2.f}, colors::Red},
			{{3.f, -2.f, 2.f}, colors::Green},
			{{ -3.f, -2.f, 2.f}, colors::Blue}
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

	for (auto& vertices_ndc : Triangles)
	{

		std::vector<Vertex> worldVertices{ vertices_ndc };
		std::vector<Vertex> vieuwVertices{};

		VertexTransformationFunction(worldVertices, vieuwVertices);

		//for (auto& v : vieuwVertices)
		//{
		//	v.position.x = v.position.x * m_Width + m_Width / 2;
		//	v.position.y = v.position.y * m_Height + m_Height / 2;
		//}

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

				TriToPix[0] = { screenPix - tri[0] };
				TriToPix[1] = { screenPix - tri[1] };
				TriToPix[2] = { screenPix - tri[2] };

				Edges[0] = { tri[1] - tri[0] };
				Edges[1] = { tri[2] - tri[1] };
				Edges[2] = { tri[0] - tri[2] };

				triCross[0] = { Vector2::Cross(Edges[0], TriToPix[0]) };
				triCross[1] = { Vector2::Cross(Edges[1], TriToPix[1]) };
				triCross[2] = { Vector2::Cross(Edges[2], TriToPix[2]) };



				float triArea{ Vector2::Cross(Edges[0], Edges[1]) };

				weight[0] = { triCross[0] / triArea };
				weight[1] = { triCross[1] / triArea };
				weight[2] = { triCross[2] / triArea };

				inerPolat[0] = { 1 / (vieuwVertices[0].position.z) * weight[0] };
				inerPolat[1] = { 1 / (vieuwVertices[1].position.z) * weight[1] };
				inerPolat[2] = { 1 / (vieuwVertices[2].position.z) * weight[2] };

				float inerPolatWeight = {1 / (inerPolat[0] + inerPolat[1] + inerPolat[2])};

				//UV[0] = { weight[0] * (vieuwVertices[0].color / vieuwVertices[0].position.z)};

				//Texture

				if ((triCross[0] > 0 || triCross[1] >= 0 || triCross[2] >= 0))
					continue;


				ColorRGB finalColor{ vertices_ndc[0].color * weight[0] + vertices_ndc[1].color * weight[1] + vertices_ndc[2].color * weight[2] };

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
