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
		{{0.f, 2.f, 0.f}, colors::White},
		{{1.f, 0.f, 0.f}, colors::White},
		{{ -1.f, 0.f, 0.f}, colors::White}
	};

	std::vector<Vertex> worldVertices{ vertices_ndc };
	std::vector<Vertex> vieuwVertices{};

	VertexTransformationFunction(worldVertices, vieuwVertices);

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector3 screenPix{ float(px),float(py),1 };

			float gradient{};

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

			//Vector3 tri{};
			//Vector3 point{ m_Camera.forward + m_Camera.origin + screenPix };

			////side 1
			//Vector3 sVector{ vieuwVertices[0].position - vieuwVertices[2].position };
			//Vector3 pTOs{ point - vieuwVertices[2].position };
			//Vector3 crossSide{ Vector3::Cross(sVector, pTOs) };
			//bool hitTri1{ Vector3::Dot(-m_Camera.forward, crossSide) > 0 };

			////if (!hitTri1)
			////	return false;

			////side 2
			//sVector = { vieuwVertices[1].position - vieuwVertices[0].position };
			//pTOs = { point - vieuwVertices[0].position };
			//crossSide = { Vector3::Cross(sVector, pTOs) };
			//bool hitTri2 = { Vector3::Dot(-m_Camera.forward, crossSide) > 0 };

			////if (!hitTri1)
			////	return false;

			////side 3
			//sVector = { vieuwVertices[2].position - vieuwVertices[1].position };
			//pTOs = { point - vieuwVertices[1].position };
			//crossSide = { Vector3::Cross(sVector, pTOs) };
			//bool hitTri3 = { Vector3::Dot(-m_Camera.forward, crossSide) > 0 };

			//if (hitTri3)
			//{
			//	gradient = 1;
			//	gradient += 1;
			//	gradient /= 2;
			//}

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
