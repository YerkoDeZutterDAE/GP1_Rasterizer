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

//#define BresenhamActive
#define TriStrip
//#define OBJ

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
	m_Camera.Initialize(60.f, { .0f,0.0f,-25.f });

#ifdef TriStrip

	m_pTexture = Texture::LoadFromFile("Resources/uv_grid_2.png");

#elif defined(OBJ)
	
	m_pTexture = Texture::LoadFromFile("Resources/tuktuk.png");

#else

	m_pTexture = Texture::LoadFromFile("Resources/uv_grid_2.png");

#endif // TriStrip
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

	//Render_1();
	Render_2();

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex_Out>& vertices_out) const
{
	vertices_out = {};
	vertices_out.reserve(vertices_in.size());
	for (auto v : vertices_in)
	{

		//std::cout << v.position.x << std::endl;
		v.position = m_Camera.invViewMatrix.TransformPoint(v.position);


		float aspec{ (float(m_Width) / float(m_Height)) };

		v.position.x = v.position.x / (aspec * m_Camera.fov) / v.position.z;
		v.position.y = v.position.y / m_Camera.fov / v.position.z;
		v.position.z = v.position.z;

		//-----------------------------------------------------------

		v.position.x = v.position.x * m_Width + m_Width / 2;
		v.position.y = v.position.y * m_Height + m_Height / 2;



		Vector4 pos = { v.position.x, v.position.y, v.position.z, 0 };

		Vertex_Out out = Vertex_Out{ pos, v.color, v.uv, v.normal };

		vertices_out.emplace_back(out);
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

void dae::Renderer::Render_1()
{

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

#elif defined(OBJ)

	std::vector<Mesh> Meshes{ {} };

	Utils::ParseOBJ("Resources/tuktuk.obj", Meshes[0].vertices, Meshes[0].indices);

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
				3,0,1, 1,4,3, 4,1,2,
				2,5,4, 6,3,4, 4,7,6,
				7,4,5, 5,8,7
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

	int pixelCount{ m_Width * m_Height };
	std::fill_n(m_pDepthBufferPixels, pixelCount, FLT_MAX);

	std::vector<uint32_t> indeces = Meshes[0].indices;

	//for (auto& vertices_ndc : Triangles)

	//int idx0{}, idx1{}, idx2{};

	std::vector<Vertex_Out> allVieuwVertices{};
	VertexTransformationFunction(Meshes[0].vertices, allVieuwVertices);

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

	std::vector<Vertex_Out> vieuwVertices
	{
		{},
		{},
		{}
	};

	std::vector<Vector2> tri
	{
		{},
		{},
		{}
	};

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

#elif defined(OBJ)

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
		idx0 = idx + 0;
		idx1 = idx + 1;
		idx2 = idx + 2;
#endif // TriStrip



		//std::vector<Vertex> vertices_ndc
		//{
		//	{Meshes[0].vertices[indeces[idx0]]},
		//	{Meshes[0].vertices[indeces[idx1]]},
		//	{Meshes[0].vertices[indeces[idx2]]}
		//};

		vieuwVertices[0] = { allVieuwVertices[indeces[idx0]] };
		vieuwVertices[1] = { allVieuwVertices[indeces[idx1]] };
		vieuwVertices[2] = { allVieuwVertices[indeces[idx2]] };

		tri[0] = Vector2{ vieuwVertices[0].position.x, vieuwVertices[0].position.y };
		tri[1] = Vector2{ vieuwVertices[1].position.x, vieuwVertices[1].position.y };
		tri[2] = Vector2{ vieuwVertices[2].position.x, vieuwVertices[2].position.y };

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



		Edges[0] = { tri[1] - tri[0] };
		Edges[1] = { tri[2] - tri[1] };
		Edges[2] = { tri[0] - tri[2] };


		float triArea{ Vector2::Cross(Edges[0], Edges[1]) };

		Vector2 BoundBoxMin = { Vector2::min(tri[0], Vector2::min(tri[1],tri[2])) };
		Vector2 BoundBoxMax = { Vector2::max(tri[0], Vector2::max(tri[1],tri[2])) };

		if (0 >= BoundBoxMin.x || 0 >= BoundBoxMin.y || m_Width <= BoundBoxMax.x || m_Height <= BoundBoxMax.y)
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

				if (m_RenderHitBox)
				{
					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(1 * 255),
						static_cast<uint8_t>(1 * 255),
						static_cast<uint8_t>(1 * 255));
					continue;
				}

				//TriToPix[0] = { screenPix - tri[0] };
				//TriToPix[1] = { screenPix - tri[1] };
				//TriToPix[2] = { screenPix - tri[2] };

				TriToPix[0] = { tri[0] - screenPix };
				TriToPix[1] = { tri[1] - screenPix };
				TriToPix[2] = { tri[2] - screenPix };

				triCross[0] = { Vector2::Cross(Edges[0], TriToPix[0]) };
				triCross[1] = { Vector2::Cross(Edges[1], TriToPix[1]) };
				triCross[2] = { Vector2::Cross(Edges[2], TriToPix[2]) };

				//Texture

				if ((triCross[0] > 0 || triCross[1] > 0 || triCross[2] > 0))
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

				UV[0] = { weight[0] * (vieuwVertices[0].uv / vieuwVertices[0].position.z) };
				UV[1] = { weight[1] * (vieuwVertices[1].uv / vieuwVertices[1].position.z) };
				UV[2] = { weight[2] * (vieuwVertices[2].uv / vieuwVertices[2].position.z) };

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

void dae::Renderer::Render_2()
{
	//Reset
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	int pixelCount{ m_Width * m_Height };
	std::fill_n(m_pDepthBufferPixels, pixelCount, FLT_MAX);

	//Reposition Points

	SetPositionInfo();

	//Render Pixel

	for (int i = 0; i < m_Meshes[0].indices.size() - 2; i++)
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

		m_vieuwVertices[0] = { m_Vertex_Outs[m_Meshes[0].indices[idx0]] };
		m_vieuwVertices[1] = { m_Vertex_Outs[m_Meshes[0].indices[idx1]] };
		m_vieuwVertices[2] = { m_Vertex_Outs[m_Meshes[0].indices[idx2]] };

		m_tri[0] = Vector2{ m_vieuwVertices[0].position.x, m_vieuwVertices[0].position.y };
		m_tri[1] = Vector2{ m_vieuwVertices[1].position.x, m_vieuwVertices[1].position.y };
		m_tri[2] = Vector2{ m_vieuwVertices[2].position.x, m_vieuwVertices[2].position.y };

		m_Edges[0] = { m_tri[1] - m_tri[0] };
		m_Edges[1] = { m_tri[2] - m_tri[1] };
		m_Edges[2] = { m_tri[0] - m_tri[2] };


		m_triArea =  Vector2::Cross(m_Edges[0], m_Edges[1]) ;

		m_BoundBoxMin = { Vector2::min(m_tri[0], Vector2::min(m_tri[1],m_tri[2])) };
		m_BoundBoxMax = { Vector2::max(m_tri[0], Vector2::max(m_tri[1],m_tri[2])) };

		if (0 >= m_BoundBoxMin.x || 0 >= m_BoundBoxMin.y || m_Width <= m_BoundBoxMax.x || m_Height <= m_BoundBoxMax.y)
			continue;


		RenderPix();
	}
}

void dae::Renderer::SetPositionInfo()
{

	//---------------------- Set Base Positions
#ifdef OBJ

	m_Meshes = { {} };

	Utils::ParseOBJ("Resources/tuktuk.obj", m_Meshes[0].vertices, m_Meshes[0].indices);

#elif defined(TriStrip)

	m_Meshes = {
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


	m_Meshes = {
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
				3,0,1, 1,4,3, 4,1,2,
				2,5,4, 6,3,4, 4,7,6,
				7,4,5, 5,8,7
			},
			PrimitiveTopology::TriangleStrip
		}
	};


#endif

	//---------------------- Reposition Points

	VertexTransformationFunction(m_Meshes[0].vertices, m_Vertex_Outs);

}

void dae::Renderer::RenderPix()
{
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector2 screenPix{ float(px),float(py) };

			float gradient{};



			if (px < m_BoundBoxMin.x || px > m_BoundBoxMax.x || py < m_BoundBoxMin.y || py > m_BoundBoxMax.y)
				continue;

			if (m_RenderHitBox)
			{
				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(1 * 255),
					static_cast<uint8_t>(1 * 255),
					static_cast<uint8_t>(1 * 255));
				continue;
			}

			//m_TriToPix[0] = { screenPix - m_tri[0] };
			//m_TriToPix[1] = { screenPix - m_tri[1] };
			//m_TriToPix[2] = { screenPix - m_tri[2] };

			m_TriToPix[0] = { m_tri[0] - screenPix };
			m_TriToPix[1] = { m_tri[1] - screenPix };
			m_TriToPix[2] = { m_tri[2] - screenPix };

			if (lastX != m_tri[0].x)
			{
				//std::cout << m_tri[0].x << std::endl;
				lastX = m_tri[0].x;
			}

			//m_TriToPix[0] = { m_tri[0] - screenPix };
			//m_TriToPix[1] = { m_tri[1] - screenPix };
			//m_TriToPix[2] = { m_tri[2] - screenPix };

			m_triCross[0] = { Vector2::Cross(m_Edges[0], m_TriToPix[0]) };
			m_triCross[1] = { Vector2::Cross(m_Edges[1], m_TriToPix[1]) };
			m_triCross[2] = { Vector2::Cross(m_Edges[2], m_TriToPix[2]) };

			//Texture

			if ((m_triCross[0] > 0 || m_triCross[1] > 0 || m_triCross[2] > 0))
				continue;

			m_weight[0] = { m_triCross[1] / m_triArea };
			m_weight[1] = { m_triCross[2] / m_triArea };
			m_weight[2] = { m_triCross[0] / m_triArea };

			//inerPolat[0] = { 1 / (vieuwVertices[0].position.z) * m_weight[0] };
			//inerPolat[1] = { 1 / (vieuwVertices[1].position.z) * m_weight[1] };
			//inerPolat[2] = { 1 / (vieuwVertices[2].position.z) * m_weight[2] };

			m_inerPolat[0] = { m_weight[0] / (m_vieuwVertices[0].position.z) };
			m_inerPolat[1] = { m_weight[1] / (m_vieuwVertices[1].position.z) };
			m_inerPolat[2] = { m_weight[2] / (m_vieuwVertices[2].position.z) };

			float inerPolatWeight = { 1.f / (m_inerPolat[0] + m_inerPolat[1] + m_inerPolat[2]) };

			int pIdx{ px + py * m_Width };

			if (m_pDepthBufferPixels[pIdx] < inerPolatWeight)
				continue;

			m_pDepthBufferPixels[pIdx] = inerPolatWeight;

			//-------------------------

			//m_UV[0] = { m_weight[0] * (vieuwVertices[0].uv / vieuwVertices0[indeces[ i + 0 ]].position.z) };
			//m_UV[1] = { m_weight[1] * (vieuwVertices[1].uv / vieuwVertices0[indeces[ i + 1 ]].position.z) };
			//m_UV[2] = { m_weight[2] * (vieuwVertices[2].uv / vieuwVertices0[indeces[ i + 2 ]].position.z) };

			m_UV[0] = { m_weight[0] * (m_vieuwVertices[0].uv / m_vieuwVertices[0].position.z) };
			m_UV[1] = { m_weight[1] * (m_vieuwVertices[1].uv / m_vieuwVertices[1].position.z) };
			m_UV[2] = { m_weight[2] * (m_vieuwVertices[2].uv / m_vieuwVertices[2].position.z) };

			Vector2 fullUV = { (m_UV[0] + m_UV[1] + m_UV[2]) * inerPolatWeight };

			ColorRGB finalColor
			{
				m_pTexture->Sample(fullUV)
			};

			//--------------------------

				//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));

		}

	}
}
