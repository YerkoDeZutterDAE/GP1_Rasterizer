#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "DataTypes.h"

#include <iostream>

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	struct Mesh;
	struct Vertex;
	class Timer;
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();

		bool SaveBufferToImage() const;

		//Exersize Parts
		void Render_1();

		// BIG CLEANUP FUNCTIONS AND VARS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			// functions

			void Render_2();

			void SetPositionInfo();
			void RenderPix();

			// vars

			std::vector<Mesh> m_Meshes{};
			std::vector<Vertex_Out> m_Vertex_Outs{};

			std::vector<Vector2> m_TriToPix			{	{},	{},	{}	};
			std::vector<Vector2> m_Edges			{	{},	{},	{}	};
			std::vector<float> m_triCross			{	{},	{},	{}	};
			std::vector<float> m_weight				{	{},	{},	{}	};
			std::vector<float> m_inerPolat			{	{},	{},	{}	};
			std::vector<Vector2> m_UV				{	{},	{},	{}	};
			std::vector<Vertex_Out> m_vieuwVertices	{	{},	{},	{}	};
			std::vector<Vector2> m_tri				{	{},	{},	{}	};

			int idx0;
			int idx1;
			int idx2;

			float m_triArea{};

			Vector2 m_BoundBoxMin{};
			Vector2 m_BoundBoxMax{};

			float lastX{};

		//END OF BIG CLEANUP !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


		//Button Press Events
		void RenderHitBox() { m_RenderHitBox = !m_RenderHitBox; std::cout << "Test HitBox Render" << std::endl; };
		void ToggleRotation() { m_RenderRotation = !m_RenderRotation; std::cout << "Test rotation Toggle" << std::endl; };
		void ToggleNormalMap() { m_RenderNormalMap = !m_RenderNormalMap; std::cout << "Test normalMap Toggle" << std::endl; };
		void CycleShadingMode() 
		{ 
			std::cout << "Test rotation Toggle" << std::endl;
		};

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		Texture* m_pTexture{nullptr};

		float* m_pDepthBufferPixels{};

		Camera m_Camera{ {0.f,0.f,-10.f}, 60.f };

		int m_Width{};
		int m_Height{};


		//Button Press Events
		bool m_RenderHitBox{false};
		bool m_RenderRotation{false};
		bool m_RenderNormalMap{false};

		//Function that transforms the vertices from the mesh from World space to Screen space
		void VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex_Out>& vertices_out) const; //W1 Version
	};
}
