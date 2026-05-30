#include "planet/render/post_processor.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include "planet/core/logger.h"

namespace planet {

static const char* FULLSCREEN_VERTEX = R"(
#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 vTexCoord;

void main() {
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}
)";

static const char* VHS_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uTime;
uniform float uIntensity;
uniform vec2 uResolution;

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec2 uv = vTexCoord;

    float jitter = (rand(vec2(uTime, uv.y * 100.0)) - 0.5) * 0.01 * uIntensity;
    uv.x += jitter;

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 color = texture(uScene, uv).rgb;

    float noise = rand(uv * uTime * 0.7) * 0.08 * uIntensity;
    color += noise;

    float r = texture(uScene, uv + vec2(0.003 * uIntensity, 0.0)).r;
    float b = texture(uScene, uv - vec2(0.003 * uIntensity, 0.0)).b;
    color = mix(color, vec3(r * 0.8, color.g, b * 0.8), uIntensity * 0.6);

    float trackingLine = step(0.97, rand(vec2(uTime * 10.0, 0.0)));
    float lineDist = abs(uv.y - rand(vec2(floor(uTime * 3.0), 0.0)));
    if (lineDist < 0.003 * uIntensity || trackingLine > 0.5) {
        color = mix(color, vec3(0.8, 0.85, 1.0), 0.5 * uIntensity);
    }

    float edgeDark = 1.0 - abs(uv.x - 0.5) * 2.0;
    color *= mix(1.0, edgeDark * 0.5 + 0.5, uIntensity * 0.3);

    color = clamp(color, 0.0, 1.0);
    FragColor = vec4(color, 1.0);
}
)";

static const char* VIGNETTE_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uTime;
uniform float uIntensity;
uniform vec2 uResolution;

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec2 uv = vTexCoord;
    vec3 color = texture(uScene, uv).rgb;

    float vignette = 1.0 - length(uv - 0.5) * 1.2 * uIntensity;
    vignette = smoothstep(0.0, 1.0, vignette);
    color *= vignette;

    float grain = rand(uv * uTime * 100.0) * 0.03 * uIntensity;
    color += grain;

    color = clamp(color, 0.0, 1.0);
    FragColor = vec4(color, 1.0);
}
)";

static const char* DREAM_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uTime;
uniform float uIntensity;
uniform vec2 uResolution;

void main() {
    vec2 uv = vTexCoord;
    vec2 texel = 1.0 / uResolution;

    vec3 color = vec3(0.0);
    float totalWeight = 0.0;
    float blurRadius = 4.0 * uIntensity;

    for (float x = -blurRadius; x <= blurRadius; x += 1.0) {
        for (float y = -blurRadius; y <= blurRadius; y += 1.0) {
            float weight = exp(-(x*x + y*y) / (blurRadius * blurRadius * 0.5));
            color += texture(uScene, uv + vec2(x, y) * texel).rgb * weight;
            totalWeight += weight;
        }
    }
    color /= totalWeight;

    color = mix(color, vec3(dot(color, vec3(0.299, 0.587, 0.114))), uIntensity * 0.2);

    float vignette = 1.0 - length(uv - 0.5) * 0.8;
    color *= vignette;

    float flicker = sin(uTime * 2.0 + uv.y * 10.0) * 0.03 * uIntensity + 1.0;
    color *= flicker;

    color = clamp(color, 0.0, 1.0);
    FragColor = vec4(color, 1.0);
}
)";

static const char* GRAYSCALE_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uTime;
uniform float uIntensity;
uniform vec2 uResolution;

void main() {
    vec3 color = texture(uScene, vTexCoord).rgb;
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    color = mix(color, vec3(gray), uIntensity);
    FragColor = vec4(color, 1.0);
}
)";

static const char* INVERT_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uTime;
uniform float uIntensity;
uniform vec2 uResolution;

void main() {
    vec3 color = texture(uScene, vTexCoord).rgb;
    color = mix(color, vec3(1.0) - color, uIntensity);
    FragColor = vec4(color, 1.0);
}
)";

static const char* BLUR_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uTime;
uniform float uIntensity;
uniform vec2 uResolution;

void main() {
    vec2 uv = vTexCoord;
    vec2 texel = 1.0 / uResolution;

    int radius = int(mix(1.0, 6.0, uIntensity));
    float sigma = float(radius) / 3.0;
    vec3 result = vec3(0.0);
    float total = 0.0;

    for (int x = -radius; x <= radius; x++) {
        for (int y = -radius; y <= radius; y++) {
            float weight = exp(-float(x*x + y*y) / (2.0 * sigma * sigma));
            result += texture(uScene, uv + vec2(x, y) * texel).rgb * weight;
            total += weight;
        }
    }
    result /= total;

    FragColor = vec4(result, 1.0);
}
)";

static const char* PIXELATE_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uTime;
uniform float uIntensity;
uniform vec2 uResolution;

void main() {
    vec2 uv = vTexCoord;
    float pixelSize = mix(2.0, 32.0, uIntensity);
    vec2 pixelUV = floor(uv * pixelSize) / pixelSize;
    pixelUV += vec2(0.5 / pixelSize);
    vec3 color = texture(uScene, pixelUV).rgb;
    FragColor = vec4(color, 1.0);
}
)";

static const char* EDGE_DETECT_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uTime;
uniform float uIntensity;
uniform vec2 uResolution;

void main() {
    vec2 uv = vTexCoord;
    vec2 texel = 1.0 / uResolution;

    vec3 tl = texture(uScene, uv + vec2(-texel.x,  texel.y)).rgb;
    vec3 t  = texture(uScene, uv + vec2( 0.0,      texel.y)).rgb;
    vec3 tr = texture(uScene, uv + vec2( texel.x,  texel.y)).rgb;
    vec3 l  = texture(uScene, uv + vec2(-texel.x,  0.0)).rgb;
    vec3 r  = texture(uScene, uv + vec2( texel.x,  0.0)).rgb;
    vec3 bl = texture(uScene, uv + vec2(-texel.x, -texel.y)).rgb;
    vec3 b  = texture(uScene, uv + vec2( 0.0,     -texel.y)).rgb;
    vec3 br = texture(uScene, uv + vec2( texel.x, -texel.y)).rgb;

    float lx = dot(-tl + tr - 2.0*l + 2.0*r - bl + br, vec3(0.299, 0.587, 0.114));
    float ly = dot(-tl - 2.0*t - tr + bl + 2.0*b + br, vec3(0.299, 0.587, 0.114));
    float edge = sqrt(lx*lx + ly*ly);

    vec3 color = texture(uScene, uv).rgb;
    color = mix(color, vec3(edge), uIntensity);
    FragColor = vec4(color, 1.0);
}
)";

static const char* SEPIA_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uTime;
uniform float uIntensity;
uniform vec2 uResolution;

void main() {
    vec3 color = texture(uScene, vTexCoord).rgb;
    vec3 sepia = vec3(
        dot(color, vec3(0.393, 0.769, 0.189)),
        dot(color, vec3(0.349, 0.686, 0.168)),
        dot(color, vec3(0.272, 0.534, 0.131))
    );
    color = mix(color, sepia, uIntensity);
    FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
)";

static const char* BLOOM_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uTime;
uniform float uIntensity;
uniform vec2 uResolution;

void main() {
    vec2 uv = vTexCoord;
    vec2 texel = 1.0 / uResolution;

    vec3 color = texture(uScene, uv).rgb;

    float brightness = dot(color, vec3(0.299, 0.587, 0.114));
    vec3 bloom = vec3(0.0);
    float threshold = mix(0.9, 0.5, uIntensity);

    if (brightness > threshold) {
        vec3 blur = vec3(0.0);
        float total = 0.0;
        int radius = 4;
        for (int x = -radius; x <= radius; x++) {
            for (int y = -radius; y <= radius; y++) {
                float weight = exp(-float(x*x + y*y) / 8.0);
                blur += texture(uScene, uv + vec2(x, y) * texel).rgb * weight;
                total += weight;
            }
        }
        blur /= total;
        bloom = blur * (brightness - threshold) / (1.0 - threshold) * uIntensity;
    }

    color += bloom;
    color = clamp(color, 0.0, 1.0);
    FragColor = vec4(color, 1.0);
}
)";

static const char* CRT_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uTime;
uniform float uIntensity;
uniform vec2 uResolution;

void main() {
    vec2 uv = vTexCoord;
    vec3 color = texture(uScene, uv).rgb;

    float scanline = sin(uv.y * uResolution.y * 3.14159 * 0.5) * 0.5 + 0.5;
    scanline = smoothstep(0.3, 0.7, scanline);
    scanline = mix(1.0, scanline, uIntensity * 0.5);
    color *= scanline;

    float vignette = 1.0 - length(uv - 0.5) * 0.7 * uIntensity;
    color *= vignette;

    vec2 distortion = (uv - 0.5) * 1.02;
    float dist = length(distortion);
    vec2 distorted = uv + distortion * dist * dist * 0.1 * uIntensity;
    if (distorted.x < 0.0 || distorted.x > 1.0 || distorted.y < 0.0 || distorted.y > 1.0) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    float r = texture(uScene, distorted + vec2(0.001 * uIntensity, 0.0)).r;
    float g = texture(uScene, distorted).g;
    float b = texture(uScene, distorted - vec2(0.001 * uIntensity, 0.0)).b;
    color = vec3(r, g, b);

    color = clamp(color, 0.0, 1.0);
    FragColor = vec4(color, 1.0);
}
)";

static const char* FISHEYE_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uTime;
uniform float uIntensity;
uniform vec2 uResolution;

void main() {
    vec2 uv = vTexCoord;
    vec2 centered = uv - 0.5;
    float dist = length(centered);
    float strength = uIntensity * 0.5;
    float distortion = 1.0 + strength * dist * dist;
    vec2 distorted = centered / distortion + 0.5;

    if (distorted.x < 0.0 || distorted.x > 1.0 || distorted.y < 0.0 || distorted.y > 1.0) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 color = texture(uScene, distorted).rgb;
    float vignette = 1.0 - dist * 0.5 * uIntensity;
    color *= vignette;
    FragColor = vec4(color, 1.0);
}
)";

static const char* MOTION_BLUR_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uTime;
uniform float uIntensity;
uniform vec2 uResolution;

void main() {
    vec2 uv = vTexCoord;
    vec2 texel = 1.0 / uResolution;

    vec3 result = vec3(0.0);
    int samples = int(mix(3.0, 24.0, uIntensity));
    vec2 dir = vec2(cos(uTime * 0.5), sin(uTime * 0.5)) * uIntensity * 0.02;

    for (int i = -samples/2; i <= samples/2; i++) {
        float t = float(i) / float(max(samples/2, 1));
        result += texture(uScene, uv + dir * t).rgb;
    }
    result /= float(samples);

    FragColor = vec4(result, 1.0);
}
)";

static const char* SHARPEN_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uTime;
uniform float uIntensity;
uniform vec2 uResolution;

void main() {
    vec2 uv = vTexCoord;
    vec2 texel = 1.0 / uResolution;

    vec3 center = texture(uScene, uv).rgb;
    vec3 top    = texture(uScene, uv + vec2( 0.0,  texel.y)).rgb;
    vec3 bottom = texture(uScene, uv + vec2( 0.0, -texel.y)).rgb;
    vec3 left   = texture(uScene, uv + vec2(-texel.x,  0.0)).rgb;
    vec3 right  = texture(uScene, uv + vec2( texel.x,  0.0)).rgb;

    vec3 sharp = center * 2.0 - (top + bottom + left + right) * 0.5;
    vec3 color = mix(center, sharp, uIntensity);
    color = clamp(color, 0.0, 1.0);
    FragColor = vec4(color, 1.0);
}
)";

static const char* EMBOSS_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uTime;
uniform float uIntensity;
uniform vec2 uResolution;

void main() {
    vec2 uv = vTexCoord;
    vec2 texel = 1.0 / uResolution;

    vec3 s = texture(uScene, uv + vec2(-texel.x, -texel.y)).rgb * -2.0
           + texture(uScene, uv + vec2( 0.0,     -texel.y)).rgb * -1.0
           + texture(uScene, uv + vec2( texel.x, -texel.y)).rgb *  0.0
           + texture(uScene, uv + vec2(-texel.x,  0.0)).rgb      * -1.0
           + texture(uScene, uv).rgb                                *  1.0
           + texture(uScene, uv + vec2( texel.x,  0.0)).rgb       *  1.0
           + texture(uScene, uv + vec2(-texel.x,  texel.y)).rgb   *  0.0
           + texture(uScene, uv + vec2( 0.0,      texel.y)).rgb   *  1.0
           + texture(uScene, uv + vec2( texel.x,  texel.y)).rgb   *  2.0;

    float gray = dot(s, vec3(0.299, 0.587, 0.114));
    gray = gray * 0.5 + 0.5;
    vec3 color = texture(uScene, uv).rgb;
    color = mix(color, vec3(gray), uIntensity);
    FragColor = vec4(color, 1.0);
}
)";

PostProcessor& PostProcessor::Instance() {
    static PostProcessor instance;
    return instance;
}

void PostProcessor::Init(int width, int height) {
    m_width = width;
    m_height = height;

    m_sceneFbo.Init(width, height);

    m_vhsShader = std::make_unique<Shader>();
    m_vhsShader->LoadFromSource(FULLSCREEN_VERTEX, VHS_FRAGMENT);

    m_vignetteShader = std::make_unique<Shader>();
    m_vignetteShader->LoadFromSource(FULLSCREEN_VERTEX, VIGNETTE_FRAGMENT);

    m_dreamShader = std::make_unique<Shader>();
    m_dreamShader->LoadFromSource(FULLSCREEN_VERTEX, DREAM_FRAGMENT);

    m_grayscaleShader = std::make_unique<Shader>();
    m_grayscaleShader->LoadFromSource(FULLSCREEN_VERTEX, GRAYSCALE_FRAGMENT);

    m_invertShader = std::make_unique<Shader>();
    m_invertShader->LoadFromSource(FULLSCREEN_VERTEX, INVERT_FRAGMENT);

    m_blurShader = std::make_unique<Shader>();
    m_blurShader->LoadFromSource(FULLSCREEN_VERTEX, BLUR_FRAGMENT);

    m_pixelateShader = std::make_unique<Shader>();
    m_pixelateShader->LoadFromSource(FULLSCREEN_VERTEX, PIXELATE_FRAGMENT);

    m_edgeDetectShader = std::make_unique<Shader>();
    m_edgeDetectShader->LoadFromSource(FULLSCREEN_VERTEX, EDGE_DETECT_FRAGMENT);

    m_sepiaShader = std::make_unique<Shader>();
    m_sepiaShader->LoadFromSource(FULLSCREEN_VERTEX, SEPIA_FRAGMENT);

    m_bloomShader = std::make_unique<Shader>();
    m_bloomShader->LoadFromSource(FULLSCREEN_VERTEX, BLOOM_FRAGMENT);

    m_crtShader = std::make_unique<Shader>();
    m_crtShader->LoadFromSource(FULLSCREEN_VERTEX, CRT_FRAGMENT);

    m_fishEyeShader = std::make_unique<Shader>();
    m_fishEyeShader->LoadFromSource(FULLSCREEN_VERTEX, FISHEYE_FRAGMENT);

    m_motionBlurShader = std::make_unique<Shader>();
    m_motionBlurShader->LoadFromSource(FULLSCREEN_VERTEX, MOTION_BLUR_FRAGMENT);

    m_sharpenShader = std::make_unique<Shader>();
    m_sharpenShader->LoadFromSource(FULLSCREEN_VERTEX, SHARPEN_FRAGMENT);

    m_embossShader = std::make_unique<Shader>();
    m_embossShader->LoadFromSource(FULLSCREEN_VERTEX, EMBOSS_FRAGMENT);

    std::vector<Vertex> quadVerts = {
        {{-1.0f,  1.0f, 0.0f}, {0,0,1}, {0,1}},
        {{-1.0f, -1.0f, 0.0f}, {0,0,1}, {0,0}},
        {{ 1.0f, -1.0f, 0.0f}, {0,0,1}, {1,0}},
        {{ 1.0f,  1.0f, 0.0f}, {0,0,1}, {1,1}},
    };
    std::vector<GLuint> quadIndices = {0, 1, 2, 0, 2, 3};
    m_fullscreenQuad.Upload(quadVerts, quadIndices);

    LOG_INFO() << "[PostProcessor] Initialized (" << width << "x" << height << ")";
}

void PostProcessor::Shutdown() {
    m_vhsShader.reset();
    m_vignetteShader.reset();
    m_dreamShader.reset();
    m_grayscaleShader.reset();
    m_invertShader.reset();
    m_blurShader.reset();
    m_pixelateShader.reset();
    m_edgeDetectShader.reset();
    m_sepiaShader.reset();
    m_bloomShader.reset();
    m_crtShader.reset();
    m_fishEyeShader.reset();
    m_motionBlurShader.reset();
    m_sharpenShader.reset();
    m_embossShader.reset();
    m_fullscreenQuad.Destroy();
    m_sceneFbo.Destroy();
    LOG_INFO() << "[PostProcessor] Shutdown";
}

void PostProcessor::Resize(int width, int height) {
    m_width = width;
    m_height = height;
    m_sceneFbo.Resize(width, height);
}

void PostProcessor::BeginCapture() {
    m_sceneFbo.Bind();
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessor::EndCapture() {
    m_sceneFbo.Unbind();
    glViewport(0, 0, m_width, m_height);

    if (m_currentEffect == PostEffect::None) {
        glad_glBindFramebuffer(GL_READ_FRAMEBUFFER, m_sceneFbo.GetFboId());
        glad_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glad_glBlitFramebuffer(0, 0, m_width, m_height,
            0, 0, m_width, m_height,
            GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glad_glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    RenderFullscreenQuad();
}

void PostProcessor::RenderFullscreenQuad() {
    Shader* shader = nullptr;

    switch (m_currentEffect) {
        case PostEffect::VHS:        shader = m_vhsShader.get(); break;
        case PostEffect::Dream:      shader = m_dreamShader.get(); break;
        case PostEffect::Vignette:   shader = m_vignetteShader.get(); break;
        case PostEffect::Grayscale:  shader = m_grayscaleShader.get(); break;
        case PostEffect::Invert:     shader = m_invertShader.get(); break;
        case PostEffect::Blur:       shader = m_blurShader.get(); break;
        case PostEffect::Pixelate:   shader = m_pixelateShader.get(); break;
        case PostEffect::EdgeDetect: shader = m_edgeDetectShader.get(); break;
        case PostEffect::Sepia:      shader = m_sepiaShader.get(); break;
        case PostEffect::Bloom:      shader = m_bloomShader.get(); break;
        case PostEffect::CRT:        shader = m_crtShader.get(); break;
        case PostEffect::FishEye:    shader = m_fishEyeShader.get(); break;
        case PostEffect::MotionBlur: shader = m_motionBlurShader.get(); break;
        case PostEffect::Sharpen:    shader = m_sharpenShader.get(); break;
        case PostEffect::Emboss:     shader = m_embossShader.get(); break;
        default: return;
    }

    if (!shader) return;

    shader->Bind();
    shader->SetInt("uScene", 0);
    shader->SetFloat("uTime", m_time);
    shader->SetFloat("uIntensity", m_intensity);
    shader->SetVec2("uResolution", glm::vec2(m_width, m_height));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_sceneFbo.GetColorTexture());

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    m_fullscreenQuad.Draw();
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    shader->Unbind();
}

void PostProcessor::SetEffectByName(const std::string& name) {
    m_currentEffect = GetEffectFromName(name);
}

std::vector<std::string> PostProcessor::GetAvailableEffectNames() {
    return {"none", "vhs", "dream", "vignette", "grayscale", "invert", "blur",
            "pixelate", "edgeDetect", "sepia", "bloom", "crt", "fishEye",
            "motionBlur", "sharpen", "emboss"};
}

std::string PostProcessor::GetEffectName(PostEffect effect) {
    switch (effect) {
        case PostEffect::None:       return "none";
        case PostEffect::VHS:        return "vhs";
        case PostEffect::Dream:      return "dream";
        case PostEffect::Vignette:   return "vignette";
        case PostEffect::Grayscale:  return "grayscale";
        case PostEffect::Invert:     return "invert";
        case PostEffect::Blur:       return "blur";
        case PostEffect::Pixelate:   return "pixelate";
        case PostEffect::EdgeDetect: return "edgeDetect";
        case PostEffect::Sepia:      return "sepia";
        case PostEffect::Bloom:      return "bloom";
        case PostEffect::CRT:        return "crt";
        case PostEffect::FishEye:    return "fishEye";
        case PostEffect::MotionBlur: return "motionBlur";
        case PostEffect::Sharpen:    return "sharpen";
        case PostEffect::Emboss:     return "emboss";
    }
    return "none";
}

PostEffect PostProcessor::GetEffectFromName(const std::string& name) {
    if (name == "vhs")       return PostEffect::VHS;
    if (name == "dream")     return PostEffect::Dream;
    if (name == "vignette")  return PostEffect::Vignette;
    if (name == "grayscale") return PostEffect::Grayscale;
    if (name == "invert")    return PostEffect::Invert;
    if (name == "blur")      return PostEffect::Blur;
    if (name == "pixelate")  return PostEffect::Pixelate;
    if (name == "edgeDetect")return PostEffect::EdgeDetect;
    if (name == "sepia")     return PostEffect::Sepia;
    if (name == "bloom")     return PostEffect::Bloom;
    if (name == "crt")       return PostEffect::CRT;
    if (name == "fishEye")   return PostEffect::FishEye;
    if (name == "motionBlur")return PostEffect::MotionBlur;
    if (name == "sharpen")   return PostEffect::Sharpen;
    if (name == "emboss")    return PostEffect::Emboss;
    return PostEffect::None;
}

void PostProcessor::SetEffectParameter(const std::string& name, float value) {
    if (name == "intensity") m_intensity = value;
}

float PostProcessor::GetEffectParameter(const std::string& name) const {
    if (name == "intensity") return m_intensity;
    return 0.0f;
}

} // namespace planet
