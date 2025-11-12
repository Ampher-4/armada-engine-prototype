#include <UIDrawSystem.hh>
#include <GlobalContext.hh>
#include <Engine.hh>
#include <SDL2/SDL.h>
#include <RenderSystem.hh>

NAMESPACE_BEGIN
extern AppContext* objptrAppContext;
extern Engine* objptrGameEngine;


void UIDrawSystem::DrawRTTRObject(rttr::instance obj)
{
    using namespace rttr;
    type t = obj.get_type();

    for (auto& prop : t.get_properties())
    {
        variant val = prop.get_value(obj);

        if (!val.is_valid())
            continue;

        const auto uiTypeMeta = prop.get_metadata("ui_type");
        std::string uiType = uiTypeMeta.is_valid() ? uiTypeMeta.get_value<std::string>() : "default";

        if (uiType == "combo")
        {
            auto names = prop.get_metadata("options").get_value<std::vector<std::string>>();
            auto values = prop.get_metadata("values");

            int currentIndex = 0;

            // 将当前属性值匹配到列表中
            if (values.is_valid())
            {
                std::vector<rttr::variant> valueList;

                // 1️⃣ 判断值类型是 vector<int>
                if (values.get_type() == rttr::type::get<std::vector<int>>())
                {
                    auto ints = values.get_value<std::vector<int>>();
                    for (auto& v : ints) valueList.emplace_back(v);
                }
                // 3️⃣ 如果本来就是 vector<variant>
                else if (values.get_type() == rttr::type::get<std::vector<rttr::variant>>())
                {
                    valueList = values.get_value<std::vector<rttr::variant>>();
                }
                else
                {
                    ImGui::Text("Unsupported metadata value type for property: %s",
                                prop.get_name().to_string().c_str());
                    continue;
                }

                // ✅ 现在可以安全使用 valueList
                int currentIndex = 0;
                for (int i = 0; i < (int)valueList.size(); ++i)
                {
                    if (val == valueList[i])
                    {
                        currentIndex = i;
                        break;
                    }
                }

                if (ImGui::BeginCombo(prop.get_name().to_string().c_str(), names[currentIndex].c_str()))
                {
                    for (int i = 0; i < (int)names.size(); ++i)
                    {
                        bool selected = (i == currentIndex);
                        if (ImGui::Selectable(names[i].c_str(), selected))
                        {
                            prop.set_value(obj, valueList[i]);
                        }
                        if (selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }
        }
        else if (val.is_type<bool>())
        {
            bool b = val.get_value<bool>();
            if (ImGui::Checkbox(prop.get_name().to_string().c_str(), &b))
                prop.set_value(obj, b);
        }
        else if (val.is_type<int>())
        {
            int v = val.get_value<int>();
            if (ImGui::InputInt(prop.get_name().to_string().c_str(), &v))
                prop.set_value(obj, v);
        }
        else if (val.is_type<float>())
        {
            float f = val.get_value<float>();
            if (ImGui::InputFloat(prop.get_name().to_string().c_str(), &f))
                prop.set_value(obj, f);
        }
        else
        {
            ImGui::Text("%s (unhandled type)", prop.get_name().to_string().c_str());
        }
    }
}

void UIDrawSystem::drawframe() {
    ImGui::Begin("ECS Editor");
    ImGui::Text("Press F10 to switch to editor");
    ImGui::Text("Press F9 to hide editor");
    ImGui::Separator();


    if (ImGui::TreeNode("RenderConfig")) {

        RenderContext& rendercontext = *objptrAppContext->aRenderContext;
        rttr::instance rendercontextinstance = rttr::instance(&rendercontext);
        DrawRTTRObject(rendercontextinstance);
        ImGui::TreePop();
    }

    StateManager* stateManager = objptrGameEngine->getStateManager();
    auto view = stateManager->view<TransformComponent>();

    for (auto entity : view)
    {
        std::string label;
        if (stateManager->all_of<NameComponent>(entity))
            label = stateManager->get<NameComponent>(entity).name;
        else
            label = "Entity " + std::to_string((uint32_t)entt::to_integral(entity));

        if (ImGui::TreeNode(label.c_str()))
        {
            if (ImGui::TreeNode("Transform")) {
                auto& transform = stateManager->get<TransformComponent>(entity);
                rttr::instance transform_instance(transform);
                rttr::type transform_type = rttr::type::get(transform);

                for (auto& prop : transform_type.get_properties())
                {
                    rttr::variant value = prop.get_value(transform_instance);

                    if (value.is_type<glm::vec3>())
                    {
                        glm::vec3 vec = value.get_value<glm::vec3>();
                        glm::vec3 new_vec = vec;

                        float speed = 0.1f;
                        if (prop.get_metadata("ui_speed").is_valid())
                            speed = prop.get_metadata("ui_speed").get_value<float>();

                        if (ImGui::DragFloat3(prop.get_name().to_string().c_str(), &new_vec.x, speed))
                            prop.set_value(transform_instance, new_vec);
                    }
                }
                ImGui::TreePop();
            }
            if (stateManager->all_of<LightComponent>(entity))  // 检查实体是否有 LightComponent
            {
                if (ImGui::TreeNode("Light"))  // Light 节点
                {
                    auto& light = stateManager->get<LightComponent>(entity);
                    rttr::instance light_instance(light);
                    rttr::type light_type = rttr::type::get(light);

                    // 遍历 LightComponent 的反射属性并绘制
                    for (auto& prop : light_type.get_properties())
                    {
                        rttr::variant value = prop.get_value(light_instance);

                        // 处理颜色（glm::vec4）
                        if (value.is_type<glm::vec4>())
                        {
                            glm::vec4 color = value.get_value<glm::vec4>();
                            glm::vec4 new_color = color;

                            // 使用颜色编辑控件（支持 RGBA）
                            if (ImGui::ColorEdit4(prop.get_name().to_string().c_str(), &new_color.x))
                                prop.set_value(light_instance, new_color);
                        }
                        // 处理强度（float）
                        else if (value.is_type<float>())
                        {
                            float val = value.get_value<float>();
                            float new_val = val;

                            // 根据属性名区分强度和范围，设置不同的拖动速度
                            float speed = (prop.get_name() == "intensity") ? 0.01f : 0.1f;
                            if (ImGui::DragFloat(prop.get_name().to_string().c_str(), &new_val, speed))
                                prop.set_value(light_instance, new_val);
                        }
                    }
                    ImGui::TreePop();  // 关闭 Light 节点
                }
            }

            ImGui::TreePop();  // 关闭实体总节点

        }
    }
    ImGui::End();

}

UIDrawSystem::UIDrawSystem()
{
    SDL_Window* window = objptrAppContext->aRenderContext->mainwindow;
    SDL_GLContext glcontext = objptrAppContext->aRenderContext->glcontext;
    this->init(window,  glcontext);
}

void UIDrawSystem::init(SDL_Window* window, SDL_GLContext glcontext)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
//    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, glcontext);
    ImGui_ImplOpenGL3_Init("#version 330");

    //register editor hide/switch_to subscriber
    auto* eventmanager = objptrGameEngine->getEventManager();
    eventmanager->subscribe(EventType::KeyPressed, [](const Event& e)
    {
        auto& ev = static_cast<const KeyPressedEvent&>(e);
        if (ev.key == SDLK_F10)
            objptrAppContext->aEditorMode = !objptrAppContext->aEditorMode;
            objptrAppContext->switchEditorMode();
        if (ev.key == SDLK_F9)
            objptrAppContext->aShowEditor = !objptrAppContext->aShowEditor;
    });

}

void UIDrawSystem::quit()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void UIDrawSystem::prerender()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}


void UIDrawSystem::postrender()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


NAMESPACE_END