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
#define BresenhamActive

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	//m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });
}

Renderer::~Renderer()
{
	//delete[] m_pDepthBufferPixels;
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
	std::vector<Vertex> vertices_ndc
	{
		{{0.f, 2.f, 0.f}, colors::Red},
		{{1.f, 0.f, 0.f}, colors::Blue},
		{{ -1.f, 0.f, 0.f}, colors::Green}
	};

	std::vector<Vertex> worldVertices{ vertices_ndc };
	std::vector<Vertex> vieuwVertices{};

	VertexTransformationFunction(worldVertices, vieuwVertices);

	for (auto& v : vieuwVertices)
	{
		//if (float(px) - (v.position.x * m_Width + m_Width / 2) > -10 && float(px) - (v.position.x * m_Width + m_Width / 2) < 10)
		//{
		//	if (float(py) - (v.position.y * m_Height + m_Height / 2) > -10 && float(py) - (v.position.y * m_Height + m_Height / 2) < 10)
		//	{
		//		gradient = 1;
		//		gradient += 1;
		//		gradient /= 2;
		//	}
		//	//std::cout << v.position.x << std::endl;
		//}

		v.position.x = v.position.x * m_Width + m_Width / 2;
		v.position.y = v.position.y * m_Height + m_Height / 2;
	}

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

	yPixels.resize(m_Height);

	//std::cout << "ok?" << std::endl;

	//std::vector<Vector2> markedPix{};

	Utils::Bresenham(tri[1], tri[0], edgePixels[0], yPixels);
	Utils::Bresenham(tri[2], tri[1], edgePixels[1], yPixels);
	Utils::Bresenham(tri[0], tri[2], edgePixels[2], yPixels);

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

	//m_pBackBufferPixels = {};
	//m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
	//m_pBackBufferPixels = (uint32_t*)SDL_FillRect;
	SDL_FillRect(m_pBackBuffer, NULL,  SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

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

			//if (std::find(edgePixels[0].begin(), edgePixels[0].end(), screenPix) != edgePixels[0].end())
			//{
			//	gradient = 1;
			//	gradient += 1;
			//	gradient /= 2;
			//}
			//else if (std::find(edgePixels[1].begin(), edgePixels[1].end(), screenPix) != edgePixels[1].end())
			//{
			//	gradient = 1;
			//	gradient += 1;
			//	gradient /= 2;
			//}
			//else if (std::find(edgePixels[2].begin(), edgePixels[2].end(), screenPix) != edgePixels[2].end())
			//{
			//	gradient = 1;
			//	gradient += 1;
			//	gradient /= 2;
			//}

			if (yPixels[py].x == 0)
			{
				//m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				//	static_cast<uint8_t>(0),
				//	static_cast<uint8_t>(0),
				//	static_cast<uint8_t>(0));
				continue;
			}

			if (yPixels[py].x <= px && yPixels[py].y >= px)
			{
				gradient = 1;
				gradient += 1;
				gradient /= 2;
			}
			else
			{
				continue;
			}

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

			if (!(triCross[0] >= 0 || triCross[1] >= 0 || triCross[2] >= 0))
			{
				gradient = 1;
				gradient += 1;
				gradient /= 2;
			}

#endif // BresenhamActive

			ColorRGB finalColor{ gradient, gradient, gradient };

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
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
