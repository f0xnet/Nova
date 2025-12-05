# Guide d'Intégration Moteur ↔ Éditeur

Ce document explique comment l'éditeur s'adapte automatiquement aux modifications du moteur NovaEngine.

## 🎯 Problème Résolu

**Avant:** Ajouter un composant au moteur nécessitait de modifier manuellement:
- EntityPropertiesPanel (affichage des propriétés)
- EntityEditorController (création d'entités)
- EditorApplication (gestion des types)
- Multiples fichiers UI

**Maintenant:** Ajouter un composant = Enregistrer dans le ComponentRegistry → **Éditeur fonctionne automatiquement!**

---

## 🏗️ Architecture

### **1. ComponentRegistry** (`Core/ComponentRegistry.hpp`)
Système de découverte dynamique des composants du moteur.

**Responsabilités:**
- Enregistrer les types de composants avec leurs métadonnées
- Fournir des factories pour créer des composants
- Organiser les composants par catégories
- Exposer les propriétés éditables

### **2. PropertyInspector** (`Core/PropertyInspector.hpp`)
Inspecteur générique qui fonctionne avec n'importe quel composant enregistré.

**Responsabilités:**
- Extraire les propriétés d'un composant
- Modifier les valeurs des propriétés
- Fournir les métadonnées UI (min, max, step)

### **3. EngineAdapter** (`Core/EngineAdapter.hpp`)
Couche d'abstraction qui isole l'éditeur de l'API du moteur.

**Responsabilités:**
- Encapsuler toutes les interactions avec NovaEngine
- Fournir une interface stable même si l'API du moteur change
- Simplifier la création d'entités et composants

### **4. EngineComponentsInit** (`Core/EngineComponentsInit.hpp`)
Fichier d'initialisation qui enregistre tous les composants.

**Responsabilités:**
- Enregistrer chaque composant au démarrage de l'éditeur
- Définir les métadonnées et propriétés
- Point central pour ajouter de nouveaux composants

---

## 📝 Comment Ajouter un Nouveau Composant

### Étape 1: Créer le Composant dans NovaEngine

```cpp
// Dans NovaEngine/ECS/Components.hpp
class PhysicsComponent : public Component {
public:
    float mass = 1.0f;
    Vec2f velocity = {0.0f, 0.0f};
    bool useGravity = true;

    COMPONENT_TYPE_ID(PhysicsComponent)

    // serialize/deserialize methods...
};
```

### Étape 2: Enregistrer dans EngineComponentsInit.hpp

```cpp
inline void registerPhysicsComponent() {
    auto& registry = ComponentRegistry::instance();

    registry.registerComponent<NovaEngine::PhysicsComponent>(
        "Physics",              // Type name
        "Physics Body",         // Display name
        "Physics",              // Category
        [](NovaEngine::PhysicsComponent* comp) {
            ComponentMetadata meta;

            // Mass property
            PropertyMetadata mass;
            mass.name = "Mass";
            mass.type = "float";
            mass.minValue = 0.1f;
            mass.maxValue = 100.0f;
            mass.getValue = [comp]() { return std::to_string(comp->mass); };
            mass.setValue = [comp](const std::string& v) { comp->mass = std::stof(v); };
            meta.properties.push_back(mass);

            // Velocity X
            PropertyMetadata velX;
            velX.name = "Velocity X";
            velX.type = "float";
            velX.getValue = [comp]() { return std::to_string(comp->velocity.x); };
            velX.setValue = [comp](const std::string& v) { comp->velocity.x = std::stof(v); };
            meta.properties.push_back(velX);

            // Velocity Y
            PropertyMetadata velY;
            velY.name = "Velocity Y";
            velY.type = "float";
            velY.getValue = [comp]() { return std::to_string(comp->velocity.y); };
            velY.setValue = [comp](const std::string& v) { comp->velocity.y = std::stof(v); };
            meta.properties.push_back(velY);

            // Use Gravity
            PropertyMetadata gravity;
            gravity.name = "Use Gravity";
            gravity.type = "bool";
            gravity.getValue = [comp]() { return comp->useGravity ? "true" : "false"; };
            gravity.setValue = [comp](const std::string& v) { comp->useGravity = (v == "true"); };
            meta.properties.push_back(gravity);

            return meta;
        }
    );
}
```

### Étape 3: Appeler dans initializeEngineComponents()

```cpp
inline void initializeEngineComponents() {
    registerTransformComponent();
    registerSpriteComponent();
    registerLightComponent();
    registerPhysicsComponent();  // <-- Add this line
}
```

### Étape 4: C'est Tout!

**L'éditeur supporte automatiquement le nouveau composant:**
- ✅ Visible dans l'inspecteur de propriétés
- ✅ Propriétés éditables
- ✅ Sauvegarde/chargement de scène
- ✅ Création via UI (si ajouté au menu)

---

## 🔧 Modification de l'API du Moteur

### Si l'API NovaEngine change, mettre à jour EngineAdapter:

**Exemple: SceneManager change son API de chargement**

Avant (dans le moteur):
```cpp
Scene* loadScene(const std::string& path);
```

Après (dans le moteur):
```cpp
bool loadScene(const std::string& path, const std::string& name);
Scene* getScene(const std::string& name);
```

**Mise à jour dans EngineAdapter.hpp:**

```cpp
// Ancienne méthode (commentée pour référence)
// NovaEngine::Scene* loadScene(const std::string& path) {
//     return m_sceneManager->loadScene(path);
// }

// Nouvelle méthode (adaptée)
NovaEngine::Scene* loadScene(const std::string& path, const std::string& name) {
    if (!m_sceneManager->loadScene(path, name)) {
        return nullptr;
    }
    return m_sceneManager->getScene(name);
}
```

**Résultat:** Le reste de l'éditeur continue de fonctionner sans modification!

---

## 📊 Types de Propriétés Supportés

| Type | Description | Exemple |
|------|-------------|---------|
| `"float"` | Nombre décimal | Position, rotation, scale |
| `"int"` | Nombre entier | Layer, count |
| `"bool"` | Booléen | Enabled, visible |
| `"string"` | Texte | Texture ID, name |
| `"vec2"` | Vecteur 2D | Position, velocity |
| `"color"` | Couleur RGBA | Tint, light color |

---

## 🎨 Catégories de Composants

Organisez vos composants par catégories logiques:

- **"Core"** - Transform, Name, Tag
- **"Rendering"** - Sprite, Light, Camera
- **"Physics"** - Rigidbody, Collider
- **"Audio"** - AudioSource, AudioListener
- **"Logic"** - Script, AI, State Machine
- **"Animation"** - Animator, SpriteAnimation

---

## 🚀 Avantages de Cette Architecture

### Pour le Développeur Moteur:
✅ Ajouter des features sans casser l'éditeur
✅ API peut évoluer librement
✅ Un seul fichier à mettre à jour (EngineComponentsInit.hpp)

### Pour le Développeur Éditeur:
✅ Code découplé du moteur
✅ Pas besoin de comprendre les détails du moteur
✅ UI générique qui s'adapte automatiquement

### Pour l'Utilisateur Final:
✅ Nouvelles features du moteur disponibles immédiatement
✅ Interface cohérente pour tous les composants
✅ Pas de bugs de synchronisation éditeur/moteur

---

## 📚 Exemples d'Utilisation

### Créer une Entité avec l'Adapter

```cpp
// Au lieu de:
Entity* entity = scene->getEntityRegistry().createEntity();
auto transform = std::make_unique<TransformComponent>();
transform->position = {100, 200};
entity->addComponent(std::move(transform));

// Utiliser:
EngineAdapter adapter(&sceneManager);
Entity* entity = adapter.createEntity(scene);
auto transform = adapter.createTransformComponent({100, 200});
adapter.addComponent(entity, std::move(transform));
```

### Inspecter un Composant

```cpp
PropertyInspector inspector;
auto results = inspector.inspectEntity(entity);

for (const auto& result : results) {
    std::cout << "Component: " << result.componentName << std::endl;
    for (const auto& prop : result.properties) {
        std::cout << "  " << prop.name << " = " << prop.getValue() << std::endl;
    }
}
```

### Modifier une Propriété

```cpp
PropertyInspector inspector;
bool success = inspector.modifyProperty(
    entity,
    "Transform",      // Component type
    "Position X",     // Property name
    "150.0"          // New value
);
```

---

## 🎯 Prochaines Améliorations Possibles

1. **Reflection automatique** - Générer les métadonnées depuis les attributs C++
2. **Custom property editors** - Éditeurs spécialisés (color picker, file selector)
3. **Component dependencies** - "Sprite requires Transform"
4. **Hot-reload** - Recharger les composants sans redémarrer l'éditeur
5. **Validation** - Vérifier les valeurs min/max automatiquement

---

## 📞 Support

Pour questions ou problèmes d'intégration, consulter:
- `ComponentRegistry.hpp` - Documentation complète du registre
- `PropertyInspector.hpp` - Système d'inspection
- `EngineAdapter.hpp` - Couche d'abstraction
- `EngineComponentsInit.hpp` - Exemples d'enregistrement

**La modularité moteur ↔ éditeur est maintenant complète!** 🎉
