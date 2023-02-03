#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"

using namespace Walnut;

struct Vec3 {
	float x, y, z;
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct Circle {
	Vec3 origin = Vec3(0.0f, 0.0f, 0.0f);
	float radius = 0.0f;
	uint32_t color = 0xffffffff;

	Circle() {}
	Circle(Vec3 origin, float radius, uint32_t color) : origin(origin), radius(radius), color(color) {}
};

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3f", m_LastRenderTime);
		if (ImGui::Button("Render")) {
			Render();
		}
		ImGui::DragFloat("Radius", &circle.radius, 1.0f);
		circle.radius = std::max(circle.radius, 0.0f);
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ));
		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		static bool circleInit = false;
		if (!circleInit) {
			circle.radius = 100;
			circle.origin = Vec3((float)m_ViewportWidth / 2, (float)m_ViewportHeight / 2, circle.radius);
			circle.color = 0xff00ffff;
			circleInit = true;
		}

		if (m_Image) {
			ImGui::Image(m_Image->GetDescriptorSet(), { (float)m_Image->GetWidth(), (float)m_Image->GetHeight() });
		}

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::ShowDemoWindow();

		Render();
	}

	void Render()
	{
		circle.radius += 10;
		Timer timer;

		if (!m_Image || m_ViewportWidth != m_Image->GetWidth() || m_ViewportHeight != m_Image->GetHeight()) {
			m_Image = std::make_shared<Image>(m_ViewportWidth, m_ViewportHeight, ImageFormat::RGBA);
			delete[] m_ImageData;
			m_ImageData = new uint32_t[m_ViewportWidth * m_ViewportHeight];
		}

		// Random noise
		//for (uint32_t i = 0; i < m_ViewportWidth * m_ViewportHeight; i++) {
		//	m_ImageData[i] = Random::UInt();
		//	m_ImageData[i] |= 0xff000000;
		//}

		for (uint32_t i = 0; i < m_ViewportHeight; i++) {
			for (uint32_t j = 0; j < m_ViewportWidth; j++) {
				// base color	
				m_ImageData[i * m_ViewportWidth + j] = 0xffebce87;

				Vec3 rayOrigin(j, i, 0);
				Vec3 rayDirection(0, 0, 1);

				float a = (rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z);
				float b = 2 * (rayOrigin.x * rayDirection.x - rayDirection.x * circle.origin.x + rayOrigin.y * rayDirection.y - rayDirection.y * circle.origin.y + rayOrigin.z * rayDirection.z - rayDirection.z * circle.origin.z);
				float c = (rayOrigin.x - circle.origin.x) * (rayOrigin.x - circle.origin.x) + (rayOrigin.y - circle.origin.y) * (rayOrigin.y - circle.origin.y) + (rayOrigin.z - circle.origin.z) * (rayOrigin.z - circle.origin.z) - circle.radius * circle.radius;
				
				// b^2 - 4ac
				float discriminant = b * b - 4 * a * c;
				if (discriminant > 0.0f) {
					float t1 = (-b + discriminant) / 2 * a;
					float t2 = (-b - discriminant) / 2 * a;
					m_ImageData[i * m_ViewportWidth + j] = circle.color - t1;
				}
			}
		}

		m_Image->SetData(m_ImageData);

		m_LastRenderTime = timer.ElapsedMillis();
	}
private:
	std::shared_ptr<Image> m_Image;
	uint32_t* m_ImageData = nullptr;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	float m_LastRenderTime = 0.0f;
	Circle circle;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}