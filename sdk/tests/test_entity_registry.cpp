#include "TestRunner.hpp"
#include "NovaEngine/ECS/EntityRegistry.hpp"
#include "NovaEngine/ECS/Entity.hpp"
#include "NovaEngine/ECS/Component.hpp"
#include <memory>

using namespace NovaEngine;

// Minimal stub components for tests
struct TagA : Component {
    COMPONENT_TYPE_ID(TagA)
    void serialize(nlohmann::json&) const override {}
    void deserialize(const nlohmann::json&) override {}
};
struct TagB : Component {
    COMPONENT_TYPE_ID(TagB)
    void serialize(nlohmann::json&) const override {}
    void deserialize(const nlohmann::json&) override {}
};

// -------------------------------------------------------

TEST_CASE("EntityRegistry: createEntity increments ID") {
    EntityRegistry reg;
    Entity* a = reg.createEntity();
    Entity* b = reg.createEntity();
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    REQUIRE(a->getID() != b->getID());
    REQUIRE_EQ(reg.getEntityCount(), (size_t)2);
}

TEST_CASE("EntityRegistry: getEntity returns correct entity") {
    EntityRegistry reg;
    Entity* e = reg.createEntity();
    u64 id = e->getID();
    REQUIRE(reg.getEntity(id) == e);
    REQUIRE(reg.getEntity(id + 9999) == nullptr);
}

TEST_CASE("EntityRegistry: destroyEntity removes entity") {
    EntityRegistry reg;
    Entity* e = reg.createEntity();
    u64 id = e->getID();
    reg.destroyEntity(id);
    REQUIRE_EQ(reg.getEntityCount(), (size_t)0);
    REQUIRE(reg.getEntity(id) == nullptr);
}

TEST_CASE("EntityRegistry: destroyEntity on unknown ID is a no-op") {
    EntityRegistry reg;
    reg.createEntity();
    reg.destroyEntity(9999);
    REQUIRE_EQ(reg.getEntityCount(), (size_t)1);
}

TEST_CASE("EntityRegistry: clear removes all entities and resets IDs") {
    EntityRegistry reg;
    reg.createEntity();
    reg.createEntity();
    reg.clear();
    REQUIRE_EQ(reg.getEntityCount(), (size_t)0);
    Entity* e = reg.createEntity();
    REQUIRE_EQ(e->getID(), (u64)1);
}

TEST_CASE("EntityRegistry: getAllEntities returns all") {
    EntityRegistry reg;
    reg.createEntity();
    reg.createEntity();
    reg.createEntity();
    auto all = reg.getAllEntities();
    REQUIRE_EQ(all.size(), (size_t)3);
}

TEST_CASE("EntityRegistry: getEntitiesWith returns only matching entities") {
    EntityRegistry reg;
    Entity* a = reg.createEntity();
    Entity* b = reg.createEntity();
    Entity* c = reg.createEntity();

    a->addComponent<TagA>(std::make_unique<TagA>());
    b->addComponent<TagA>(std::make_unique<TagA>());
    b->addComponent<TagB>(std::make_unique<TagB>());
    c->addComponent<TagB>(std::make_unique<TagB>());

    reg.invalidateQueryCache();

    auto withA = reg.getEntitiesWith({TagA{}.getTypeID()});
    REQUIRE_EQ(withA.size(), (size_t)2);

    auto withB = reg.getEntitiesWith({TagB{}.getTypeID()});
    REQUIRE_EQ(withB.size(), (size_t)2);

    auto withAB = reg.getEntitiesWith({TagA{}.getTypeID(), TagB{}.getTypeID()});
    REQUIRE_EQ(withAB.size(), (size_t)1);
    REQUIRE(withAB[0] == b);
}

TEST_CASE("EntityRegistry: query cache returns same result on second call") {
    EntityRegistry reg;
    Entity* e = reg.createEntity();
    e->addComponent<TagA>(std::make_unique<TagA>());
    reg.invalidateQueryCache();

    auto r1 = reg.getEntitiesWith({TagA{}.getTypeID()});
    auto r2 = reg.getEntitiesWith({TagA{}.getTypeID()});
    REQUIRE_EQ(r1.size(), (size_t)1);
    REQUIRE(r1[0] == r2[0]);
}

TEST_CASE("EntityRegistry: cache is invalidated after createEntity") {
    EntityRegistry reg;
    Entity* e1 = reg.createEntity();
    e1->addComponent<TagA>(std::make_unique<TagA>());
    reg.invalidateQueryCache();

    auto r1 = reg.getEntitiesWith({TagA{}.getTypeID()});
    REQUIRE_EQ(r1.size(), (size_t)1);

    Entity* e2 = reg.createEntity();  // invalidates cache automatically
    e2->addComponent<TagA>(std::make_unique<TagA>());
    reg.invalidateQueryCache();

    auto r2 = reg.getEntitiesWith({TagA{}.getTypeID()});
    REQUIRE_EQ(r2.size(), (size_t)2);
}

TEST_CASE("EntityRegistry: empty signature matches all entities") {
    EntityRegistry reg;
    reg.createEntity();
    reg.createEntity();
    auto all = reg.getEntitiesWith({});
    REQUIRE_EQ(all.size(), (size_t)2);
}
