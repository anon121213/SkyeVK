# SkyRenderer — Claude Session Bootstrap

Этот файл читается в начале каждой сессии Claude чтобы восстановить контекст. **Он не про пользователя — он про меня (Claude), чтобы я знал что делать и как.**

## Проект в двух строках

**SkyRenderer** — standalone AAA-квалити low-level render framework на Vulkan. Standalone git-репа. Подключится в `SlyEngine` (движок пользователя) как git submodule. Цель — production-quality, portfolio-ready для позиции render programmer в AAA.

## Полный roadmap

**Читай `ROADMAP.md`** — там всё: locked architectural decisions, целевая структура репы, 12 фаз с задачами, timeline, метрики "готово". Это авторитетный документ.

## Code conventions (обязательно)

**Читай `~/.claude/projects/-Users-artur-Projects-TestVulkanRenderWithClaude/memory/code_conventions.md`**. Ключевое:
- **Все комменты в коде — только на English.** Русский OK только в docs/README/CLAUDE.md/chat.
- **Every enum value MUST have a comment** — inline описание после каждого value.
- `Sky::RHI::` namespace, `SKY_RHI_*` macros, `enum class : uint32_t`, `[[nodiscard]]`, `explicit`, m_PascalCase fields, camelCase methods.
- Handle-based RHI, RAII Graphics.

## Как я работаю с этим пользователем

**Читай `~/.claude/projects/-Users-artur-Projects-TestVulkanRenderWithClaude/memory/teaching_approach.md`**. Ключевые пункты:

1. **Я учитель, не code generator.** User учится Vulkan и C++, хочет уметь сам. Если я даю готовый код и он копирует — учёбы не будет.
2. **Что даю готовым:** инфраструктура (CMake, PCH, макросы), boilerplate, массовые механические рефакторы.
3. **Что даю как ТЗ (user пишет сам):** Vulkan логика, RAII wrappers, C++ паттерны которым он учится.
4. **Формат ТЗ:** концепт «зачем это» → спецификация «что должно быть» → подсказки «на что обратить внимание». Не диктовать код, оставить логику.
5. **После user кода:** проверяю через Read, отдельно 🔴 критические баги / 🟡 стилевые. Объясняю **почему** — не просто «должно быть X».
6. **Explanations:** развёрнуто с аналогиями (Unity, C#, OpenGL, Hazel). User просит объяснять «почему так, а не как иначе».
7. **User часто ошибается:** путает похожие имена (`deviceCount` vs `devicesCount`), забывает `{}` инициализацию структур, забывает `#include` явно, инвертирует boolean в `if/throw`, локальная переменная вместо `&m_Field` в `vkCreate*`.

## Locked architectural decisions

### Namespace

```
Sky::RHI::             ← весь наш renderer (Device, Buffer, Texture, CommandList, FrameGraph, ...)
Sky::Graphics::        ← optional high-level facade (Mesh, Material, MeshRenderer)
Sky::                  ← оставлен для consumer SlyEngine, не занимать
```

Ничего renderer-specific напрямую под `Sky::X` (без подnamespace) — только в `Sky::RHI::` или `Sky::Graphics::`.

### Macros

```
SKY_RHI_TRACE / INFO / WARN / ERROR / CRITICAL     ← наш логгер
SKY_RHI_VK_CHECK(expr, "msg")                       ← VkResult check
```

Никогда не использовать `SKY_INFO` etc. без префикса `_RHI_` — оставлен для engine.

### Two-tier API

```
Sky::Graphics ─────── (opt-in, RAII, для простых случаев)
       ↓ uses
Sky::RHI ──────────── (mandatory, handle-based, для ECS/DOTS)
```

### RHI = handle-based

```cpp
Sky::RHI::BufferHandle vb = device.createBuffer({...});   // POD, uint64_t
```

**Не** ownership-based (`std::unique_ptr<Buffer>`) в RHI. RAII только в Graphics.

### Threading

- API **thread-safe с дня 1** (mutex вокруг resource creation)
- Реализация **single-threaded** пока (до Phase 6 — Frame Graph advanced)
- Реальный MT recording приходит через FG, user сам `std::thread` не пишет

### Multi-window

**Один Device, много Swapchain**. Не «много Renderer».

### Backend polymorphism

**Пока не вводим.** Только Vulkan. Публичный API backend-agnostic по семантике, реализация прямо через Vulkan. Полиморфизм добавим когда придёт D3D12.

## Current state (обновлять при каждой фазе)

**Phase 0 (Bootstrap) — DONE ✅**
- Треугольник рендерится
- CMake, PCH, spdlog (`SPDLOG_USE_STD_FORMAT ON`), VMA v3.3.0, GLFW 3.4
- VulkanX классы: Instance, Device (+ VMA), Surface, Swapchain, RenderPass, Framebuffers, ShaderModule, Pipeline, CommandPool, Renderer

**Phase R (RHI Abstraction Refactor) — DONE ✅**
- Public API surface: только `include/SkyRHI/{Device,Types,Handle}.h`
- Все VulkanX переехали в `src/Vulkan/` (h+cpp вместе, internal)
- Core/Log переехали в `src/Core/` (internal)
- PCH в `src/skypch.h` — `PRIVATE` в CMake
- `Sky::RHI::Device` с PIMPL — hide VulkanX from consumer
- `Sky::RHI::Types.h` — 25+ enums (BackendType, Format, BufferUsage, MemoryType, ShaderStage, etc.)
- `Sky::RHI::Handle<Tag>` — phantom type pattern, 12 handle types
- Consumer main.cpp — 40 строк, без VulkanX includes
- Треугольник продолжает работать

**Phase 1 (Frame Graph MVP) — NEXT 🚧**
Задачи:
1. `include/SkyRHI/CommandList.h` — recording API
2. `include/SkyRHI/Swapchain.h` — public handle-based (сейчас внутри Device)
3. `include/SkyRHI/FrameGraph.h` — pass declaration API
4. `src/Common/FrameGraphCompiler.cpp` — DAG анализ
5. Автоматические `vkCmdPipelineBarrier` между passes
6. Треугольник через FG (один RasterPass)
7. Тесты корректности FG

**Треугольник должен продолжать работать после каждой фазы.**

**Open questions для Phase 1 старта:**
- `Sky::RHI::Log::init()` теперь private — как consumer управляет логгированием? Auto-init в Device / public entry point.
- Shader paths хак в `DeviceCreateInfo` — при Phase 1 уйдёт (FG exposes pipeline creation).

## Ключевые файлы

**Публичный API (currently):**
- `include/skypch.h` — PCH
- `include/Core/Core.h` — `Sky::RHI::Ref<T>`, `Sky::RHI::Scope<T>`
- `include/Core/Log.h` — `Sky::RHI::Log`, макросы `SKY_RHI_*`

**Внутренний код (Phase R переезд):**
- `include/SkyRenderer/*.h` (10 файлов) — все переедут в `src/Vulkan/`

**App (не library):**
- `SkyApp/src/main.cpp` — demo, использует renderer
- `SkyApp/src/Window.h/cpp` — GLFW wrapper (app-side)
- `SkyApp/shaders/` — GLSL, компилятся glslc через CMake

## Стэк зависимостей

| Библиотека | Версия | Через | Комментарий |
|---|---|---|---|
| Vulkan SDK | 1.4.350.0 | `find_package` | ~/VulkanSDK/, env vars настроены |
| spdlog | v1.15.0 | FetchContent | **обязательно `SPDLOG_USE_STD_FORMAT ON`** — иначе Apple Clang ломается на fmt v11 consteval |
| VMA | v3.3.0 | FetchContent | В `src/Vulkan/VMAImplementation.cpp` — единственный TU с `#define VMA_IMPLEMENTATION` |
| GLFW | 3.4 | FetchContent | Только в SkyApp, не в SkyRenderer library |

## macOS quirks

- **VK_KHR_portability_enumeration** — обязательный instance extension
- **VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR** — обязательный instance flag
- **VK_KHR_portability_subset** — обязательный device extension (MoltenVK экспортирует)
- **VK_KHR_swapchain** — device extension (пропустил в начале, было `Driver's function pointer was NULL`)
- **MoltenVK не поддерживает hardware ray tracing** → RT на Mac через compute-based path tracer

## Как начать сессию

1. **Прочти этот файл**
2. **Прочти ROADMAP.md** — актуальные фазы и задачи
3. **Прочти memory files** — user preferences, teaching approach
4. Спроси user куда двигаться: продолжить текущую phase или ревью существующего кода
5. **Помни:** ТЫ УЧИТЕЛЬ. Не выдавай готовый код без ТЗ, если user не попросил явно "давай код".

## Обновления этого файла

Каждый раз когда:
- Завершается фаза → обновить "Current state"
- Меняются locked decisions → обновить соответствующую секцию
- User озвучивает предпочтения → добавить в teaching approach
- Обнаруживается новый quirk → добавить в quirks

Не боятся редактировать. Этот файл — living document.
