// This library implements a Pineda-style software rasterizer inspired from Larrabee's rasterizer.
// See "A Parallel Algorithm for Polygon Rasterization", by Juan Pineda, SIGGRAPH '88:
// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.157.4621&rep=rep1&type=pdf
// Also see Michael Abrash's article "Rasterization on Larrabee":
// https://software.intel.com/en-us/articles/rasterization-on-larrabee
// For a modern take on this algorithm, see Fabian Giesen's GPU pipeline and Software Occlusion Culling blog series:
// https://fgiesen.wordpress.com/2011/07/09/a-trip-through-the-graphics-pipeline-2011-index/
// https://fgiesen.wordpress.com/2013/02/17/optimizing-sw-occlusion-culling-index/

#include "dvr_rasterizer.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

// Configuration
// ------------------
// Which instruction set to use
// Haswell New Instructions (AVX2)
//#define USE_HSWni

// Can disable perfcounters since they cost performance (QPC is slow)
#define ENABLE_PERFCOUNTERS
// ------------------

// Sized according to the Larrabee rasterizer's description
// The tile size must be up to 128x128
//    this is because any edge that isn't trivially accepted or rejected
//    can be rasterized with 32 bits inside a 128x128 tile
// moved it down to 64x64 since it allows more parallelism
#define TILE_WIDTH_IN_PIXELS 64
#define COARSE_BLOCK_WIDTH_IN_PIXELS 16
#define FINE_BLOCK_WIDTH_IN_PIXELS 4

// Convenience
#define PIXELS_PER_TILE (TILE_WIDTH_IN_PIXELS * TILE_WIDTH_IN_PIXELS)
#define PIXELS_PER_COARSE_BLOCK (COARSE_BLOCK_WIDTH_IN_PIXELS * COARSE_BLOCK_WIDTH_IN_PIXELS)
#define PIXELS_PER_FINE_BLOCK (FINE_BLOCK_WIDTH_IN_PIXELS * FINE_BLOCK_WIDTH_IN_PIXELS)

#define TILE_WIDTH_IN_COARSE_BLOCKS (TILE_WIDTH_IN_PIXELS / COARSE_BLOCK_WIDTH_IN_PIXELS)
#define COARSE_BLOCK_WIDTH_IN_FINE_BLOCKS (COARSE_BLOCK_WIDTH_IN_PIXELS / FINE_BLOCK_WIDTH_IN_PIXELS)
#define COARSE_BLOCKS_PER_TILE (PIXELS_PER_TILE / PIXELS_PER_COARSE_BLOCK)
 
// The swizzle masks, using alternating yxyxyx bit pattern for morton-code swizzling pixels in a tile.
// This makes the pixels morton code swizzled within every rasterization level (fine/coarse/tile)
// The tiles themselves are stored row major.
// For examples of this concept, see:
// https://software.intel.com/en-us/node/514045
// https://msdn.microsoft.com/en-us/library/windows/desktop/dn770442%28v=vs.85%29.aspx
#define TILE_X_SWIZZLE_MASK (0x55555555 & (PIXELS_PER_TILE - 1))
#define TILE_Y_SWIZZLE_MASK (0xAAAAAAAA & (PIXELS_PER_TILE - 1))

#define COARSE_BLOCK_X_SWIZZLE_MASK (TILE_X_SWIZZLE_MASK & (PIXELS_PER_COARSE_BLOCK - 1))
#define COARSE_BLOCK_Y_SWIZZLE_MASK (TILE_Y_SWIZZLE_MASK & (PIXELS_PER_COARSE_BLOCK - 1))

#define FINE_BLOCK_X_SWIZZLE_MASK (TILE_X_SWIZZLE_MASK & (PIXELS_PER_FINE_BLOCK - 1))
#define FINE_BLOCK_Y_SWIZZLE_MASK (TILE_Y_SWIZZLE_MASK & (PIXELS_PER_FINE_BLOCK - 1))

// If there are too many commands and this buffer gets filled up,
// then the command buffer for that tile must be flushed.
#define TILE_COMMAND_BUFFER_SIZE_IN_DWORDS 1024

// parallel bit deposit low-order source bits according to mask bits
#ifdef USE_HSWni
__forceinline uint32_t pdep_u32(uint32_t source, uint32_t mask)
{
    // AVX2 implementation
    return _pdep_u32(source, mask);
}
#else
__forceinline uint32_t pdep_u32(uint32_t source, uint32_t mask)
{
    // generic implementation
    uint32_t temp = source;
    uint32_t dest = 0;
    uint32_t m = 0, k = 0;
    while (m < 32)
    {
        if (mask & (1 << m))
        {
            dest = (dest & ~(1 << m)) | (((temp & (1 << k)) >> k) << m);
            k = k + 1;
        }
        m = m+ 1;
    }
    return dest;
}
#endif

// parallel bit extract low-order source bits according to mask bits
#ifdef USE_HSWni
__forceinline uint32_t pext_u32(uint32_t source, uint32_t mask)
{
    // AVX2 implementation
    return _pext_u32(source, mask);
}
#else
__forceinline uint32_t pext_u32(uint32_t source, uint32_t mask)
{
    // generic implementation
    uint32_t temp = source;
    uint32_t dest = 0;
    uint32_t m = 0, k = 0;
    while (m < 32)
    {
        if (mask & (1 << m))
        {
            dest = (dest & ~(1 << k)) | (((temp & (1 << m)) >> m) << k);
            k = k + 1;
        }
        m = m + 1;
    }
    return dest;
}
#endif

// count leading zeros (32 bits)
#ifdef USE_HSWni
__forceinline uint32_t lzcnt(uint32_t value)
{
    // AVX2 implementation
    return __lzcnt(value);
}
#elif defined(_MSC_VER)
__forceinline uint32_t lzcnt(uint32_t value)
{
    // MSVC implementation
    unsigned long index;
    if (_BitScanReverse(&index, value))
    {
        return (32 - 1) - index;
    }
    else
    {
        return 32;
    }
}
#else
__forceinline uint32_t lzcnt(uint32_t value)
{
    // generic implementation
    uint32_t i;
    for (i = 0; i < 32; i++)
    {
        if (value & 0x80000000)
            break;
        
        value = value << 1;
    }
    return i;
}
#endif

// count leading zeros (64 bits)
#ifdef USE_HSWni
__forceinline uint64_t lzcnt64(uint64_t value)
{
    return __lzcnt64(value);
}
#elif defined(_MSC_VER)
__forceinline uint64_t lzcnt64(uint64_t value)
{
    // MSVC implementation
    unsigned long index;
    if (_BitScanReverse64(&index, value))
    {
        return (64 - 1) - index;
    }
    else
    {
        return 64;
    }
}
#else
__forceinline uint64_t lzcnt64(uint64_t value)
{
    // generic implementation
    uint64_t i;
    for (i = 0; i < 64; i++)
    {
        if (value & 0x8000000000000000ULL)
            break;

        value = value << 1ULL;
    }
    return i;
}
#endif

#ifdef ENABLE_PERFCOUNTERS
#ifdef _WIN32
uint64_t qpc()
{
    LARGE_INTEGER pc;
    QueryPerformanceCounter(&pc);
    return pc.QuadPart;
}
#else
"Missing QPC implementation for this platform!";
#endif

#ifdef _WIN32
uint64_t qpf()
{
    LARGE_INTEGER pf;
    QueryPerformanceFrequency(&pf);
    return pf.QuadPart;
}
#else
"Missing QPF implementation for this platform!";
#endif
#endif

static int32_t s1516_add(int32_t a, int32_t b)
{
    int32_t result;
    result = a + b;
    return result;
}

static int32_t s1516_add_sat(int32_t a, int32_t b)
{
    int32_t result;
    int64_t tmp;

    tmp = (int64_t)a + (int64_t)b;
    if (tmp > (int64_t)0x7FFFFFFF)
        tmp = (int64_t)0x7FFFFFFF;
    if (tmp < -(int64_t)0x80000000)
        tmp = -(int64_t)0x80000000;
    result = (int32_t)tmp;

    return result;
}

// saturate to range of int32_t
static int32_t s1516_sat(int64_t x)
{
    if (x >(int64_t)0x7FFFFFFF) return (int64_t)0x7FFFFFFF;
    else if (x < -(int64_t)0x80000000) return -(int64_t)0x80000000;
    else return (int32_t)x;
}

static int32_t s1516_mul(int32_t a, int32_t b)
{
    int32_t result;
    int64_t temp;

    temp = (int64_t)a * (int64_t)b;
    // Rounding: mid values are rounded up
    temp += 1 << 15;
    // Correct by dividing by base and saturate result
    result = s1516_sat(temp >> 16);

    return result;
}

static int32_t s1516_div(int32_t a, int32_t b)
{
    int32_t result;
    int64_t temp;

    // pre-multiply by the base
    temp = (int64_t)a << 16;
    // Rounding: mid values are rounded up (down for negative values)
    if ((temp >= 0 && b >= 0) || (temp < 0 && b < 0))
        temp += b / 2;
    else
        temp -= b / 2;
    result = (int32_t)(temp/ b);

    return result;
}

static int32_t s1516_fma(int32_t a, int32_t b, int32_t c)
{
    int32_t result;
    int64_t temp;

    temp = (int64_t)a * (int64_t)b + ((int64_t)c << 16);

    // Rounding: mid values are rounded up
    temp += 1 << 15;

    // Correct by dividing by base and saturate result
    result = s1516_sat(temp >> 16);

    return result;
}

static float s1516_get(int32_t a)
{
	return (float)a / (float)0xFFFF;
}

static int32_t s1516_int(int32_t i)
{
    return i << 16;
}

static int32_t s1516_flt(float f)
{
    return (int32_t)(f * 0xffff);
}

static int32_t s168_s1516(int32_t s1516)
{
    return s1516_div(s1516, s1516_int(256));
}

static float s168_get(int32_t a)
{
	return (float)a * 256.f / (float)0xFFFF;
}

static float s168_get(int64_t a)
{
	return (float)a * 256.f / (float)0xFFFF;
}

typedef struct tile_cmdbuf_t
{
    // start and past-the-end of the allocation for the buffer
    uint32_t* cmdbuf_start;
    uint32_t* cmdbuf_end;
    // the next location where to read and write commands
    uint32_t* cmdbuf_read;
    uint32_t* cmdbuf_write;
} tile_cmdbuf_t;

typedef enum tilecmd_id_t
{
    tilecmd_id_resetbuf, // when there's not enough space in the command ring buffer and the ring loops
    tilecmd_id_drawsmalltri,
    tilecmd_id_drawlargetri_0edgemask,
    tilecmd_id_drawlargetri_7edgemask = tilecmd_id_drawlargetri_0edgemask + 7,
    tilecmd_id_cleartile
} tilecmd_id_t;

typedef struct framebuffer_perfcounters_t
{
#ifdef ENABLE_PERFCOUNTERS
    uint64_t clipping;
    uint64_t common_setup;
    uint64_t smalltri_setup;
    uint64_t largetri_setup;
#else
    uint64_t unused;
#endif
} framebuffer_perfcounters_t;

const char* kFramebufferPerfcounterNames[] = {
#ifdef ENABLE_PERFCOUNTERS
    "clipping",
    "common_setup",
    "smalltri_setup",
    "largetri_setup"
#else
    ""
#endif
};

static_assert(sizeof(kFramebufferPerfcounterNames) / sizeof(*kFramebufferPerfcounterNames) == sizeof(framebuffer_perfcounters_t) / sizeof(uint64_t), "Names for perfcounters");

typedef struct framebuffer_tile_perfcounters_t
{
#ifdef ENABLE_PERFCOUNTERS
    uint64_t smalltri_raster;
    uint64_t largetri_raster;
    uint64_t clear;
#else
    uint64_t unused;
#endif
} framebuffer_tile_perfcounters_t;

const char* kFramebufferTilePerfcounterNames[] = {
#ifdef ENABLE_PERFCOUNTERS
    "smalltri_raster",
    "largetri_raster",
    "clear",
#else
    ""
#endif
};

static_assert(sizeof(kFramebufferTilePerfcounterNames) / sizeof(*kFramebufferTilePerfcounterNames) == sizeof(framebuffer_tile_perfcounters_t) / sizeof(uint64_t), "Names for perfcounters");

typedef struct xyzw_i32_t
{
    int32_t x, y, z, w;
} xyzw_i32_t;

typedef struct tilecmd_drawsmalltri_t
{
    uint32_t tilecmd_id;
    int32_t edges[3];
    int32_t edge_dxs[3];
    int32_t edge_dys[3];
    int32_t vert_Zs[3];
    uint32_t max_Z, min_Z;
    uint32_t shifted_triarea2;
    uint32_t rcp_triarea2_mantissa;
    int32_t rcp_triarea2_rshift;
} tilecmd_drawsmalltri_t;

typedef struct tilecmd_drawtile_t
{
    uint32_t tilecmd_id;
    int32_t edges[3];
    int32_t edge_dxs[3];
    int32_t edge_dys[3];
    int32_t shifted_es[3];
    int32_t vert_Zs[3];
    uint32_t max_Z, min_Z;
    uint32_t shifted_triarea2;
    uint32_t rcp_triarea2_mantissa;
    int32_t rcp_triarea2_rshift;
} tilecmd_drawtile_t;

typedef struct tilecmd_cleartile_t
{
    uint32_t tilecmd_id;
    uint32_t color;
} tilecmd_cleartile_t;

typedef struct framebuffer_t
{
    uint32_t* backbuffer;
    uint32_t* depthbuffer;
    
    uint32_t* tile_cmdpool;
    tile_cmdbuf_t* tile_cmdbufs;
    
    int32_t width_in_pixels;
    int32_t height_in_pixels;

    int32_t width_in_tiles;
    int32_t height_in_tiles;
    int32_t total_num_tiles;
    
    // num_tiles_per_row * num_pixels_per_tile
    int32_t pixels_per_row_of_tiles;

    // pixels_per_row_of_tiles * num_tile_rows
    int32_t pixels_per_slice;

#ifdef ENABLE_PERFCOUNTERS
    // performance counters
    uint64_t pc_frequency;
    framebuffer_perfcounters_t perfcounters;
    framebuffer_tile_perfcounters_t* tile_perfcounters;
#endif

} framebuffer_t;

framebuffer_t* new_framebuffer(int32_t width, int32_t height)
{
    // limits of the rasterizer's precision
    // this is based on an analysis of the range of results of the 2D cross product between two fixed16.8 numbers.
    assert(width < 16384);
    assert(height < 16384);

    framebuffer_t* fb = (framebuffer_t*)malloc(sizeof(framebuffer_t));
    assert(fb);

    fb->width_in_pixels = width;
    fb->height_in_pixels = height;

    // pad framebuffer up to size of next tile
    // that way the rasterization code doesn't have to handlep otential out of bounds access after tile binning
    int32_t padded_width_in_pixels = (width + (TILE_WIDTH_IN_PIXELS - 1)) & -TILE_WIDTH_IN_PIXELS;
    int32_t padded_height_in_pixels = (height + (TILE_WIDTH_IN_PIXELS - 1)) & -TILE_WIDTH_IN_PIXELS;
    
    fb->width_in_tiles = padded_width_in_pixels / TILE_WIDTH_IN_PIXELS;
    fb->height_in_tiles = padded_height_in_pixels / TILE_WIDTH_IN_PIXELS;
    fb->total_num_tiles = fb->width_in_tiles * fb->height_in_tiles;

    fb->pixels_per_row_of_tiles = padded_width_in_pixels * TILE_WIDTH_IN_PIXELS;
    fb->pixels_per_slice = padded_height_in_pixels / TILE_WIDTH_IN_PIXELS * fb->pixels_per_row_of_tiles;

    fb->backbuffer = (uint32_t*)_aligned_malloc(fb->pixels_per_slice * sizeof(uint32_t), 32);
    assert(fb->backbuffer);

    // clear to black/transparent initially
    memset(fb->backbuffer, 0, fb->pixels_per_slice * sizeof(uint32_t));

    fb->depthbuffer = (uint32_t*)_aligned_malloc(fb->pixels_per_slice * sizeof(uint32_t), 32);
    assert(fb->depthbuffer);
    
    // clear to infinity initially
    memset(fb->depthbuffer, 0xFF, fb->pixels_per_slice * sizeof(uint32_t));

    // allocate command lists for each tile
    fb->tile_cmdpool = (uint32_t*)malloc(fb->total_num_tiles * TILE_COMMAND_BUFFER_SIZE_IN_DWORDS * sizeof(uint32_t));
    assert(fb->tile_cmdpool);

    fb->tile_cmdbufs = (tile_cmdbuf_t*)malloc(fb->total_num_tiles * sizeof(tile_cmdbuf_t));
    assert(fb->tile_cmdbufs);

    // command lists are circular queues that are initially empty
    for (int32_t i = 0; i < fb->total_num_tiles; i++)
    {
        fb->tile_cmdbufs[i].cmdbuf_start = &fb->tile_cmdpool[i * TILE_COMMAND_BUFFER_SIZE_IN_DWORDS];
        fb->tile_cmdbufs[i].cmdbuf_end = fb->tile_cmdbufs[i].cmdbuf_start + TILE_COMMAND_BUFFER_SIZE_IN_DWORDS;
        fb->tile_cmdbufs[i].cmdbuf_read = fb->tile_cmdbufs[i].cmdbuf_start;
        fb->tile_cmdbufs[i].cmdbuf_write = fb->tile_cmdbufs[i].cmdbuf_start;
    }

#ifdef ENABLE_PERFCOUNTERS
    fb->pc_frequency = qpf();

    memset(&fb->perfcounters, 0, sizeof(framebuffer_perfcounters_t));

    fb->tile_perfcounters = (framebuffer_tile_perfcounters_t*)malloc(fb->total_num_tiles * sizeof(framebuffer_tile_perfcounters_t));
    memset(fb->tile_perfcounters, 0, fb->total_num_tiles * sizeof(framebuffer_tile_perfcounters_t));
#endif
    
    return fb;
}

void delete_framebuffer(framebuffer_t* fb)
{
    if (!fb)
        return;

#ifdef ENABLE_PERFCOUNTERS
    free(fb->tile_perfcounters);
#endif

    free(fb->tile_cmdbufs);
    free(fb->tile_cmdpool);
    _aligned_free(fb->depthbuffer);
    _aligned_free(fb->backbuffer);
    free(fb);
}

static void draw_fine_block_smalltri_scalar(framebuffer_t* fb, int32_t fine_dst_i, const tilecmd_drawsmalltri_t* drawcmd)
{
    int32_t edge_dxs[3];
    int32_t edge_dys[3];
    for (int32_t v = 0; v < 3; v++)
    {
        edge_dxs[v] = drawcmd->edge_dxs[v];
        edge_dys[v] = drawcmd->edge_dys[v];
    }

    int32_t edges[3];
    for (int32_t v = 0; v < 3; v++)
    {
        edges[v] = drawcmd->edges[v];
    }
    
    for (
        uint32_t px_y = 0, px_y_bits = 0;
        px_y < FINE_BLOCK_WIDTH_IN_PIXELS;
        px_y++, px_y_bits = (px_y_bits - FINE_BLOCK_Y_SWIZZLE_MASK) & FINE_BLOCK_Y_SWIZZLE_MASK)
    {
        int32_t edges_row[3];
        for (int32_t v = 0; v < 3; v++)
        {
            edges_row[v] = edges[v];
        }

        for (
            uint32_t px_x = 0, px_x_bits = 0;
            px_x < FINE_BLOCK_WIDTH_IN_PIXELS;
            px_x++, px_x_bits = (px_x_bits - FINE_BLOCK_X_SWIZZLE_MASK) & FINE_BLOCK_X_SWIZZLE_MASK)
        {
            int32_t dst_i = fine_dst_i + (px_y_bits | px_x_bits);

            int32_t pixel_discarded = 0;
            for (int32_t v = 0; v < 3; v++)
            {
                if (edges_row[v] >= 0)
                {
                    pixel_discarded = 1;
                    break;
                }
            }

            if (!pixel_discarded)
            {
                uint32_t rcp_triarea2_mantissa = drawcmd->rcp_triarea2_mantissa;
                int32_t rcp_triarea2_rshift = drawcmd->rcp_triarea2_rshift;

                // note: off by one because -1 maps to 0
                uint32_t shifted_e2 = -edges_row[2] - 1;
                uint32_t shifted_e0 = -edges_row[0] - 1;
                if (rcp_triarea2_rshift < 0)
                {
                    shifted_e2 = shifted_e2 << -rcp_triarea2_rshift;
                    shifted_e0 = shifted_e0 << -rcp_triarea2_rshift;
                }
                else
                {
                    shifted_e2 = shifted_e2 >> rcp_triarea2_rshift;
                    shifted_e0 = shifted_e0 >> rcp_triarea2_rshift;
                }

                // clamp to triangle area
                if (shifted_e2 > drawcmd->shifted_triarea2)
                    shifted_e2 = drawcmd->shifted_triarea2;

                if (shifted_e0 > drawcmd->shifted_triarea2)
                    shifted_e0 = drawcmd->shifted_triarea2;

                assert((shifted_e0 & 0xFFFF) == shifted_e0);
                assert((shifted_e2 & 0xFFFF) == shifted_e2);
                assert((rcp_triarea2_mantissa & 0xFFFF) == rcp_triarea2_mantissa);

                // compute non-perspective-correct barycentrics for vertices 1 and 2
                uint32_t u = (shifted_e2 * rcp_triarea2_mantissa) >> 15;
                uint32_t v = (shifted_e0 * rcp_triarea2_mantissa) >> 15;

                if (u + v > 0xFFFF)
                    v = 0xFFFF - u;

                assert(u >= 0 && u <= 0xFFFF);
                assert(v >= 0 && v <= 0xFFFF);

                // not related to vertex w. Just third barycentric. Bad naming.
                uint32_t w = 0xFFFF - u - v;
                assert(w >= 0 && w <= 0xFFFF);

                assert(u + v + w == 0xFFFF);

                // compute interpolated depth
                uint32_t pixel_Z = (drawcmd->vert_Zs[0] << 16)
                    + u * (drawcmd->vert_Zs[1] - drawcmd->vert_Zs[0])
                    + v * (drawcmd->vert_Zs[2] - drawcmd->vert_Zs[0]);

                assert(pixel_Z >= drawcmd->min_Z << 16);
                assert(pixel_Z <= drawcmd->max_Z << 16);

				printf("         draw pixel, fine_dst_i=%d, px_y_bits=0x%X, px_x_bits=0x%X | %d\n", fine_dst_i, px_y_bits, px_x_bits, dst_i);

                if (pixel_Z < fb->depthbuffer[dst_i])
                {
                    fb->depthbuffer[dst_i] = pixel_Z;
                    fb->backbuffer[dst_i] = (0xFF << 24) | ((w * 0xFF / 0xFFFF) << 16) | ((u * 0xFF / 0xFFFF) << 8) | (v * 0xFF / 0xFFFF);
                }
            }

            for (int32_t v = 0; v < 3; v++)
            {
                edges_row[v] += edge_dxs[v];
            }
        }

        for (int32_t v = 0; v < 3; v++)
        {
            edges[v] += edge_dys[v];
        }
    }
}

static void draw_coarse_block_smalltri_scalar(framebuffer_t* fb, int32_t coarse_dst_i, const tilecmd_drawsmalltri_t* drawcmd)
{
    int32_t fine_edge_dxs[3];
    int32_t fine_edge_dys[3];
    for (int32_t v = 0; v < 3; v++)
    {
        fine_edge_dxs[v] = drawcmd->edge_dxs[v] * FINE_BLOCK_WIDTH_IN_PIXELS;
        fine_edge_dys[v] = drawcmd->edge_dys[v] * FINE_BLOCK_WIDTH_IN_PIXELS;
    }

    int32_t edges[3];
    for (int32_t v = 0; v < 3; v++)
    {
        edges[v] = drawcmd->edges[v];
    }

    int32_t edge_trivRejs[3];
    for (int32_t v = 0; v < 3; v++)
    {
        edge_trivRejs[v] = drawcmd->edges[v];
        if (fine_edge_dxs[v] < 0) edge_trivRejs[v] += fine_edge_dxs[v];
        if (fine_edge_dys[v] < 0) edge_trivRejs[v] += fine_edge_dys[v];
    }

    const uint32_t mask_x = pdep_u32(-FINE_BLOCK_WIDTH_IN_PIXELS, COARSE_BLOCK_X_SWIZZLE_MASK);
    const uint32_t mask_y = pdep_u32(-FINE_BLOCK_WIDTH_IN_PIXELS, COARSE_BLOCK_Y_SWIZZLE_MASK);

    for (
        uint32_t fine_y = 0, fine_y_bits = 0;
        fine_y < COARSE_BLOCK_WIDTH_IN_FINE_BLOCKS;
        fine_y++, fine_y_bits = (fine_y_bits - mask_y) & mask_y)
    {
        int32_t edges_row[3];
        for (int32_t v = 0; v < 3; v++)
        {
            edges_row[v] = edges[v];
        }

        int32_t edge_row_trivRejs[3];
        for (int32_t v = 0; v < 3; v++)
        {
            edge_row_trivRejs[v] = edge_trivRejs[v];
        }

        for (
            uint32_t fine_x = 0, fine_x_bits = 0;
            fine_x < COARSE_BLOCK_WIDTH_IN_FINE_BLOCKS;
            fine_x++, fine_x_bits = (fine_x_bits - mask_x) & mask_x)
        {
            // trivial reject if at least one edge doesn't cover the coarse block at all
            int32_t trivially_rejected = 0;
            for (int32_t v = 0; v < 3; v++)
            {
                if (edge_row_trivRejs[v] >= 0)
                {
                    trivially_rejected = 1;
                    break;
                }
            }

            if (!trivially_rejected)
            {
                tilecmd_drawsmalltri_t fbargs = *drawcmd;

                for (int32_t v = 0; v < 3; v++)
                {
                    fbargs.edges[v] = edges_row[v];
                }

                int32_t dst_i = coarse_dst_i + (fine_y_bits | fine_x_bits);
				printf("      draw fine block, coarse_dst_i=%d, fine_x_bits=0x%X, find_y_bits=0x%X | dst_i=%d\n", 
					coarse_dst_i, fine_x_bits, fine_y_bits, dst_i);
                draw_fine_block_smalltri_scalar(fb, dst_i, &fbargs);
            }

            for (int32_t v = 0; v < 3; v++)
            {
                edges_row[v] += fine_edge_dxs[v];
            }

            for (int32_t v = 0; v < 3; v++)
            {
                edge_row_trivRejs[v] += fine_edge_dxs[v];
            }
        }

        for (int32_t v = 0; v < 3; v++)
        {
            edges[v] += fine_edge_dys[v];
        }

        for (int32_t v = 0; v < 3; v++)
        {
            edge_trivRejs[v] += fine_edge_dys[v];
        }
    }
}

static void draw_tile_smalltri_scalar(framebuffer_t* fb, int32_t tile_id, const tilecmd_drawsmalltri_t* drawcmd)
{
    int32_t coarse_edge_dxs[3];
    int32_t coarse_edge_dys[3];
    for (int32_t v = 0; v < 3; v++)
    {
        coarse_edge_dxs[v] = drawcmd->edge_dxs[v] * COARSE_BLOCK_WIDTH_IN_PIXELS;
        coarse_edge_dys[v] = drawcmd->edge_dys[v] * COARSE_BLOCK_WIDTH_IN_PIXELS;
    }

    int32_t edges[3];
    for (int32_t v = 0; v < 3; v++)
    {
        edges[v] = drawcmd->edges[v];
    }

    int32_t edge_trivRejs[3];
    for (int32_t v = 0; v < 3; v++)
    {
        edge_trivRejs[v] = drawcmd->edges[v];
        if (coarse_edge_dxs[v] < 0) edge_trivRejs[v] += coarse_edge_dxs[v];
        if (coarse_edge_dys[v] < 0) edge_trivRejs[v] += coarse_edge_dys[v];
    }

    const uint32_t mask_x = pdep_u32(-COARSE_BLOCK_WIDTH_IN_PIXELS, TILE_X_SWIZZLE_MASK);
    const uint32_t mask_y = pdep_u32(-COARSE_BLOCK_WIDTH_IN_PIXELS, TILE_Y_SWIZZLE_MASK);

    uint32_t tile_dst_i = tile_id * PIXELS_PER_TILE;

    for (
        uint32_t cb_y = 0, cb_y_bits = 0;
        cb_y < TILE_WIDTH_IN_COARSE_BLOCKS;
        cb_y++, cb_y_bits = (cb_y_bits - mask_y) & mask_y)
    {
        int32_t edges_row[3];
        for (int32_t v = 0; v < 3; v++)
        {
            edges_row[v] = edges[v];
        }

        int32_t edge_row_trivRejs[3];
        for (int32_t v = 0; v < 3; v++)
        {
            edge_row_trivRejs[v] = edge_trivRejs[v];
        }

        for (
            uint32_t cb_x = 0, cb_x_bits = 0;
            cb_x < TILE_WIDTH_IN_COARSE_BLOCKS;
            cb_x++, cb_x_bits = (cb_x_bits - mask_x) & mask_x)
        {
            // trivial reject if at least one edge doesn't cover the coarse block at all
            int32_t trivially_rejected = 0;
            for (int32_t v = 0; v < 3; v++)
            {
                if (edge_row_trivRejs[v] >= 0)
                {
                    trivially_rejected = 1;
                    break;
                }
            }

            if (!trivially_rejected)
            {
                tilecmd_drawsmalltri_t cbargs = *drawcmd;

                for (int32_t v = 0; v < 3; v++)
                {
                    cbargs.edges[v] = edges_row[v];
                }

                uint32_t dst_i = tile_dst_i + (cb_y_bits | cb_x_bits);
				printf("   draw small tri in coarse: tile_id=%d(%d x %d X %d), x_bits=0x%X, y_bits=0x%X | dst_i=%d\n", 
					tile_dst_i, tile_id, TILE_WIDTH_IN_PIXELS, TILE_WIDTH_IN_PIXELS,
					cb_x_bits, cb_y_bits, dst_i);
                draw_coarse_block_smalltri_scalar(fb, dst_i, &cbargs);
            }

            for (int32_t v = 0; v < 3; v++)
            {
                edges_row[v] += coarse_edge_dxs[v];
            }

            for (int32_t v = 0; v < 3; v++)
            {
                edge_row_trivRejs[v] += coarse_edge_dxs[v];
            }
        }

        for (int32_t v = 0; v < 3; v++)
        {
            edges[v] += coarse_edge_dys[v];
        }

        for (int32_t v = 0; v < 3; v++)
        {
            edge_trivRejs[v] += coarse_edge_dys[v];
        }
    }
}

#ifdef USE_HSWni
static void draw_fine_block_smalltri_avx2(framebuffer_t* fb, int32_t fine_dst_i, const tilecmd_drawsmalltri_t* pDrawcmd)
{
    // pixels are stored in fine blocks according to a morton code ordering:
    //  0  1  4  5
    //  2  3  6  7
    //  8  9 12 13
    // 10 11 14 15
    // Thus, the 4x4 fine block is rasterized in two iterations.
    // one iteration for the top 4x2, and one for the bottom 4x2.

    tilecmd_drawsmalltri_t drawcmd = *pDrawcmd;

    __m256i edges[3];
    // for (int32_t i = 0; i < 3; i++)
    {
        __m256i edge0 = _mm256_set1_epi32(drawcmd.edges[0]);
        __m256i edge1 = _mm256_set1_epi32(drawcmd.edges[1]);
        __m256i edge2 = _mm256_set1_epi32(drawcmd.edges[2]);

        int32_t dy0 = drawcmd.edge_dys[0];
        int32_t dy1 = drawcmd.edge_dys[1];
        int32_t dy2 = drawcmd.edge_dys[2];

        // initialize 0 0 dy dy 0 0 dy dy
        __m256i yoffset0 = _mm256_unpacklo_epi64(_mm256_setzero_si256(), _mm256_set1_epi32(dy0));
        __m256i yoffset1 = _mm256_unpacklo_epi64(_mm256_setzero_si256(), _mm256_set1_epi32(dy1));
        __m256i yoffset2 = _mm256_unpacklo_epi64(_mm256_setzero_si256(), _mm256_set1_epi32(dy2));
        
        edge0 = _mm256_add_epi32(edge0, yoffset0);
        edge1 = _mm256_add_epi32(edge1, yoffset1);
        edge2 = _mm256_add_epi32(edge2, yoffset2);

        int32_t dx0 = drawcmd.edge_dxs[0];
        int32_t dx1 = drawcmd.edge_dxs[1];
        int32_t dx2 = drawcmd.edge_dxs[2];
        
        // initialize 0 dx 0 dx dx2 dx3 dx2 dx3
        const __m256i dx2_mask = _mm256_setr_epi32(0, 0, 0, 0, -1, -1, -1, -1);
        
        __m256i mdx0 = _mm256_set1_epi32(dx0);
        __m256i m2dx0 = _mm256_add_epi32(mdx0, mdx0);
        __m256i xoffset0 = _mm256_unpacklo_epi32(_mm256_setzero_si256(), mdx0);
        xoffset0 = _mm256_add_epi32(xoffset0, _mm256_and_si256(m2dx0, dx2_mask));
        edge0 = _mm256_add_epi32(edge0, xoffset0);

        __m256i mdx1 = _mm256_set1_epi32(dx1);
        __m256i m2dx1 = _mm256_add_epi32(mdx1, mdx1);
        __m256i xoffset1 = _mm256_unpacklo_epi32(_mm256_setzero_si256(), mdx1);
        xoffset1 = _mm256_add_epi32(xoffset1, _mm256_and_si256(m2dx1, dx2_mask));
        edge1 = _mm256_add_epi32(edge1, xoffset1);

        __m256i mdx2 = _mm256_set1_epi32(dx2);
        __m256i m2dx2 = _mm256_add_epi32(mdx2, mdx2);
        __m256i xoffset2 = _mm256_unpacklo_epi32(_mm256_setzero_si256(), mdx2);
        xoffset2 = _mm256_add_epi32(xoffset2, _mm256_and_si256(m2dx2, dx2_mask));
        edge2 = _mm256_add_epi32(edge2, xoffset2);

        edges[0] = edge0;
        edges[1] = edge1;
        edges[2] = edge2;
    }

    // pre-compute triarea2 related stuff
    int32_t rcp_triarea2_mantissa = drawcmd.rcp_triarea2_mantissa;
    int32_t rcp_triarea2_rshift = drawcmd.rcp_triarea2_rshift;
    __m256i rcp_triarea2_mantissa256 = _mm256_set1_epi32(rcp_triarea2_mantissa);

    // pre-compute depth related stuff
    __m256i d0 = _mm256_set1_epi32(drawcmd.vert_Zs[0] << 16);
    __m256i dd1 = _mm256_set1_epi32(drawcmd.vert_Zs[1] - drawcmd.vert_Zs[0]);
    __m256i dd2 = _mm256_set1_epi32(drawcmd.vert_Zs[2] - drawcmd.vert_Zs[0]);

    // rasterize both fine block halves
    for (int32_t fineblock_half = 0; fineblock_half < 2; fineblock_half++)
    {
        // compute all pixels passing the edge equation
        __m256i coverage_pass = edges[0];
        coverage_pass = _mm256_and_si256(coverage_pass, edges[1]);
        coverage_pass = _mm256_and_si256(coverage_pass, edges[2]);
        
        int coverage_mask = _mm256_movemask_epi8(coverage_pass) & 0x88888888;

        // early-out if no pixels pass the test
        if (!coverage_mask)
            goto end_fineblock_half;

        // shift edge equations to be on the same scale as the triangle area
        __m256i shifted_e2 = _mm256_sub_epi32(_mm256_sub_epi32(_mm256_setzero_si256(), edges[2]), _mm256_set1_epi32(1));
        __m256i shifted_e0 = _mm256_sub_epi32(_mm256_sub_epi32(_mm256_setzero_si256(), edges[0]), _mm256_set1_epi32(1));
        if (rcp_triarea2_rshift < 0)
        {
            shifted_e2 = _mm256_slli_epi32(shifted_e2, -rcp_triarea2_rshift);
            shifted_e0 = _mm256_slli_epi32(shifted_e0, -rcp_triarea2_rshift);
        }
        else
        {
            shifted_e2 = _mm256_srli_epi32(shifted_e2, rcp_triarea2_rshift);
            shifted_e0 = _mm256_srli_epi32(shifted_e0, rcp_triarea2_rshift);
        }

        // clamp to triangle area
        shifted_e0 = _mm256_min_epi32(_mm256_set1_epi32(drawcmd.shifted_triarea2), shifted_e0);
        shifted_e2 = _mm256_min_epi32(_mm256_set1_epi32(drawcmd.shifted_triarea2), shifted_e2);

        // compute non-perspective-correct barycentrics for vertices 1 and 2
        __m256i u = _mm256_srli_epi32(_mm256_mullo_epi32(shifted_e2, rcp_triarea2_mantissa256), 15);
        __m256i v = _mm256_srli_epi32(_mm256_mullo_epi32(shifted_e0, rcp_triarea2_mantissa256), 15);

        // ensure barycentrics sum to 1
        __m256i one_minus_u = _mm256_sub_epi32(_mm256_set1_epi32(0xFFFF), u);
        v = _mm256_min_epi32(v, one_minus_u);

        // not related to vertex w. Just third barycentric. Bad naming.
        __m256i w = _mm256_sub_epi32(_mm256_set1_epi32(0xFFFF), _mm256_add_epi32(u, v));

        // compute interpolated depth
        __m256i src_depth = d0;
        src_depth = _mm256_add_epi32(src_depth, _mm256_mullo_epi32(u, dd1));
        src_depth = _mm256_add_epi32(src_depth, _mm256_mullo_epi32(v, dd2));

        __m256i dst_depth = _mm256_load_si256((__m256i*)&fb->depthbuffer[fine_dst_i]);
        
        // note: unsigned compare implemented using signed compare, done by subtracting 2^31
        __m256i depth_pass = _mm256_cmpgt_epi32(_mm256_sub_epi32(dst_depth, _mm256_set1_epi32(0x80000000)), _mm256_sub_epi32(src_depth, _mm256_set1_epi32(0x80000000)));

        // combine coverage and depth masks
        depth_pass = _mm256_and_si256(coverage_pass, depth_pass);

        // early out if all depth tests fail
        int depth_pass_mask = _mm256_movemask_epi8(depth_pass);
        if (!depth_pass_mask)
            goto end_fineblock_half;
        
        // blend depth into depthbuffer
        _mm256_maskstore_epi32((int32_t*)&fb->depthbuffer[fine_dst_i], depth_pass, src_depth);

        // set color based on barycentrics.
        __m256i src_color = _mm256_set1_epi32(0xFF << 24);
        src_color = _mm256_or_si256(src_color, _mm256_slli_epi32(_mm256_srli_epi32(_mm256_mullo_epi32(w, _mm256_set1_epi32(0xFF)), 16), 16));
        src_color = _mm256_or_si256(src_color, _mm256_slli_epi32(_mm256_srli_epi32(_mm256_mullo_epi32(u, _mm256_set1_epi32(0xFF)), 16), 8));
        src_color = _mm256_or_si256(src_color, _mm256_slli_epi32(_mm256_srli_epi32(_mm256_mullo_epi32(v, _mm256_set1_epi32(0xFF)), 16), 0));

        // write color into backbuffer
        _mm256_maskstore_epi32((int32_t*)&fb->backbuffer[fine_dst_i], depth_pass, src_color);

    end_fineblock_half:
        // offset edge equations down for the second half
        // for (int32_t i = 0; i < 3; i++)
        {
            __m256i dy2_0 = _mm256_set1_epi32(drawcmd.edge_dys[0] * 2);
            __m256i dy2_1 = _mm256_set1_epi32(drawcmd.edge_dys[1] * 2);
            __m256i dy2_2 = _mm256_set1_epi32(drawcmd.edge_dys[2] * 2);

            edges[0] = _mm256_add_epi32(edges[0], dy2_0);
            edges[1] = _mm256_add_epi32(edges[1], dy2_1);
            edges[2] = _mm256_add_epi32(edges[2], dy2_2);
        }

        // offset destination to the next half of the fine block
        fine_dst_i += PIXELS_PER_FINE_BLOCK / 2;
    }
}
#endif

#ifdef USE_HSWni
static void draw_coarse_block_smalltri_avx2(framebuffer_t* fb, int32_t coarse_dst_i, const tilecmd_drawsmalltri_t* pDrawcmd)
{
    // coarse blocks are made out of 4x4 fine blocks, organized as:
    //  0  1  4  5
    //  2  3  6  7
    //  8  9 12 13
    // 10 11 14 15
    // therefore, coarse blocks are rasterized by shifting around the fine block's edge equations.

    tilecmd_drawsmalltri_t drawcmd = *pDrawcmd;

    __m256i edges[3];
    __m256i edge_trivRejs[3];

    // for (int32_t i = 0; i < 3; i++)
    {
        __m256i edge0 = _mm256_set1_epi32(drawcmd.edges[0]);
        __m256i edge1 = _mm256_set1_epi32(drawcmd.edges[1]);
        __m256i edge2 = _mm256_set1_epi32(drawcmd.edges[2]);

        int32_t dy0 = drawcmd.edge_dys[0] * FINE_BLOCK_WIDTH_IN_PIXELS;
        int32_t dy1 = drawcmd.edge_dys[1] * FINE_BLOCK_WIDTH_IN_PIXELS;
        int32_t dy2 = drawcmd.edge_dys[2] * FINE_BLOCK_WIDTH_IN_PIXELS;

        __m256i mdy0 = _mm256_set1_epi32(dy0);
        __m256i mdy1 = _mm256_set1_epi32(dy1);
        __m256i mdy2 = _mm256_set1_epi32(dy2);

        // initialize 0 0 dy dy 0 0 dy dy
        __m256i yoffset0 = _mm256_unpacklo_epi64(_mm256_setzero_si256(), mdy0);
        __m256i yoffset1 = _mm256_unpacklo_epi64(_mm256_setzero_si256(), mdy1);
        __m256i yoffset2 = _mm256_unpacklo_epi64(_mm256_setzero_si256(), mdy2);

        edge0 = _mm256_add_epi32(edge0, yoffset0);
        edge1 = _mm256_add_epi32(edge1, yoffset1);
        edge2 = _mm256_add_epi32(edge2, yoffset2);

        int32_t dx0 = drawcmd.edge_dxs[0] * FINE_BLOCK_WIDTH_IN_PIXELS;
        int32_t dx1 = drawcmd.edge_dxs[1] * FINE_BLOCK_WIDTH_IN_PIXELS;
        int32_t dx2 = drawcmd.edge_dxs[2] * FINE_BLOCK_WIDTH_IN_PIXELS;

        // initialize 0 dx 0 dx dx2 dx3 dx2 dx3
        const __m256i dx2_mask = _mm256_setr_epi32(0, 0, 0, 0, -1, -1, -1, -1);

        __m256i mdx0 = _mm256_set1_epi32(dx0);
        __m256i m2dx0 = _mm256_add_epi32(mdx0, mdx0);
        __m256i xoffset0 = _mm256_unpacklo_epi32(_mm256_setzero_si256(), mdx0);
        xoffset0 = _mm256_add_epi32(xoffset0, _mm256_and_si256(m2dx0, dx2_mask));
        edge0 = _mm256_add_epi32(edge0, xoffset0);

        __m256i mdx1 = _mm256_set1_epi32(dx1);
        __m256i m2dx1 = _mm256_add_epi32(mdx1, mdx1);
        __m256i xoffset1 = _mm256_unpacklo_epi32(_mm256_setzero_si256(), mdx1);
        xoffset1 = _mm256_add_epi32(xoffset1, _mm256_and_si256(m2dx1, dx2_mask));
        edge1 = _mm256_add_epi32(edge1, xoffset1);

        __m256i mdx2 = _mm256_set1_epi32(dx2);
        __m256i m2dx2 = _mm256_add_epi32(mdx2, mdx2);
        __m256i xoffset2 = _mm256_unpacklo_epi32(_mm256_setzero_si256(), mdx2);
        xoffset2 = _mm256_add_epi32(xoffset2, _mm256_and_si256(m2dx2, dx2_mask));
        edge2 = _mm256_add_epi32(edge2, xoffset2);

        edges[0] = edge0;
        edges[1] = edge1;
        edges[2] = edge2;

        __m256i edge_trivRej0 = edge0;
        __m256i edge_trivRej1 = edge1;
        __m256i edge_trivRej2 = edge2;

        edge_trivRej0 = _mm256_add_epi32(edge_trivRej0, _mm256_min_epi32(_mm256_setzero_si256(), mdx0));
        edge_trivRej1 = _mm256_add_epi32(edge_trivRej1, _mm256_min_epi32(_mm256_setzero_si256(), mdx1));
        edge_trivRej2 = _mm256_add_epi32(edge_trivRej2, _mm256_min_epi32(_mm256_setzero_si256(), mdx2));

        edge_trivRej0 = _mm256_add_epi32(edge_trivRej0, _mm256_min_epi32(_mm256_setzero_si256(), mdy0));
        edge_trivRej1 = _mm256_add_epi32(edge_trivRej1, _mm256_min_epi32(_mm256_setzero_si256(), mdy1));
        edge_trivRej2 = _mm256_add_epi32(edge_trivRej2, _mm256_min_epi32(_mm256_setzero_si256(), mdy2));

        edge_trivRejs[0] = edge_trivRej0;
        edge_trivRejs[1] = edge_trivRej1;
        edge_trivRejs[2] = edge_trivRej2;
    }

    int32_t dst_i = coarse_dst_i;

    for (int32_t coarseblock_half = 0; coarseblock_half < 2; coarseblock_half++)
    {
        // draw each fine block in the coarse block half
        __declspec(align(32)) int32_t fineblock_edges[3][8];
        _mm256_store_si256((__m256i*)&fineblock_edges[0][0], edges[0]);
        _mm256_store_si256((__m256i*)&fineblock_edges[1][0], edges[1]);
        _mm256_store_si256((__m256i*)&fineblock_edges[2][0], edges[2]);

        __m256i trivRej_pass = _mm256_cmpgt_epi32(_mm256_setzero_si256(), edge_trivRejs[0]);
        trivRej_pass = _mm256_and_si256(trivRej_pass, _mm256_cmpgt_epi32(_mm256_setzero_si256(), edge_trivRejs[1]));
        trivRej_pass = _mm256_and_si256(trivRej_pass, _mm256_cmpgt_epi32(_mm256_setzero_si256(), edge_trivRejs[2]));

        int trivRej_pass_mask = _mm256_movemask_epi8(trivRej_pass);
        if (!trivRej_pass_mask)
        {
            dst_i += PIXELS_PER_FINE_BLOCK * 8;
            goto coarseblock_half_end;
        }

        tilecmd_drawsmalltri_t finecmd = drawcmd;
        for (int32_t i = 0; i < 8; i++)
        {
            if (trivRej_pass_mask & (1 << (i * 4)))
            {
                finecmd.edges[0] = fineblock_edges[0][i];
                finecmd.edges[1] = fineblock_edges[1][i];
                finecmd.edges[2] = fineblock_edges[2][i];

                draw_fine_block_smalltri_avx2(fb, dst_i, &finecmd);
                // draw_fine_block_smalltri_scalar(fb, dst_i, &finecmd);
            }

            dst_i += PIXELS_PER_FINE_BLOCK;
        }

    coarseblock_half_end:
        // for (int32_t i = 0; i < 3; i++)
        {
            __m256i dy2_0 = _mm256_set1_epi32(drawcmd.edge_dys[0] * 2 * FINE_BLOCK_WIDTH_IN_PIXELS);
            __m256i dy2_1 = _mm256_set1_epi32(drawcmd.edge_dys[1] * 2 * FINE_BLOCK_WIDTH_IN_PIXELS);
            __m256i dy2_2 = _mm256_set1_epi32(drawcmd.edge_dys[2] * 2 * FINE_BLOCK_WIDTH_IN_PIXELS);

            edges[0] = _mm256_add_epi32(edges[0], dy2_0);
            edges[1] = _mm256_add_epi32(edges[1], dy2_1);
            edges[2] = _mm256_add_epi32(edges[2], dy2_2);

            edge_trivRejs[0] = _mm256_add_epi32(edge_trivRejs[0], dy2_0);
            edge_trivRejs[1] = _mm256_add_epi32(edge_trivRejs[1], dy2_1);
            edge_trivRejs[2] = _mm256_add_epi32(edge_trivRejs[2], dy2_2);
        }

    }
}
#endif

#ifdef USE_HSWni
static void draw_tile_smalltri_avx2(framebuffer_t* fb, int32_t tile_id, const tilecmd_drawsmalltri_t* drawcmd)
{
    // tiles are made out of 4x4 coarse blocks, organized as:
    //  0  1  4  5
    //  2  3  6  7
    //  8  9 12 13
    // 10 11 14 15
    // therefore, tiles are rasterized by shifting around the fine block's edge equations.

    __m256i edges[3];
    __m256i edge_trivRejs[3];
    
    for (int32_t i = 0; i < 3; i++)
    {
        int32_t dx = drawcmd->edge_dxs[i] * COARSE_BLOCK_WIDTH_IN_PIXELS;
        int32_t dy = drawcmd->edge_dys[i] * COARSE_BLOCK_WIDTH_IN_PIXELS;

        edges[i] = _mm256_add_epi32(
            _mm256_set1_epi32(drawcmd->edges[i]), 
            _mm256_setr_epi32(0, dx, dy, dx + dy, dx * 2, dx * 3, dx * 2 + dy, dx * 3 + dy));

        edge_trivRejs[i] = edges[i];
        if (dx < 0) edge_trivRejs[i] = _mm256_add_epi32(edge_trivRejs[i], _mm256_set1_epi32(dx));
        if (dy < 0) edge_trivRejs[i] = _mm256_add_epi32(edge_trivRejs[i], _mm256_set1_epi32(dy));
    }

    int32_t dst_i = tile_id * PIXELS_PER_TILE;

    for (int32_t tile_half = 0; tile_half < 2; tile_half++)
    {
        // draw each coarse block in the tile half
        __declspec(align(32)) int32_t coarseblock_edges[3][8];
        _mm256_store_si256((__m256i*)&coarseblock_edges[0][0], edges[0]);
        _mm256_store_si256((__m256i*)&coarseblock_edges[1][0], edges[1]);
        _mm256_store_si256((__m256i*)&coarseblock_edges[2][0], edges[2]);

        __m256i trivRej_pass = _mm256_cmpgt_epi32(_mm256_setzero_si256(), edge_trivRejs[0]);
        trivRej_pass = _mm256_and_si256(trivRej_pass, _mm256_cmpgt_epi32(_mm256_setzero_si256(), edge_trivRejs[1]));
        trivRej_pass = _mm256_and_si256(trivRej_pass, _mm256_cmpgt_epi32(_mm256_setzero_si256(), edge_trivRejs[2]));

        int trivRej_pass_mask = _mm256_movemask_epi8(trivRej_pass);
        if (!trivRej_pass_mask)
        {
            dst_i += PIXELS_PER_COARSE_BLOCK * 8;
            goto tile_half_end;
        }

        tilecmd_drawsmalltri_t coarsecmd = *drawcmd;
        for (int32_t i = 0; i < 8; i++)
        {
            if (trivRej_pass_mask & (1 << (i*4)))
            {
                coarsecmd.edges[0] = coarseblock_edges[0][i];
                coarsecmd.edges[1] = coarseblock_edges[1][i];
                coarsecmd.edges[2] = coarseblock_edges[2][i];

                draw_coarse_block_smalltri_avx2(fb, dst_i, &coarsecmd);
            }

            dst_i += PIXELS_PER_COARSE_BLOCK;
        }

    tile_half_end:
        for (int32_t i = 0; i < 3; i++)
        {
            __m256i dy2 = _mm256_set1_epi32(drawcmd->edge_dys[i] * COARSE_BLOCK_WIDTH_IN_PIXELS * 2);
            edges[i] = _mm256_add_epi32(edges[i], dy2);
            edge_trivRejs[i] = _mm256_add_epi32(edge_trivRejs[i], dy2);
        }
    }
}
#endif

template<uint32_t TestEdgeMask>
static void draw_fine_block_largetri_scalar(framebuffer_t* fb, int32_t fine_dst_i, const tilecmd_drawtile_t* drawcmd, uint32_t fine_x, uint32_t fine_y)
{
	//printf("      draw fine(%d,%d), edge=%d\n", fine_x, fine_y, TestEdgeMask);

    int32_t edge_dxs[3];
    int32_t edge_dys[3];
    for (int32_t v = 0; v < 3; v++)
    {
        edge_dxs[v] = drawcmd->edge_dxs[v];
        edge_dys[v] = drawcmd->edge_dys[v];
    }

    int32_t edges[3];
    for (int32_t v = 0; v < 3; v++)
    {
        edges[v] = drawcmd->edges[v];
    }

    for (
        uint32_t px_y = 0, px_y_bits = 0;
        px_y < FINE_BLOCK_WIDTH_IN_PIXELS;
        px_y++, px_y_bits = (px_y_bits - FINE_BLOCK_Y_SWIZZLE_MASK) & FINE_BLOCK_Y_SWIZZLE_MASK)
    {
        int32_t edges_row[3];
        for (int32_t v = 0; v < 3; v++)
        {
            edges_row[v] = edges[v];
        }

        for (
            uint32_t px_x = 0, px_x_bits = 0;
            px_x < FINE_BLOCK_WIDTH_IN_PIXELS;
            px_x++, px_x_bits = (px_x_bits - FINE_BLOCK_X_SWIZZLE_MASK) & FINE_BLOCK_X_SWIZZLE_MASK)
        {
            int32_t pixel_discarded = 0;
            for (int32_t v = 0; v < 3; v++)
            {
                if (TestEdgeMask & (1 << v))
                {
                    if (edges_row[v] >= 0)
                    {
                        pixel_discarded = 1;
                        break;
                    }
                }
            }

            if (!pixel_discarded)
            {
				printf("[%d,%d] ", px_x, px_y);

                uint32_t rcp_triarea2_mantissa = drawcmd->rcp_triarea2_mantissa;
                int32_t rcp_triarea2_rshift = drawcmd->rcp_triarea2_rshift;

                // note: off by one because -1 maps to 0
                int32_t shifted_e2 = -edges_row[2] - 1;
                int32_t shifted_e0 = -edges_row[0] - 1;
                if (rcp_triarea2_rshift < 0)
                {
                    shifted_e2 = shifted_e2 << -rcp_triarea2_rshift;
                    shifted_e0 = shifted_e0 << -rcp_triarea2_rshift;
                }
                else
                {
                    shifted_e2 = shifted_e2 >> rcp_triarea2_rshift;
                    shifted_e0 = shifted_e0 >> rcp_triarea2_rshift;
                }

                shifted_e2 += drawcmd->shifted_es[2];
                shifted_e0 += drawcmd->shifted_es[0];

                // clamp to triangle area
                if ((uint32_t)shifted_e2 > drawcmd->shifted_triarea2)
                    shifted_e2 = drawcmd->shifted_triarea2;

                if ((uint32_t)shifted_e0 > drawcmd->shifted_triarea2)
                    shifted_e0 = drawcmd->shifted_triarea2;

                assert((shifted_e0 & 0xFFFF) == shifted_e0);
                assert((shifted_e2 & 0xFFFF) == shifted_e2);
                assert((rcp_triarea2_mantissa & 0xFFFF) == rcp_triarea2_mantissa);

                // compute non-perspective-correct barycentrics for vertices 1 and 2
                uint32_t u = ((uint32_t)shifted_e2 * rcp_triarea2_mantissa) >> 15;
                uint32_t v = ((uint32_t)shifted_e0 * rcp_triarea2_mantissa) >> 15;

                if (u + v > 0xFFFF)
                    v = 0xFFFF - u;

                assert(u >= 0 && u <= 0xFFFF);
                assert(v >= 0 && v <= 0xFFFF);

                // not related to vertex w. Just third barycentric. Bad naming.
                uint32_t w = 0xFFFF - u - v;
                assert(w >= 0 && w <= 0xFFFF);

                assert(u + v + w == 0xFFFF);

                // compute interpolated depth
                uint32_t pixel_Z = (drawcmd->vert_Zs[0] << 16)
                    + u * (drawcmd->vert_Zs[1] - drawcmd->vert_Zs[0])
                    + v * (drawcmd->vert_Zs[2] - drawcmd->vert_Zs[0]);

                assert(pixel_Z >= drawcmd->min_Z << 16);
                assert(pixel_Z <= drawcmd->max_Z << 16);

                int32_t dst_i = fine_dst_i + (px_y_bits | px_x_bits);

                if (pixel_Z < fb->depthbuffer[dst_i])
                {
                    fb->depthbuffer[dst_i] = pixel_Z;
                    fb->backbuffer[dst_i] = (0xFF << 24) | ((w * 0xFF / 0xFFFF) << 16) | ((u * 0xFF / 0xFFFF) << 8) | (v * 0xFF / 0xFFFF);
                }
            }

            for (int32_t v = 0; v < 3; v++)
            {
                edges_row[v] += edge_dxs[v];
            }
        }
        
        for (int32_t v = 0; v < 3; v++)
        {
            edges[v] += edge_dys[v];
        }
    }
	printf("\n");
}

template<uint32_t TestEdgeMask>
static void draw_coarse_block_largetri_scalar(framebuffer_t* fb, int32_t tile_id, int32_t coarse_dst_i, const tilecmd_drawtile_t* drawcmd, uint32_t cb_x, uint32_t cb_y)
{
	//printf("   draw coarse(%d, %d), edge=%d\n", cb_x, cb_y, TestEdgeMask);
	printf("      [Coarse%d,%d], Edges=<%f,%f,%f>, Mask=%d\n", tile_id, cb_x+cb_y*4, 
		s168_get(drawcmd->edges[0]), s168_get(drawcmd->edges[1]), s168_get(drawcmd->edges[2]),
		TestEdgeMask);

	int32_t fine_edge_dxs[3] = { 0 };
	int32_t fine_edge_dys[3] = { 0 };
    for (int32_t v = 0; v < 3; v++)
    {
        fine_edge_dxs[v] = drawcmd->edge_dxs[v] * FINE_BLOCK_WIDTH_IN_PIXELS;
        fine_edge_dys[v] = drawcmd->edge_dys[v] * FINE_BLOCK_WIDTH_IN_PIXELS;
    }

    int32_t edges[3];
    for (int32_t v = 0; v < 3; v++)
    {
        edges[v] = drawcmd->edges[v];
    }

	int32_t edge_trivRejs[3] = { 0 };
    for (int32_t v = 0; v < 3; v++)
    {
        if (TestEdgeMask & (1 << v))
        {
            edge_trivRejs[v] = drawcmd->edges[v];
            if (fine_edge_dxs[v] < 0) edge_trivRejs[v] += fine_edge_dxs[v];
            if (fine_edge_dys[v] < 0) edge_trivRejs[v] += fine_edge_dys[v];
        }
    }
	printf("         reject<%f,%f,%f>\n", s168_get(edge_trivRejs[0]), s168_get(edge_trivRejs[1]), s168_get(edge_trivRejs[2]));

    const uint32_t mask_x = pdep_u32(-FINE_BLOCK_WIDTH_IN_PIXELS, COARSE_BLOCK_X_SWIZZLE_MASK);
    const uint32_t mask_y = pdep_u32(-FINE_BLOCK_WIDTH_IN_PIXELS, COARSE_BLOCK_Y_SWIZZLE_MASK);

    for (uint32_t fine_y = 0, fine_y_bits = 0;
        fine_y < COARSE_BLOCK_WIDTH_IN_FINE_BLOCKS;
        fine_y++, fine_y_bits = (fine_y_bits - mask_y) & mask_y)
    {
        int32_t edges_row[3];
        for (int32_t v = 0; v < 3; v++)
        {
            edges_row[v] = edges[v];
        }

        int32_t edge_row_trivRejs[3];
        for (int32_t v = 0; v < 3; v++)
        {
            if (TestEdgeMask & (1 << v))
            {
                edge_row_trivRejs[v] = edge_trivRejs[v];
            }
        }

        for (
            uint32_t fine_x = 0, fine_x_bits = 0;
            fine_x < COARSE_BLOCK_WIDTH_IN_FINE_BLOCKS;
            fine_x++, fine_x_bits = (fine_x_bits - mask_x) & mask_x)
        {
            // trivial reject if at least one edge doesn't cover the coarse block at all
            int32_t trivially_rejected = 0;
            for (int32_t v = 0; v < 3; v++)
            {
                if (TestEdgeMask & (1 << v))
                {
                    if (edge_row_trivRejs[v] >= 0)
                    {
                        trivially_rejected = 1;
                        break;
                    }
                }
            }

            if (!trivially_rejected)
            {
				printf("            [fine %d,%d]\n", fine_x, fine_y);

                tilecmd_drawtile_t fbargs = *drawcmd;

                for (int32_t v = 0; v < 3; v++)
                {
                    fbargs.edges[v] = edges_row[v];
                }

                int32_t dst_i = coarse_dst_i + (fine_y_bits | fine_x_bits);
                draw_fine_block_largetri_scalar<TestEdgeMask>(fb, dst_i, &fbargs, fine_x, fine_y);
            }

            for (int32_t v = 0; v < 3; v++)
            {
                edges_row[v] += fine_edge_dxs[v];
            }

            for (int32_t v = 0; v < 3; v++)
            {
                if (TestEdgeMask & (1 << v))
                {
                    edge_row_trivRejs[v] += fine_edge_dxs[v];
                }
            }
        }

        for (int32_t v = 0; v < 3; v++)
        {
            edges[v] += fine_edge_dys[v];
        }

        for (int32_t v = 0; v < 3; v++)
        {
            if (TestEdgeMask & (1 << v))
            {
                edge_trivRejs[v] += fine_edge_dys[v];
            }
        }
    }
}

template<uint32_t TestEdgeMask>
static void draw_tile_largetri_scalar(framebuffer_t* fb, int32_t tile_id, const tilecmd_drawtile_t* drawcmd)
{
	printf("---- tile_id=%d, EdgeMask=%d, Edge:[%f,%f,%f]------\n", tile_id, TestEdgeMask, 
		s168_get(drawcmd->edges[0]), s168_get(drawcmd->edges[1]), s168_get(drawcmd->edges[2]));

    int32_t coarse_edge_dxs[3];
    int32_t coarse_edge_dys[3];
    for (int32_t v = 0; v < 3; v++)
    {
        coarse_edge_dxs[v] = drawcmd->edge_dxs[v] * COARSE_BLOCK_WIDTH_IN_PIXELS;
        coarse_edge_dys[v] = drawcmd->edge_dys[v] * COARSE_BLOCK_WIDTH_IN_PIXELS;
    }

    int32_t edges[3];
    for (int32_t v = 0; v < 3; v++)
    {
        edges[v] = drawcmd->edges[v];
    }

	int32_t edge_trivRejs[3] = { 0 };
	int32_t edge_trivAccs[3] = { 0 };
    for (int32_t v = 0; v < 3; v++)
    {
        if (TestEdgeMask & (1 << v))
        {
            edge_trivRejs[v] = drawcmd->edges[v];
            edge_trivAccs[v] = drawcmd->edges[v];
            if (coarse_edge_dxs[v] < 0) edge_trivRejs[v] += coarse_edge_dxs[v];
            if (coarse_edge_dxs[v] > 0) edge_trivAccs[v] += coarse_edge_dxs[v];
            if (coarse_edge_dys[v] < 0) edge_trivRejs[v] += coarse_edge_dys[v];
            if (coarse_edge_dys[v] > 0) edge_trivAccs[v] += coarse_edge_dys[v];
        }
    }

	printf("reject[%f,%f,%f]\n", s168_get(edge_trivRejs[0]), s168_get(edge_trivRejs[1]), s168_get(edge_trivRejs[2]));
	printf("accept[%f,%f,%f]\n", s168_get(edge_trivAccs[0]), s168_get(edge_trivAccs[1]), s168_get(edge_trivAccs[2]));

    const uint32_t mask_x = pdep_u32(-COARSE_BLOCK_WIDTH_IN_PIXELS, TILE_X_SWIZZLE_MASK);
    const uint32_t mask_y = pdep_u32(-COARSE_BLOCK_WIDTH_IN_PIXELS, TILE_Y_SWIZZLE_MASK);

    uint32_t tile_dst_i = tile_id * PIXELS_PER_TILE;

    // figure out which coarse blocks pass the reject and accept tests
    for (
        uint32_t cb_y = 0, cb_y_bits = 0;
        cb_y < TILE_WIDTH_IN_COARSE_BLOCKS; 
        cb_y++, cb_y_bits = (cb_y_bits - mask_y) & mask_y)
    {
        int32_t edges_row[3];
        for (int32_t v = 0; v < 3; v++)
        {
            edges_row[v] = edges[v];
        }

		int32_t edge_row_trivRejs[3] = { 0 };
		int32_t edge_row_trivAccs[3] = { 0 };
        for (int32_t v = 0; v < 3; v++)
        {
            if (TestEdgeMask & (1 << v))
            {
                edge_row_trivRejs[v] = edge_trivRejs[v];
                edge_row_trivAccs[v] = edge_trivAccs[v];
            }
        }

        for (
            uint32_t cb_x = 0, cb_x_bits = 0;
            cb_x < TILE_WIDTH_IN_COARSE_BLOCKS; 
            cb_x++, cb_x_bits = (cb_x_bits - mask_x) & mask_x)
        {

            // trivial reject if at least one edge doesn't cover the coarse block at all
            int32_t trivially_rejected = 0;
            for (int32_t v = 0; v < 3; v++)
            {
                if (TestEdgeMask & (1 << v))
                {
                    if (edge_row_trivRejs[v] >= 0)
                    {
                        trivially_rejected = 1;
                        break;
                    }
                }
            }
			//printf("   <%d,%d>:\n      Rej:%f,%f,%f|%d\n", cb_x, cb_y,
			//	s168_get(edge_row_trivRejs[0]), s168_get(edge_row_trivRejs[1]), s168_get(edge_row_trivRejs[2]), trivially_rejected);

            if (!trivially_rejected)
            {

                tilecmd_drawtile_t cbargs = *drawcmd;

                uint32_t newTestEdgeMask = TestEdgeMask;
                for (int32_t v = 0; v < 3; v++)
                {
                    if (TestEdgeMask & (1 << v))
                    {
                        if (edge_row_trivAccs[v] < 0)
                        {
                            newTestEdgeMask &= ~(1 << v);
                        }
                    }
                }

                uint32_t dst_i = tile_dst_i + (cb_y_bits | cb_x_bits);

                for (int32_t v = 0; v < 3; v++)
                {
                    cbargs.edges[v] = edges_row[v];
                }

                switch (newTestEdgeMask)
                {
                case 0:
					draw_coarse_block_largetri_scalar<0>(fb, tile_id, dst_i, &cbargs, cb_x, cb_y);
					break;
                case 1:
                    draw_coarse_block_largetri_scalar<1>(fb, tile_id, dst_i, &cbargs, cb_x, cb_y);
                    break;
                case 2:
                    draw_coarse_block_largetri_scalar<2>(fb, tile_id, dst_i, &cbargs, cb_x, cb_y);
                    break;
                case 3:
                    draw_coarse_block_largetri_scalar<3>(fb, tile_id, dst_i, &cbargs, cb_x, cb_y);
                    break;
                case 4:
                    draw_coarse_block_largetri_scalar<4>(fb, tile_id, dst_i, &cbargs, cb_x, cb_y);
                    break;
                case 5:
                    draw_coarse_block_largetri_scalar<5>(fb, tile_id, dst_i, &cbargs, cb_x, cb_y);
                    break;
                case 6:
                    draw_coarse_block_largetri_scalar<6>(fb, tile_id, dst_i, &cbargs, cb_x, cb_y);
                    break;
                case 7:
                    draw_coarse_block_largetri_scalar<7>(fb, tile_id, dst_i, &cbargs, cb_x, cb_y);
                    break;
                }
            }

            for (int32_t v = 0; v < 3; v++)
            {
                edges_row[v] += coarse_edge_dxs[v];
            }

            for (int32_t v = 0; v < 3; v++)
            {
                if (TestEdgeMask & (1 << v))
                {
                    edge_row_trivRejs[v] += coarse_edge_dxs[v];
                    edge_row_trivAccs[v] += coarse_edge_dxs[v];
                }
            }
        }

        for (int32_t v = 0; v < 3; v++)
        {
            edges[v] += coarse_edge_dys[v];
        }

        for (int32_t v = 0; v < 3; v++)
        {
            if (TestEdgeMask & (1 << v))
            {
                edge_trivRejs[v] += coarse_edge_dys[v];
                edge_trivAccs[v] += coarse_edge_dys[v];
            }
        }
    }
}

#if 0
#ifdef USE_HSWni
template<uint32_t TestEdgeMask>
static void draw_tile_largetri_avx2(framebuffer_t* fb, int32_t tile_id, const tilecmd_drawtile_t* drawcmd)
{
    // tiles are made out of 4x4 coarse blocks, organized as:
    //  0  1  4  5
    //  2  3  6  7
    //  8  9 12 13
    // 10 11 14 15
    // therefore, tiles are rasterized by shifting around the fine block's edge equations.

    __m256i edges[3];
    __m256i edge_trivRejs[kNumTestEdges > 0 ? kNumTestEdges : 1];
    __m256i edge_trivAccs[kNumTestEdges > 0 ? kNumTestEdges : 1];

    for (int32_t i = 0; i < 3; i++)
    {
        int32_t dx = drawcmd->edge_dxs[i] * COARSE_BLOCK_WIDTH_IN_PIXELS;
        int32_t dy = drawcmd->edge_dys[i] * COARSE_BLOCK_WIDTH_IN_PIXELS;

        edges[i] = _mm256_add_epi32(
            _mm256_set1_epi32(drawcmd->edges[i]),
            _mm256_setr_epi32(0, dx, dy, dx + dy, dx * 2, dx * 3, dx * 2 + dy, dx * 3 + dy));

        edge_trivRejs[i] = edges[i];
        if (dx < 0) edge_trivRejs[i] = _mm256_add_epi32(edge_trivRejs[i], _mm256_set1_epi32(dx));
        if (dx > 0) edge_trivAccs[v] = _mm256_add_epi32(edge_trivAccs[i], _mm256_set1_epi32(dx));
        if (dy < 0) edge_trivRejs[i] = _mm256_add_epi32(edge_trivRejs[i], _mm256_set1_epi32(dy));
        if (dy > 0) edge_trivAccs[v] = _mm256_add_epi32(edge_trivAccs[i], _mm256_set1_epi32(dy));
    }

    uint32_t dst_i = tile_id * PIXELS_PER_TILE;

    for (int32_t tile_half = 0; tile_half < 2; tile_half++)
    {
        // draw each coarse block in the tile half
        __declspec(align(32)) int32_t coarseblock_edges[3][8];
        _mm256_store_si256((__m256i*)&coarseblock_edges[0][0], edges[0]);
        _mm256_store_si256((__m256i*)&coarseblock_edges[1][0], edges[1]);
        _mm256_store_si256((__m256i*)&coarseblock_edges[2][0], edges[2]);

        __m256i trivRej_pass = _mm256_cmpgt_epi32(_mm256_setzero_si256(), edge_trivRejs[0]);
        trivRej_pass = _mm256_and_si256(trivRej_pass, _mm256_cmpgt_epi32(_mm256_setzero_si256(), edge_trivRejs[1]));
        trivRej_pass = _mm256_and_si256(trivRej_pass, _mm256_cmpgt_epi32(_mm256_setzero_si256(), edge_trivRejs[2]));

        int trivRej_pass_mask = _mm256_movemask_epi8(trivRej_pass);
        if (!trivRej_pass_mask)
        {
            dst_i += PIXELS_PER_COARSE_BLOCK * 8;
            goto tile_half_end;
        }

        __m256i edge_needs_test[3];
        int32_t num_tests_necessary = 0;
        for (int32_t v = 0; v < kNumTestEdges; v++)
        {
            if ((edge_needs_test[v] = edge_row_trivAccs[v] >= 0))
            {
                num_tests_necessary++;
            }
        }
        
        tilecmd_drawsmalltri_t coarsecmd = *drawcmd;
        for (int32_t i = 0; i < 8; i++)
        {
            if (trivRej_pass_mask & (1 << (i * 4)))
            {
                coarsecmd.edges[0] = coarseblock_edges[0][i];
                coarsecmd.edges[1] = coarseblock_edges[1][i];
                coarsecmd.edges[2] = coarseblock_edges[2][i];

                drawtilecmd.tilecmd_id = tilecmd_id_drawtile_0edge + num_tests_necessary;

                int32_t vertex_rotation = 0;

                if (num_tests_necessary == 1) {
                    if (edge_needs_test[1]) vertex_rotation = 1;
                    else if (edge_needs_test[2]) vertex_rotation = 2;
                }
                else if (num_tests_necessary == 2) {
                    if (!edge_needs_test[0]) vertex_rotation = 1;
                    else if (!edge_needs_test[1]) vertex_rotation = 2;
                }

                for (int32_t v = 0; v < 3; v++)
                {
                    int32_t rotated_v = (v + vertex_rotation) % 3;

                    drawtilecmd.edges[v] = edges_row[rotated_v];
                    drawtilecmd.edge_dxs[v] = drawcmd->edge_dxs[rotated_v];
                    drawtilecmd.edge_dys[v] = drawcmd->edge_dys[rotated_v];
                    drawtilecmd.shifted_es[v] = drawcmd->shifted_es[rotated_v];
                    drawtilecmd.vert_Zs[v] = drawcmd->vert_Zs[rotated_v];
                }

                uint32_t dst_i = tile_dst_i + (cb_y_bits | cb_x_bits);

                switch (drawtilecmd.tilecmd_id)
                {
                case tilecmd_id_drawtile_0edge:
                    draw_coarse_block_largetri_scalar<0>(fb, tile_id, dst_i, &drawtilecmd);
                    break;
                case tilecmd_id_drawtile_1edge:
                    draw_coarse_block_largetri_scalar<1>(fb, tile_id, dst_i, &drawtilecmd);
                    break;
                case tilecmd_id_drawtile_2edge:
                    draw_coarse_block_largetri_scalar<2>(fb, tile_id, dst_i, &drawtilecmd);
                    break;
                case tilecmd_id_drawtile_3edge:
                    draw_coarse_block_largetri_scalar<3>(fb, tile_id, dst_i, &drawtilecmd);
                    break;
                }
            }

            dst_i += PIXELS_PER_COARSE_BLOCK;
        }

    tile_half_end:
        for (int32_t i = 0; i < 3; i++)
        {
            __m256i dy2 = _mm256_set1_epi32(drawcmd->edge_dys[i] * COARSE_BLOCK_WIDTH_IN_PIXELS * 2);
            edges[i] = _mm256_add_epi32(edges[i], dy2);
            edge_trivRejs[i] = _mm256_add_epi32(edge_trivRejs[i], dy2);
            edge_trivAccs[i] = _mm256_add_epi32(edge_trivAccs[i], dy2);
        }
    }
}
#endif
#endif

static void clear_tile(framebuffer_t* fb, int32_t tile_id, tilecmd_cleartile_t* cmd)
{
    int32_t tile_start_i = PIXELS_PER_TILE * tile_id;
    int32_t tile_end_i = tile_start_i + PIXELS_PER_TILE;
    uint32_t color = cmd->color;
    for (int32_t px = tile_start_i; px < tile_end_i; px++)
    {
        fb->backbuffer[px] = color;
        fb->depthbuffer[px] = 0xFFFFFFFF;
    }
}

static void debugprint_cmdbuf(tile_cmdbuf_t* cmdbuf)
{
    int32_t read_i = (int32_t)(cmdbuf->cmdbuf_read - cmdbuf->cmdbuf_start);
    int32_t write_i = (int32_t)(cmdbuf->cmdbuf_write - cmdbuf->cmdbuf_start);
    int32_t sz = (int32_t)(cmdbuf->cmdbuf_end - cmdbuf->cmdbuf_start);
    for (int32_t i = 0; i < sz; i++)
    {
        if (i == write_i)
            printf(" W");
        else
            printf("--");
    }
    printf("\n");
    for (int32_t i = 0; i < sz; i++)
    {
        printf("| ");
    }
    printf("|\n");
    for (int32_t i = 0; i < sz; i++)
    {
        if (i == read_i)
            printf(" R");
        else
            printf("--");
    }
    printf("\n");
}

static const char* debugprint_cmdid(uint32_t cmd_id)
{
	if (cmd_id == tilecmd_id_resetbuf) return "reset_buf";
	else if (cmd_id == tilecmd_id_drawsmalltri) return "draw_small_tri";
	else if (cmd_id >= tilecmd_id_drawlargetri_0edgemask && cmd_id <= tilecmd_id_drawlargetri_7edgemask) {
		const char* str_large_id[] = { "draw_large_ri_0", "draw_large_ri_1", "draw_large_ri_2", "draw_large_ri_3", "draw_large_ri_4", "draw_large_ri_5", "draw_large_ri_6", "draw_large_ri_7" };
		return str_large_id[cmd_id - tilecmd_id_drawlargetri_0edgemask];
	}
	else if (cmd_id == tilecmd_id_cleartile) {
		return "clear_title";
	}
	return "<err>";
}

static void framebuffer_resolve_tile(framebuffer_t* fb, int32_t tile_id)
{
    tile_cmdbuf_t* cmdbuf = &fb->tile_cmdbufs[tile_id];
    
    uint32_t* cmd;
    for (cmd = cmdbuf->cmdbuf_read; cmd != cmdbuf->cmdbuf_write; )
    {
        uint32_t tilecmd_id = *cmd;
        
        // debugging code for logging commands
        //printf("Reading command [id: %d]\n", tilecmd_id);
        //debugprint_cmdbuf(cmdbuf);

        if (tilecmd_id == tilecmd_id_resetbuf)
        {
            cmd = cmdbuf->cmdbuf_start;
        }
        else if (tilecmd_id == tilecmd_id_drawsmalltri)
        {
#ifdef ENABLE_PERFCOUNTERS
            uint64_t smalltri_start_pc = qpc();
#endif

#ifdef USE_HSWni
            draw_tile_smalltri_avx2(fb, tile_id, (tilecmd_drawsmalltri_t*)cmd);
#else
            draw_tile_smalltri_scalar(fb, tile_id, (tilecmd_drawsmalltri_t*)cmd);
#endif

#ifdef ENABLE_PERFCOUNTERS
            fb->tile_perfcounters[tile_id].smalltri_raster += qpc() - smalltri_start_pc;
#endif

            cmd += sizeof(tilecmd_drawsmalltri_t) / sizeof(uint32_t);
        }
        else if (tilecmd_id >= tilecmd_id_drawlargetri_0edgemask && tilecmd_id <= tilecmd_id_drawlargetri_7edgemask)
        {   
#ifdef ENABLE_PERFCOUNTERS
            uint64_t largetri_start_pc = qpc();
#endif

#if defined(USE_HSWni) && 0
            switch (tilecmd_id - tilecmd_id_drawlargetri_0edgemask)
            {
            case 0:
                draw_tile_largetri_avx2<0>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            case 1:
                draw_tile_largetri_avx2<1>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            case 2:
                draw_tile_largetri_avx2<2>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            case 3:
                draw_tile_largetri_avx2<3>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            case 4:
                draw_tile_largetri_avx2<4>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            case 5:
                draw_tile_largetri_avx2<5>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            case 6:
                draw_tile_largetri_avx2<6>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            case 7:
                draw_tile_largetri_avx2<7>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            }
#else
            switch (tilecmd_id - tilecmd_id_drawlargetri_0edgemask)
            {
            case 0:
                draw_tile_largetri_scalar<0>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            case 1:
                draw_tile_largetri_scalar<1>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            case 2:
                draw_tile_largetri_scalar<2>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            case 3:
                draw_tile_largetri_scalar<3>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            case 4:
                draw_tile_largetri_scalar<4>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            case 5:
                draw_tile_largetri_scalar<5>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            case 6:
                draw_tile_largetri_scalar<6>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            case 7:
                draw_tile_largetri_scalar<7>(fb, tile_id, (tilecmd_drawtile_t*)cmd);
                break;
            }
#endif

#ifdef ENABLE_PERFCOUNTERS
            fb->tile_perfcounters[tile_id].largetri_raster += qpc() - largetri_start_pc;
#endif

            cmd += sizeof(tilecmd_drawtile_t) / sizeof(uint32_t);
        }
        else if (tilecmd_id == tilecmd_id_cleartile)
        {
#ifdef ENABLE_PERFCOUNTERS
            uint64_t clear_start_pc = qpc();
#endif

            clear_tile(fb, tile_id, (tilecmd_cleartile_t*)cmd);

#ifdef ENABLE_PERFCOUNTERS
            fb->tile_perfcounters[tile_id].clear += qpc() - clear_start_pc;
#endif

            cmd += sizeof(tilecmd_cleartile_t) / sizeof(uint32_t);
        }
        else
        {
            assert(!"Unknown tile command");
        }

        if (cmd == cmdbuf->cmdbuf_end)
        {
            cmd = cmdbuf->cmdbuf_start;
            
            if (cmdbuf->cmdbuf_write == cmdbuf->cmdbuf_end)
            {
                break;
            }
        }
    }

    // read ptr should never be at the end ptr after interpreting
    assert(cmd != cmdbuf->cmdbuf_end);

    cmdbuf->cmdbuf_read = cmd;
}

static void framebuffer_push_tilecmd(framebuffer_t* fb, int32_t tile_id, const uint32_t* cmd_dwords, int32_t num_dwords)
{
    assert(tile_id < fb->total_num_tiles);

    tile_cmdbuf_t* cmdbuf = &fb->tile_cmdbufs[tile_id];

    // read should never be at the end.
    assert(cmdbuf->cmdbuf_read != cmdbuf->cmdbuf_end);

    // debugging code for logging commands
    //printf("Writing command [tile_id: %d, id: %s, sz: %d]\n", tile_id, debugprint_cmdid(cmd_dwords[0]), num_dwords);
    //debugprint_cmdbuf(cmdbuf);

    if (cmdbuf->cmdbuf_read - cmdbuf->cmdbuf_write > 0 && cmdbuf->cmdbuf_read - cmdbuf->cmdbuf_write < num_dwords + 1)
    {
        // read ptr is after write ptr and there's not enough room in between
        // therefore, need to flush
        // note: write is not allowed to "catch up" to read from behind, hence why a +1 is added to keep them separate.
        framebuffer_resolve_tile(fb, tile_id);

        // after resolve, read should now have "caught up" to write from behind
        assert(cmdbuf->cmdbuf_read == cmdbuf->cmdbuf_write);
    }

    // At this point, the read head can't be a problem. However, it's possible there isn't enough memory at the end.
    if (cmdbuf->cmdbuf_end - cmdbuf->cmdbuf_write < num_dwords)
    {
        // not enough room in the buffer to write the commands, need to loop around

        // should never be at the end of the buffer since it always loops at the end of this function
        assert(cmdbuf->cmdbuf_write != cmdbuf->cmdbuf_end);

        // abandon the rest of the slop at the end of the buffer
        *cmdbuf->cmdbuf_write = tilecmd_id_resetbuf;

        if (cmdbuf->cmdbuf_start == cmdbuf->cmdbuf_read)
        {
            // write is not allowed to catch up to read,
            // so make sure read catches up to write instead.
            framebuffer_resolve_tile(fb, tile_id);

            // reset read back to start since we'll set write back to start also
            cmdbuf->cmdbuf_read = cmdbuf->cmdbuf_start;
        }

        cmdbuf->cmdbuf_write = cmdbuf->cmdbuf_start;

        // After loping around the buffer, it's possible that the read head is in the way again.
        if (cmdbuf->cmdbuf_read - cmdbuf->cmdbuf_write > 0 && cmdbuf->cmdbuf_read - cmdbuf->cmdbuf_write < num_dwords + 1)
        {
            // read ptr is after write ptr and there's not enough room in between
            // therefore, need to flush
            // note: write is not allowed to "catch up" to read from behind, hence why a +1 is added to keep them separate.
            framebuffer_resolve_tile(fb, tile_id);

            // after resolve, read should now have "caught up" to write from behind
            assert(cmdbuf->cmdbuf_read == cmdbuf->cmdbuf_write);
        }
    }

    // assert enough room in the buffer after the write
    assert(cmdbuf->cmdbuf_end - cmdbuf->cmdbuf_write >= num_dwords);
    // assert that the read head isn't in the way
    assert((cmdbuf->cmdbuf_read <= cmdbuf->cmdbuf_write) || (cmdbuf->cmdbuf_read > cmdbuf->cmdbuf_write && cmdbuf->cmdbuf_read - cmdbuf->cmdbuf_write >= num_dwords + 1));

    // finally actually write the command
    for (int32_t i = 0; i < num_dwords; i++)
    {
        cmdbuf->cmdbuf_write[i] = cmd_dwords[i];
    }
    cmdbuf->cmdbuf_write += num_dwords;

    // write is not allowed to catch up to read
    assert(cmdbuf->cmdbuf_write != cmdbuf->cmdbuf_read);

    // loop around the buffer if necessary
    if (cmdbuf->cmdbuf_write == cmdbuf->cmdbuf_end)
    {
        if (cmdbuf->cmdbuf_start == cmdbuf->cmdbuf_read)
        {
            // write is not allowed to catch up to read,
            // so make sure read catches up to write instead.
            framebuffer_resolve_tile(fb, tile_id);

            // since the resolve made read ptr catch up to write ptr, that means read reached the end
            // that also means it currently looped back to the start, so the write ptr can be put there too
            // this is the case where the whole buffer gets consumed in one go
        }

        cmdbuf->cmdbuf_write = cmdbuf->cmdbuf_start;
    }

    // DEBUGGING: Always flush. Helpful since it gives you a straight call stack through the command list.
    // framebuffer_resolve_tile(fb, tile_id);
}

void framebuffer_resolve(framebuffer_t* fb)
{
    assert(fb);

    int32_t tile_i = 0;
    for (int32_t tile_y = 0; tile_y < fb->height_in_tiles; tile_y++)
    {
        for (int32_t tile_x = 0; tile_x < fb->width_in_tiles; tile_x++)
        {
            framebuffer_resolve_tile(fb, tile_i);
            tile_i++;
        }
    }
}

void framebuffer_pack_row_major(framebuffer_t* fb, attachment_t attachment, int32_t x, int32_t y, int32_t width, int32_t height, pixelformat_t format, void* data)
{
    assert(fb);
    assert(x >= 0 && x < fb->width_in_pixels);
    assert(y >= 0 && y < fb->height_in_pixels);
    assert(width >= 0 && width <= fb->width_in_pixels);
    assert(height >= 0 && height <= fb->height_in_pixels);
    assert(x + width <= fb->width_in_pixels);
    assert(y + height <= fb->height_in_pixels);
    assert(data);

    int32_t topleft_tile_y = y / TILE_WIDTH_IN_PIXELS;
    int32_t topleft_tile_x = x / TILE_WIDTH_IN_PIXELS;
    int32_t bottomright_tile_y = (y + (height - 1)) / TILE_WIDTH_IN_PIXELS;
    int32_t bottomright_tile_x = (x + (width - 1)) / TILE_WIDTH_IN_PIXELS;

    int32_t curr_tile_row_start = topleft_tile_y * fb->pixels_per_row_of_tiles + topleft_tile_x * PIXELS_PER_TILE;
    for (int32_t tile_y = topleft_tile_y; tile_y <= bottomright_tile_y; tile_y++)
    {
        int32_t curr_tile_start = curr_tile_row_start;

        for (int32_t tile_x = topleft_tile_x; tile_x <= bottomright_tile_x; tile_x++)
        {
            int32_t topleft_y = tile_y * TILE_WIDTH_IN_PIXELS;
            int32_t topleft_x = tile_x * TILE_WIDTH_IN_PIXELS;
            int32_t bottomright_y = topleft_y + TILE_WIDTH_IN_PIXELS;
            int32_t bottomright_x = topleft_x + TILE_WIDTH_IN_PIXELS;
            int32_t pixel_y_min = topleft_y < y ? y : topleft_y;
            int32_t pixel_x_min = topleft_x < x ? x : topleft_x;
            int32_t pixel_y_max = bottomright_y > y + height ? y + height : bottomright_y;
            int32_t pixel_x_max = bottomright_x > x + width ? x + width : bottomright_x;

            for (int32_t pixel_y = pixel_y_min, pixel_y_bits = pdep_u32(topleft_y, TILE_Y_SWIZZLE_MASK);
                pixel_y < pixel_y_max;
                pixel_y++, pixel_y_bits = (pixel_y_bits - TILE_Y_SWIZZLE_MASK) & TILE_Y_SWIZZLE_MASK)
            {
                for (int32_t pixel_x = pixel_x_min, pixel_x_bits = pdep_u32(topleft_x, TILE_X_SWIZZLE_MASK);
                    pixel_x < pixel_x_max;
                    pixel_x++, pixel_x_bits = (pixel_x_bits - TILE_X_SWIZZLE_MASK) & TILE_X_SWIZZLE_MASK)
                {
                    int32_t rel_pixel_y = pixel_y - y;
                    int32_t rel_pixel_x = pixel_x - x;
                    int32_t dst_i = rel_pixel_y * width + rel_pixel_x;

                    int32_t src_i = curr_tile_start + (pixel_y_bits | pixel_x_bits);
                    if (attachment == attachment_color0)
                    {
                        uint32_t src = fb->backbuffer[src_i];
                        if (format == pixelformat_r8g8b8a8_unorm)
                        {
                            uint8_t* dst = (uint8_t*)data + dst_i * 4;
                            dst[0] = (uint8_t)((src & 0x00FF0000) >> 16);
                            dst[1] = (uint8_t)((src & 0x0000FF00) >> 8);
                            dst[2] = (uint8_t)((src & 0x000000FF) >> 0);
                            dst[3] = (uint8_t)((src & 0xFF000000) >> 24);
                        }
                        else if (format == pixelformat_b8g8r8a8_unorm)
                        {
                            uint8_t* dst = (uint8_t*)data + dst_i * 4;
                            dst[0] = (uint8_t)((src & 0x000000FF) >> 0);
                            dst[1] = (uint8_t)((src & 0x0000FF00) >> 8);
                            dst[2] = (uint8_t)((src & 0x00FF0000) >> 16);
                            dst[3] = (uint8_t)((src & 0xFF000000) >> 24);
                        }
                        else
                        {
                            assert(!"Unknown color pixel format");
                        }
                    }
                    else if (attachment == attachment_depth)
                    {
                        uint32_t src = fb->depthbuffer[src_i];
                        if (format == pixelformat_r32_unorm)
                        {
                            uint32_t* dst = (uint32_t*)data + dst_i;
                            *dst = src;
                        }
                        else
                        {
                            assert(!"Unknown depth pixel format");
                        }
                    }
                }
            }

            curr_tile_start += PIXELS_PER_TILE;
        }

        curr_tile_row_start += fb->pixels_per_row_of_tiles;
    }
}

void framebuffer_clear(framebuffer_t* fb, uint32_t color)
{
    tilecmd_cleartile_t tilecmd;
    tilecmd.tilecmd_id = tilecmd_id_cleartile;
    tilecmd.color = color;

    for (int32_t tile_id = 0; tile_id < fb->total_num_tiles; tile_id++)
    {
        framebuffer_push_tilecmd(fb, tile_id, &tilecmd.tilecmd_id, sizeof(tilecmd) / sizeof(uint32_t));
    }
}

static void rasterize_triangle(
    framebuffer_t* fb,
    xyzw_i32_t clipVerts[3])
{
#ifdef ENABLE_PERFCOUNTERS
    uint64_t clipping_start_pc = qpc();
#endif

    int32_t fully_clipped = 0;

	for (int v = 0; v < 3; v++) {
		printf("input v[%d]=<%f,%f,%f>\n", v, s1516_get(clipVerts[v].x), s1516_get(clipVerts[v].y), s1516_get(clipVerts[v].z));
	}
    // perform near plane clipping
    {
        // check which vertices are behind the near plane
        int32_t vert_near_clipped[3];
        vert_near_clipped[0] = clipVerts[0].z < 0;
        vert_near_clipped[1] = clipVerts[1].z < 0;
        vert_near_clipped[2] = clipVerts[2].z < 0;

        int32_t num_near_clipped = vert_near_clipped[0] + vert_near_clipped[1] + vert_near_clipped[2];

        if (num_near_clipped == 3)
        {
            // clip whole triangles with 3 vertices behind the near plane
            fully_clipped = 1;
            goto clipping_end;
        }

        if (num_near_clipped == 2)
        {
            // Two vertices behind the near plane. In this case, cut the associated edges short.
            int32_t unclipped_vert = 0;
            if (!vert_near_clipped[1]) unclipped_vert = 1;
            else if (!vert_near_clipped[2]) unclipped_vert = 2;

            int32_t v1 = (unclipped_vert + 1) % 3;
            int32_t v2 = (unclipped_vert + 2) % 3;

            // clip the first edge
            int32_t a1 = s1516_div(clipVerts[unclipped_vert].z, clipVerts[unclipped_vert].z - clipVerts[v1].z);
            int32_t one_minus_a1 = s1516_int(1) - a1;
            clipVerts[v1].x = s1516_mul(one_minus_a1, clipVerts[unclipped_vert].x) + s1516_mul(a1, clipVerts[v1].x);
            clipVerts[v1].y = s1516_mul(one_minus_a1, clipVerts[unclipped_vert].y) + s1516_mul(a1, clipVerts[v1].y);
            clipVerts[v1].z = 0;
            clipVerts[v1].w = s1516_mul(one_minus_a1, clipVerts[unclipped_vert].w) + s1516_mul(a1, clipVerts[v1].w);
            assert(clipVerts[v1].w != 0);

            // clip the second edge
            int32_t a2 = s1516_div(clipVerts[unclipped_vert].z, clipVerts[unclipped_vert].z - clipVerts[v2].z);
            int32_t one_minus_a2 = s1516_int(1) - a2;
            clipVerts[v2].x = s1516_mul(one_minus_a2, clipVerts[unclipped_vert].x) + s1516_mul(a2, clipVerts[v2].x);
            clipVerts[v2].y = s1516_mul(one_minus_a2, clipVerts[unclipped_vert].y) + s1516_mul(a2, clipVerts[v2].y);
            clipVerts[v2].z = 0;
            clipVerts[v2].w = s1516_mul(one_minus_a2, clipVerts[unclipped_vert].w) + s1516_mul(a2, clipVerts[v2].w);
            assert(clipVerts[v2].w != 0);
        }

        if (num_near_clipped == 1)
        {
            // One vertex behind the near plane. In this case, triangulate the triangle into two triangles.
            int32_t clipped_vert = 0;
            if (vert_near_clipped[1]) clipped_vert = 1;
            else if (vert_near_clipped[2]) clipped_vert = 2;

            int32_t v1 = (clipped_vert + 1) % 3;
            int32_t v2 = (clipped_vert + 2) % 3;

            // clip the first edge
            xyzw_i32_t clipped1;
            int32_t a1 = s1516_div(clipVerts[clipped_vert].z, clipVerts[clipped_vert].z - clipVerts[v1].z);
            int32_t one_minus_a1 = s1516_int(1) - a1;
            clipped1.x = s1516_mul(one_minus_a1, clipVerts[clipped_vert].x) + s1516_mul(a1, clipVerts[v1].x);
            clipped1.y = s1516_mul(one_minus_a1, clipVerts[clipped_vert].y) + s1516_mul(a1, clipVerts[v1].y);
            clipped1.z = 0;
            clipped1.w = s1516_mul(one_minus_a1, clipVerts[clipped_vert].w) + s1516_mul(a1, clipVerts[v1].w);
            assert(clipped1.w != 0);

            // clip the second edge
            xyzw_i32_t clipped2;
            int32_t a2 = s1516_div(clipVerts[clipped_vert].z, clipVerts[clipped_vert].z - clipVerts[v2].z);
            int32_t one_minus_a2 = s1516_int(1) - a2;
            clipped2.x = s1516_mul(one_minus_a2, clipVerts[clipped_vert].x) + s1516_mul(a2, clipVerts[v2].x);
            clipped2.y = s1516_mul(one_minus_a2, clipVerts[clipped_vert].y) + s1516_mul(a2, clipVerts[v2].y);
            clipped2.z = 0;
            clipped2.w = s1516_mul(one_minus_a2, clipVerts[clipped_vert].w) + s1516_mul(a2, clipVerts[v2].w);
            assert(clipped2.w != 0);

            // output the first clipped triangle (note: recursive call)
            xyzw_i32_t clipVerts1[3] = { clipVerts[0], clipVerts[1], clipVerts[2] };
            clipVerts1[clipped_vert] = clipped1;

#ifdef ENABLE_PERFCOUNTERS
            fb->perfcounters.clipping += qpc() - clipping_start_pc;
#endif

            rasterize_triangle(fb, clipVerts1);
            
#ifdef ENABLE_PERFCOUNTERS
            clipping_start_pc = qpc();
#endif

            // set self up to output the second clipped triangle
            clipVerts[clipped_vert] = clipped2;
            clipVerts[v1] = clipped1;
        }
    }

    // perform far plane clipping
    {
        // check which vertices are behind (or on) the far plane
        int32_t vert_far_clipped[3];
        vert_far_clipped[0] = clipVerts[0].z >= clipVerts[0].w;
        vert_far_clipped[1] = clipVerts[1].z >= clipVerts[1].w;
        vert_far_clipped[2] = clipVerts[2].z >= clipVerts[2].w;

        int32_t num_far_clipped = vert_far_clipped[0] + vert_far_clipped[1] + vert_far_clipped[2];

        if (num_far_clipped == 3)
        {
            // clip whole triangles with 3 vertices behind the far plane
            fully_clipped = 1;
            goto clipping_end;
        }

        if (num_far_clipped == 2)
        {
            // Two vertices behind the far plane. In this case, cut the associated edges short.
            int32_t unclipped_vert = 0;
            if (!vert_far_clipped[1]) unclipped_vert = 1;
            else if (!vert_far_clipped[2]) unclipped_vert = 2;

            int32_t v1 = (unclipped_vert + 1) % 3;
            int32_t v2 = (unclipped_vert + 2) % 3;

            // clip the first edge
            int32_t a1 = s1516_div(clipVerts[unclipped_vert].z - clipVerts[unclipped_vert].w, (clipVerts[unclipped_vert].z - clipVerts[unclipped_vert].w) - (clipVerts[v1].z - clipVerts[v1].w));
            int32_t one_minus_a1 = s1516_int(1) - a1;
            clipVerts[v1].x = s1516_mul(one_minus_a1, clipVerts[unclipped_vert].x) + s1516_mul(a1, clipVerts[v1].x);
            clipVerts[v1].y = s1516_mul(one_minus_a1, clipVerts[unclipped_vert].y) + s1516_mul(a1, clipVerts[v1].y);
            clipVerts[v1].w = s1516_mul(one_minus_a1, clipVerts[unclipped_vert].w) + s1516_mul(a1, clipVerts[v1].w);
            clipVerts[v1].z = clipVerts[v1].w - 1;
            assert(clipVerts[v1].w != 0);

            // clip the second edge
            int32_t a2 = s1516_div(clipVerts[unclipped_vert].z - clipVerts[unclipped_vert].w, (clipVerts[unclipped_vert].z - clipVerts[unclipped_vert].w) - (clipVerts[v2].z - clipVerts[v2].w));
            int32_t one_minus_a2 = s1516_int(1) - a2;
            clipVerts[v2].x = s1516_mul(one_minus_a2, clipVerts[unclipped_vert].x) + s1516_mul(a2, clipVerts[v2].x);
            clipVerts[v2].y = s1516_mul(one_minus_a2, clipVerts[unclipped_vert].y) + s1516_mul(a2, clipVerts[v2].y);
            clipVerts[v2].w = s1516_mul(one_minus_a2, clipVerts[unclipped_vert].w) + s1516_mul(a2, clipVerts[v2].w);
            clipVerts[v2].z = clipVerts[v2].w - 1;
            assert(clipVerts[v2].w != 0);
        }

        if (num_far_clipped == 1)
        {
            // One vertex behind the near plane. In this case, triangulate the triangle into two triangles.
            int32_t clipped_vert = 0;
            if (vert_far_clipped[1]) clipped_vert = 1;
            else if (vert_far_clipped[2]) clipped_vert = 2;

            int32_t v1 = (clipped_vert + 1) % 3;
            int32_t v2 = (clipped_vert + 2) % 3;

            // clip the first edge
            xyzw_i32_t clipped1;
            int32_t a1 = s1516_div(clipVerts[clipped_vert].z - clipVerts[clipped_vert].w, (clipVerts[clipped_vert].z - clipVerts[clipped_vert].w) - (clipVerts[v1].z - clipVerts[v1].w));
            int32_t one_minus_a1 = s1516_int(1) - a1;
            clipped1.x = s1516_mul(one_minus_a1, clipVerts[clipped_vert].x) + s1516_mul(a1, clipVerts[v1].x);
            clipped1.y = s1516_mul(one_minus_a1, clipVerts[clipped_vert].y) + s1516_mul(a1, clipVerts[v1].y);
            clipped1.w = s1516_mul(one_minus_a1, clipVerts[clipped_vert].w) + s1516_mul(a1, clipVerts[v1].w);
            clipped1.z = clipped1.w - 1;
            assert(clipped1.w != 0);

            // clip the second edge
            xyzw_i32_t clipped2;
            int32_t a2 = s1516_div(clipVerts[clipped_vert].z - clipVerts[clipped_vert].w, (clipVerts[clipped_vert].z - clipVerts[clipped_vert].w) - (clipVerts[v2].z - clipVerts[v2].w));
            int32_t one_minus_a2 = s1516_int(1) - a2;
            clipped2.x = s1516_mul(one_minus_a2, clipVerts[clipped_vert].x) + s1516_mul(a2, clipVerts[v2].x);
            clipped2.y = s1516_mul(one_minus_a2, clipVerts[clipped_vert].y) + s1516_mul(a2, clipVerts[v2].y);
            clipped2.w = s1516_mul(one_minus_a2, clipVerts[clipped_vert].w) + s1516_mul(a2, clipVerts[v2].w);
            clipped2.z = clipped2.w - 1;
            assert(clipped2.w != 0);

            // output the first clipped triangle (note: recursive call)
            xyzw_i32_t clipVerts1[3] = { clipVerts[0], clipVerts[1], clipVerts[2] };
            clipVerts1[clipped_vert] = clipped1;

#ifdef ENABLE_PERFCOUNTERS
            fb->perfcounters.clipping += qpc() - clipping_start_pc;
#endif

            rasterize_triangle(fb, clipVerts1);
            
#ifdef ENABLE_PERFCOUNTERS
            clipping_start_pc = qpc();
#endif

            // set self up to output the second clipped triangle
            clipVerts[clipped_vert] = clipped2;
            clipVerts[v1] = clipped1;
        }
    }

clipping_end:

#ifdef ENABLE_PERFCOUNTERS
    fb->perfcounters.clipping += qpc() - clipping_start_pc;
#endif

    if (fully_clipped)
    {
        return;
    }

#ifdef ENABLE_PERFCOUNTERS
    uint64_t commonsetup_start_pc = qpc();
#endif

    // transform vertices from clip space to window coordinates
    xyzw_i32_t verts[3];
    for (int32_t v = 0; v < 3; v++)
    {
        int32_t one_over_w = s1516_div(s1516_int(1), clipVerts[v].w);

        // convert s15.16 (in clip space) to s16.8 window coordinates
        // note to self: should probably avoid round-to-zero here? otherwise geometry warps inwards to the center of the screen
        verts[v].x = s168_s1516(s1516_mul(s1516_div(s1516_add(s1516_mul(+clipVerts[v].x, one_over_w), s1516_int(1)), s1516_int(2)), s1516_int(fb->width_in_pixels)));
        verts[v].y = s168_s1516(s1516_mul(s1516_div(s1516_add(s1516_mul(-clipVerts[v].y, one_over_w), s1516_int(1)), s1516_int(2)), s1516_int(fb->height_in_pixels)));

        // TODO: clip things that are outside the guard band

        // perform z/w, rounding down to maintain the z < w upper bound
        verts[v].z = ((int64_t)clipVerts[v].z * one_over_w - (clipVerts[v].w / 2)) >> 16;
        if (verts[v].z < 0)
            verts[v].z = 0;

        // Should be 0 <= z < w, thanks to near and far plane clipping
        assert(verts[v].z >= 0 && verts[v].z <= 0xFFFF);

        verts[v].w = clipVerts[v].w;

		//debug out
		printf("window coord: verts[%d]=<%f,%f,%f>\n",v, s168_get(verts[v].x), s168_get(verts[v].y), s168_get(verts[v].z));
    }

    uint32_t min_Z = verts[0].z;
    uint32_t max_Z = verts[0].z;
    for (int32_t v = 1; v < 3; v++)
    {
        if ((uint32_t)verts[v].z < min_Z)
            min_Z = (uint32_t)verts[v].z;
        
        if ((uint32_t)verts[v].z > max_Z)
            max_Z = (uint32_t)verts[v].z;
    }
    
    // get window coordinates bounding box
    int32_t bbox_min_x = verts[0].x;
    if (verts[1].x < bbox_min_x) bbox_min_x = verts[1].x;
    if (verts[2].x < bbox_min_x) bbox_min_x = verts[2].x;
    int32_t bbox_max_x = verts[0].x;
    if (verts[1].x > bbox_max_x) bbox_max_x = verts[1].x;
    if (verts[2].x > bbox_max_x) bbox_max_x = verts[2].x;
    int32_t bbox_min_y = verts[0].y;
    if (verts[1].y < bbox_min_y) bbox_min_y = verts[1].y;
    if (verts[2].y < bbox_min_y) bbox_min_y = verts[2].y;
    int32_t bbox_max_y = verts[0].y;
    if (verts[1].y > bbox_max_y) bbox_max_y = verts[1].y;
    if (verts[2].y > bbox_max_y) bbox_max_y = verts[2].y;

    // clip triangles that are fully outside the scissor rect (scissor rect = whole window)
    if (bbox_max_x < 0 ||
        bbox_max_y < 0 ||
        bbox_min_x >= (int32_t)(fb->width_in_pixels << 8) ||
        bbox_min_y >= (int32_t)(fb->height_in_pixels << 8))
    {
        fully_clipped = 1;
        goto commonsetup_end;
    }

    int32_t clamped_bbox_min_x = bbox_min_x, clamped_bbox_max_x = bbox_max_x;
    int32_t clamped_bbox_min_y = bbox_min_y, clamped_bbox_max_y = bbox_max_y;

    // clamp bbox to scissor rect
    if (clamped_bbox_min_x < 0) clamped_bbox_min_x = 0;
    if (clamped_bbox_min_y < 0) clamped_bbox_min_y = 0;
    if (clamped_bbox_max_x >= (int32_t)(fb->width_in_pixels << 8)) clamped_bbox_max_x = ((int32_t)fb->width_in_pixels << 8) - 1;
    if (clamped_bbox_max_y >= (int32_t)(fb->height_in_pixels << 8)) clamped_bbox_max_y = ((int32_t)fb->height_in_pixels << 8) - 1;

    // "small" triangles are no wider than a tile.
    int32_t is_large =
        (bbox_max_x - bbox_min_x) >= (TILE_WIDTH_IN_PIXELS << 8) ||
        (bbox_max_y - bbox_min_y) >= (TILE_WIDTH_IN_PIXELS << 8);

commonsetup_end:

#ifdef ENABLE_PERFCOUNTERS
    fb->perfcounters.common_setup += qpc() - commonsetup_start_pc;
#endif

    if (fully_clipped)
    {
        return;
    }

#ifdef ENABLE_PERFCOUNTERS
    uint64_t setup_start_pc = qpc();
#endif

    if (!is_large)
    {
        // since this is a small triangle, that means the triangle is smaller than a tile.
        // that means it can overlap at most 2x2 adjacent tiles if it's in the middle of all of them.
        // just need to figure out which boxes are overlapping the triangle's bbox
        int32_t first_tile_x = (bbox_min_x >> 8) / TILE_WIDTH_IN_PIXELS;
        int32_t first_tile_y = (bbox_min_y >> 8) / TILE_WIDTH_IN_PIXELS;
        int32_t last_tile_x = (bbox_max_x >> 8) / TILE_WIDTH_IN_PIXELS;
        int32_t last_tile_y = (bbox_max_y >> 8) / TILE_WIDTH_IN_PIXELS;
        
        // pixel coordinates of the first and last tile of the (up to) 2x2 block of tiles
        int32_t first_tile_px_x = (first_tile_x << 8) * TILE_WIDTH_IN_PIXELS;
        int32_t first_tile_px_y = (first_tile_y << 8) * TILE_WIDTH_IN_PIXELS;
        int32_t last_tile_px_x = (last_tile_x << 8) * TILE_WIDTH_IN_PIXELS;
        int32_t last_tile_px_y = (last_tile_y << 8) * TILE_WIDTH_IN_PIXELS;

        // range of coarse blocks affected (relative to top left of 2x2 tile block)
        int32_t first_rel_cb_x = ((bbox_min_x - first_tile_px_x) >> 8) / COARSE_BLOCK_WIDTH_IN_PIXELS;
        int32_t first_rel_cb_y = ((bbox_min_y - first_tile_px_y) >> 8) / COARSE_BLOCK_WIDTH_IN_PIXELS;
        int32_t last_rel_cb_x = ((bbox_max_x - first_tile_px_x) >> 8) / COARSE_BLOCK_WIDTH_IN_PIXELS;
        int32_t last_rel_cb_y = ((bbox_max_y - first_tile_px_y) >> 8) / COARSE_BLOCK_WIDTH_IN_PIXELS;

        tilecmd_drawsmalltri_t drawsmalltricmd;
        drawsmalltricmd.tilecmd_id = tilecmd_id_drawsmalltri;
		printf("last tile =(%f,%f)\n", s168_get(last_tile_px_x), s168_get(last_tile_px_y));
        // make vertices relative to the last tile they're in
        for (int32_t v = 0; v < 3; v++)
        {
            // the point of making them relative is to lower the required precision to 4 hex digits
            assert((verts[v].x - last_tile_px_x) >= (-TILE_WIDTH_IN_PIXELS << 8) && (verts[v].x - last_tile_px_x) <= ((TILE_WIDTH_IN_PIXELS << 8) - 1));
            assert((verts[v].y - last_tile_px_y) >= (-TILE_WIDTH_IN_PIXELS << 8) && (verts[v].y - last_tile_px_y) <= ((TILE_WIDTH_IN_PIXELS << 8) - 1));

            verts[v].x -= last_tile_px_x;
            verts[v].y -= last_tile_px_y;
			printf("base last tile verts[%d]=<%f,%f,%f>\n", v, s168_get(verts[v].x), s168_get(verts[v].y), s168_get(verts[v].z));
		}

        int32_t triarea2 = (verts[1].x - verts[0].x) * (verts[2].y - verts[0].y) - (verts[1].y - verts[0].y) * (verts[2].x - verts[0].x);

        // round away from zero to guarantee edge equations don't become greater than area
        if (triarea2 < 0)
            triarea2 -= 0x7F;
        else if (triarea2 > 0)
            triarea2 += 0x7F;

        if (triarea2 < 0 && triarea2 > -256)
        {
            // force to zero, since right shift of negative numbers never reach zero
            triarea2 = 0;
        }

        triarea2 = triarea2 >> 8;
        
        if (triarea2 == 0)
        {
            goto setup_end;
        }

        if (triarea2 < 0)
        {
            xyzw_i32_t tmp = verts[1];
            verts[1] = verts[2];
            verts[2] = tmp;
            triarea2 = -triarea2;

            // backface culling
            goto setup_end;
        }

        // compute 1/(2triarea) and convert to a pseudo 8.16 floating point value
        int32_t triarea2_lzcnt = lzcnt(triarea2);
        int32_t triarea2_mantissa_rshift = (31 - 16) - triarea2_lzcnt;
        int32_t triarea2_mantissa;
        if (triarea2_mantissa_rshift < 0)
            triarea2_mantissa = triarea2 << -triarea2_mantissa_rshift;
        else
            triarea2_mantissa = triarea2 >> triarea2_mantissa_rshift;
        assert(triarea2_mantissa & 0x10000);
        
        // perform the reciprocal
        // note: triarea2_mantissa is currently normalized as 1.16, and so is the numerator of the division (before being adjusted for rounding)
        int32_t rcp_triarea2_mantissa = 0xFFFFFFFF / triarea2_mantissa;
        assert(rcp_triarea2_mantissa != 0);
        
        // ensure the mantissa is denormalized so it fits in 16 bits
        int32_t rcp_triarea2_mantissa_rshift = (31 - 15) - lzcnt(rcp_triarea2_mantissa);
        if (rcp_triarea2_mantissa_rshift < 0)
            rcp_triarea2_mantissa = rcp_triarea2_mantissa << -rcp_triarea2_mantissa_rshift;
        else
            rcp_triarea2_mantissa = rcp_triarea2_mantissa >> rcp_triarea2_mantissa_rshift;
        assert(!(rcp_triarea2_mantissa & 0xFFFF0000));
        assert(rcp_triarea2_mantissa & 0x8000);

        assert(rcp_triarea2_mantissa < 0x10000);
        rcp_triarea2_mantissa = rcp_triarea2_mantissa & 0xFFFF;
        rcp_triarea2_mantissa_rshift = (triarea2_mantissa_rshift + 1) - rcp_triarea2_mantissa_rshift;

        drawsmalltricmd.shifted_triarea2 = triarea2_mantissa >> 1;
        drawsmalltricmd.rcp_triarea2_mantissa = rcp_triarea2_mantissa;
        drawsmalltricmd.rcp_triarea2_rshift = rcp_triarea2_mantissa_rshift;

        // compute edge equations with reduced precision thanks to being localized to the tiles

        int32_t edges[3];
        int32_t edge_dxs[3], edge_dys[3];
        for (int32_t v = 0; v < 3; v++)
        {
            int32_t v1 = (v + 1) % 3;
            int32_t v2 = (v + 2) % 3;

            // find how the edge equation varies along x and y
            edge_dxs[v] = verts[v1].y - verts[v].y;
            edge_dys[v] = verts[v].x - verts[v1].x;

            // compute edge equation
            // |  x  y  z |
            // | ax ay  0 |
            // | bx by  0 |
            // = ax*by - ay*bx
            // eg: a = (px-v0), b = (v1-v0)
            // note: evaluated at px = (0.5,0.5) because the vertices are relative to the last tile
            const int32_t s168_zero_pt_five = 0x80;
			printf("%f\n", s168_get(s168_zero_pt_five));

            edges[v] = ((s168_zero_pt_five - verts[v].x) * edge_dxs[v]) - ((s168_zero_pt_five - verts[v].y) * -edge_dys[v]);

            // round to negative infinity
            if (edges[v] < 0)
                edges[v] = edges[v] - 0xFF;

            edges[v] = edges[v] >> 8;

            // Top-left rule: shift top-left edges ever so slightly outward to make the top-left edges be the tie-breakers when rasterizing adjacent triangles
            if ((verts[v].y == verts[v1].y && verts[v].x < verts[v1].x) || verts[v].y > verts[v1].y) edges[v]--;
        }

        drawsmalltricmd.min_Z = min_Z;
        drawsmalltricmd.max_Z = max_Z;

        for (int32_t v = 0; v < 3; v++)
        {
            drawsmalltricmd.edge_dxs[v] = edge_dxs[v];
            drawsmalltricmd.edge_dys[v] = edge_dys[v];
            drawsmalltricmd.vert_Zs[v] = verts[v].z;
        }

        // draw top left tile
        int32_t first_tile_id = first_tile_y * fb->width_in_tiles + first_tile_x;
        if (first_tile_x >= 0 && first_tile_y >= 0)
        {
            for (int32_t v = 0; v < 3; v++)
            {
                drawsmalltricmd.edges[v] = edges[v] + (
                    edge_dxs[v] * (first_tile_x - last_tile_x) +
                    edge_dys[v] * (first_tile_y - last_tile_y)) * TILE_WIDTH_IN_PIXELS;
            }

#ifdef ENABLE_PERFCOUNTERS
            fb->perfcounters.smalltri_setup += qpc() - setup_start_pc;
#endif

            framebuffer_push_tilecmd(fb, first_tile_id, &drawsmalltricmd.tilecmd_id, sizeof(drawsmalltricmd) / sizeof(uint32_t));

#ifdef ENABLE_PERFCOUNTERS
            setup_start_pc = qpc();
#endif
        }

        // draw top right tile
        if (last_tile_x > first_tile_x &&
            last_tile_x < fb->width_in_tiles && first_tile_y >= 0)
        {
            for (int32_t v = 0; v < 3; v++)
            {
                drawsmalltricmd.edges[v] = edges[v] + edge_dys[v] * (first_tile_y - last_tile_y) * TILE_WIDTH_IN_PIXELS;
            }

            int32_t tile_id_right = first_tile_id + 1;

#ifdef ENABLE_PERFCOUNTERS
            fb->perfcounters.smalltri_setup += qpc() - setup_start_pc;
#endif

            framebuffer_push_tilecmd(fb, tile_id_right, &drawsmalltricmd.tilecmd_id, sizeof(drawsmalltricmd) / sizeof(uint32_t));

#ifdef ENABLE_PERFCOUNTERS
            setup_start_pc = qpc();
#endif
        }

        // draw bottom left tile
        if (last_tile_y > first_tile_y &&
            first_tile_x >= 0 && last_tile_y < fb->height_in_tiles)
        {
            for (int32_t v = 0; v < 3; v++)
            {
                drawsmalltricmd.edges[v] = edges[v] + edge_dxs[v] * (first_tile_x - last_tile_x) * TILE_WIDTH_IN_PIXELS;
            }

            int32_t tile_id_down = first_tile_id + fb->width_in_tiles;

#ifdef ENABLE_PERFCOUNTERS
            fb->perfcounters.smalltri_setup += qpc() - setup_start_pc;
#endif

            framebuffer_push_tilecmd(fb, tile_id_down, &drawsmalltricmd.tilecmd_id, sizeof(drawsmalltricmd) / sizeof(uint32_t));

#ifdef ENABLE_PERFCOUNTERS
            setup_start_pc = qpc();
#endif
        }

        // draw bottom right tile
        if (last_tile_x > first_tile_x && last_tile_y > first_tile_y &&
            last_tile_x < fb->width_in_tiles && last_tile_y < fb->height_in_tiles)
        {
            for (int32_t v = 0; v < 3; v++)
            {
                drawsmalltricmd.edges[v] = edges[v];
            }

            int32_t tile_id_downright = first_tile_id + 1 + fb->width_in_tiles;

#ifdef ENABLE_PERFCOUNTERS
            fb->perfcounters.smalltri_setup += qpc() - setup_start_pc;
#endif

            framebuffer_push_tilecmd(fb, tile_id_downright, &drawsmalltricmd.tilecmd_id, sizeof(drawsmalltricmd) / sizeof(uint32_t));

#ifdef ENABLE_PERFCOUNTERS
            setup_start_pc = qpc();
#endif
        }
    }
    else // large triangle
    {
        // for large triangles, test each tile in their bbox for overlap
        // done using scalar code for simplicity, since rasterization dominates large triangle performance anyways.
        int32_t first_tile_x = (clamped_bbox_min_x >> 8) / TILE_WIDTH_IN_PIXELS;
        int32_t first_tile_y = (clamped_bbox_min_y >> 8) / TILE_WIDTH_IN_PIXELS;
        int32_t last_tile_x = (clamped_bbox_max_x >> 8) / TILE_WIDTH_IN_PIXELS;
        int32_t last_tile_y = (clamped_bbox_max_y >> 8) / TILE_WIDTH_IN_PIXELS;

        // evaluate edge equation at the top left tile
        int32_t first_tile_px_x = (first_tile_x << 8) * TILE_WIDTH_IN_PIXELS;
        int32_t first_tile_px_y = (first_tile_y << 8) * TILE_WIDTH_IN_PIXELS;

		printf("first_tile_x=%d, first_tile_y=%d, last_tile_x=%d, last_tile_y=%d\n",
			first_tile_x, first_tile_y, last_tile_x, last_tile_y);
		printf("first_tile_px_x=%f, first_tile_px_y=%f\n", s168_get(first_tile_px_x), s168_get(first_tile_px_y));
        // 64 bit integers are used for the edge equations here because multiplying two 16.8 numbers requires up to 48 bits
        // this results in some extra overhead, but it's not a big deal when you consider that this happens only for large triangles.
        // The tens of thousands of pixels that large triangles generate outweigh the cost of slightly more expensive setup.

        int64_t triarea2 = ((int64_t)verts[1].x - verts[0].x) * ((int64_t)verts[2].y - verts[0].y) - ((int64_t)verts[1].y - verts[0].y) * ((int64_t)verts[2].x - verts[0].x);

        // round away from zero to guarantee edge equations don't become greater than area
        if (triarea2 < 0)
            triarea2 -= 0x7F;
        else if (triarea2 > 0)
            triarea2 += 0x7F;

        if (triarea2 < 0 && triarea2 > -256)
        {
            // force to zero, since right shift of negative numbers never reach zero
            triarea2 = 0;
        }

        triarea2 = triarea2 >> 8;
        
        if (triarea2 == 0)
        {
            goto setup_end;
        }

        if (triarea2 < 0)
        {
            xyzw_i32_t tmp = verts[1];
            verts[1] = verts[2];
            verts[2] = tmp;
            triarea2 = -triarea2;
            
            // backface culling
            goto setup_end;
        }

        // compute 1/(2triarea) and convert to a pseudo 8.16 floating point value
        int32_t triarea2_lzcnt = (int32_t)lzcnt64(triarea2);
        int32_t triarea2_mantissa_rshift = (63 - 16) - triarea2_lzcnt;
        int32_t triarea2_mantissa;
        if (triarea2_mantissa_rshift < 0)
            triarea2_mantissa = (int32_t)(triarea2 << -triarea2_mantissa_rshift);
        else
            triarea2_mantissa = (int32_t)(triarea2 >> triarea2_mantissa_rshift);
        assert(triarea2_mantissa & 0x10000);

        // perform the reciprocal
        // note: triarea2_mantissa is currently normalized as 1.16, and so is the numerator of the division (before being adjusted for rounding)
        int32_t rcp_triarea2_mantissa = 0xFFFFFFFF / triarea2_mantissa;
        assert(rcp_triarea2_mantissa != 0);

        // ensure the mantissa is denormalized so it fits in 16 bits
        int32_t rcp_triarea2_mantissa_rshift = (31 - 15) - lzcnt(rcp_triarea2_mantissa);
        if (rcp_triarea2_mantissa_rshift < 0)
            rcp_triarea2_mantissa = rcp_triarea2_mantissa << -rcp_triarea2_mantissa_rshift;
        else
            rcp_triarea2_mantissa = rcp_triarea2_mantissa >> rcp_triarea2_mantissa_rshift;
        assert(!(rcp_triarea2_mantissa & 0x10000));
        assert(rcp_triarea2_mantissa & 0x8000);

        assert(rcp_triarea2_mantissa < 0x10000);
        rcp_triarea2_mantissa = rcp_triarea2_mantissa & 0xFFFF;
        rcp_triarea2_mantissa_rshift = (triarea2_mantissa_rshift + 1) - rcp_triarea2_mantissa_rshift;

        int64_t edges[3];
        int64_t edge_dxs[3], edge_dys[3];
        for (int32_t v = 0; v < 3; v++)
        {
            int32_t v1 = (v + 1) % 3;
            int32_t v2 = (v + 2) % 3;

            // find how the edge equation varies along x and y
            edge_dxs[v] = verts[v1].y - verts[v].y;
            edge_dys[v] = verts[v].x - verts[v1].x;

            // compute edge equation
            // |  x  y  z |
            // | ax ay  0 |
            // | bx by  0 |
            // = ax*by - ay*bx
            // eg: a = (px-v0), b = (v1-v0)
            // note: evaluated at px + (0.5,0.5)
            const int32_t s168_zero_pt_five = 0x80;
            edges[v] = ((int64_t)first_tile_px_x + s168_zero_pt_five - verts[v].x) * edge_dxs[v] - ((int64_t)first_tile_px_y + s168_zero_pt_five - verts[v].y) * -edge_dys[v];

            // round to negative infinity
            if (edges[v] < 0)
                edges[v] = edges[v] - 0xFF;

            edges[v] = edges[v] >> 8;


            // Top-left rule: shift top-left edges ever so slightly outward to make the top-left edges be the tie-breakers when rasterizing adjacent triangles
            if ((verts[v].y == verts[v1].y && verts[v].x < verts[v1].x) || verts[v].y > verts[v1].y) edges[v]--;
		
		}
		printf("dx=<%f,%f,%f>\ndy=<%f,%f,%f>\n", 
			s168_get(edge_dys[0]), s168_get(edge_dys[1]), s168_get(edge_dys[2]),
			s168_get(edge_dxs[0]), s168_get(edge_dxs[1]), s168_get(edge_dxs[2]));
		printf("edges=<%f,%f,%f>\n", s168_get(edges[0]), s168_get(edges[1]), s168_get(edges[2]));

        int64_t tile_edge_dxs[3];
        int64_t tile_edge_dys[3];
        for (int32_t v = 0; v < 3; v++)
        {
            tile_edge_dxs[v] = edge_dxs[v] * TILE_WIDTH_IN_PIXELS;
            tile_edge_dys[v] = edge_dys[v] * TILE_WIDTH_IN_PIXELS;
        }

        int64_t edge_trivRejs[3];
        int64_t edge_trivAccs[3];

        for (int32_t v = 0; v < 3; v++)
        {
            edge_trivRejs[v] = edges[v];
            edge_trivAccs[v] = edges[v];
            if (tile_edge_dxs[v] < 0) edge_trivRejs[v] += tile_edge_dxs[v];
            if (tile_edge_dxs[v] > 0) edge_trivAccs[v] += tile_edge_dxs[v];
            if (tile_edge_dys[v] < 0) edge_trivRejs[v] += tile_edge_dys[v];
            if (tile_edge_dys[v] > 0) edge_trivAccs[v] += tile_edge_dys[v];
        }

//		printf("tile x from %d to %d\n", first_tile_x, last_tile_x);
//		printf("tile y from %d to %d\n", first_tile_y, last_tile_y);

        int32_t tile_row_start = first_tile_y * fb->width_in_tiles + first_tile_x;
        for (int32_t tile_y = first_tile_y; tile_y <= last_tile_y; tile_y++)
        {
            int64_t tile_i_edges[3];
            int64_t tile_i_edge_trivRejs[3];
            int64_t tile_i_edge_trivAccs[3];
            for (int32_t v = 0; v < 3; v++)
            {
                tile_i_edges[v] = edges[v];
                tile_i_edge_trivRejs[v] = edge_trivRejs[v];
                tile_i_edge_trivAccs[v] = edge_trivAccs[v];
            }

            int32_t tile_i = tile_row_start;

            for (int32_t tile_x = first_tile_x; tile_x <= last_tile_x; tile_x++)
            {
                // trivial reject if at least one edge doesn't cover the tile at all
                int32_t trivially_rejected = tile_i_edge_trivRejs[0] >= 0 || tile_i_edge_trivRejs[1] >= 0 || tile_i_edge_trivRejs[2] >= 0;

				printf("[%d:%d], reject=<%f,%f,%f>, accept=<%f,%f,%f>\n", tile_x, tile_y,
					s168_get(tile_i_edge_trivRejs[0]), s168_get(tile_i_edge_trivRejs[1]), s168_get(tile_i_edge_trivRejs[2]),
					s168_get(tile_i_edge_trivAccs[0]), s168_get(tile_i_edge_trivAccs[1]), s168_get(tile_i_edge_trivAccs[2]));

                if (!trivially_rejected)
                {
                    tilecmd_drawtile_t drawtilecmd;

                    int32_t edge_needs_test[3];
                    edge_needs_test[0] = tile_i_edge_trivAccs[0] >= 0;
                    edge_needs_test[1] = tile_i_edge_trivAccs[1] >= 0;
                    edge_needs_test[2] = tile_i_edge_trivAccs[2] >= 0;

                    drawtilecmd.tilecmd_id = tilecmd_id_drawlargetri_0edgemask;
                    drawtilecmd.tilecmd_id += edge_needs_test[0];
                    drawtilecmd.tilecmd_id += edge_needs_test[1] << 1;
                    drawtilecmd.tilecmd_id += edge_needs_test[2] << 2;

                    for (int32_t v = 0; v < 3; v++)
                    {
                        assert(edge_dxs[v] >= INT32_MIN && edge_dxs[v] <= INT32_MAX);
                        assert(edge_dys[v] >= INT32_MIN && edge_dys[v] <= INT32_MAX);

                        // the maximum change in area over one tile shouldn't exceed int32
                        assert(tile_i_edge_trivAccs[v] - tile_i_edge_trivRejs[v] <= INT32_MAX);

                        if (edge_needs_test[v])
                        {
                            // ensure edges to test are within range of 32 bits (they should be, since trivial accept/reject only keeps nearby edges)
                            assert(tile_i_edges[v] >= INT32_MIN && tile_i_edges[v] <= INT32_MAX);
                            drawtilecmd.edges[v] = (int32_t)tile_i_edges[v];

                            // don't need any extra shift since we know there won't be any overflow
                            drawtilecmd.shifted_es[v] = 0;
                        }
                        else
                        {
                            // make edge relative to the corner of the tile to prevent overflows
                            drawtilecmd.edges[v] = 0;

                            // compute an initial offset for the unnormalized barycentric coordinates
                            int64_t shifted_e = -tile_i_edges[v];
                            if (rcp_triarea2_mantissa_rshift < 0)
                                shifted_e = shifted_e << -rcp_triarea2_mantissa_rshift;
                            else
                                shifted_e = shifted_e >> rcp_triarea2_mantissa_rshift;

                            assert(shifted_e >= INT32_MIN && shifted_e <= INT32_MAX);
                            drawtilecmd.shifted_es[v] = (int32_t)shifted_e;
                        }

                        drawtilecmd.edge_dxs[v] = (int32_t)edge_dxs[v];
                        drawtilecmd.edge_dys[v] = (int32_t)edge_dys[v];

                        drawtilecmd.vert_Zs[v] = verts[v].z;
                    }

                    drawtilecmd.min_Z = min_Z;
                    drawtilecmd.max_Z = max_Z;

                    drawtilecmd.shifted_triarea2 = triarea2_mantissa >> 1;
                    drawtilecmd.rcp_triarea2_mantissa = rcp_triarea2_mantissa;
                    drawtilecmd.rcp_triarea2_rshift = rcp_triarea2_mantissa_rshift;

#ifdef ENABLE_PERFCOUNTERS
                    fb->perfcounters.largetri_setup += qpc() - setup_start_pc;
#endif
                    framebuffer_push_tilecmd(fb, tile_i, &drawtilecmd.tilecmd_id, sizeof(drawtilecmd) / sizeof(uint32_t));

#ifdef ENABLE_PERFCOUNTERS
                    setup_start_pc = qpc();
#endif
				}
				else
				{
					//printf("REJECT!\n");
				}

                tile_i++;
                for (int32_t v = 0; v < 3; v++)
                {
                    tile_i_edges[v] += tile_edge_dxs[v];
                    tile_i_edge_trivRejs[v] += tile_edge_dxs[v];
                    tile_i_edge_trivAccs[v] += tile_edge_dxs[v];
                }
            }

            tile_row_start += fb->width_in_tiles;
            for (int32_t v = 0; v < 3; v++)
            {
                edges[v] += tile_edge_dys[v];
                edge_trivRejs[v] += tile_edge_dys[v];
                edge_trivAccs[v] += tile_edge_dys[v];
            }
        }
    }

setup_end:;
    if (is_large)
    {
#ifdef ENABLE_PERFCOUNTERS
        fb->perfcounters.largetri_setup += qpc() - setup_start_pc;
#endif
    }
    else
    {
#ifdef ENABLE_PERFCOUNTERS
        fb->perfcounters.smalltri_setup += qpc() - setup_start_pc;
#endif
    }
} 

void framebuffer_draw(
    framebuffer_t* fb,
    const int32_t* vertices,
    uint32_t num_vertices)
{
    assert(fb);
    assert(vertices);
    assert(num_vertices % 3 == 0);

    for (uint32_t vertex_id = 0, cmpt_id = 0; vertex_id < num_vertices; vertex_id += 3, cmpt_id += 12)
    {
        xyzw_i32_t verts[3];

        verts[0].x = vertices[cmpt_id + 0];
        verts[0].y = vertices[cmpt_id + 1];
        verts[0].z = vertices[cmpt_id + 2];
        verts[0].w = vertices[cmpt_id + 3];
        verts[1].x = vertices[cmpt_id + 4];
        verts[1].y = vertices[cmpt_id + 5];
        verts[1].z = vertices[cmpt_id + 6];
        verts[1].w = vertices[cmpt_id + 7];
        verts[2].x = vertices[cmpt_id + 8];
        verts[2].y = vertices[cmpt_id + 9];
        verts[2].z = vertices[cmpt_id + 10];
        verts[2].w = vertices[cmpt_id + 11];

        rasterize_triangle(fb, verts);
    }
}

void framebuffer_draw_indexed(
    framebuffer_t* fb,
    const int32_t* vertices,
    const uint32_t* indices,
    uint32_t num_indices)
{
    assert(fb);
    assert(vertices);
    assert(indices);
    assert(num_indices % 3 == 0);

    for (uint32_t index_id = 0; index_id < num_indices; index_id += 3)
    {
        xyzw_i32_t verts[3];

        uint32_t cmpt_i0 = indices[index_id + 0] * 4;
        uint32_t cmpt_i1 = indices[index_id + 1] * 4;
        uint32_t cmpt_i2 = indices[index_id + 2] * 4;

        verts[0].x = vertices[cmpt_i0 + 0];
        verts[0].y = vertices[cmpt_i0 + 1];
        verts[0].z = vertices[cmpt_i0 + 2];
        verts[0].w = vertices[cmpt_i0 + 3];
        verts[1].x = vertices[cmpt_i1 + 0];
        verts[1].y = vertices[cmpt_i1 + 1];
        verts[1].z = vertices[cmpt_i1 + 2];
        verts[1].w = vertices[cmpt_i1 + 3];
        verts[2].x = vertices[cmpt_i2 + 0];
        verts[2].y = vertices[cmpt_i2 + 1];
        verts[2].z = vertices[cmpt_i2 + 2];
        verts[2].w = vertices[cmpt_i2 + 3];

        rasterize_triangle(fb, verts);
    }
}

int32_t framebuffer_get_total_num_tiles(framebuffer_t* fb)
{
    assert(fb);
    return fb->total_num_tiles;
}

uint64_t framebuffer_get_perfcounter_frequency(framebuffer_t* fb)
{
    assert(fb);
#ifdef ENABLE_PERFCOUNTERS
    return fb->pc_frequency;
#else
    return 1;
#endif
}

void framebuffer_reset_perfcounters(framebuffer_t* fb)
{
#ifdef ENABLE_PERFCOUNTERS
    memset(&fb->perfcounters, 0, sizeof(framebuffer_perfcounters_t));
    memset(fb->tile_perfcounters, 0, sizeof(framebuffer_tile_perfcounters_t) * fb->total_num_tiles);
#endif
}

int32_t framebuffer_get_num_perfcounters(framebuffer_t* fb)
{
    assert(fb);

#ifdef ENABLE_PERFCOUNTERS
    return sizeof(framebuffer_perfcounters_t) / sizeof(uint64_t);
#else
    return 0;
#endif
}

void framebuffer_get_perfcounter_names(framebuffer_t* fb, const char** names)
{
    assert(fb);
    assert(names);

#ifdef ENABLE_PERFCOUNTERS
    memcpy(names, kFramebufferPerfcounterNames, sizeof(kFramebufferPerfcounterNames));
#endif
}

void framebuffer_get_perfcounters(framebuffer_t* fb, uint64_t* pcs)
{
    assert(fb);
    assert(pcs);

#ifdef ENABLE_PERFCOUNTERS
    memcpy(pcs, &fb->perfcounters, sizeof(framebuffer_perfcounters_t));
#endif
}

int32_t framebuffer_get_num_tile_perfcounters(framebuffer_t* fb)
{
    assert(fb);
    
#ifdef ENABLE_PERFCOUNTERS
    return sizeof(framebuffer_tile_perfcounters_t) / sizeof(uint64_t);
#else
    return 0;
#endif
}

void framebuffer_get_tile_perfcounter_names(framebuffer_t* fb, const char** names)
{
    assert(fb);
    assert(names);

#ifdef ENABLE_PERFCOUNTERS
    memcpy(names, kFramebufferTilePerfcounterNames, sizeof(kFramebufferTilePerfcounterNames));
#endif
}

void framebuffer_get_tile_perfcounters(framebuffer_t* fb, uint64_t* tile_pcs)
{
    assert(fb);
    assert(tile_pcs);

#ifdef ENABLE_PERFCOUNTERS
    memcpy(tile_pcs, fb->tile_perfcounters, sizeof(framebuffer_tile_perfcounters_t) * fb->total_num_tiles);
#endif
}