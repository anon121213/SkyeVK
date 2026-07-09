#pragma once

#include <cstdint>
#include <type_traits>

namespace Sky::RHI
{

// ============================================================
// Bitmask enum helper
// ------------------------------------------------------------
// enum class blocks operator| by default (type-safety).
// For combinable flags (Usage, ShaderStage, ...) — expand this macro next to
// the enum. It defines & | ^ ~ operators and the has() test function.
// ============================================================
#define SKY_RHI_DEFINE_BITMASK_OPS(EnumType)                                                   \
    constexpr EnumType  operator|(EnumType a, EnumType b) noexcept {                            \
        using U = std::underlying_type_t<EnumType>;                                             \
        return static_cast<EnumType>(static_cast<U>(a) | static_cast<U>(b));                    \
    }                                                                                           \
    constexpr EnumType  operator&(EnumType a, EnumType b) noexcept {                            \
        using U = std::underlying_type_t<EnumType>;                                             \
        return static_cast<EnumType>(static_cast<U>(a) & static_cast<U>(b));                    \
    }                                                                                           \
    constexpr EnumType  operator^(EnumType a, EnumType b) noexcept {                            \
        using U = std::underlying_type_t<EnumType>;                                             \
        return static_cast<EnumType>(static_cast<U>(a) ^ static_cast<U>(b));                    \
    }                                                                                           \
    constexpr EnumType  operator~(EnumType a) noexcept {                                        \
        using U = std::underlying_type_t<EnumType>;                                             \
        return static_cast<EnumType>(~static_cast<U>(a));                                       \
    }                                                                                           \
    constexpr EnumType& operator|=(EnumType& a, EnumType b) noexcept { return a = a | b; }      \
    constexpr EnumType& operator&=(EnumType& a, EnumType b) noexcept { return a = a & b; }      \
    constexpr EnumType& operator^=(EnumType& a, EnumType b) noexcept { return a = a ^ b; }      \
    constexpr bool has(EnumType flags, EnumType bit) noexcept {                                 \
        using U = std::underlying_type_t<EnumType>;                                             \
        return (static_cast<U>(flags) & static_cast<U>(bit)) != 0;                              \
    }

// ============================================================
// Backend selection — which graphics API to use
// ============================================================
enum class BackendType : uint32_t
{
    Auto,       // Pick automatically based on platform (Vulkan on Mac/Linux, D3D12 on Windows)
    Vulkan,     // Vulkan 1.2+ backend (currently the only implemented one)
    D3D12,      // Direct3D 12 backend (future work, Windows only)
    Metal,      // Apple Metal backend (maybe future for optimal macOS/iOS support)
};

// ============================================================
// Queue types — which GPU queue family to submit commands to
// ------------------------------------------------------------
// Not a bitmask, used as a selector when acquiring a queue from Device.
// In Vulkan this maps to queue families, in D3D12 to command list types.
// ============================================================
enum class QueueType : uint32_t
{
    Graphics,   // Main graphics queue — supports draw, compute, transfer implicitly
    Compute,    // Async compute queue — runs in parallel with graphics on supported GPUs
    Transfer,   // Dedicated transfer queue — for async streaming of textures and buffers
};

// ============================================================
// Format — pixel format for textures, vertex attributes, and buffer views
// ------------------------------------------------------------
// Subset of what Vulkan/D3D12 support. Naming follows Vulkan convention:
//   <channels><bits>_<numeric-format>
// Numeric formats:
//   UNORM   — unsigned normalized [0, 1] (typical for color)
//   SNORM   — signed normalized [-1, 1] (typical for normals)
//   UINT    — unsigned integer (no normalization, integer sampling in shader)
//   SINT    — signed integer
//   SFLOAT  — signed floating-point
//   UFLOAT  — unsigned floating-point (rare, only packed formats)
//   SRGB    — UNORM + gamma correction applied by hardware on read/write
// ============================================================
enum class Format : uint32_t
{
    Undefined,              // No format specified — used as null/default value

    // ---- 8-bit ----
    R8_UNORM,               // 8-bit red channel, unsigned normalized — common for masks / grayscale
    R8_SNORM,               // 8-bit red channel, signed normalized — rare, sometimes for signed masks
    R8_UINT,                // 8-bit red channel as raw unsigned integer — for lookup textures / IDs
    RG8_UNORM,              // 16-bit two-channel unsigned normalized — motion vectors (low-precision)
    RG8_SNORM,              // 16-bit two-channel signed normalized — compact 2D vectors
    RGBA8_UNORM,            // 32-bit RGBA unsigned normalized — most common color texture format
    RGBA8_SNORM,            // 32-bit RGBA signed normalized — occasionally for signed data
    RGBA8_UINT,              // 32-bit RGBA as raw unsigned integers — for GPU-side ID buffers
    RGBA8_SRGB,             // 32-bit RGBA + sRGB gamma — standard for authored color textures (albedo)
    BGRA8_UNORM,            // BGRA channel order — used by some legacy image loaders
    BGRA8_SRGB,             // BGRA + sRGB — very common swapchain format on Windows/macOS

    // ---- 16-bit per channel ----
    R16_UNORM,              // 16-bit red channel unsigned normalized — high-precision heightmaps
    R16_SFLOAT,             // 16-bit red channel half-float — depth or scalar HDR
    RG16_UNORM,             // 32-bit two-channel unsigned normalized — precise motion vectors
    RG16_SFLOAT,            // 32-bit two-channel half-float — screen-space vectors, encoded normals
    RGBA16_UNORM,           // 64-bit RGBA unsigned normalized — HDR-ish intermediate targets
    RGBA16_SFLOAT,          // 64-bit RGBA half-float — standard HDR intermediate render target

    // ---- 32-bit per channel ----
    R32_UINT,               // 32-bit red channel unsigned integer — for indices, IDs, counters
    R32_SFLOAT,             // 32-bit red channel float — full-precision scalar textures
    RG32_SFLOAT,            // 64-bit two-channel float — 2D vertex positions, UVs at high precision
    RGB32_SFLOAT,           // 96-bit three-channel float — typical 3D vertex position attribute
    RGBA32_SFLOAT,          // 128-bit RGBA float — maximum precision for accumulation buffers

    // ---- Packed HDR ----
    R11G11B10_UFLOAT,       // 32-bit packed HDR — R11+G11+B10 unsigned floats, half memory of RGBA16F
    RGB10A2_UNORM,          // 32-bit packed — 10-bit RGB + 2-bit alpha, HDR monitors output format

    // ---- Depth / stencil ----
    D16_UNORM,              // 16-bit depth — cheap depth for shadow maps at low precision
    D32_SFLOAT,             // 32-bit float depth — high-precision main depth buffer
    D24_UNORM_S8_UINT,      // 24-bit depth + 8-bit stencil — common Windows GPU depth format
    D32_SFLOAT_S8_UINT,     // 32-bit float depth + 8-bit stencil — maximum precision + stencil ops

    // ---- Block-compressed (BC family, DXT successors) ----
    BC1_RGBA_UNORM,         // DXT1 4-bpp — RGBA with 1-bit alpha, cheapest compression
    BC1_RGBA_SRGB,          // BC1 + sRGB — compressed authored color textures
    BC3_RGBA_UNORM,         // DXT5 8-bpp — RGBA with smooth alpha, common for textures with alpha
    BC3_RGBA_SRGB,          // BC3 + sRGB — compressed color textures with transparency
    BC5_RG_UNORM,           // Two-channel 8-bpp — best format for compressed normal maps
    BC6H_RGB_UFLOAT,        // HDR compressed 8-bpp — for cubemaps and HDR environment textures
    BC7_RGBA_UNORM,         // Modern high-quality 8-bpp — near-lossless RGBA, replaces BC3
    BC7_RGBA_SRGB,          // BC7 + sRGB — highest quality compressed authored color
};

// ============================================================
// Memory type — semantic hint about where a resource lives
// ------------------------------------------------------------
// Not a direct mapping to Vulkan memory type indices — this is a high-level
// abstraction. Backend (VMA) picks the actual memory type based on usage.
// ============================================================
enum class MemoryType : uint32_t
{
    GpuOnly,        // Device-local, not CPU-visible — fastest for GPU (vertex/index buffers, textures, render targets)
    CpuToGpu,       // Host-visible + coherent — CPU writes frequently, GPU reads (per-frame UBOs, dynamic buffers)
    GpuToCpu,       // Host-visible + cached — GPU writes, CPU reads back (screenshots, readback queries)
    CpuOnly,        // Host memory only, transient — staging buffers used once and freed
};

// ============================================================
// Buffer usage flags — bitmask specifying what a buffer can be used for
// ------------------------------------------------------------
// A single buffer often serves multiple roles (e.g. Vertex | TransferDst
// when uploading via staging). Combine flags with operator|.
// ============================================================
enum class BufferUsage : uint32_t
{
    None            = 0,        // No usage specified — will fail resource creation
    TransferSrc     = 1 <<  0,  // Source for vkCmdCopyBuffer — needed for staging buffers
    TransferDst     = 1 <<  1,  // Destination for copy — needed when uploading via staging
    Vertex          = 1 <<  2,  // Bound as vertex buffer via vkCmdBindVertexBuffers
    Index           = 1 <<  3,  // Bound as index buffer via vkCmdBindIndexBuffer
    Uniform         = 1 <<  4,  // Bound as uniform buffer (UBO) — small read-only shader data
    Storage         = 1 <<  5,  // Bound as storage buffer (SSBO) — larger read/write shader data
    Indirect        = 1 <<  6,  // Contains indirect draw commands — GPU-driven rendering
    AccelStructure  = 1 <<  7,  // Storage for ray tracing acceleration structure (BLAS/TLAS)
    ShaderBindTable = 1 <<  8,  // Shader binding table used by ray tracing pipelines
};
SKY_RHI_DEFINE_BITMASK_OPS(BufferUsage)

// ============================================================
// Texture usage flags — bitmask specifying what a texture can be used for
// ============================================================
enum class TextureUsage : uint32_t
{
    None                    = 0,        // No usage specified — will fail resource creation
    TransferSrc             = 1 << 0,   // Source for copy/blit operations — needed for readback / mipmap generation
    TransferDst             = 1 << 1,   // Destination for copy/blit — needed for texture upload
    Sampled                 = 1 << 2,   // Read via sampler in shader — standard texture usage
    Storage                 = 1 << 3,   // Read/write in compute shader as image — for post-processing
    ColorAttachment         = 1 << 4,   // Bound as color render target in a render pass
    DepthStencilAttachment  = 1 << 5,   // Bound as depth/stencil render target in a render pass
    InputAttachment         = 1 << 6,   // Read in a later subpass via input attachment (tile-based deferred)
    Transient               = 1 << 7,   // Never stored to VRAM — Frame Graph may alias its memory
};
SKY_RHI_DEFINE_BITMASK_OPS(TextureUsage)

// ============================================================
// Texture type — shape and dimensionality of a texture
// ============================================================
enum class TextureType : uint32_t
{
    Type1D,             // Single-row texture — rarely used, mostly for lookup tables
    Type2D,             // Standard 2D texture — most common (albedo, normal, etc.)
    Type3D,             // Volumetric texture — 3D LUTs, volume rendering, fog textures
    TypeCube,           // Six-face cubemap — environment / skybox / IBL prefilter
    Type1DArray,        // Array of 1D textures — rarely used
    Type2DArray,        // Array of 2D textures — texture atlases, cascaded shadow maps
    TypeCubeArray,      // Array of cubemaps — point-light shadow atlas, IBL probe grid
};

// ============================================================
// Image layout — GPU memory arrangement for an image
// ------------------------------------------------------------
// Different GPU operations require different memory layouts (tiled, compressed).
// Frame Graph inserts transitions automatically — consumers rarely set these directly.
// ============================================================
enum class ImageLayout : uint32_t
{
    Undefined,                  // Initial state — contents may be discarded, safe as source when contents don't matter
    General,                    // Read/write from shader storage — flexibly usable but less optimized
    ColorAttachment,            // Being written by fragment shader as color output
    DepthStencilAttachment,     // Being written for depth or stencil tests
    DepthStencilReadOnly,       // Read as depth in shader (shadow lookups, depth-based effects)
    ShaderReadOnly,             // Sampled as regular texture in shader
    TransferSrc,                // Source of a copy/blit operation
    TransferDst,                // Destination of a copy/blit operation
    PresentSrc,                 // Ready to be presented via vkQueuePresentKHR — swapchain images end here
};

// ============================================================
// Shader stage flags — bitmask indicating pipeline stages
// ------------------------------------------------------------
// Pipeline layouts and push constants specify which stages access resources.
// Combine with operator| for descriptors visible to multiple stages.
// ============================================================
enum class ShaderStage : uint32_t
{
    None            = 0,        // No stage — invalid, will fail pipeline layout creation
    Vertex          = 1 << 0,   // Vertex shader — runs once per vertex
    Fragment        = 1 << 1,   // Fragment (pixel) shader — runs once per rasterized fragment
    Compute         = 1 << 2,   // Compute shader — general-purpose GPU computation
    Geometry        = 1 << 3,   // Geometry shader — generates primitives from vertices (rare in modern engines)
    TessControl     = 1 << 4,   // Tessellation control — sets tessellation levels per patch
    TessEvaluation  = 1 << 5,   // Tessellation evaluation — positions vertices in tessellated patches
    Task            = 1 << 6,   // Task shader — first stage of mesh shader pipeline
    Mesh            = 1 << 7,   // Mesh shader — replaces vertex/geometry stages with compute-like model
    RayGen          = 1 << 8,   // Ray generation — entry point of a ray tracing pipeline
    Miss            = 1 << 9,   // Ray miss — invoked when a ray hits nothing
    ClosestHit      = 1 << 10,  // Ray closest hit — invoked for the nearest intersection
    AnyHit          = 1 << 11,  // Ray any-hit — invoked for every candidate intersection (alpha testing)
    Intersection    = 1 << 12,  // Custom ray intersection — for procedural geometry (spheres, etc.)
    Callable        = 1 << 13,  // Callable shader — invocable from other RT stages, like a function

    AllGraphics   = Vertex | Fragment | Geometry | TessControl | TessEvaluation, // Convenience mask for classic graphics pipeline
    AllRayTracing = RayGen | Miss | ClosestHit | AnyHit | Intersection | Callable, // Convenience mask for RT pipelines
    All           = ~0u,        // All stages — used for push constants shared across everything
};
SKY_RHI_DEFINE_BITMASK_OPS(ShaderStage)

// ============================================================
// Primitive topology — how vertices are assembled into primitives
// ============================================================
enum class PrimitiveTopology : uint32_t
{
    PointList,          // Each vertex is a point — for particle systems and debug rendering
    LineList,           // Each pair of vertices forms a line — for wireframe debug
    LineStrip,          // Connected polyline — first pair is a line, each next vertex extends it
    TriangleList,       // Each triplet forms a triangle — most common, standard mesh format
    TriangleStrip,      // Connected triangles — first triplet, then each new vertex forms a triangle with the previous two
    TriangleFan,        // Fan from first vertex — deprecated on modern APIs, some backends may not support
    PatchList,          // Patches of N control points — required input for tessellation shaders
};

// ============================================================
// Rasterization polygon fill mode
// ============================================================
enum class PolygonMode : uint32_t
{
    Fill,               // Fill the interior of polygons — standard rendering
    Line,               // Wireframe rendering — requires GPU feature fillModeNonSolid
    Point,              // Vertices as points only — rare, mostly for debug
};

// ============================================================
// Face culling mode — which faces to discard before fragment shading
// ============================================================
enum class CullMode : uint32_t
{
    None,               // Do not cull — draw both front and back faces (for 2-sided geometry)
    Front,              // Cull front-facing triangles — used for shadow map back-face rendering
    Back,               // Cull back-facing triangles — standard for opaque meshes
    FrontAndBack,       // Cull everything — effectively disables geometry, rarely useful
};

// ============================================================
// Front-face winding order — which winding is considered "front"
// ============================================================
enum class FrontFace : uint32_t
{
    CounterClockwise,   // CCW winding is front — OpenGL / Vulkan native default
    Clockwise,          // CW winding is front — Direct3D convention, common in DCC exports
};

// ============================================================
// Comparison operator — used for depth test, stencil test, sampler compare
// ============================================================
enum class CompareOp : uint32_t
{
    Never,              // Test always fails — write nothing
    Less,               // Pass when new value is less than existing — standard depth test
    Equal,              // Pass on exact equality — for stencil match tests
    LessOrEqual,        // Less-or-equal — useful for depth prepass where equality means "same fragment"
    Greater,            // Greater than existing — for reversed-Z depth buffer (better precision)
    NotEqual,           // Any value except exact match — for stencil masking
    GreaterOrEqual,     // Greater-or-equal — for reversed-Z with equality allowed
    Always,             // Test always passes — effectively disables the test
};

// ============================================================
// Blend factor — coefficient applied to source or destination in blending
// ============================================================
enum class BlendFactor : uint32_t
{
    Zero,                       // Multiply by 0 — factor contributes nothing
    One,                        // Multiply by 1 — factor passes through unchanged
    SrcColor,                   // Multiply by source RGB — for pre-multiplied effects
    OneMinusSrcColor,           // Multiply by (1 - source RGB) — inverse
    DstColor,                   // Multiply by destination RGB — reads existing framebuffer
    OneMinusDstColor,           // Multiply by (1 - destination RGB)
    SrcAlpha,                   // Multiply by source alpha — standard alpha blending source
    OneMinusSrcAlpha,           // Multiply by (1 - source alpha) — standard alpha blending destination
    DstAlpha,                   // Multiply by destination alpha
    OneMinusDstAlpha,           // Multiply by (1 - destination alpha)
    ConstantColor,              // Use a pipeline-set constant RGB — configurable at bind time
    OneMinusConstantColor,      // Use inverse of pipeline-set constant RGB
    ConstantAlpha,              // Use pipeline-set constant alpha
    OneMinusConstantAlpha,      // Use inverse of pipeline-set constant alpha
    SrcAlphaSaturate,           // Special: min(src alpha, 1 - dst alpha) — for edge antialiasing
};

// ============================================================
// Blend operation — how source and destination values are combined
// ============================================================
enum class BlendOp : uint32_t
{
    Add,                // src * srcFactor + dst * dstFactor — standard blend, alpha blending, additive
    Subtract,           // src * srcFactor - dst * dstFactor — dimming effects
    ReverseSubtract,    // dst * dstFactor - src * srcFactor — reverse dimming
    Min,                // Component-wise min(src, dst) — for min-value accumulation buffers
    Max,                // Component-wise max(src, dst) — for max-value accumulation buffers
};

// ============================================================
// Color component mask — which color channels to write to attachment
// ============================================================
enum class ColorComponent : uint32_t
{
    None    = 0,                        // Write nothing — effectively disables color output
    R       = 1 << 0,                   // Write red channel
    G       = 1 << 1,                   // Write green channel
    B       = 1 << 2,                   // Write blue channel
    A       = 1 << 3,                   // Write alpha channel
    RGB     = R | G | B,                // Write RGB, ignore alpha — for opaque passes
    RGBA    = R | G | B | A,            // Write all channels — the typical default
};
SKY_RHI_DEFINE_BITMASK_OPS(ColorComponent)

// ============================================================
// Attachment load operation — what to do with attachment at start of pass
// ============================================================
enum class AttachmentLoadOp : uint32_t
{
    Load,               // Preserve existing content — expensive on tile-based GPUs, needed for overlay passes
    Clear,              // Clear to a specified clearValue — fast on all GPUs, standard for main pass
    DontCare,           // Content is undefined — fastest, use when the pass writes every pixel
};

// ============================================================
// Attachment store operation — what to do with attachment at end of pass
// ============================================================
enum class AttachmentStoreOp : uint32_t
{
    Store,              // Write result back to VRAM — needed for anything read later or presented
    DontCare,           // Discard result — cheap, use for depth after main pass or transient targets
};

// ============================================================
// Sampler texel filter mode
// ============================================================
enum class Filter : uint32_t
{
    Nearest,            // Point sampling — pixel-art, integer scaling, no interpolation
    Linear,             // Bilinear / trilinear interpolation — standard for photorealistic textures
};

// ============================================================
// Sampler mipmap filter mode — how to blend between mip levels
// ============================================================
enum class SamplerMipmapMode : uint32_t
{
    Nearest,            // Pick the nearest mip level — cheap, may show mip transitions
    Linear,             // Trilinear blend between two mip levels — smooth transitions, standard
};

// ============================================================
// Sampler address mode — behavior when UV falls outside [0, 1]
// ============================================================
enum class AddressMode : uint32_t
{
    Repeat,             // Tile the texture infinitely — for tiled textures (bricks, ground)
    MirroredRepeat,     // Tile with mirroring on each repeat — avoids seams on some patterns
    ClampToEdge,        // Clamp to the last row/column of pixels — standard for UI, IBL
    ClampToBorder,      // Return a specified border color outside [0, 1] — for shadow maps
    MirrorClampToEdge,  // Mirror once then clamp — rarely used
};

// ============================================================
// Swapchain present mode — how frames are queued for display
// ============================================================
enum class PresentMode : uint32_t
{
    Immediate,          // No vsync — tearing possible, lowest latency, for benchmarking
    Mailbox,            // Triple-buffered vsync — no tearing, low latency, not always supported
    Fifo,               // Traditional vsync — always available per Vulkan spec
    FifoRelaxed,        // vsync with a tearing fallback when a frame misses — reduces stutter
};

// ============================================================
// Surface color space — how output colors are interpreted by the display
// ============================================================
enum class ColorSpace : uint32_t
{
    SrgbNonLinear,      // Standard SDR sRGB — every regular monitor
    HDR10,              // HDR10 output — Rec.2020 primaries with PQ transfer, TVs and HDR monitors
    Extended_SRGB,      // Windows extended-sRGB HDR — scRGB-like, values above 1.0 are HDR
};

// ============================================================
// Index buffer element size
// ============================================================
enum class IndexType : uint32_t
{
    UInt16,             // 16-bit indices — up to 65535 vertices per mesh, half the memory
    UInt32,             // 32-bit indices — up to ~4 billion vertices, needed for large scenes / GPU-driven
};

} // namespace Sky::RHI
