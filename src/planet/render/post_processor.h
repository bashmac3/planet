#pragma once

#include "planet/render/framebuffer.h"
#include "planet/render/shader.h"
#include "planet/render/mesh.h"
#include <memory>
#include <string>

namespace planet {

enum class PostEffect {
    None,
    VHS,
    Dream,
    Vignette,
    Grayscale,
    Invert,
    Blur,
    Pixelate,
    EdgeDetect,
    Sepia,
    Bloom,
    CRT,
    FishEye,
    MotionBlur,
    Sharpen,
    Emboss
};

class PostProcessor {
public:
    static PostProcessor& Instance();

    void Init(int width, int height);
    void Shutdown();
    void Resize(int width, int height);

    void BeginCapture();
    void EndCapture();

    void SetEffect(PostEffect effect) { m_currentEffect = effect; }
    PostEffect GetEffect() const { return m_currentEffect; }
    void SetEffectByName(const std::string& name);

    void SetEffectIntensity(float intensity) { m_intensity = intensity; }
    float GetEffectIntensity() const { return m_intensity; }

    float GetTime() const { return m_time; }
    void SetTime(float t) { m_time = t; }

    static std::vector<std::string> GetAvailableEffectNames();
    static std::string GetEffectName(PostEffect effect);
    static PostEffect GetEffectFromName(const std::string& name);

    void SetEffectParameter(const std::string& name, float value);
    float GetEffectParameter(const std::string& name) const;

private:
    PostProcessor() = default;

    void RenderFullscreenQuad();

    Framebuffer m_sceneFbo;
    std::unique_ptr<Shader> m_vhsShader;
    std::unique_ptr<Shader> m_vignetteShader;
    std::unique_ptr<Shader> m_dreamShader;
    std::unique_ptr<Shader> m_grayscaleShader;
    std::unique_ptr<Shader> m_invertShader;
    std::unique_ptr<Shader> m_blurShader;
    std::unique_ptr<Shader> m_pixelateShader;
    std::unique_ptr<Shader> m_edgeDetectShader;
    std::unique_ptr<Shader> m_sepiaShader;
    std::unique_ptr<Shader> m_bloomShader;
    std::unique_ptr<Shader> m_crtShader;
    std::unique_ptr<Shader> m_fishEyeShader;
    std::unique_ptr<Shader> m_motionBlurShader;
    std::unique_ptr<Shader> m_sharpenShader;
    std::unique_ptr<Shader> m_embossShader;
    Mesh m_fullscreenQuad;

    PostEffect m_currentEffect = PostEffect::None;
    float m_intensity = 1.0f;
    float m_time = 0.0f;
    int m_width = 0;
    int m_height = 0;
};

} // namespace planet
