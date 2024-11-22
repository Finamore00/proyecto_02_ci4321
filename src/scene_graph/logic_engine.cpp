#include "logic_engine.hpp"

#include "sceneobject.hpp"
#include "component.hpp"

LogicEngine *LogicEngine::g_instance = nullptr;

LogicEngine::LogicEngine()
{
    if (g_instance != nullptr) {
        throw;
    }

    g_instance = this;
}

LogicEngine* LogicEngine::get_instance()
{
    return g_instance;
}

void LogicEngine::register_component(SceneObject* obj, Component* comp)
{
    if (m_comps.find(obj) == m_comps.end())
    {
        m_comps[obj] = {comp};
        return;
    }

    m_comps[obj].push_back(comp);
}

void LogicEngine::update(float dt) const
{
    std::cout << "[WARNING] NOT USING WEAK_PTR FOR LOGIC UPDATE. CHANGE ASAP" << std::endl;
    for (auto oiter = m_comps.begin(); oiter != m_comps.end(); oiter++)
    {
        if (!oiter->first->active)
            continue;

        for (auto citer = oiter->second.begin(); citer != oiter->second.end(); citer++)
        {
            if (!(*citer)->enabled)
                continue;
                
            (*citer)->update(dt);
        }
    }   
}