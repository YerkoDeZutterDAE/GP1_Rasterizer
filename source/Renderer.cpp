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
#include <algorithm>

using namespace dae;

//#define BresenhamActive
//#define TriStrip
#define OBJ

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
	
	float aspec{ (float(m_Width) / float(m_Height)) };

	//m_Camera.Initialize(60.f, { .0f,0.0f,-25.f });

#ifdef TriStrip

	m_Camera.Initialize(60.f, aspec, { .0f,0.0f,-20.f });
	m_pTexture = Texture::LoadFromFile("Resources/uv_grid_2.png");

#elif defined(OBJ)

	m_Camera.Initialize(60.f, aspec, { .0f,5.0f,-50.f });
	//m_pTexture = Texture::LoadFromFile("Resources/tuktuk.png");
	m_pTexture = Texture::LoadFromFile("Resources/vehicle_diffuse.png");
	m_pNormalMap = Texture::LoadFromFile("Resources/vehicle_normal.png");
	m_pGlossMap = Texture::LoadFromFile("Resources/vehicle_gloss.png");
	m_pSpecularMap = Texture::LoadFromFile("Resources/vehicle_specular.png");

#else

	m_Camera.Initialize(60.f, aspec, { .0f,0.0f,-25.f });
	m_pTexture = Texture::LoadFromFile("Resources/uv_grid_2.png");

#endif // TriStrip

	SetPositionInfo();
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	delete m_pTexture;
	delete m_pNormalMap;
	delete m_pGlossMap;
	delete m_pSpecularMap;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
	RotateOBJ(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	//Render_1();
	RenderS();

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
	//Matrix testMatrix{ m_Camera.viewMatrix * m_Meshes[0].worldMatrix };
	Matrix testMatrix{ m_Meshes[0].worldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix };

	for (auto v : vertices_in)
	{
		Vertex_Out vOut{};

		vOut.position = testMatrix.TransformPoint({ v.position, 1.0f });

		//////-----------------------------------------------------------

		vOut.viewDirection = { vOut.position.x , vOut.position.y , vOut.position.z};
		vOut.viewDirection.Normalize();

		vOut.position.x /= vOut.position.w;
		vOut.position.y /= vOut.position.w;
		vOut.position.z /= vOut.position.w;

		vOut.position.x = vOut.position.x * m_Width + m_Width / 2;
		vOut.position.y = vOut.position.y * m_Height + m_Height / 2;

		vOut.normal = m_Meshes[0].worldMatrix.TransformVector(v.normal);
		vOut.normal.Normalize();

		vOut.tangent = m_Meshes[0].worldMatrix.TransformVector(v.tangent);
		vOut.normal.Normalize();


		Vector4 pos = { vOut.position.x, vOut.position.y, vOut.position.z, vOut.position.w };

		Vertex_Out out = Vertex_Out{ pos, v.color, v.uv, vOut.normal, vOut.tangent, vOut.viewDirection };

		vertices_out.emplace_back(out);
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

void dae::Renderer::RenderS()
{

	//Reset
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	int pixelCount{ m_Width * m_Height };
	std::fill_n(m_pDepthBufferPixels, pixelCount, FLT_MAX);

	//Reposition Points

	VertexTransformationFunction(m_Meshes[0].vertices, m_Vertex_Outs);

	//Render Pixel

	for (int i = 0; i < m_Meshes[0].indices.size(); i+=3)
	{
#ifdef TriStrip

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

#elif defined(OBJ)

		idx0 = i;
		idx1 = i + 1;
		idx2 = i + 2;

#endif // TriStrip

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

	//Utils::ParseOBJ("Resources/tuktuk.obj", m_Meshes[0].vertices, m_Meshes[0].indices);
	Utils::ParseOBJ("Resources/vehicle.obj", m_Meshes[0].vertices, m_Meshes[0].indices);

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

	m_Meshes[0].worldMatrix = Matrix::CreateScale({ 1.f,1.f,1.f }) * Matrix::CreateRotationY(0.3f) * Matrix::CreateTranslation(Vector3{ 0.f,0.f,50.f });
}

void dae::Renderer::RenderPix()
{
	for (int px{ static_cast<int>(m_BoundBoxMin.x) }; px < m_BoundBoxMax.x; ++px)
	{
		for (int py{ static_cast<int>(m_BoundBoxMin.y) }; py < m_BoundBoxMax.y; ++py)
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

			m_TriToPix[0] = { screenPix - m_tri[0] };
			m_TriToPix[1] = { screenPix - m_tri[1] };
			m_TriToPix[2] = { screenPix - m_tri[2] };



			m_triCross[0] = { Vector2::Cross(m_Edges[0], m_TriToPix[0]) };
			m_triCross[1] = { Vector2::Cross(m_Edges[1], m_TriToPix[1]) };
			m_triCross[2] = { Vector2::Cross(m_Edges[2], m_TriToPix[2]) };

			//Texture

			if ((m_triCross[0] <= 0 || m_triCross[1] <= 0 || m_triCross[2] <= 0))
				continue;

			m_weight[0] = { m_triCross[1] / m_triArea };
			m_weight[1] = { m_triCross[2] / m_triArea };
			m_weight[2] = { m_triCross[0] / m_triArea };

			// interpolar Z

			m_inerPolat[0] = { m_weight[0] / (m_vieuwVertices[0].position.z) };
			m_inerPolat[1] = { m_weight[1] / (m_vieuwVertices[1].position.z) };
			m_inerPolat[2] = { m_weight[2] / (m_vieuwVertices[2].position.z) };

			float inerPolatWeight = { 1.f / (m_inerPolat[0] + m_inerPolat[1] + m_inerPolat[2]) };

			int pIdx{ px + py * m_Width };

			if (m_pDepthBufferPixels[pIdx] < inerPolatWeight)
				continue;
			if (inerPolatWeight < m_Camera.tmin, inerPolatWeight > m_Camera.tmax)
				continue;

			m_pDepthBufferPixels[pIdx] = inerPolatWeight;

			if (BigestD < inerPolatWeight)
				BigestD = inerPolatWeight;

			//---------Interpolar W-----------

			m_inerPolat[0] = { m_weight[0] / (m_vieuwVertices[0].position.w) };
			m_inerPolat[1] = { m_weight[1] / (m_vieuwVertices[1].position.w) };
			m_inerPolat[2] = { m_weight[2] / (m_vieuwVertices[2].position.w) };

			float inerPolatW = { 1.f / (m_inerPolat[0] + m_inerPolat[1] + m_inerPolat[2]) };

			//UV CREATION

			m_UV[0] = { m_weight[0] * (m_vieuwVertices[0].uv / m_vieuwVertices[0].position.w) };
			m_UV[1] = { m_weight[1] * (m_vieuwVertices[1].uv / m_vieuwVertices[1].position.w) };
			m_UV[2] = { m_weight[2] * (m_vieuwVertices[2].uv / m_vieuwVertices[2].position.w) };

			Vector2 fullUV = { (m_UV[0] + m_UV[1] + m_UV[2]) * inerPolatW };

			//----------Tangent----------

			Vector3 tanInterpolar0 = {m_weight[0] * (m_vieuwVertices[0].tangent / m_vieuwVertices[0].position.w)};
			Vector3 tanInterpolar1 = {m_weight[1] * (m_vieuwVertices[1].tangent / m_vieuwVertices[1].position.w)};
			Vector3 tanInterpolar2 = {m_weight[2] * (m_vieuwVertices[2].tangent / m_vieuwVertices[2].position.w)};

			Vector3 tangent = { tanInterpolar0 + tanInterpolar1 + tanInterpolar2 };
			tangent *= inerPolatW;
			tangent.Normalize();

			//-----------view dir---------

			Vector3 vDInterpolar0 = { m_weight[0] * (m_vieuwVertices[0].viewDirection / m_vieuwVertices[0].position.w) };
			Vector3 vDInterpolar1 = { m_weight[1] * (m_vieuwVertices[1].viewDirection / m_vieuwVertices[1].position.w) };
			Vector3 vDInterpolar2 = { m_weight[2] * (m_vieuwVertices[2].viewDirection / m_vieuwVertices[2].position.w) };

			Vector3 viewDir = { vDInterpolar0 + vDInterpolar1 + vDInterpolar2 };
			viewDir *= inerPolatW;
			viewDir.Normalize();

			//----------NORMAL---------

			// W normal creation
			Vector3 Wnormal = m_vieuwVertices[0].normal * m_weight[0] +
				m_vieuwVertices[1].normal * m_weight[1] +
				m_vieuwVertices[2].normal * m_weight[2];

			Wnormal.Normalize();

			// render normalmap

			if (m_RenderNormalMap)
			{
				//axis creation
				Vector3 norTanCross = { Wnormal.Cross(Wnormal, tangent) };
				Matrix tanAxis = { tangent , norTanCross , Wnormal , {0,0,0} };

				//normal sample map
				ColorRGB ColorSampeldNormalMap = { 2.f * m_pNormalMap->Sample(fullUV) - ColorRGB{1.f,1.f,1.f} };
				Vector3 sampeldNormalMap = { ColorSampeldNormalMap.r,
					ColorSampeldNormalMap.g,
					ColorSampeldNormalMap.b };

				Wnormal = tanAxis.TransformVector(sampeldNormalMap);
				Wnormal.Normalize();
			}

			//-------------------------

			float lightI = {7.f};

			ColorRGB finalColor
			{};
			float observed{};
			ColorRGB lambert{};
			ColorRGB Specular{};

			float d{};

			float c{};
			float d2{};

			if (m_RenderDepth)
			{
				//d = { (1.0f / inerPolatWeight) };
				d = { inerPolatWeight * 100 - 99 };

				//Remap();

				c = { std::clamp(inerPolatWeight,  0.999f, 1.f) };
				d2 = { (c - 0.999f) / (1.f - 0.999f) };

				finalColor =
				{
					d2, d2, d2
				};

				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}

			observed = { Wnormal.Dot(Wnormal, (Vector3{ 0.577f, -0.577f, 0.577f })) };

			//observed = abs(observed);
			observed = std::min(observed, 0.f);
			observed = abs(observed);

			switch (m_RenderMode)
			{
			case dae::Renderer::RenderMode::Full:

				Specular = m_pSpecularMap->Sample(fullUV) * Utils::Phong(1.f, (m_pGlossMap->Sample(fullUV).r * 25.f), { 0.577f, -0.577f, 0.577f }, viewDir, Wnormal);
				lambert = m_pTexture->Sample(fullUV) / float(M_PI);

				finalColor =
				{
					lightI * lambert + Specular
				};
				finalColor *= observed;

				//--------------------------

					//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
				break;

			case dae::Renderer::RenderMode::Observed:
				finalColor = 
				{
					observed,
					observed,
					observed
				};

				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
				break;
			case dae::Renderer::RenderMode::Difuse:
				finalColor = 
				{
					m_pTexture->Sample(fullUV) / float(M_PI)
				};

				finalColor *= observed * lightI;
				 
				finalColor *= lightI;
				finalColor *= observed;

				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
				break;
			case dae::Renderer::RenderMode::Spec:
				finalColor =
				{
					m_pSpecularMap->Sample(fullUV)* Utils::Phong(1.f, (m_pGlossMap->Sample(fullUV).r * 25.f), { 0.577f, -0.577f, 0.577f }, viewDir, Wnormal)
				};

				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
				break;
			default:
				break;
			}

		}

	}
}

void dae::Renderer::RotateOBJ(Timer* pTimer)
{
	if (m_RenderRotation)
	{
		float addedAngle = { 45.0f * pTimer->GetElapsed() * TO_RADIANS };
		m_Meshes[0].worldMatrix = Matrix::CreateRotationY(addedAngle) * m_Meshes[0].worldMatrix;
	}
}
