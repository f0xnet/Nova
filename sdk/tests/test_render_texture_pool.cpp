#include "TestRunner.hpp"
#include "MockGraphicsBackend.hpp"
#include "NovaEngine/Rendering/RenderTexturePool.hpp"

using namespace NovaEngine;
using NovaTest::MockGraphicsBackend;

TEST_CASE("RenderTexturePool: acquire creates a new texture") {
    MockGraphicsBackend mock;
    RenderTexturePool pool;
    pool.initialize(&mock);

    auto h = pool.acquire(320, 180);
    REQUIRE(h != INVALID_HANDLE);
    REQUIRE(mock.createRTCount == 1);
}

TEST_CASE("RenderTexturePool: released texture is reused") {
    MockGraphicsBackend mock;
    RenderTexturePool pool;
    pool.initialize(&mock);

    auto h1 = pool.acquire(320, 180);
    pool.release(h1);

    auto h2 = pool.acquire(320, 180);
    REQUIRE(h2 == h1);          // same handle reused
    REQUIRE(mock.createRTCount == 1); // no second allocation
}

TEST_CASE("RenderTexturePool: different sizes get separate textures") {
    MockGraphicsBackend mock;
    RenderTexturePool pool;
    pool.initialize(&mock);

    auto h1 = pool.acquire(320, 180);
    auto h2 = pool.acquire(640, 360);
    REQUIRE(h1 != h2);
    REQUIRE(mock.createRTCount == 2);
}

TEST_CASE("RenderTexturePool: two simultaneous acquires of same size create two textures") {
    MockGraphicsBackend mock;
    RenderTexturePool pool;
    pool.initialize(&mock);

    auto h1 = pool.acquire(320, 180);
    auto h2 = pool.acquire(320, 180); // still in use → must allocate new
    REQUIRE(h1 != h2);
    REQUIRE(mock.createRTCount == 2);

    pool.release(h1);
    pool.release(h2);

    // Both are free — next acquire reuses one
    auto h3 = pool.acquire(320, 180);
    REQUIRE(mock.createRTCount == 2);
    (void)h3;
}

TEST_CASE("RenderTexturePool: shutdown unloads all textures") {
    MockGraphicsBackend mock;
    RenderTexturePool pool;
    pool.initialize(&mock);

    pool.acquire(320, 180);
    pool.acquire(640, 360);

    REQUIRE(pool.size() == 2);
    pool.shutdown();
    REQUIRE(mock.unloadRTCount == 2);
    REQUIRE(pool.size() == 0);
}

TEST_CASE("RenderTexturePool: release of unknown handle is a no-op") {
    MockGraphicsBackend mock;
    RenderTexturePool pool;
    pool.initialize(&mock);

    pool.release(9999); // must not crash
    REQUIRE(mock.unloadRTCount == 0);
}
