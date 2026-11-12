#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <string>
#include <optional>

#include "json/json.hpp"

namespace ultramodern {
    namespace renderer {
        enum class Resolution {
            Original,
            Original2x,
            Auto,
            OptionCount
        };
        enum class WindowMode {
            Windowed,
            Fullscreen,
            OptionCount
        };
        enum class HUDRatioMode {
            Original,
            Clamp16x9,
            Full,
            OptionCount
        };
        enum class GraphicsApi {
            Auto,
            D3D12,
            Vulkan,
            Metal,
            OptionCount
        };
        enum class AspectRatio {
            Original,
            Expand,
            Manual,
            OptionCount
        };
        enum class Antialiasing {
            None,
            MSAA2X,
            MSAA4X,
            MSAA8X,
            OptionCount
        };
        enum class RefreshRate {
            Original,
            Display,
            Manual,
            OptionCount
        };
        enum class HighPrecisionFramebuffer {
            Auto,
            On,
            Off,
            OptionCount
        };
        enum class TextureFiltering {
            On,
            Off,
            OptionCount
        };
        enum class TransparencyMode {
            Smooth,
            Flicker,
            OptionCount
        };
        enum class AdvancedLighting {
            Off,
            On,
            OptionCount
        };
        enum class SecondBounce {
            Off,
            On,
            OptionCount
        };
        enum class AdvancedWater {
            Off,
            On,
            OptionCount
        };
        // Whether the game skips scenery and characters away from the camera, which also drops their ray traced shadows.
        enum class ObjectCulling {
            Off,
            On,
            OptionCount
        };
        // Upscaler of the ray traced image (Ray Traced Lighting on).
        enum class RtUpscaler {
            Off,
            FSR1,
            FSR3,
            OptionCount
        };
        enum class RtUpscalerQuality {
            NativeAA,
            Quality,
            Balanced,
            Performance,
            UltraPerformance,
            OptionCount
        };
        // Rays per pixel and frame for the sun shadow, light, bounce and occlusion rays (Ray Traced Lighting on).
        enum class RtLightSamples {
            One,
            Two,
            Four,
            OptionCount
        };
        // Edge-aware spatial filter of the ray traced lighting.
        enum class RtDenoiser {
            Off,
            Low,
            High,
            OptionCount
        };
        // How many frames of ray traced lighting are blended together.
        enum class RtAccumulation {
            Responsive,
            Balanced,
            Stable,
            OptionCount
        };

        class GraphicsConfig {
        public:
            bool developer_mode;
            Resolution res_option;
            WindowMode wm_option;
            HUDRatioMode hr_option;
            GraphicsApi api_option;
            AspectRatio ar_option;
            Antialiasing msaa_option;
            RefreshRate rr_option;
            HighPrecisionFramebuffer hpfb_option;
            TextureFiltering tf_option;
            TransparencyMode transparency_option;
            AdvancedLighting advanced_lighting_option;
            SecondBounce second_bounce_option;
            AdvancedWater advanced_water_option;
            ObjectCulling object_culling_option;
            RtUpscaler rt_upscaler_option;
            RtUpscalerQuality rt_upscaler_quality_option;
            // From 0 to 100.
            int rt_upscaler_sharpness;
            RtLightSamples rt_light_samples_option;
            RtDenoiser rt_denoiser_option;
            RtAccumulation rt_accumulation_option;
            int rr_manual_value;
            int ds_option;

            virtual ~GraphicsConfig() = default;

            auto operator<=>(const GraphicsConfig& rhs) const = default;
        };

        const GraphicsConfig& get_graphics_config();
        void set_graphics_config(const GraphicsConfig& new_config);

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::Resolution, {
            {ultramodern::renderer::Resolution::Original, "Original"},
            {ultramodern::renderer::Resolution::Original2x, "Original2x"},
            {ultramodern::renderer::Resolution::Auto, "Auto"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::WindowMode, {
            {ultramodern::renderer::WindowMode::Windowed, "Windowed"},
            {ultramodern::renderer::WindowMode::Fullscreen, "Fullscreen"}
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::HUDRatioMode, {
            {ultramodern::renderer::HUDRatioMode::Original, "Original"},
            {ultramodern::renderer::HUDRatioMode::Clamp16x9, "Clamp16x9"},
            {ultramodern::renderer::HUDRatioMode::Full, "Full"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::GraphicsApi, {
            {ultramodern::renderer::GraphicsApi::Auto, "Auto"},
            {ultramodern::renderer::GraphicsApi::D3D12, "D3D12"},
            {ultramodern::renderer::GraphicsApi::Vulkan, "Vulkan"},
            {ultramodern::renderer::GraphicsApi::Metal, "Metal"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::AspectRatio, {
            {ultramodern::renderer::AspectRatio::Original, "Original"},
            {ultramodern::renderer::AspectRatio::Expand, "Expand"},
            {ultramodern::renderer::AspectRatio::Manual, "Manual"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::Antialiasing, {
            {ultramodern::renderer::Antialiasing::None, "None"},
            {ultramodern::renderer::Antialiasing::MSAA2X, "MSAA2X"},
            {ultramodern::renderer::Antialiasing::MSAA4X, "MSAA4X"},
            {ultramodern::renderer::Antialiasing::MSAA8X, "MSAA8X"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::RefreshRate, {
            {ultramodern::renderer::RefreshRate::Original, "Original"},
            {ultramodern::renderer::RefreshRate::Display, "Display"},
            {ultramodern::renderer::RefreshRate::Manual, "Manual"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::HighPrecisionFramebuffer, {
            {ultramodern::renderer::HighPrecisionFramebuffer::Auto, "Auto"},
            {ultramodern::renderer::HighPrecisionFramebuffer::On, "On"},
            {ultramodern::renderer::HighPrecisionFramebuffer::Off, "Off"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::TextureFiltering, {
            {ultramodern::renderer::TextureFiltering::On, "On"},
            {ultramodern::renderer::TextureFiltering::Off, "Off"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::TransparencyMode, {
            {ultramodern::renderer::TransparencyMode::Smooth, "Smooth"},
            {ultramodern::renderer::TransparencyMode::Flicker, "Flicker"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::AdvancedLighting, {
            {ultramodern::renderer::AdvancedLighting::Off, "Off"},
            {ultramodern::renderer::AdvancedLighting::On, "On"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::SecondBounce, {
            {ultramodern::renderer::SecondBounce::Off, "Off"},
            {ultramodern::renderer::SecondBounce::On, "On"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::AdvancedWater, {
            {ultramodern::renderer::AdvancedWater::Off, "Off"},
            {ultramodern::renderer::AdvancedWater::On, "On"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::ObjectCulling, {
            {ultramodern::renderer::ObjectCulling::Off, "Off"},
            {ultramodern::renderer::ObjectCulling::On, "On"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::RtUpscaler, {
            {ultramodern::renderer::RtUpscaler::Off, "Off"},
            {ultramodern::renderer::RtUpscaler::FSR1, "FSR1"},
            {ultramodern::renderer::RtUpscaler::FSR3, "FSR3"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::RtUpscalerQuality, {
            {ultramodern::renderer::RtUpscalerQuality::NativeAA, "NativeAA"},
            {ultramodern::renderer::RtUpscalerQuality::Quality, "Quality"},
            {ultramodern::renderer::RtUpscalerQuality::Balanced, "Balanced"},
            {ultramodern::renderer::RtUpscalerQuality::Performance, "Performance"},
            {ultramodern::renderer::RtUpscalerQuality::UltraPerformance, "UltraPerformance"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::RtLightSamples, {
            {ultramodern::renderer::RtLightSamples::One, "One"},
            {ultramodern::renderer::RtLightSamples::Two, "Two"},
            {ultramodern::renderer::RtLightSamples::Four, "Four"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::RtDenoiser, {
            {ultramodern::renderer::RtDenoiser::Off, "Off"},
            {ultramodern::renderer::RtDenoiser::Low, "Low"},
            {ultramodern::renderer::RtDenoiser::High, "High"},
        });

        NLOHMANN_JSON_SERIALIZE_ENUM(ultramodern::renderer::RtAccumulation, {
            {ultramodern::renderer::RtAccumulation::Responsive, "Responsive"},
            {ultramodern::renderer::RtAccumulation::Balanced, "Balanced"},
            {ultramodern::renderer::RtAccumulation::Stable, "Stable"},
        });
    }
}

#endif
